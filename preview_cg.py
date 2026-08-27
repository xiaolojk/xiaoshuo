#!/usr/bin/env python3
from PIL import Image
import sys
sys.path.insert(0,"/workspace")
import gen_cg

scenes={1:"bg_storm",2:"bg_dawn",3:"bg_pier",4:"bg_sun"}
for i,p in scenes.items():
    pal,idx=gen_cg.build("/workspace/assets/%s.jpg"%p)
    im=Image.new("RGB",(gen_cg.W,gen_cg.H))
    im.putdata([tuple(pal[c*3:c*3+3]) for c in idx])
    im.save("/workspace/assets/preview_%d.png"%i)
sheet=Image.new("RGB",(gen_cg.W*2,gen_cg.H*2),0)
for i in [1,2]:
    sheet.paste(Image.open("/workspace/assets/preview_%d.png"%i),((i-1)*gen_cg.W,0))
for i in [3,4]:
    sheet.paste(Image.open("/workspace/assets/preview_%d.png"%i),((i-3)*gen_cg.W,gen_cg.H))
sheet.save("/workspace/assets/cg_preview_sheet.png")
print("sheet",sheet.size)