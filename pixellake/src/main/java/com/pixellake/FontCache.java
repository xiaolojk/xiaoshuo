package com.pixellake;

import java.awt.Color;
import java.awt.Font;
import java.awt.FontMetrics;
import java.awt.Graphics2D;
import java.awt.RenderingHints;
import java.awt.image.BufferedImage;
import java.io.File;
import java.util.HashMap;
import java.util.Map;

/**
 * 字体缓存：中/英/繁三套字体，按码点路由（与 C 版 ttf_font_for 逻辑一致）。
 * 优先加载发行版字体包 fonts/{en,zhong,trad}.ttf；缺失时回退到系统逻辑字体，
 * 保证游戏永不因缺字体而崩溃。
 */
public final class FontCache {
    /** 字符逻辑像素高度。 */
    public static final int H = 12;

    private static final int EN = 0, ZH = 1, TRAD = 2;
    private static Font[] faces;
    private static boolean[] avail;
    private static final Map<Integer, Glyph> cache = new HashMap<>();

    public static final class Glyph {
        public int w, h, xoff, yoff, adv;
        public boolean[] m;
    }

    static { load(); }

    public static synchronized void load() {
        faces = new Font[3];
        avail = new boolean[3];
        Font fallback = new Font(Font.SANS_SERIF, Font.PLAIN, H);
        tryLoad(EN,   new String[]{"fonts/en.ttf",     "./fonts/en.ttf",     "fonts/zhong.ttf", "./fonts/zhong.ttf"}, fallback);
        tryLoad(ZH,   new String[]{"fonts/zhong.ttf",  "./fonts/zhong.ttf"}, fallback);
        tryLoad(TRAD, new String[]{"fonts/trad.ttf",   "./fonts/trad.ttf",   "fonts/zhong.ttf", "./fonts/zhong.ttf"}, fallback);
        for (int i = 0; i < 3; i++) if (faces[i] == null) { faces[i] = fallback; avail[i] = false; }
    }

    public static boolean externalFontsLoaded() {
        return avail[ZH] || avail[TRAD] || avail[EN];
    }

    private static void tryLoad(int idx, String[] paths, Font fallback) {
        for (String p : paths) {
            File f = new File(p);
            if (!f.exists()) continue;
            try {
                Font ff = Font.createFont(Font.TRUETYPE_FONT, f).deriveFont(Font.PLAIN, H);
                faces[idx] = ff;
                avail[idx] = true;
                return;
            } catch (Exception ignore) { /* 尝试下一个 */ }
        }
        faces[idx] = fallback;
        avail[idx] = false;
    }

    public static Font fontFor(int cp) {
        if (cp < 0x2E80) return faces[EN];
        if ((cp >= 0xF900 && cp <= 0xFAFF) || (cp >= 0xFF00 && cp <= 0xFFEF)) return faces[TRAD];
        return faces[ZH];
    }

    public static Glyph glyph(int cp) {
        Glyph g = cache.get(cp);
        if (g != null) return g;
        synchronized (FontCache.class) {
            g = cache.get(cp);
            if (g == null) {
                g = render(cp, fontFor(cp));
                cache.put(cp, g);
            }
        }
        return g;
    }

    private static Glyph render(int cp, Font f) {
        int pad = H + 6;
        int size = pad * 2;
        BufferedImage bi = new BufferedImage(size, size, BufferedImage.TYPE_INT_RGB);
        Graphics2D g2 = bi.createGraphics();
        g2.setFont(f);
        g2.setColor(Color.WHITE);
        g2.setBackground(Color.BLACK);
        g2.clearRect(0, 0, size, size);
        g2.setRenderingHint(RenderingHints.KEY_TEXT_ANTIALIASING, RenderingHints.VALUE_TEXT_ANTIALIAS_OFF);
        g2.setRenderingHint(RenderingHints.KEY_FRACTIONALMETRICS, RenderingHints.VALUE_FRACTIONALMETRICS_OFF);
        FontMetrics fm = g2.getFontMetrics();
        String s = new String(Character.toChars(cp));
        int adv = fm.charWidth(cp);
        int asc = fm.getAscent();
        g2.drawString(s, pad / 2, pad / 2 + asc);
        g2.dispose();

        int minX = size, minY = size, maxX = -1, maxY = -1;
        for (int y = 0; y < size; y++) {
            for (int x = 0; x < size; x++) {
                if ((bi.getRGB(x, y) & 0xFFFFFF) != 0) {
                    if (x < minX) minX = x;
                    if (x > maxX) maxX = x;
                    if (y < minY) minY = y;
                    if (y > maxY) maxY = y;
                }
            }
        }

        Glyph g = new Glyph();
        if (maxX < minX) { // 空白字符（空格等）
            g.w = 0; g.h = 0; g.xoff = 0; g.yoff = 0;
            g.adv = Math.max(adv, H / 3);
            g.m = new boolean[0];
            return g;
        }
        int w = maxX - minX + 1, h = maxY - minY + 1;
        boolean[] m = new boolean[w * h];
        for (int y = minY; y <= maxY; y++) {
            for (int x = minX; x <= maxX; x++) {
                if ((bi.getRGB(x, y) & 0xFFFFFF) != 0) {
                    m[(y - minY) * w + (x - minX)] = true;
                }
            }
        }
        g.w = w; g.h = h;
        g.xoff = minX - pad / 2;
        g.yoff = minY - pad / 2;
        g.adv = adv;
        g.m = m;
        return g;
    }
}