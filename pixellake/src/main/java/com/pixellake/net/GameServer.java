package com.pixellake.net;

import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.nio.charset.StandardCharsets;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.atomic.AtomicInteger;

/**
 * PixelLakeHeart 权威联机服务器（专用进程，无界面，可独立运行）。
 *
 * 职责：维护房间内玩家表，权威分配 id，并中继 JOIN/LEAVE/POS/CHAT 到所有客户端；
 * 同时响应局域网广播发现(SEARCH -> ANNO)。不模拟具体钓鱼逻辑（由各客户端本地运行），
 * 只做轻量状态同步与消息转发。
 *
 * 用法：
 *   java -cp PixelLakeHeart.jar com.pixellake.net.GameServer [端口] [服务器名]
 */
public class GameServer implements AutoCloseable {

    public static final class ClientInfo {
        public final InetAddress addr;
        public final int port;
        public int id;
        public String name;
        public int skin;
        public int x, anim, score;
        public volatile long lastSeen;

        ClientInfo(InetAddress addr, int port) {
            this.addr = addr;
            this.port = port;
            this.name = "玩家";
            this.lastSeen = System.currentTimeMillis();
        }
    }

    private final DatagramSocket socket;
    private final Map<String, ClientInfo> clients = new ConcurrentHashMap<>();
    private final AtomicInteger nextId = new AtomicInteger(1);
    private volatile boolean running = true;
    private final Thread thread;
            private final Thread cleanupThread;
            private final boolean verbose;
            private final String serverName;

    public GameServer(int port, String serverName, boolean verbose) throws Exception {
        this.serverName = (serverName == null || serverName.trim().isEmpty()) ? "PixelLakeHeart" : serverName.trim();
        this.verbose = verbose;
        this.socket = new DatagramSocket(port);
        this.socket.setBroadcast(true);
        this.thread = new Thread(this::run, "pixellake-server");
                this.thread.setDaemon(true);
                this.cleanupThread = new Thread(this::cleanupLoop, "pixellake-cleanup");
                this.cleanupThread.setDaemon(true);
            }

            public void start() { thread.start(); cleanupThread.start(); }

    public int port() { return socket.getLocalPort(); }
    public int online() { return clients.size(); }

    private void log(String s) { if (verbose) System.out.println("[srv] " + s); }

            /** 定时清理：掉线(超时未上报)的客户端自动移除并广播，防止占位。 */
            private void cleanupLoop() {
                while (running) {
                    try { Thread.sleep(5000); } catch (InterruptedException e) { break; }
                    long now = System.currentTimeMillis();
                    for (ClientInfo ci : clients.values().toArray(new ClientInfo[0])) {
                        if (now - ci.lastSeen > 12000) {
                            clients.remove(key(ci.addr, ci.port));
                            broadcast(Protocol.S_LEAVE, str(ci.id));
                            log("超时移除 #" + ci.id + " " + ci.name + "  在线 " + clients.size() + "/" + Protocol.MAX_PLAYERS);
                        }
                    }
                }
            }

    private void run() {
        byte[] buf = new byte[8192];
        while (running) {
            DatagramPacket pkt = new DatagramPacket(buf, buf.length);
            try {
                socket.receive(pkt);
            } catch (Exception e) {
                if (running) continue;
                break;
            }
            String data = new String(pkt.getData(), pkt.getOffset(), pkt.getLength(), StandardCharsets.UTF_8);
            for (String line : data.split("\n")) {
                String t = line.trim();
                if (!t.isEmpty()) handle(t, pkt.getAddress(), pkt.getPort());
            }
        }
    }

    private String key(InetAddress a, int p) { return a.getHostAddress() + ":" + p; }

