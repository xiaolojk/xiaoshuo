package com.pixellake;

import com.pixellake.net.GameClient;
import com.pixellake.net.Protocol;

import java.util.ArrayList;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

/**
 * 应用主控：游戏状态机 + 主菜单(单人/多人) + 单人钓鱼玩法 + 局域网联机 + 聊天与指令。
 *
 * 状态：标题 -> 选择(单人/多人/退出) -> 多人则进入房间列表 -> 加入后进入游戏(单人/联机共用一套玩法)。
 */
public final class App implements GameClient.Listener {

    // ---- 状态机 ----
    private static final int ST_TITLE = 0;    // 主菜单
    private static final int ST_PLAY = 1;     // 游戏进行（单人 / 联机中）
    private static final int ST_LAN = 2;      // 多人房间列表
    private int state = ST_TITLE;

    // ---- 钓鱼阶段 ----
    private static final int PH_IDLE = 0;
    private static final int PH_CAST = 1;      // 抛竿飞行
    private static final int PH_WAIT = 2;      // 浮标等待
    private static final int PH_NIBBLE = 3;    // 上钩，需快速收杆
    private static final int PH_MISS = 4;      // 脱钩
    private static final int PH_CAUGHT = 5;    // 成功
    private int phase = PH_IDLE;
    private float phaseTimer = 0;

    // ---- 玩家状态 ----
    private int playerX = 120;                 // 码头 x 位置
    private float castX = 0, castY = 0;        // 浮标起飞/落点
    private float bobX = 0, bobY = 0;          // 浮标当前位置
    private int score = 0;
    private int coins = 0;
    private float gameClock = 0;               // 秒
    private int pendingFish = -1;              // 待上钩的鱼索引

    // ---- 主菜单 / 房间列表选择 ----
    private int sel = 0;
    private final String[] menuItems = {"单人游戏", "多人游戏", "退出"};
    private List<GameClient.Room> roomList = new ArrayList<>();
    private float rescanTimer = 0;

    // ---- 联机 ----
    private GameClient net = null;
    private final Map<Integer, RemotePlayer> players = new LinkedHashMap<>();
    private int myId = -1;
    private String myName = "玩家";
    private float posSyncTimer = 0;

    // ---- 聊天 ----
    private final List<String> chat = new ArrayList<>();
    private int textMode = 0;                  // 0 无, 1 聊天, 2 输名字/IP
    private static final int TM_NONE = 0, TM_CHAT = 1, TM_JOINIP = 2;

    private final Main main;
    private final java.util.Random rnd = new java.util.Random();

    /** 联机中的远端玩家。 */
    static final class RemotePlayer {
        int id; String name; int skin; int x, anim, score;
    }

    /** 鱼表（与 C 版 FISHES 对应：名称/分值/体色）。 */
    private static final class FishDef {
        final String cn, en; final int value; final int color;
        FishDef(String cn, String en, int value, int color) {
            this.cn = cn; this.en = en; this.value = value; this.color = color;
        }
    }
    private static final FishDef[] FISHES = {
        new FishDef("鲤鱼",     "CARP",       3,  0xFFD08040),
        new FishDef("鲈鱼",     "PERCH",      5,  0xFF60A060),
        new FishDef("银鲤",     "SILVER CARP",9,  0xFFB0B8C0),
        new FishDef("鲶鱼",     "CATFISH",   12,  0xFF806830),
        new FishDef("大口鲈",   "LARGEMOUTH",14,  0xFF4E8A4E),
        new FishDef("蓝鳃鱼",   "BLUEGILL",  22,  0xFF4080C0),
        new FishDef("鲑鱼",     "SALMON",    30,  0xFFC06050),
        new FishDef("金鳟",     "GOLD TROUT",40,  0xFFF0C040),
        new FishDef("电鳗",     "ELECTRIC EEL",55, 0xFFC0C820),
        new FishDef("湖龙",     "LAKE DRAGON",150, 0xFF9050C0),
    };

    public App(Main main) {
        this.main = main;
    }

