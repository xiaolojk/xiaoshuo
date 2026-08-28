package com.pixellake.net;

import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

/** UDP 联机客户端：房间发现、加入、位置上报、聊天收发（事件经 Listener 回传给上层）。 */
public class GameClient implements AutoCloseable {

    public static final class Room {
        public final String host;
        public final int port;
        public final String name;
        public final int players;
        public final int max;
        public long seen;

        Room(String host, int port, String name, int players, int max) {
            this.host = host;
            this.port = port;
            this.name = name;
            this.players = players;
            this.max = max;
            this.seen = System.currentTimeMillis();
        }

        @Override
        public String toString() { return name + "  (" + host + ":" + port + ")  " + players + "/" + max; }
    }

    public interface Listener {
        void onWelcome(int id);
        void onEnter(int id, String name, int skin, int x, int anim, int score);
        void onLeave(int id);
        void onPos(int id, int x, int anim, int score);
        void onChat(int id, String name, String text);
        void onError(String msg);
        void onKicked();
        void onFull();
    }

    private final Listener listener;
    private DatagramSocket socket;
    private final Thread thread;
    private volatile boolean running = true;
    private int myId = -1;
    private InetAddress serverAddr;
    private int serverPort = Protocol.PORT;
    private final Map<String, Room> rooms = new LinkedHashMap<>();

    public GameClient(Listener l) {
        this.listener = l;
        try {
            socket = new DatagramSocket();
            socket.setBroadcast(true);
            socket.setSoTimeout(400);
        } catch (Exception e) {
            if (listener != null) listener.onError("网络初始化失败: " + e.getMessage());
        }
        thread = new Thread(this::run, "pixellake-client");
        thread.setDaemon(true);
        thread.start();
    }

    public int myId() { return myId; }
    public boolean connected() { return myId >= 0; }

    public List<Room> rooms() {
        synchronized (rooms) { return new ArrayList<>(rooms.values()); }
    }

    /** 广播搜索房间（结果经 room 列表可读，同时不清空旧房间，由 App 决定刷新时机）。 */
    public void search() {
        synchronized (rooms) { rooms.clear(); }
        broadcast(Protocol.C_SEARCH);
    }

    public void join(String host, int port, String name, int skin) {
        try {
            serverAddr = InetAddress.getByName(host);
        } catch (Exception e) {
            if (listener != null) listener.onError("无效地址: " + host);
            return;
        }
        serverPort = port;
        send(Protocol.C_JOIN, name, String.valueOf(skin));
    }

    public void sendPos(int x, int anim, int score) {
        if (connected()) send(Protocol.C_POS, String.valueOf(x), String.valueOf(anim), String.valueOf(score));
    }

    public void sendChat(String text) {
        if (connected()) send(Protocol.C_CHAT, text);
    }

    public void kick(int targetId) {
        if (connected()) send(Protocol.C_KICK, String.valueOf(targetId));
    }

    public void leave() {
        if (connected()) send(Protocol.C_LEAVE);
        myId = -1;
    }

    private void broadcast(String... fields) {
        if (socket == null) return;
        byte[] data = Protocol.encode(fields);
        try {
            socket.send(new DatagramPacket(data, data.length, InetAddress.getByName("255.255.255.255"), Protocol.PORT));
        } catch (Exception ignore) { }
    }

    private void send(String... fields) {
        if (socket == null || serverAddr == null) return;
        byte[] data = Protocol.encode(fields);
        try { socket.send(new DatagramPacket(data, data.length, serverAddr, serverPort)); } catch (Exception ignore) { }
    }

    private void run() {
        byte[] buf = new byte[8192];
        while (running) {
            DatagramPacket pkt = new DatagramPacket(buf, buf.length);
            try {
                socket.receive(pkt);
            } catch (Exception e) {
                if (!running) break;
                continue;
            }
            String data = new String(pkt.getData(), pkt.getOffset(), pkt.getLength(), StandardCharsets.UTF_8);
            for (String line : data.split("\n")) {
                String t = line.trim();
                if (t.isEmpty()) continue;
                try { dispatch(t, pkt.getAddress().getHostAddress(), pkt.getPort()); } catch (Exception ignore) { }
            }
            long now = System.currentTimeMillis();
            synchronized (rooms) {
                rooms.values().removeIf(r -> now - r.seen > 4000);
            }
        }
    }

    private void dispatch(String line, String host, int port) {
        String[] f = Protocol.decode(line);
        if (f.length == 0) return;
        switch (f[0]) {
            case Protocol.S_ANNO -> {
                String name = f.length > 1 ? f[1] : "";
                int players = f.length > 2 ? Protocol.pint(f[2], 0) : 0;
                int max = f.length > 3 ? Protocol.pint(f[3], 0) : 0;
                synchronized (rooms) {
                    Room r = new Room(host, port, name, players, max);
                    rooms.put(host + ":" + port, r);
                }
            }
            case Protocol.S_WELCOME -> {
                myId = f.length > 1 ? Protocol.pint(f[1], -1) : -1;
                if (listener != null) listener.onWelcome(myId);
            }
            case Protocol.S_ENTER -> {
                int id = f.length > 1 ? Protocol.pint(f[1], -1) : -1;
                String name = f.length > 2 ? f[2] : "";
                int skin = f.length > 3 ? Protocol.pint(f[3], 0) : 0;
                int x = f.length > 4 ? Protocol.pint(f[4], 0) : 0;
                int anim = f.length > 5 ? Protocol.pint(f[5], 0) : 0;
                int score = f.length > 6 ? Protocol.pint(f[6], 0) : 0;
                if (listener != null) listener.onEnter(id, name, skin, x, anim, score);
            }
            case Protocol.S_LEAVE -> {
                int id = f.length > 1 ? Protocol.pint(f[1], -1) : -1;
                if (listener != null) listener.onLeave(id);
            }
            case Protocol.S_POSB -> {
                int id = f.length > 1 ? Protocol.pint(f[1], -1) : -1;
                int x = f.length > 2 ? Protocol.pint(f[2], 0) : 0;
                int anim = f.length > 3 ? Protocol.pint(f[3], 0) : 0;
                int score = f.length > 4 ? Protocol.pint(f[4], 0) : 0;
                if (listener != null) listener.onPos(id, x, anim, score);
            }
            case Protocol.S_CHATB -> {
                int id = f.length > 1 ? Protocol.pint(f[1], -1) : -1;
                String name = f.length > 2 ? f[2] : "";
                String text = f.length > 3 ? f[3] : "";
                if (listener != null) listener.onChat(id, name, text);
            }
            case Protocol.S_KICK -> { if (listener != null) listener.onKicked(); }
            case Protocol.S_FULL -> { if (listener != null) listener.onFull(); }
            default -> { }
        }
    }

    @Override
    public void close() {
        running = false;
        if (socket != null) { try { socket.close(); } catch (Exception ignore) { } }
    }
}