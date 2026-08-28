package com.pixellake;

/** 逻辑像素帧缓冲：低分辨率(480x270)软渲染，由窗口做整数倍近邻放大，保证像素颗粒清晰。 */
public final class Screen {
    public static final int LW = 480;
    public static final int LH = 270;

    private final int[] px = new int[LW * LH];

    public int w() { return LW; }
    public int h() { return LH; }
    public int[] pixels() { return px; }

    public void clear(int c) { java.util.Arrays.fill(px, c); }

    public void set(int x, int y, int c) {
        if (x < 0 || y < 0 || x >= LW || y >= LH) return;
        px[y * LW + x] = c;
    }

    public void rect(int x, int y, int w, int h, int c) {
        int x1 = Math.max(0, x), y1 = Math.max(0, y);
        int x2 = Math.min(LW, x + w), y2 = Math.min(LH, y + h);
        for (int yy = y1; yy < y2; yy++) {
            int base = yy * LW;
            for (int xx = x1; xx < x2; xx++) px[base + xx] = c;
        }
    }

    public void box(int x, int y, int w, int h, int c) {
        rect(x, y, w, 1, c);
        rect(x, y + h - 1, w, 1, c);
        rect(x, y, 1, h, c);
        rect(x + w - 1, y, 1, h, c);
    }

    public void vline(int x, int y, int h, int c) { rect(x, y, 1, h, c); }
    public void hline(int x, int y, int w, int c) { rect(x, y, w, 1, c); }

    /** 画单个字形（由 FontCache 提供 mask）。 */
    private void glyph(FontCache.Glyph g, int x, int y, int c) {
        if (g == null || g.m.length == 0) return;
        for (int yy = 0; yy < g.h; yy++) {
            int sy = y + g.yoff + yy;
            if (sy < 0 || sy >= LH) continue;
            int row = yy * g.w;
            for (int xx = 0; xx < g.w; xx++) {
                if (g.m[row + xx]) {
                    int sx = x + g.xoff + xx;
                    if (sx >= 0 && sx < LW) px[sy * LW + sx] = c;
                }
            }
        }
    }

    /** 绘制 UTF-8 文本（支持中/英/繁），x,y 为左上角。 */
    public void text(String s, int x, int y, int c) {
        if (s == null || s.isEmpty()) return;
        int cx = x;
        for (int i = 0; i < s.length(); ) {
            int cp = s.codePointAt(i);
            i += Character.charCount(cp);
            FontCache.Glyph g = FontCache.glyph(cp);
            glyph(g, cx, y, c);
            cx += (g != null && g.adv > 0) ? g.adv : FontCache.H;
        }
    }

    public int textWidth(String s) {
        if (s == null || s.isEmpty()) return 0;
        int w = 0;
        for (int i = 0; i < s.length(); ) {
            int cp = s.codePointAt(i);
            i += Character.charCount(cp);
            FontCache.Glyph g = FontCache.glyph(cp);
            w += (g != null && g.adv > 0) ? g.adv : FontCache.H;
        }
        return w;
    }

    public void textCenter(String s, int y, int c) {
        text(s, (LW - textWidth(s)) / 2, y, c);
    }
}