    // ================= 更新 =================
    public void update(float dt, Main.Input in) {
        gameClock += dt;

        // 处理文本输入提交
        pollText();

        switch (state) {
            case ST_TITLE -> updateTitle(in);
            case ST_LAN -> updateLan(dt, in);
            case ST_PLAY -> updatePlay(dt, in);
        }
    }

    private void updateTitle(Main.Input in) {
        if (in.up) sel = (sel + menuItems.length - 1) % menuItems.length;
        if (in.down) sel = (sel + 1) % menuItems.length;
        if (in.left || in.right) sel ^= 1;
        if (in.accept) {
            switch (sel) {
                case 0 -> state = ST_PLAY;                 // 单人
                case 1 -> enterLan();
                case 2 -> System.exit(0);
            }
        }
    }

    private void enterLan() {
        if (net == null) net = new GameClient(this);
        roomList.clear();
        net.search();
        rescanTimer = 0;
        sel = 0;
        state = ST_LAN;
    }

    private void updateLan(float dt, Main.Input in) {
        rescanTimer -= dt;
        if (rescanTimer <= 0) { net.search(); rescanTimer = 2.0f; }
        roomList = net.rooms();
        int n = roomList.size();
        if (in.up) sel = (sel + n - 1) % Math.max(1, n);
        if (in.down) sel = (sel + 1) % Math.max(1, n);
        if (in.back || in.escape) { state = ST_TITLE; return; }
        if (n > 0 && in.accept) {
            GameClient.Room r = roomList.get(sel);
            net.join(r.host, r.port, myName, 0);
        }
        // 输入 IP 加入（按 I）
        if (in.t && n == 0) { textMode = TM_JOINIP; main.openText(""); }
        if (in.e) { textMode = TM_JOINIP; main.openText(""); }
    }

    private void updatePlay(float dt, Main.Input in) {
        // 聊天快捷键
        if (in.t && textMode == TM_NONE) {
            if (net != null && net.connected()) {
                textMode = TM_CHAT; main.openText("");
            } else if (net == null) {
                // 单人也可用聊天做指令练习，但无网络则仅本地
                textMode = TM_CHAT; main.openText("");
            }
        }
        if (in.m && textMode == TM_NONE) {
            if (net != null && net.connected()) {
                net.leave(); leaveRoom();
                state = ST_TITLE;
            }
        }

        // 移动（联机中与单人都用左右移动）
        if (textMode == TM_NONE) {
            float spd = 60f;
            if (in.heldLeft) playerX = Math.max(30, (int)(playerX - spd * dt));
            if (in.heldRight) playerX = Math.min(Screen.LW - 30, (int)(playerX + spd * dt));
        }

        // 钓鱼阶段机
        switch (phase) {
            case PH_IDLE -> {
                if (in.accept && textMode == TM_NONE) {
                    phase = PH_CAST; phaseTimer = 0.4f;
                    castX = playerX; castY = 200;
                    bobX = playerX; bobY = 200;
                    pendingFish = -1;
                }
            }
            case PH_CAST -> {
                phaseTimer -= dt;
                float t = 1 - phaseTimer / 0.4f;
                bobX = castX + (240 + castX * 0.2f - castX) * t;
                bobY = castY - 80 * (float) Math.sin(t * Math.PI);
                if (phaseTimer <= 0) {
                    phase = PH_WAIT;
                    phaseTimer = 1.5f + rnd.nextFloat() * 4.0f;
                    pendingFish = pickFish();
                }
            }
            case PH_WAIT -> {
                phaseTimer -= dt;
                if (phaseTimer <= 0) { phase = PH_NIBBLE; phaseTimer = 1.1f; }
            }
            case PH_NIBBLE -> {
                phaseTimer -= dt;
                if (in.accept && textMode == TM_NONE) { catchFish(); }
                if (phaseTimer <= 0) { phase = PH_MISS; phaseTimer = 1.2f; }
            }
            case PH_MISS -> {
                phaseTimer -= dt;
                if (phaseTimer <= 0) phase = PH_IDLE;
            }
            case PH_CAUGHT -> {
                phaseTimer -= dt;
                if (phaseTimer <= 0) phase = PH_IDLE;
            }
        }

        // 联机：周期上报位置，并把阶段映射为 anim
        if (net != null && net.connected()) {
            posSyncTimer -= dt;
            if (posSyncTimer <= 0) {
                posSyncTimer = 0.3f;
                net.sendPos(playerX, phase, score);
            }
        }
    }