    private void handle(String line, InetAddress from, int fromPort) {
        String[] f = Protocol.decode(line);
        if (f.length == 0) return;
        String cmd = f[0];
        String k = key(from, fromPort);
        switch (cmd) {
            case Protocol.C_SEARCH -> {
                send(from, fromPort, Protocol.S_ANNO, serverName, str(clients.size()), str(Protocol.MAX_PLAYERS));
            }
            case Protocol.C_JOIN -> {
                ClientInfo existing = clients.get(k);
                if (existing == null && clients.size() >= Protocol.MAX_PLAYERS) {
                    send(from, fromPort, Protocol.S_FULL);
                    break;
                }
                String name = f.length > 1 && !f[1].isBlank() ? f[1] : "玩家" + (nextId.get());
                int skin = f.length > 2 ? Protocol.pint(f[2], 0) : 0;
                ClientInfo ci = existing;
                if (ci == null) {
                    ci = new ClientInfo(from, fromPort);
                    ci.id = nextId.getAndIncrement();
                    clients.put(k, ci);
                }
                ci.name = name;
                ci.skin = skin;
                ci.lastSeen = System.currentTimeMillis();
                send(from, fromPort, Protocol.S_WELCOME, str(ci.id));
                for (ClientInfo o : clients.values()) {
                    if (o == ci) continue;
                    send(from, fromPort, Protocol.S_ENTER, str(o.id), o.name, str(o.skin), str(o.x), str(o.anim), str(o.score));
                }
                broadcast(Protocol.S_ENTER, str(ci.id), ci.name, str(ci.skin), str(ci.x), str(ci.anim), str(ci.score));
                log("加入 #" + ci.id + " " + ci.name + "  在线 " + clients.size() + "/" + Protocol.MAX_PLAYERS);
            }
            case Protocol.C_LEAVE -> {
                ClientInfo ci = clients.remove(k);
                if (ci != null) {
                    broadcast(Protocol.S_LEAVE, str(ci.id));
                    log("离开 #" + ci.id + " " + ci.name);
                }
            }
            case Protocol.C_POS -> {
                ClientInfo ci = clients.get(k);
                if (ci == null) break;
                ci.x = f.length > 1 ? Protocol.pint(f[1], ci.x) : ci.x;
                ci.anim = f.length > 2 ? Protocol.pint(f[2], ci.anim) : ci.anim;
                ci.score = f.length > 3 ? Protocol.pint(f[3], ci.score) : ci.score;
                ci.lastSeen = System.currentTimeMillis();
                broadcastExcept(from, fromPort, Protocol.S_POSB, str(ci.id), str(ci.x), str(ci.anim), str(ci.score));
            }
            case Protocol.C_CHAT -> {
                ClientInfo ci = clients.get(k);
                String text = f.length > 1 ? f[1] : "";
                if (text.isBlank()) break;
                String name = ci != null ? ci.name : "?";
                String id = ci != null ? str(ci.id) : "?";
                broadcast(Protocol.S_CHATB, id, name, text);
                log("<" + name + "> " + text);
            }
            case Protocol.C_KICK -> {
                ClientInfo ci = clients.get(k);
                if (ci == null || ci.id != 1) break;   // 仅房主(#1)可踢人
                int target = f.length > 1 ? Protocol.pint(f[1], -1) : -1;
                for (ClientInfo o : clients.values().toArray(new ClientInfo[0])) {
                    if (o.id == target) {
                        send(o.addr, o.port, Protocol.S_KICK);
                        broadcast(Protocol.S_LEAVE, str(target));
                        clients.remove(key(o.addr, o.port));
                        log("踢出 #" + target + " " + o.name);
                        break;
                    }
                }
            }
            default -> { }
        }
    }

    private void broadcast(String... fields) {
        byte[] data = Protocol.encode(fields);
        for (ClientInfo ci : clients.values()) sendRaw(ci.addr, ci.port, data);
    }

    private void broadcastExcept(InetAddress skipAddr, int skipPort, String... fields) {
        byte[] data = Protocol.encode(fields);
        for (ClientInfo ci : clients.values()) {
            if (ci.addr.equals(skipAddr) && ci.port == skipPort) continue;
            sendRaw(ci.addr, ci.port, data);
        }
    }

    private void send(InetAddress a, int p, String... fields) { sendRaw(a, p, Protocol.encode(fields)); }

    private void sendRaw(InetAddress a, int p, byte[] data) {
        try { socket.send(new DatagramPacket(data, data.length, a, p)); } catch (Exception ignore) { }
    }

    @Override
    public void close() {
        running = false;
        try { socket.close(); } catch (Exception ignore) { }
    }

    static String str(int v) { return String.valueOf(v); }

            /** 列出本机 IPv4（供客户端按 I 直接输 IP 加入）。 */
            private static void printLocalIps() {
                try {
                    java.util.Enumeration<java.net.NetworkInterface> ifs = java.net.NetworkInterface.getNetworkInterfaces();
                    System.out.println("  本机局域网 IP（客户端按 I 输入其中一个即可加入）:");
                    while (ifs.hasMoreElements()) {
                        java.net.NetworkInterface ni = ifs.nextElement();
                        if (!ni.isUp() || ni.isLoopback()) continue;
                        java.util.Enumeration<java.net.InetAddress> addrs = ni.getInetAddresses();
                        while (addrs.hasMoreElements()) {
                            java.net.InetAddress a = addrs.nextElement();
                            if (a instanceof java.net.Inet4Address)
                                System.out.println("    " + a.getHostAddress());
                        }
                    }
                } catch (Exception ignore) { }
            }

            // --- 独立运行入口 ---
    public static void main(String[] args) throws Exception {
        int port = Protocol.PORT;
        String name = "PixelLakeHeart";
        if (args.length >= 1) port = Integer.parseInt(args[0]);
        if (args.length >= 2) name = args[1];
        GameServer srv = new GameServer(port, name, true);
        srv.start();
        System.out.println("PixelLakeHeart 联机服务器已启动");
        System.out.println("  监听端口: " + srv.port());
        System.out.println("  房间名:   " + name);
                printLocalIps();
                System.out.println("  按 Ctrl+C 停止。");
        // 阻塞主线程不让进程退出
        Thread.currentThread().join();
    }
}