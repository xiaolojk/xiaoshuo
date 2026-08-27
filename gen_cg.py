#!/usr/bin/env python3
"""Convert Seedream-generated CG backdrops into game palette-indexed C arrays.
Output: cg_scenes.h  (CG_PAL[][][3] + CG_IMG[][512*288])"""
from PIL import Image
import os

W, H = 512, 288
NCOL = 96  # per-scene palette size after quantization
OUT = "/workspace/cg_scenes.h"

def build(path):
    im = Image.open(path).convert("RGB")
    # cover-crop to 16:9 then downscale (Lanczos) -> crisp, then quantize
    tw, th = im.size
    target = 16/9.0
    cur = tw/float(th)
    if cur > target:
        nw = int(th*target); x0 = (tw-nw)//2; im = im.crop((x0,0,x0+nw,th))
    else:
        nh = int(tw/target); y0 = (th-nh)//2; im = im.crop((0,y0,tw,y0+nh))
    im = im.resize((W,H), Image.LANCZOS)
    # posterize a bit for retro feel
    q = im.quantize(colors=NCOL, method=Image.MEDIANCUT, dither=Image.FLOYDSTEINBERG)
    pal = q.getpalette()[:NCOL*3]
    idx = list(q.getdata())  # W*H
    return pal, idx

def fmt_pal(pal):
    rows=[]
    for i in range(0,len(pal),3):
        rows.append("{%d,%d,%d}"%(pal[i],pal[i+1],pal[i+2]))
    return ",".join(rows)

def fmt_idx(idx):
    rows=[]
    for i in range(0,len(idx),48):
        rows.append(",".join(str(idx[i+j]) for j in range(min(48,len(idx)-i))))
    return ",\n".join(rows)

def main():
    scenes={1:"bg_storm",2:"bg_dawn",3:"bg_pier",4:"bg_sun"}
    out=[]
    out.append("/* Auto-generated CG backdrops. Do not edit. */")
    out.append("#ifndef CG_SCENES_H")
    out.append("#define CG_SCENES_H")
    out.append("#define CG_W 512")
    out.append("#define CG_H 288")
    out.append("#define CG_N %d"%len(scenes))
    out.append("#define CG_NCOL %d"%NCOL)
    out.append("static const int CG_PALCNT[CG_N]=%s;"%("{"+",".join(str(NCOL) for _ in scenes)+"}"))
    out.append("static const unsigned char CG_PAL[CG_N][CG_NCOL][3]={")
    pals=[]
    for i,p in scenes.items():
        pals.append("{"+fmt_pal(build(os.path.join("/workspace/assets",p+".jpg"))[0])+"}")
    out.append(",".join(pals))
    out.append("};")
    out.append("static const unsigned char CG_IMG[CG_N][CG_W*CG_H]={")
    imgs=[]
    for i,p in scenes.items():
        idx=build(os.path.join("/workspace/assets",p+".jpg"))[1]
        imgs.append("{\n"+fmt_idx(idx)+"}")
    out.append(",".join(imgs))
    out.append("};")
    out.append("#endif")
    with open(OUT,"w") as f:
        f.write("\n".join(out))
    print("wrote", OUT, os.path.getsize(OUT), "bytes")

if __name__=="__main__":
    main()