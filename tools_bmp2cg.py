# -*- coding: utf-8 -*-
"""bg/*.bmp -> cg_scenes.h (与旧头文件同 API: CG_PAL/CG_IMG/CG_W/CG_H/CG_N/CG_NCOL)
场景顺序与 game.c 的 CGS_* 枚举一致:
0-3 码头(day/dawn/dusk/night)  4-7 木桥  8-11 芦苇滩  12-15 湖心岛  16-19 深水
20=storm 21=title 22=rod 23=pier(捏脸) 24=shop
"""
import os
from PIL import Image

HERE = os.path.dirname(os.path.abspath(__file__))
BG = os.path.join(HERE, 'bg')
SCENES = ['day', 'dawn', 'dusk', 'night',
          'bridge_day', 'bridge_dawn', 'bridge_dusk', 'bridge_night',
          'reed_day', 'reed_dawn', 'reed_dusk', 'reed_night',
          'isle_day', 'isle_dawn', 'isle_dusk', 'isle_night',
          'deep_day', 'deep_dawn', 'deep_dusk', 'deep_night',
          'storm', 'title', 'rod', 'pier', 'shop']
NCOL = 96

def load(name):
    im = Image.open(os.path.join(BG, name + '.bmp')).convert('RGB')
    assert im.size == (512, 288), (name, im.size)
    return list(im.getdata())

def main():
    out = [ '/* Auto-generated CG backdrops (AI pixel art). Do not edit. */',
            '#ifndef CG_SCENES_H', '#define CG_SCENES_H',
            '#define CG_W 512', '#define CG_H 288',
            '#define CG_N %d' % len(SCENES),
            '#define CG_NCOL %d' % NCOL, '' ]
    cnts, pals, imgs = [], [], []
    for name in SCENES:
        px = load(name)
        pal = {}
        for c in px:
            if c not in pal:
                if len(pal) >= NCOL:
                    raise SystemExit('%s: >%d colors' % (name, NCOL))
                pal[c] = len(pal)
        cnts.append(len(pal))
        pals.append([list(c) for c in sorted(pal, key=lambda k: pal[k])])
        imgs.append([pal[c] for c in px])
        print('%s: %d colors' % (name, len(pal)))
    out.append('static const int CG_PALCNT[CG_N]={%s};' % ','.join(map(str, cnts)))
    out.append('static const unsigned char CG_PAL[CG_N][CG_NCOL][3]={')
    for p in pals:
        rows = ','.join('{%d,%d,%d}' % (c[0], c[1], c[2]) for c in p)
        rows += ',' + ','.join('{0,0,0}' for _ in range(NCOL - len(p)))
        out.append('{%s},' % rows)
    out.append('};')
    out.append('static const unsigned char CG_IMG[CG_N][CG_W*CG_H]={')
    for idx, data in enumerate(imgs):
        out.append('/* %s */ {' % SCENES[idx])
        vals = [str(v) for v in data]
        CH = 60
        for i in range(0, len(vals), CH):
            out.append(','.join(vals[i:i + CH]) + ',')
        out.append('},')
    out.append('};')
    out.append('#endif')
    with open(os.path.join(HERE, 'cg_scenes.h'), 'w') as f:
        f.write('\n'.join(out) + '\n')
    print('cg_scenes.h written, %d bytes' % os.path.getsize(os.path.join(HERE, 'cg_scenes.h')))

if __name__ == '__main__':
    main()
