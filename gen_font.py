#!/usr/bin/env python3
"""Generate 32x32 CJK bitmap font as C array from WQY Micro Hei"""
from PIL import Image, ImageFont
import sys

FONT_PATH = "/usr/share/fonts/truetype/wqy/wqy-microhei.ttc"
FONT_SIZE = 32
OUT = "cjk_font_32x32.h"

# Collect all unique CJK characters needed in the game
CHARS = set()
strings = open("cn_strings.txt", "r", encoding="utf-8").read()
for ch in strings:
    if ord(ch) >= 0x80:      # any non-ASCII (CJK + full-width punctuation)
        CHARS.add(ch)
# Also include ASCII chars that are used in mixed text
CHARS = sorted(CHARS, key=lambda c: ord(c))

print(f"Total CJK chars to render: {len(CHARS)}")

font = ImageFont.truetype(FONT_PATH, FONT_SIZE)
W, H = 32, 32
# Build a mapping: char -> index
char_to_idx = {ch: i for i, ch in enumerate(CHARS)}

# Generate bitmap data
# For each char, we store 32*32 bits = 128 bytes (1 bit per pixel)
# But we need to handle the font rendering to get the actual bitmap

# Actually, let's use a simpler approach: store as 32x32 byte array (0/1)
# 32*32 = 1024 bytes per char

# We'll store as uint8_t data[32][NCHARS] or as a flat array
# Better: flat byte array of size NCHARS * 32 * 32, row-major, 1=on

# But the existing font system uses 8x8 with byte-rows.
# For 32x32 with byte-rows, each char = 32 bytes (like the 8x8 system but scaled)
# Each byte row = 32 bits = 4 bytes... hmm.

# Actually, let's keep it simple: each char is a 32*32 = 1024 byte array (0/1 per pixel)
# Grouped as uint8_t char_data[32][32] per char

# Render each char
import struct

def render_char(ch):
    """Render a CJK character to 32x32 bitmap, return 32x32 byte array (0/1)"""
    img = Image.new("L", (W, H), 0)
    # Use font.getmask to get bitmap
    from PIL import ImageDraw
    draw = ImageDraw.Draw(img)
    draw.text((0, 0), ch, font=font, fill=255)
    # Convert to 0/1 array
    bits = []
    for y in range(H):
        row = []
        for x in range(W):
            px = img.getpixel((x, y))
            row.append(1 if px > 128 else 0)
        bits.append(row)
    return bits

# Output as C header
with open(OUT, "w", encoding="utf-8") as f:
    f.write(f"/* Auto-generated CJK 32x32 bitmap font - {len(CHARS)} chars */\n")
    f.write(f"#ifndef CJK_FONT_32X32_H\n#define CJK_FONT_32X32_H\n\n")
    f.write(f"#define CJK_FONT_N {len(CHARS)}\n")
    f.write(f"#define CJK_FONT_W 32\n")
    f.write(f"#define CJK_FONT_H 32\n\n")
    
    # Write each char's bitmap as a static const array
    f.write(f"static const unsigned char CJK_BITS[{len(CHARS)}][32][32] = {{\n")
    for i, ch in enumerate(CHARS):
        if i % 10 == 0:
            print(f"  Rendering {i}/{len(CHARS)}: '{ch}' (U+{ord(ch):04X})")
        bits = render_char(ch)
        f.write(f"  /* U+{ord(ch):04X} '{ch}' */ {{\n")
        for y in range(32):
            row_str = ",".join(str(bits[y][x]) for x in range(32))
            f.write(f"    {{{row_str}}},\n")
        f.write(f"  }},\n")
    f.write("};\n\n")
    
    # Write the character lookup table
    f.write(f"static const unsigned int CJK_CODEPOINTS[{len(CHARS)}] = {{\n")
    for ch in CHARS:
        f.write(f"  {ord(ch)},\n")
    f.write("};\n\n")
    
    # Write a helper macro to find char index
    f.write(f"/* Find char index by codepoint, returns -1 if not found */\n")
    f.write(f"static inline int cjk_find(unsigned int cp) {{\n")
    # Binary search for efficiency
    f.write(f"  int lo=0, hi=CJK_FONT_N-1;\n")
    f.write(f"  while(lo<=hi) {{\n")
    f.write(f"    int mid=(lo+hi)/2;\n")
    f.write(f"    if(CJK_CODEPOINTS[mid]==cp) return mid;\n")
    f.write(f"    if(CJK_CODEPOINTS[mid]<cp) lo=mid+1; else hi=mid-1;\n")
    f.write(f"  }}\n")
    f.write(f"  return -1;\n")
    f.write("}\n\n")
    f.write("#endif /* CJK_FONT_32X32_H */\n")

print(f"Done! Generated {len(CHARS)} CJK characters in {OUT}")
print(f"Chars: {''.join(CHARS)}")