    private int pickFish() {
        // 按分值权重（越稀有越难）简单加权
        int total = 0;
        for (FishDef f : FISHES) total += Math.max(1, 10 - f.value / 16);
        int x = rnd.nextInt(Math.max(1, total));
        int acc = 0;
        for (int i = 0; i < FISHES.length; i++) {
            acc += Math.max(1, 10 - FISHES[i].value / 16);
            if (x < acc) return i;
        }
        return 0;
    }

    private void catchFish() {
        phase = PH_CAUGHT; phaseTimer = 2.0f;
        if (pendingFish >= 0 && pendingFish < FISHES.length) {
            FishDef f = FISHES[pendingFish];
            score += f.value;
            coins += Math.max(1, f.value / 3);
            chatAdd("钓到了 " + f.cn + " (" + f.en + ") +" + f.value);
        } else {
            chatAdd("钓到了鱼!");
        }
        pendingFish = -1;
    }

    // ================= 联机回调 =================
    @Override public void onWelcome(int id) {
        myId = id;
        players.put(id, selfPlayer());
        state = ST_PLAY;
        chatAdd("已加入房间 (你是 #" + id + ")");
    }
    @Override public void onEnter(int id, String name, int skin, int x, int anim, int score) {
        RemotePlayer p = new RemotePlayer();
        p.id = id; p.name = name; p.skin = skin; p.x = x; p.anim = anim; p.score = score;
        players.put(id, p);
        if (id != myId) chatAdd(name + " 加入了房间");
    }
    @Override public void onLeave(int id) {
        RemotePlayer p = players.remove(id);
        if (p != null && id != myId) chatAdd(p.name + " 离开了");
    }
    @Override public void onPos(int id, int x, int anim, int score) {
        RemotePlayer p = players.get(id);
        if (p != null) { p.x = x; p.anim = anim; p.score = score; }
    }
    @Override public void onChat(int id, String name, String text) {
        chatAdd("[" + name + "] " + text);
    }
    @Override public void onError(String msg) {
        chatAdd("[错误] " + msg);
    }
    @Override public void onKicked() {
        chatAdd("你被房主踢出了房间");
        leaveRoom();
        state = ST_TITLE;
    }
    @Override public void onFull() {
        chatAdd("房间已满，无法加入");
        if (state == ST_LAN) return;
        state = ST_LAN;
    }

    private RemotePlayer selfPlayer() {
        RemotePlayer p = new RemotePlayer();
        p.id = myId; p.name = myName; p.skin = 0; p.x = playerX; p.anim = phase; p.score = score;
        return p;
    }

    private void leaveRoom() {
        players.clear();
        myId = -1;
        textMode = TM_NONE;
    }

    // ================= 文本输入 / 聊天 / 指令 =================
    private void pollText() {
        if (textMode == TM_NONE) return;
        String s = main.pollSubmittedText();
        if (s == null) {
            // 用户 Esc 取消：关闭输入
            if (!main.textOpen()) textMode = TM_NONE;
            return;
        }
        String t = s.trim();
        if (textMode == TM_CHAT) {
            if (!t.isEmpty()) handleChatInput(t);
        } else if (textMode == TM_JOINIP) {
            if (!t.isEmpty()) {
                myName = "玩家";
                // 解析 host[:port]
                String host = t; int port = Protocol.PORT;
                int ci = t.lastIndexOf(':');
                if (ci > 0) {
                    try { port = Integer.parseInt(t.substring(ci + 1)); host = t.substring(0, ci); }
                    catch (Exception ignore) { }
                }
                if (net == null) net = new GameClient(this);
                net.join(host, port, myName, 0);
                chatAdd("正在加入 " + host + ":" + port + " ...");
            }
        }
        textMode = TM_NONE;
    }

