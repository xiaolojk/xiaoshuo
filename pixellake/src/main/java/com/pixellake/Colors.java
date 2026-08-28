package com.pixellake;

/** 调色板：统一的 ARGB 颜色常量，与 C 版 PACKED 颜色一一对应。 */
public final class Colors {
    private Colors() {}

    public static final int BLACK   = 0xFF000000;
    public static final int DARK    = 0xFF12141E;
    public static final int WHITE   = 0xFFFFFFFF;
    public static final int GRAY    = 0xFF9AA0AA;
    public static final int SILVER  = 0xFFA8B0BE;
    public static final int GOLD    = 0xFFF0C040;
    public static final int YELLOW  = 0xFFFFFF60;
    public static final int CYAN    = 0xFF60D0E0;
    public static final int RED     = 0xFFE06060;
    public static final int GREEN   = 0xFF60C060;
    public static final int BLUE    = 0xFF6080E0;
    public static final int ORANGE  = 0xFFF09050;
    public static final int PURPLE  = 0xFFB080E0;

    // 场景色
    public static final int SKY_DAY    = 0xFF6FB9E8;
    public static final int SKY_DUSK   = 0xFFC87848;
    public static final int SKY_NIGHT  = 0xFF1A2450;
    public static final int WATER      = 0xFF2E6FB0;
    public static final int WATER_DARK = 0xFF1E4E80;
    public static final int GRASS      = 0xFF4E9A3C;
    public static final int GRASS_DARK = 0xFF35742A;
    public static final int SAND       = 0xFFD8C07A;
    public static final int WOOD       = 0xFF8A5A32;
    public static final int PIER       = 0xFF7A5A42;

    // 聊天气泡/面板
    public static final int PANEL_BG     = 0xFF06080E;
    public static final int PANEL_EDGE   = 0xFF384358;
    public static final int PANEL_EDGE_D = 0xFF1C222E;

    public static int rgb(int r, int g, int b) {
        return 0xFF000000 | ((r & 0xFF) << 16) | ((g & 0xFF) << 8) | (b & 0xFF);
    }

    public static int lerp(int a, int b, float t) {
        int ar = (a >> 16) & 0xFF, ag = (a >> 8) & 0xFF, ab = a & 0xFF;
        int br = (b >> 16) & 0xFF, bg = (b >> 8) & 0xFF, bb = b & 0xFF;
        int r = Math.round(ar + (br - ar) * t);
        int g = Math.round(ag + (bg - ag) * t);
        int bl = Math.round(ab + (bb - ab) * t);
        return rgb(r, g, bl);
    }
}