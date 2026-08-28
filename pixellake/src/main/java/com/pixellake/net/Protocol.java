package com.pixellake.net;

import java.nio.charset.StandardCharsets;

/** UDP 联机协议：文本行，UTF-8，用 TAB 分隔字段，'\n' 结尾。 */
public final class Protocol {
    private Protocol() {}

    public static final int PORT = 3317;
    public static final int MAX_PLAYERS = 8;

    public static final String SEP = "\t";

    // 客户端 -> 服务端
    public static final String C_SEARCH = "SEARCH";   // 广播：房间发现
    public static final String C_JOIN   = "JOIN";     // 加入：name, skin
    public static final String C_LEAVE  = "LEAVE";    // 离开
    public static final String C_POS    = "POS";      // 上报：x, anim, score
    public static final String C_CHAT   = "CHAT";     // 聊天：text(可含空格/中文)
    public static final String C_KICK   = "KICK";     // 房主踢人：目标 id

    // 服务端 -> 客户端
    public static final String S_ANNO    = "ANNO";    // 房间公告：name, players, max
    public static final String S_WELCOME = "WELCOME"; // 分配 id
    public static final String S_ENTER   = "ENTER";   // 玩家进入：id, name, skin, x, anim, score
    public static final String S_LEAVE   = "LEAVE";   // 玩家离开：id
    public static final String S_POSB    = "POSB";    // 位置同步：id, x, anim, score
    public static final String S_CHATB   = "CHATB";   // 聊天广播：id, name, text
    public static final String S_KICK    = "KICKED";  // 你被踢出
    public static final String S_FULL    = "FULL";    // 房间已满

    public static byte[] encode(String... fields) {
        return (String.join(SEP, fields) + "\n").getBytes(StandardCharsets.UTF_8);
    }

    /** 解析一行协议数据，返回字段数组（不含命令做特殊处理，字段 0 即命令）。 */
    public static String[] decode(String line) {
        if (line.endsWith("\n")) line = line.substring(0, line.length() - 1);
        if (line.endsWith("\r")) line = line.substring(0, line.length() - 1);
        return line.split(SEP, -1);
    }

    public static int pint(String s, int def) {
        try { return Integer.parseInt(s.trim()); } catch (Exception e) { return def; }
    }
}