    private void handleChatInput(String raw) {
        if (raw.startsWith("/")) { runCommand(raw.substring(1).trim()); return; }
        if (net != null && net.connected()) {
            net.sendChat(raw);
            chatAdd("[" + myName + "] " + raw);
        } else {
            chatAdd("[" + myName + "] " + raw);
            chatAdd("[系统] 未连接服务器，消息仅本地可见");
        }
    }

    private void chatAdd(String s) {
        chat.add(s);
        while (chat.size() > 8) chat.remove(0);
    }

    private void runCommand(String cmd) {
        String[] parts = cmd.split("\\s+");
        if (parts.length == 0) return;
        switch (parts[0].toLowerCase()) {
            case "help" -> {
                chatAdd("/help 帮助");
                chatAdd("/clear 清屏");
                chatAdd("/where 显示位置");
                chatAdd("/time 显示时间");
                chatAdd("/list 在线玩家");
                chatAdd("/kick <id> 踢人(房主)");
                chatAdd("/score 我的分数");
            }
            case "clear" -> chat.clear();
            case "where" -> chatAdd("位置 x=" + playerX + " 阶段=" + phaseName(phase));
            case "time" -> {
                int min = (int)(gameClock / 60);
                int h = 6 + (min / 60) % 24;
                int m = min % 60;
                chatAdd(String.format("游戏时间 %02d:%02d", h, m));
            }
            case "list" -> {
                if (net == null || !net.connected()) { chatAdd("未连接"); break; }
                chatAdd("在线玩家 " + players.size() + " 人:");
                for (RemotePlayer p : players.values())
                    chatAdd("  #" + p.id + " " + p.name + "  x=" + p.x);
            }
            case "score" -> chatAdd("分数 " + score + "  金币 " + coins);
            case "kick" -> {
                if (net == null || !net.connected()) { chatAdd("未连接"); break; }
                if (myId != 1) { chatAdd("只有房主(#1)可以踢人"); break; }
                if (parts.length < 2) { chatAdd("用法 /kick <id>"); break; }
                int id = Protocol.pint(parts[1], -1);
                if (id <= 0) { chatAdd("无效 id"); break; }
                net.kick(id);
                chatAdd("已请求踢出 #" + id);
            }
            default -> chatAdd("未知指令: /" + parts[0] + " (输入 /help)");
        }
    }

    private String phaseName(int p) {
        return switch (p) {
            case PH_IDLE -> "待机";
            case PH_CAST -> "抛竿";
            case PH_WAIT -> "等待";
            case PH_NIBBLE -> "上钩!";
            case PH_MISS -> "脱钩";
            case PH_CAUGHT -> "成功";
            default -> "?";
        };
    }

    // ================= 渲染 =================
    public void render(Screen s) {
        switch (state) {
            case ST_TITLE -> renderTitle(s);
            case ST_LAN -> renderLan(s);
            case ST_PLAY -> renderGame(s);
        }
        renderChat(s);
    }

    private void renderTitle(Screen s) {
        s.clear(Colors.DARK);
        renderSkyWater(s);
        s.textCenter("PIXEL LAKE HEART", 60, Colors.GOLD);
        s.textCenter("像素湖心", 84, Colors.SILVER);
        int y = 150;
        for (int i = 0; i < menuItems.length; i++) {
            int col = (i == sel) ? Colors.YELLOW : Colors.GRAY;
            String label = (i == sel ? "> " : "  ") + menuItems[i];
            s.textCenter(label, y + i * 18, col);
        }
        s.textCenter("W/S 选择  回车确认", 232, Colors.SILVER);
    }

