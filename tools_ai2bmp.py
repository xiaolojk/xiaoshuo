# -*- coding: utf-8 -*-
"""AI 图 -> 512x288 像素风 BMP（游戏背景）
流程: 找水线 -> 裁 16:9 窗口(水线对齐 y=150/288) -> BOX 直接采样
      到 512x288 -> 量化 48 色 -> 存 24bit BMP
"""
import sys, os
from PIL import Image

TARGET_W, TARGET_H = 512, 288
SMALL_W, SMALL_H = 256, 144
WATER_Y = 150          # 逻辑分辨率里程序化水面的位置
SRC = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'deliver', 'ai_src')
OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'bg')

def w2_w(w, h2, h):
    """窗口高度 h2 时的等比 16:9 窗口宽度 (与源图同宽高比缩放)"""
    return w * (h2 / float(h))

def find_waterline(im):
    """自动找水线: 在 30%-75% 高度带里找亮度下降最陡的行"""
    g = im.convert('L')
    w, h = g.size
    px = g.load()
    rows = []
    for y in range(h):
        s = 0
        for x in range(0, w, 8):
            s += px[x, y]
        rows.append(s / (w // 8))
    lo, hi = int(h*0.28), int(h*0.78)
    best_y, best_d = lo, 0
    for y in range(lo+2, hi-2):
        d = rows[y-2] - rows[y+2]          # 亮度骤降 = 天空->水面
        if d > best_d:
            best_d, best_y = d, y
    return best_y, rows

def convert(src, dst, waterline=None, align=True, debug=False):
    im = Image.open(src).convert('RGB')
    w, h = im.size
    if waterline is None:
        waterline, _ = find_waterline(im)
    top = 0
    if align and waterline is not None and 0 < waterline < h:
        # 目标: 水线落在输出的 WATER_Y/288 处 => 窗口内占比 frac
        frac = WATER_Y / float(TARGET_H)
        if waterline < frac * h:              # 水线偏高 -> 裁底部
            h2 = int(waterline / frac)
        else:                                 # 水线偏低 -> 裁顶部
            h2 = int((h - waterline) / (1.0 - frac))
        h2 = max(int(h * 0.70), min(h2, h))   # 最多裁 30%
        top = waterline - int(round(frac * h2))
        top = max(0, min(top, h - h2))
        w2 = int(round(w2_w(w, h2, h)))
        if h2 < h:
            x0 = max(0, (w - w2) // 2)
            im = im.crop((x0, top, x0 + w2, top + h2))
    # BOX 直接采样到 512x288（比 256x144 再 2x NEAREST 细腻一倍, 像素感更低）
    big = im.resize((TARGET_W, TARGET_H), Image.BOX)
    # 量化 48 色 (去渐变, 保留色块感)
    big = big.quantize(colors=48, method=Image.MEDIANCUT, dither=Image.NONE).convert('RGB')
    big.save(dst, 'BMP')
    print('%s -> %s  waterline_src=%d crop_top=%d' % (os.path.basename(src), os.path.basename(dst), waterline, top))
    return top

if __name__ == '__main__':
    if not os.path.isdir(OUT):
        os.makedirs(OUT)
    debug = '--debug' in sys.argv
    manual = {}
    no_align = {'ai_rod', 'ai_shop', 'ai_pierbg'}   # 特写/内景, 不做水线对齐
    for f in sorted(os.listdir(SRC)):
        if not f.lower().endswith(('.jpg', '.png')):
            continue
        name = os.path.splitext(f)[0]
        convert(os.path.join(SRC, f), os.path.join(OUT, name.replace('ai_', '') + '.bmp'),
                manual.get(name), align=(name not in no_align), debug=debug)
