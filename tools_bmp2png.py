import sys, struct, zlib

def bmp2png(src, dst, maxw=960):
    with open(src, 'rb') as f:
        data = f.read()
    off = struct.unpack_from('<I', data, 10)[0]
    w, h = struct.unpack_from('<ii', data, 18)
    bpp = struct.unpack_from('<H', data, 28)[0]
    flip = h > 0
    h = abs(h)
    rowsize = ((w * bpp + 31) // 32) * 4
    raw = data[off:off + rowsize * h]
    rows = []
    for y in range(h):
        row = raw[y * rowsize:(y + 1) * rowsize]
        px = []
        for x in range(w):
            b = row[x * (bpp // 8)]
            g = row[x * (bpp // 8) + 1]
            r = row[x * (bpp // 8) + 2]
            px.append((r, g, b))
        rows.append(px)
    if flip:
        rows.reverse()
    scale = maxw / float(w)
    nh = int(h * scale)
    out = []
    for y in range(nh):
        sy = int(y / scale)
        line = []
        for x in range(maxw):
            sx = int(x / scale)
            line.append(rows[sy][sx])
        out.append(line)
    def chunk(t, d):
        c = t + d
        return struct.pack('>I', len(d)) + c + struct.pack('>I', zlib.crc32(c) & 0xffffffff)
    ihdr = struct.pack('>IIBBBBB', maxw, nh, 8, 2, 0, 0, 0)
    rawdata = b''
    for row in out:
        rawdata += b'\x00' + b''.join(bytes(p) for p in row)
    png = b'\x89PNG\r\n\x1a\n' + chunk(b'IHDR', ihdr) + chunk(b'IDAT', zlib.compress(rawdata)) + chunk(b'IEND', b'')
    with open(dst, 'wb') as f:
        f.write(png)
    print('ok', src, '->', dst, w, h, 'bpp', bpp)

if __name__ == '__main__':
    bmp2png(sys.argv[1], sys.argv[2])
