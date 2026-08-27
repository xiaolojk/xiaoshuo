import sys, struct, zlib
def loadbmp(src):
    with open(src,'rb') as f: data=f.read()
    off=struct.unpack_from('<I',data,10)[0]
    w,h=struct.unpack_from('<ii',data,18)
    bpp=struct.unpack_from('<H',data,28)[0]
    flip=h>0; h=abs(h)
    rs=((w*bpp+31)//32)*4
    raw=data[off:off+rs*h]
    rows=[]
    for y in range(h):
        row=raw[y*rs:(y+1)*rs]; px=[]
        for x in range(w):
            px.append((row[x*(bpp//8)],row[x*(bpp//8)+1],row[x*(bpp//8)+2]))
        rows.append(px)
    if flip: rows.reverse()
    return w,h,rows
def crop(src,dst,x0,y0,x1,y1,scale=4):
    w,h,rows=loadbmp(src)
    out=[]; 
    for y in range(y0,y1):
        line=[]
        for x in range(x0,x1): line.append(rows[y][x])
        out.append(line)
    W=(x1-x0)*scale; H=(y1-y0)*scale
    big=[[out[y//scale][x//scale] for x in range(W)] for y in range(H)]
    def chunk(t,d):
        c=t+d; return struct.pack('>I',len(d))+c+struct.pack('>I',zlib.crc32(c)&0xffffffff)
    ihdr=struct.pack('>IIBBBBB',W,H,8,2,0,0,0)
    rd=b''
    for row in big: rd+=b'\x00'+b''.join(bytes(p) for p in row)
    png=b'\x89PNG\r\n\x1a\n'+chunk(b'IHDR',ihdr)+chunk(b'IDAT',zlib.compress(rd))+chunk(b'IEND',b'')
    open(dst,'wb').write(png); print('ok',dst,W,H)
if __name__=='__main__':
    crop(sys.argv[1],sys.argv[2],int(sys.argv[3]),int(sys.argv[4]),int(sys.argv[5]),int(sys.argv[6]),int(sys.argv[7]) if len(sys.argv)>7 else 4)