    private void renderLan(Screen s) {
        s.clear(Colors.DARK);
        renderSkyWater(s);
        s.textCenter("局域网房间", 50, Colors.GOLD);
        s.textCenter("T 搜索 / 回车加入 / I 输IP  / Esc 返回", 66, Colors.SILVER);
        if (roomList.isEmpty()) {
            s.textCenter("扫描中... (若无房间，按 I 输入主机 IP)", 130, Colors.GRAY);
        } else {
            int y = 92;
            for (int i = 0; i < roomList.size() && i < 8; i++) {
                GameClient.Room r = roomList.get(i);
                int col = (i == sel) ? Colors.YELLOW : Colors.GRAY;
                String label = (i == sel ? "> " : "  ") + r.toString();
                s.text(label, 60, y + i * 16, col);
            }
        }
    }

    private void renderGame(Screen s) {
        renderSkyWater(s);
        renderPier(s);
        renderFish(s);
        renderBobber(s);
        renderPlayers(s);

        // HUD
        s.text("分数 " + score, 6, 4, Colors.WHITE);
        s.text("金币 " + coins, 6, 18, Colors.GOLD);
        int min = (int)(gameClock / 60);
        int h = 6 + (min / 60) % 24, m = min % 60;
        s.text(String.format("%02d:%02d", h, m), Screen.LW - 52, 4, Colors.WHITE);

        if (net != null && net.connected()) {
            s.text("在线 " + players.size(), 6, 32, Colors.CYAN);
            s.text("T 聊天  M 离开", Screen.LW - 110, 18, Colors.GRAY);
        } else {
            s.text("单人模式", 6, 32, Colors.CYAN);
        }

        // 阶段提示
        switch (phase) {
            case PH_IDLE -> s.textCenter("按 空格 抛竿", 246, Colors.WHITE);
            case PH_NIBBLE -> {
                s.textCenter("上钩了! 快按 空格 收杆!", 246, Colors.RED);
                s.text("!!!", (int) bobX - 8, 210, Colors.YELLOW);
            }
            case PH_MISS -> s.textCenter("鱼跑了...", 246, Colors.GRAY);
            case PH_CAUGHT -> s.textCenter("收杆成功!", 246, Colors.GREEN);
        }
    }

    // ---- 场景绘制 ----
    private void renderSkyWater(Screen s) {
        // 天空渐变（昼夜）
        int h = (int)(gameClock / 60) % 24 + 6;
        int skyTop, skyBot, waterC;
        if (h >= 6 && h < 18) { skyTop = Colors.rgb(120,190,235); skyBot = Colors.rgb(180,220,240); waterC = Colors.WATER; }
        else if (h >= 18 && h < 21) { skyTop = Colors.rgb(200,120,72); skyBot = Colors.rgb(240,180,120); waterC = Colors.WATER_DARK; }
        else { skyTop = Colors.rgb(26,36,80); skyBot = Colors.rgb(40,60,110); waterC = Colors.WATER_DARK; }

        for (int y = 0; y < 190; y++) {
            int c = Colors.lerp(skyTop, skyBot, y / 190f);
            s.rect(0, y, Screen.LW, 1, c);
        }
        // 水面（带波纹）
        for (int y = 190; y < Screen.LH; y++) {
            int c = Colors.lerp(waterC, Colors.WATER_DARK, (y - 190) / 80f);
            s.rect(0, y, Screen.LW, 1, c);
        }
        for (int x = 0; x < Screen.LW; x += 16) {
            int ry = 195 + ((x / 8) % 3) * 4;
            s.set(x, ry, Colors.rgb(90,170,220));
        }
    }

    private void renderPier(Screen s) {
        // 码头木板
        s.rect(0, 165, Screen.LW, 4, Colors.WOOD);
        s.rect(0, 169, Screen.LW, 22, Colors.PIER);
        for (int x = 0; x < Screen.LW; x += 32) {
            s.vline(x, 169, 22, Colors.rgb(90,70,60));
        }
        // 左侧草地
        s.rect(0, 150, 40, 20, Colors.GRASS);
        s.rect(40, 160, 20, 10, Colors.GRASS_DARK);
    }

