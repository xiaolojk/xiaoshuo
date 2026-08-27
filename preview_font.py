#!/usr/bin/env python3
"""Preview the generated 32x32 CJK font as a sprite sheet PNG"""
from PIL import Image
import re

# Parse the generated header roughly - actually just re-render via PIL for preview
from PIL import ImageFont
text = open("cn_strings.txt", encoding="utf-8").read()
chars = sorted({c for c in text if ord(c) >= 0x4E00})
# include a few common
cols = 16
rows = (len(chars) + cols - 1) // cols
cell = 32
img = Image.new("RGB", (cols*cell, rows*cell), (10,15,35))
font = ImageFont.truetype("/usr/share/fonts/truetype/wqy/wqy-microhei.ttc", 32)
from PIL import ImageDraw
dr = ImageDraw.Draw(img)
for i, c in enumerate(chars):
    r, col = divmod(i, cols)
    # render in a bright color on dark bg to visualize bitmap quality
    tmp = Image.new("L", (cell, cell), 0)
    ImageDraw.Draw(tmp).text((0, 0), c, font=font, fill=255)
    px = tmp.load()
    for y in range(cell):
        for x in range(cell):
            if px[x, y] > 100:
                img.putpixel((col*cell+x, r*cell+y), (220, 220, 235))
img = img.resize((cols*cell*2, rows*cell*2), Image.NEAREST)
img.save("font_preview.png")
print(f"preview saved: {len(chars)} chars, {cols}x{rows} grid -> font_preview.png")