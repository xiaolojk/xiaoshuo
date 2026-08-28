package com.pixellake;

import javax.swing.JFrame;
import javax.swing.JLayeredPane;
import javax.swing.JPanel;
import javax.swing.JTextField;
import javax.swing.SwingUtilities;
import java.awt.Color;
import java.awt.Dimension;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.RenderingHints;
import java.awt.event.ActionEvent;
import java.awt.event.KeyEvent;
import java.awt.event.KeyListener;
import java.awt.image.BufferedImage;
import java.util.concurrent.ConcurrentLinkedQueue;

/**
 * 主入口：负责窗口、固定 60FPS 主循环、键盘输入采集、以及 480x270 逻辑帧到窗口的
 * 整数倍近邻放大。文本输入（聊天 / 输 IP）走原生 JTextField，保证中文/英文/繁体
 * 输入法(IME)正常工作。
 */
public final class Main extends JPanel implements KeyListener, Runnable {

    public static final int LW = 480;
    public static final int LH = 270;
    static final int SCALE = 2;                 // 窗口 960x540（2x 整数放大）
    static final int WIN_W = LW * SCALE;
    static final int WIN_H = LH * SCALE;

    private final App app = new App(this);
    private final Screen screen = new Screen();
    private final BufferedImage frame = new BufferedImage(LW, LH, BufferedImage.TYPE_INT_ARGB);

    private final boolean[] held = new boolean[512];
    private final ConcurrentLinkedQueue<Integer> pressed = new ConcurrentLinkedQueue<>();
    private volatile boolean running = true;

    // 文本输入 overlay（聊天 / IP），原生 IME
    private final JTextField textField = new JTextField();
    private final ConcurrentLinkedQueue<String> submittedText = new ConcurrentLinkedQueue<>();
    private volatile boolean textOpen = false;

    public Main() {
        setPreferredSize(new Dimension(WIN_W, WIN_H));
        setBackground(Color.BLACK);
        setFocusable(true);
        addKeyListener(this);
    }

    // ---- 文本输入接口（由 App 调用）----
    void openText(String initial) {
        textField.setText(initial == null ? "" : initial);
        SwingUtilities.invokeLater(() -> {
            textField.setVisible(true);
            textField.requestFocusInWindow();
            textOpen = true;
        });
    }

    void closeText() {
        SwingUtilities.invokeLater(() -> {
            textField.setVisible(false);
            textOpen = false;
        });
    }

    boolean textOpen() { return textOpen; }
    String pollSubmittedText() { return submittedText.poll(); }

    // ---- 输入快照 ----
    static final class Input {
        boolean space, accept, back, up, down, e, b, m, t, f, one, two;
        boolean left, right;              // 边沿（菜单左右切换）
        boolean heldLeft, heldRight;      // 持续（移动）
        boolean escape;
    }

    private void snapshot(Input in) {
        Integer code;
        while ((code = pressed.poll()) != null) {
            int c = code;
            switch (c) {
                case KeyEvent.VK_SPACE, KeyEvent.VK_ENTER -> { in.space = true; in.accept = true; }
                case KeyEvent.VK_ESCAPE -> { in.back = true; in.escape = true; }
                case KeyEvent.VK_UP, KeyEvent.VK_W -> in.up = true;
                case KeyEvent.VK_DOWN, KeyEvent.VK_S -> in.down = true;
                case KeyEvent.VK_LEFT -> { in.left = true; }
                case KeyEvent.VK_RIGHT -> { in.right = true; }
                case KeyEvent.VK_E -> in.e = true;
                case KeyEvent.VK_B -> in.b = true;
                case KeyEvent.VK_M -> in.m = true;
                case KeyEvent.VK_T -> in.t = true;
                case KeyEvent.VK_F -> in.f = true;
                case KeyEvent.VK_1 -> in.one = true;
                case KeyEvent.VK_2 -> in.two = true;
                default -> { }
            }
        }
        in.heldLeft = held[KeyEvent.VK_LEFT];
        in.heldRight = held[KeyEvent.VK_RIGHT];
    }

    // ---- 渲染：逻辑帧 -> 窗口 ----
    private void present() {
        frame.setRGB(0, 0, LW, LH, screen.pixels(), 0, LW);
    }

    @Override
    protected void paintComponent(Graphics g) {
        super.paintComponent(g);
        Graphics2D g2 = (Graphics2D) g.create();
        g2.setRenderingHint(RenderingHints.KEY_INTERPOLATION, RenderingHints.VALUE_INTERPOLATION_NEAREST_NEIGHBOR);
        g2.setRenderingHint(RenderingHints.KEY_RENDERING, RenderingHints.VALUE_RENDER_SPEED);
        g2.drawImage(frame, 0, 0, getWidth(), getHeight(), null);
        g2.dispose();
    }

    // ---- KeyListener ----
    @Override public void keyPressed(KeyEvent e) {
        held[e.getKeyCode()] = true;
        pressed.add(e.getKeyCode());
    }
    @Override public void keyReleased(KeyEvent e) { held[e.getKeyCode()] = false; }
    @Override public void keyTyped(KeyEvent e) { }

    // ---- 主循环 ----
    @Override
    public void run() {
        long prev = System.nanoTime();
        while (running) {
            long now = System.nanoTime();
            float dt = Math.min(0.1f, (now - prev) / 1_000_000_000f);
            prev = now;

            Input in = new Input();
            snapshot(in);
            app.update(dt, in);
            app.render(screen);
            present();
            repaint();

            long frameNs = 16_666_667L;
            long sleep = frameNs - (System.nanoTime() - now);
            if (sleep > 0) {
                try { Thread.sleep(sleep / 1_000_000, (int) (sleep % 1_000_000)); }
                catch (InterruptedException ignore) { }
            }
        }
    }

    // ---- 启动 ----
    public static void main(String[] args) {
        Main panel = new Main();
        JFrame f = new JFrame("PIXEL LAKE HEART - 像素湖心");
        JLayeredPane lp = new JLayeredPane();
        lp.setPreferredSize(new Dimension(WIN_W, WIN_H));

        panel.setBounds(0, 0, WIN_W, WIN_H);
        lp.add(panel, JLayeredPane.DEFAULT_LAYER);

        panel.textField.setBounds(40, WIN_H - 52, WIN_W - 80, 28);
        panel.textField.setVisible(false);
        panel.textField.setFont(new java.awt.Font(java.awt.Font.SANS_SERIF, java.awt.Font.PLAIN, 14));
        panel.textField.addActionListener((ActionEvent ae) -> {
            panel.submittedText.add(panel.textField.getText());
            panel.closeText();
        });
        // Esc 取消
        panel.textField.getInputMap().put(javax.swing.KeyStroke.getKeyStroke("ESCAPE"), "cancel");
        panel.textField.getActionMap().put("cancel", new javax.swing.AbstractAction() {
            @Override public void actionPerformed(ActionEvent e) {
                panel.submittedText.add(null);
                panel.closeText();
            }
        });
        lp.add(panel.textField, JLayeredPane.PALETTE_LAYER);

        f.setContentPane(lp);
        f.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        f.setResizable(false);
        f.pack();
        f.setLocationRelativeTo(null);
        f.setVisible(true);
        panel.requestFocusInWindow();

        Thread game = new Thread(panel, "pixellake-main");
        game.setDaemon(false);
        game.start();
    }
}