    private void renderPlayers(Screen s) {
        // 自己
        drawPlayer(s, playerX, 156, 0, true);
        // 远端玩家
        for (RemotePlayer p : players.values()) {
            if (p.id == myId) continue;
            drawPlayer(s, p.x, 156, p.skin, false);
        }
    }

    private void drawPlayer(Screen s, int x, int y, int skin, boolean mine) {
        int body = mine ? 0xFFE06060 : 0xFF6080E0;
        int skinCol = Colors.rgb(235,190,150);
        // 头
        s.rect(x - 4, y - 16, 8, 8, skinCol);
        s.set(x - 4, y - 14, Colors.rgb(30,30,30)); // 眼
        s.set(x + 2, y - 14, Colors.rgb(30,30,30));
        // 身体
        s.rect(x - 5, y - 8, 10, 9, body);
        // 鱼竿
        s.hline(x + 4, y - 10, 26, Colors.rgb(90,60,40));
        s.vline(x + 30, y - 10, 14, Colors.rgb(70,50,35));
    }

    private void renderBobber(Screen s) {
        if (phase == PH_IDLE || phase == PH_CAUGHT) return;
        int bx = (int)bobX, by = (int)bobY;
        if (phase == PH_CAST || phase == PH_WAIT || phase == PH_NIBBLE || phase == PH_MISS) {
            s.set(bx, by - 2, Colors.RED);
            s.set(bx, by - 1, Colors.RED);
            s.set(bx, by, Colors.RED);
            // 鱼线（从竿尖到浮标）
            int fx = playerX + 30, fy = 156 - 10;
            line(s, fx, fy, bx, by, Colors.rgb(200,200,210));
        }
    }

    private void renderFish(Screen s) {
        if (phase != PH_NIBBLE || pendingFish < 0) return;
        // 上钩瞬间在水下画一条鱼
        FishDef f = FISHES[pendingFish];
        int fx = (int)bobX - 8, fy = (int)bobY + 14;
        drawFishSprite(s, fx, fy, f.color);
    }

    private void drawFishSprite(Screen s, int x, int y, int color) {
        // Minecraft 方块鱼：身体 + 眼 + 尾
        s.rect(x, y, 16, 8, color);
        s.rect(x, y, 4, 8, Colors.rgb(255,255,255));          // 头
        s.set(x + 2, y + 2, Colors.rgb(0,0,0));               // 眼
        s.rect(x + 16, y + 2, 4, 4, Colors.rgb(0,0,0));       // 尾
        // 体表像素纹理
        for (int i = 0; i < 8; i++) {
            int tx = x + 4 + i * 2;
            int ty = y + ((i % 2 == 0) ? 2 : 5);
            s.set(tx, ty, Colors.rgb(255,255,255));
        }
    }

    private void line(Screen s, int x0, int y0, int x1, int y1, int c) {
        int dx = Math.abs(x1 - x0), dy = Math.abs(y1 - y0);
        int sx = x0 < x1 ? 1 : -1, sy = y0 < y1 ? 1 : -1;
        int err = dx - dy;
        while (true) {
            s.set(x0, y0, c);
            if (x0 == x1 && y0 == y1) break;
            int e2 = 2 * err;
            if (e2 > -dy) { err -= dy; x0 += sx; }
            if (e2 < dx) { err += dx; y0 += sy; }
        }
    }

    private void renderChat(Screen s) {
        if (chat.isEmpty() && textMode == TM_NONE) return;
        int lines = Math.min(chat.size(), 5);
        int y = Screen.LH - 8 - lines * 14;
        for (int i = 0; i < lines; i++) {
            String line = chat.get(chat.size() - lines + i);
            s.text(line, 4, y + i * 14, Colors.WHITE);
        }
        if (textMode != TM_NONE) {
            s.rect(4, Screen.LH - 6, Screen.LW - 8, 6, Colors.BLACK);
            s.text("输入中... (回车发送 / Esc 取消)", 6, Screen.LH - 30, Colors.YELLOW);
        }
    }
}