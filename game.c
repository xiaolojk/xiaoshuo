/* ============================================================
   PIXEL LAKE HEART  -  a low-end fishing game for Win7/Win10 x64
   Engine: SDL2, pure software pixel-art
   v3: Minecraft-style hard-pixel 64x64 fish sprites (Seedream),
   16x16 CJK font, ambient fish schools (Dave-style lake),
   fish-approach bite, no night fishing, integer-scale letterbox
   ============================================================ */
#define SDL_MAIN_HANDLED  /* we provide our own entry point on Windows */

/* ============================================================
   LAN multiplayer: cross-platform unreliable UDP (no external lib)
   -- POSIX sockets (Linux/macOS) or Winsock (Windows)
   -- non-blocking sockets polled each frame inside the SDL loop
   ============================================================ */
#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <ctype.h>
#include <dirent.h>
#include <SDL.h>
#include "cjk_font_16x16.h"
#include "cg_scenes.h"
#include "fish_sprites.h"

#define IN_W 512     /* logic grid (all layout code) */
#define IN_H 288
#define PH_W 1536    /* true physical canvas - INTEGER 3x pixel scale */
#define PH_H 864
#define FPS 60
#define FRAME_MS (1000/FPS)
/* logic->phys exact grid map: 1536/512 = 864/288 = 3.
   Every logic pixel is a perfectly square 3x3 physical block - crisp uniform
   Minecraft pixels (the old 3.75x map produced uneven 3px/4px cells that made
   the whole scene look fuzzy and "slapped-on"). */
#define M2P(v) (((v)*3))

/* ---------- work palette ---------- */
typedef struct { int r,g,b; } RGB3;
static const int NPAL=40;
static const RGB3 PALRGB[40]={
  {120,190,235},{60,140,190},{40,110,170},{230,210,160},{150,110,60},
  {110,80,45},{90,170,70},{90,60,40},{60,130,60},{10,10,10},
  {220,70,50},{255,255,255},{245,245,245},{15,15,20},{120,120,120},
  {210,55,45},{70,180,90},{245,205,60},{60,110,200},{140,90,50},
  {235,140,40},{235,150,180},{25,30,45},{250,190,50},{20,60,110},
  {30,40,60},{20,30,45},{235,190,150},{70,48,30},{205,60,45},
  {60,120,190},{80,60,45},{90,70,60},{150,110,70},{150,80,200},
  {60,200,200},{185,195,205},{10,15,35},{235,140,40},{70,200,110}
};
static Uint32 PACKED[40];
enum{
  C_SKY=0,C_WATER=1,C_WATER2=2,C_SAND=3,C_DOCK=4,C_DOCK2=5,
  C_GRASS=6,C_TRUNK=7,C_BUSH=8,C_LINE=9,C_FLOATA=10,C_FLOATB=11,
  C_WHITE=12,C_BLACK=13,C_GREY=14,C_RED=15,C_GREEN=16,C_YELLOW=17,
  C_BLUE=18,C_BROWN=19,C_ORANGE=20,C_PINK=21,C_DARK=22,C_GOLD=23,
  C_DEEPWATER=24,C_SHADOW=25,C_DIALOG=26,C_SKIN=27,C_HAIR=28,C_CAP=29,
  C_SHIRT=30,C_PANTS=31,C_BOOT=32,C_ROD=33,C_PURPLE=34,C_CYAN=35,
  C_SILVER=36,C_NIGHT=37,C_WARN=38,C_OK=39
};
static int WIND_W=1536,WIND_H=864;
/* scene-wide multiplicative lift for voxel texels (night keeps MC pixels alive) */
static int g_nightlift=0;

static SDL_Surface*screen; static Uint32*scr; static int scrpitch;

static inline void setpix(int x,int y,Uint32 c){
  if(x>=0&&x<IN_W&&y>=0&&y<IN_H){
    int px0=M2P(x),px1=M2P(x+1),py0=M2P(y),py1=M2P(y+1);
    for(int py=py0;py<py1;py++){ Uint32*row=scr+py*scrpitch;
      for(int xx=px0;xx<px1;xx++)row[xx]=c; }
  }
}
static void fill(int x,int y,int w,int h,Uint32 c){
  int x0=x<0?0:x,x1=x+w>IN_W?IN_W:x+w,y0=y<0?0:y,y1=y+h>IN_H?IN_H:y+h;
  if(x1<=x0||y1<=y0)return;
  int px0=M2P(x0),px1=M2P(x1),py0=M2P(y0),py1=M2P(y1);
  for(int py=py0;py<py1;py++){ Uint32*row=scr+py*scrpitch;
    for(int xx=px0;xx<px1;xx++)row[xx]=c; }
}
static void fill_grad(int x,int y,int w,int h,Uint32 c0,Uint32 c1){
  int r0=c0>>16&0xFF,g0=c0>>8&0xFF,b0=c0&0xFF;
  int r1=c1>>16&0xFF,g1=c1>>8&0xFF,b1=c1&0xFF;
  int x0=x<0?0:x,x1=x+w>IN_W?IN_W:x+w,y0=y<0?0:y,y1=y+h>IN_H?IN_H:y+h;
  if(x1<=x0||y1<=y0)return;
  int px0=M2P(x0),px1=M2P(x1),py0=M2P(y0),py1=M2P(y1);
  for(int py=py0;py<py1;py++){
    float t=(py1-py0<=1)?1:(float)(py-py0)/(py1-py0-1);
    Uint32 col=0xFF000000u|((Uint32)(r0+(r1-r0)*t)<<16)|((Uint32)(g0+(g1-g0)*t)<<8)|((Uint32)(b0+(b1-b0)*t));
    Uint32*row=scr+py*scrpitch;
    for(int xx=px0;xx<px1;xx++)row[xx]=col;
  }
}
static Uint32 packrgb(int r,int g,int b){ return 0xFF000000u|((Uint32)r<<16)|((Uint32)g<<8)|(Uint32)b; }
static void blit_rgba(int dx,int dy,int scale,const Uint32*px,int w,int h){
  for(int sy=0;sy<h;sy++)for(int sx=0;sx<w;sx++){
    Uint32 c=px[sy*w+sx]; if(!c) continue;
    for(int a=0;a<scale;a++)for(int b=0;b<scale;b++) setpix(dx+sx*scale+b,dy+sy*scale+a,c);
  }
}
static void dim_overlay(int x,int y,int w,int h){
  for(int yy=y;yy<y+h;yy++)for(int xx=x;xx<x+w;xx++)
    if(((xx+yy)&1)==0) setpix(xx,yy,PACKED[C_DARK]);
}

/* ---------- 8x8 upper font ---------- */
#define GY(n,a0,a1,a2,a3,a4,a5,a6) static const Uint8 n[7]={a0,a1,a2,a3,a4,a5,a6}
GY(F_A,0x3C,0x66,0x66,0x7E,0x66,0x66,0x66); GY(F_B,0x7C,0x66,0x7C,0x66,0x66,0x66,0x7C);
GY(F_C,0x3C,0x66,0x60,0x60,0x60,0x66,0x3C); GY(F_D,0x78,0x6C,0x66,0x66,0x66,0x6C,0x78);
GY(F_E,0x7E,0x60,0x60,0x7C,0x60,0x60,0x7E); GY(F_F,0x7E,0x60,0x60,0x7C,0x60,0x60,0x60);
GY(F_G,0x3C,0x66,0x60,0x6E,0x66,0x66,0x3E); GY(F_H,0x66,0x66,0x66,0x7E,0x66,0x66,0x66);
GY(F_I,0x3C,0x18,0x18,0x18,0x18,0x18,0x3C); GY(F_J,0x06,0x06,0x06,0x06,0x66,0x66,0x3C);
GY(F_K,0x66,0x6C,0x78,0x6C,0x66,0x66,0x66); GY(F_L,0x60,0x60,0x60,0x60,0x60,0x60,0x7E);
GY(F_M,0x63,0x77,0x7F,0x6B,0x63,0x63,0x63); GY(F_N,0x66,0x76,0x7E,0x6E,0x66,0x66,0x66);
GY(F_O,0x3C,0x66,0x66,0x66,0x66,0x66,0x3C); GY(F_P,0x7C,0x66,0x66,0x7C,0x60,0x60,0x60);
GY(F_Q,0x3C,0x66,0x66,0x6E,0x3E,0x06,0x00); GY(F_R,0x7C,0x66,0x66,0x7C,0x6C,0x66,0x66);
GY(F_S,0x3C,0x66,0x38,0x0C,0x06,0x66,0x3C); GY(F_T,0x7E,0x18,0x18,0x18,0x18,0x18,0x18);
GY(F_U,0x66,0x66,0x66,0x66,0x66,0x66,0x3C); GY(F_V,0x66,0x66,0x66,0x66,0x3C,0x3C,0x18);
GY(F_W,0x63,0x63,0x6B,0x7F,0x77,0x63,0x63); GY(F_X,0x66,0x66,0x3C,0x18,0x3C,0x66,0x66);
GY(F_Y,0x66,0x66,0x3C,0x18,0x18,0x18,0x18); GY(F_Z,0x7E,0x06,0x0C,0x30,0x60,0x7E,0x00);
GY(F_0,0x3C,0x66,0x6E,0x76,0x66,0x66,0x3C); GY(F_1,0x18,0x38,0x18,0x18,0x18,0x18,0x7E);
GY(F_2,0x3C,0x66,0x0C,0x18,0x30,0x60,0x7E); GY(F_3,0x3C,0x66,0x0C,0x06,0x66,0x66,0x3C);
GY(F_4,0x0C,0x1C,0x3C,0x6C,0x7E,0x0C,0x0C); GY(F_5,0x7E,0x60,0x7C,0x06,0x66,0x66,0x3C);
GY(F_6,0x3C,0x66,0x60,0x7C,0x66,0x66,0x3C); GY(F_7,0x7E,0x06,0x0C,0x18,0x30,0x60,0x60);
GY(F_8,0x3C,0x66,0x3C,0x66,0x66,0x66,0x3C); GY(F_9,0x3C,0x66,0x3E,0x06,0x66,0x66,0x3C);
static const Uint8 F_SP[7]={0,0,0,0,0,0,0}; static const Uint8 F_DOT[7]={0,0,0,0,0,0x18,0x18};
static const Uint8 F_COMMA[7]={0,0,0,0,0,0x18,0x08}; static const Uint8 F_BANG[7]={0x18,0x18,0x18,0x18,0,0x18,0};
static const Uint8 F_QUES[7]={0x3C,0x66,0x0C,0x18,0,0x18,0}; static const Uint8 F_COLON[7]={0,0x18,0,0,0x18,0,0};
static const Uint8 F_DASH[7]={0,0,0,0x7E,0,0,0}; static const Uint8 F_SLASH[7]={0x02,0x06,0x0C,0x18,0x30,0x60,0x00};
static const Uint8 F_PCT[7]={0x63,0x66,0x0C,0x18,0x33,0x63,0}; static const Uint8 F_DOLLAR[7]={0x18,0x7E,0x60,0x7C,0x06,0x7E,0x18};
static const Uint8 F_QUOTE[7]={0x36,0x36,0,0,0,0,0}; static const Uint8 F_PLUS[7]={0x18,0x18,0x7E,0x18,0x18,0,0};
static const Uint8 F_UNDER[7]={0,0,0,0,0,0,0x7E};
static const Uint8* glyph_of(char c){
  static const Uint8* const L[26]={F_A,F_B,F_C,F_D,F_E,F_F,F_G,F_H,F_I,F_J,F_K,F_L,F_M,F_N,F_O,F_P,F_Q,F_R,F_S,F_T,F_U,F_V,F_W,F_X,F_Y,F_Z};
  static const Uint8* const D[10]={F_0,F_1,F_2,F_3,F_4,F_5,F_6,F_7,F_8,F_9};
  if(c>='A'&&c<='Z')return L[c-'A'];
  if(c>='0'&&c<='9')return D[c-'0'];
  switch(c){case' ':return F_SP;case'.':return F_DOT;case',':return F_COMMA;case'!':return F_BANG;
    case'?':return F_QUES;case':':return F_COLON;case'-':return F_DASH;case'/':return F_SLASH;
    case'%':return F_PCT;case'$':return F_DOLLAR;case'\'':return F_QUOTE;case'+':return F_PLUS;case'_':return F_UNDER;}
  return F_SP;
}
static int draw_char(int x,int y,char c,Uint32 col,int scale){
  const Uint8*g=glyph_of(c);
  for(int r=0;r<7;r++){Uint8 row=g[r];for(int b=0;b<8;b++)
    if((row>>(7-b))&1)for(int a=0;a<scale;a++)for(int e=0;e<scale;e++)setpix(x+b*scale+e,y+r*scale+a,col);}
  return 8*scale;
}
/* ---------- CJK 16x16 ---------- */
static int draw_cjk_char(int x,int y,unsigned cp,Uint32 col,int scale){
  int idx=cjk_find(cp);
  if(idx<0){for(int r=0;r<12;r++)for(int b=0;b<12;b++){
    int on=(r==0||r==11||b==0||b==11);
    if(on)for(int a=0;a<scale;a++)for(int e=0;e<scale;e++)setpix(x+b*scale+e,y+r*scale+a,col);
    }return 13*scale;}
  for(int r=0;r<16;r++){const unsigned char*row=CJK_BITS[idx][r];
    for(int b=0;b<16;b++)if(row[b])for(int a=0;a<scale;a++)for(int e=0;e<scale;e++)setpix(x+b*scale+e,y+r*scale+a,col);}
  return 17*scale;
}
static int is_utf8_cjk(unsigned char c){ return c>=0xE0; }
static unsigned utf8_cp(const unsigned char*s){
  if((s[0]>>5)==0x6)return((s[0]&0x1F)<<6)|(s[1]&0x3F);
  return((s[0]&0x0F)<<12)|((s[1]&0x3F)<<6)|(s[2]&0x3F);
}
static void draw_text(int x,int y,const char*s,Uint32 col,int scale){
  while(*s){ unsigned char c=(unsigned char)*s;
    if(is_utf8_cjk(c)){ unsigned cp=utf8_cp((const unsigned char*)s); x+=draw_cjk_char(x,y,cp,col,scale); s+=3; }
    else{ char ch=c; if(ch>='a'&&ch<='z')ch-=32; x+=draw_char(x,y,ch,col,scale); s++; } }
}
static int text_w(const char*s,int scale){
  int n=0; while(*s){ unsigned char c=(unsigned char)*s;
    if(is_utf8_cjk(c)){ n+=(cjk_find(utf8_cp((const unsigned char*)s))>=0?17:13)*scale; s+=3; }
    else{ char ch=c; if(ch>='a'&&ch<='z')ch-=32; n+=8*scale; s++; } }
  return n;
}
static void center_text(int cy,const char*s,Uint32 col,int scale){
  draw_text((IN_W-text_w(s,scale))/2,cy,s,col,scale);
}
static inline int clampij(int v,int lo,int hi){ return v<lo?lo:(v>hi?hi:v); }

/* language */
static int lang=0;
static const char* T(const char*en,const char*cn){ return lang?cn:en; }

/* ---------- RNG ---------- */
static unsigned long long rngstate;
static void seed_rng(unsigned s){ rngstate=(unsigned long long)s*2654435761ULL+0x9E3779B9ULL; }
static unsigned rnd(void){ rngstate=rngstate*6364136223846793005ULL+1; return (unsigned)(rngstate>>32); }
static int rndi(int n){ return n<=0?0:(int)(rnd()%n); }
static float rndf(void){ return (float)((double)(rnd()%1000000)/1000000.0); }

/* ---------- fish table + 64x64 art ---------- */
static Uint32 colr(RGB3 c){ return packrgb(c.r,c.g,c.b); }
static Uint32 mulc(Uint32 c,int p){ return packrgb((c>>16&0xFF)*p/255,(c>>8&0xFF)*p/255,(c&0xFF)*p/255); }
static Uint32 dimc(Uint32 c){ return packrgb((c>>16&0xFF)*4/5,(c>>8&0xFF)*4/5,(c&0xFF)*4/5); }

typedef struct{
  const char*en; const char*cn; int value,exp,weight,region,time,diff;
}FISH;
static FISH FISHES[10]={
 /*en            cn       val exp wt reg t dif  (night fishing banned: all t=0/1)
   shore pool = CARP+PERCH+SILVER CARP+BLUEGILL(day) so the first casts already
   show 4 species -- before this shore had ONLY carp+perch (100% "all perch"). */
 {"CARP","鲤鱼",    3,1,26,0,0,0},
 {"PERCH","鲈鱼",   5,1,26,0,0,0},
 {"SILVER CARP","银鲤",9,2,14,0,0,1},
 {"CATFISH","鲶鱼",12,2,24,1,0,1},
 {"LARGEMOUTH","大口鲈",14,2,22,1,1,2},
 {"BLUEGILL","蓝鳃鱼",22,3,12,0,1,2},
 {"SALMON","鲑鱼", 30,4,18,2,0,3},
 {"GOLD TROUT","金鳟",40,4,12,2,1,3},
 {"ELECTRIC EEL","电鳗",55,5,9,2,0,4},
 {"LAKE DRAGON","湖龙",150,8,5,2,0,4},
};
#define NFISH 10
static Uint32 fsp[64*64];
/* packed sprite palettes + facing (1 = head on LEFT of the 64x64 grid) */
static Uint32 FSPR_PACK[10][16];
static const int FSPR_FACE_LEFT[10]={1,0,1,1,1,1,0,0,1,1};
/* mutable 64x64 fish pixel grids (initialized from fish_sprites.h, overridable by mods) */
static unsigned char fsp_pix[10][4096];
static void prep_fishspr(void){
  for(int s=0;s<10;s++)for(int i=0;i<16;i++)
    FSPR_PACK[s][i]=packrgb(FISHSPR_PAL[s][i][0],FISHSPR_PAL[s][i][1],FISHSPR_PAL[s][i][2]);
  for(int s=0;s<10;s++)for(int k=0;k<4096;k++) fsp_pix[s][k]=FISHSPR_PIX[s][k];
}

/* Deterministic per-fish helper: shade body column top->bottom */
static Uint32 mix3(Uint32 a,Uint32 b,float t){
  int r0=a>>16&0xFF,g0=a>>8&0xFF,b0=a&0xFF,r1=b>>16&0xFF,g1=b>>8&0xFF,b1=b&0xFF;
  return packrgb((int)(r0+(r1-r0)*t),(int)(g0+(g1-g0)*t),(int)(b0+(b1-b0)*t));
}
/* color modes: 0 normal, 1 dimmed (uncaught), 2 underwater tint */
static Uint32 fish_mode_col(Uint32 c,int mode){
  if(mode==1) return dimc(c);
  if(mode==2) return mulc(mix3(c,packrgb(60,120,170),0.25f),205); /* visible through water */
  return c;
}
/* ============ voxel-pixel tools: Minecraft-style per-pixel texture ============ */
static Uint32 shade_c(Uint32 c,int dv){ /* brightness shift, clamped */
  int r=(int)(c>>16&0xFF)+dv,g=(int)(c>>8&0xFF)+dv,b=(int)(c&0xFF)+dv;
  if(r<0)r=0;if(r>255)r=255;if(g<0)g=0;if(g>255)g=255;if(b<0)b=0;if(b>255)b=255;
  return packrgb(r,g,b);
}
static unsigned hash2(int x,int y){ /* stable per-pixel hash -> deterministic texture */
  unsigned h=(unsigned)(x*374761393u+y*668265263u);
  h=(h^(h>>13))*1274126177u; return h^(h>>16);
}
static int mc_texel(int h,int spread){ /* MC 16x16 style: 5 WELL-SEPARATED tones */
  int r=(int)(h%100);
  if(r<12) return -spread;           /* 12% deep dark   */
  if(r<30) return -spread*2/3;       /* 18% dark        */
  if(r<62) return -spread/5;         /* 32% near-base   */
  if(r<86) return spread*2/3;        /* 24% light       */
  return spread;                     /* 14% bright      */
}
/* MC texel as a PERCENT tone: biassed toward the base colour, rare strong
   speckles. Used MULTIPLICATIVELY so the texture stays visible on dark blocks
   (boots, trunks, deep water) where additive brightness is invisible. */
static int mc_texel_pct(int h){
  int r=(int)(h%100);
  if(r<10) return -40;   /* deep dark texel   */
  if(r<26) return -18;   /* dark texel        */
  if(r<52) return -5;    /* near-base low     */
  if(r<78) return  5;    /* near-base high    */
  if(r<92) return 18;    /* light texel       */
  return 40;             /* bright texel      */
}
/* apply a texel percent to a colour multiplicatively (visible on dark + light) */
static Uint32 texel_c(Uint32 c,int pct){
  int f=100+pct+g_nightlift; if(f<52)f=52; if(f>162)f=162;
  return mulc(c,f);
}
/* ---- Minecraft texel texture --------------------------------------------------
   Every texel of a face is INDIVIDUALLY visible and differs from its neighbours,
   but it is sampled on a coarse 2x2 lattice so nearby texels CLUSTER into blobs,
   exactly like MC grass/dirt/plank 16x16 textures (not flat, not static noise). */
static void vox_tex(int x,int y,int w,int h,Uint32 c,int spread,int seed){
  for(int yy=y;yy<y+h;yy++)for(int xx=x;xx<x+w;xx++){
    unsigned hn=hash2((xx>>1)*13+seed*7,(yy>>1)*17+seed*3);
    int d=mc_texel_pct((int)hn);
    unsigned hk=hash2(xx*11+seed*2,yy*7+seed*13);
    if((hk&15)==0) d=(hk&64)?-55:55;              /* rare strong light/dark */
    setpix(xx,yy,texel_c(c,d));
  }
}
/* ---- one Minecraft cube -------------------------------------------------------
   Every face is a grid of INDIVIDUALLY visible texels: each texel CLUSTERS on a
   2x2 lattice AND is forced to differ from the pixel to its left, so the face
   reads as a coherent MC texture where NO two adjacent pixels share a colour.
   Each cube has THREE shading zones - sunlit TOP band / textured BODY / shaded
   BOTTOM band - plus a left AO edge, a right sunlit edge and a full dark
   under-seam, so stacked cubes read as SEPARATE 3D blocks with real depth.
   Fixed light: upper-right. */
static void vox_cube(int x,int y,int w,int h,Uint32 c,int light,int spread,int seed){
  if(w<=0||h<=0)return;
  int th=h/3; if(th<1)th=1;
  int bh=h/3; if(bh<1)bh=1;
  if(th+bh>=h){ th=1; bh=1; }
  int mid=h-th-bh;
  int L=light*3/2;                    /* amplified band contrast: faces clearly read */
  for(int yy=y;yy<y+h;yy++){
    int band;
    if(yy-y<th)                 band= L;      /* sunlit TOP face   */
    else if(y+h-1-yy<bh)        band=-L;      /* shaded BOTTOM face*/
    else if(yy-y<th+mid/2)      band= L/2;    /* upper body half-lit*/
    else                        band= 0;      /* lower body base   */
    int prevd=1000;                          /* guarantee left-neighbour differs */
    for(int xx=x;xx<x+w;xx++){
      unsigned hc=hash2(((xx>>1)*13+seed*7),((yy>>1)*17+seed*3));
      int d=mc_texel_pct((int)hc);            /* clustered 2x2 blob, percent */
      /* per-texel wake: every pixel toggles between two well-separated tones so
         the final 8-bit colours really differ (tiny ±1 steps collapse once the
         colour is quantized; a ≥6% swing survives on every channel) */
      int wake=((xx^yy)&1)?6:-6;
      if((int)hash2(xx*11+seed*3,yy*7+seed*5)%5==0) wake=(hash2(xx*3,yy*13)&1)?-16:16;
      d+=wake;
      if(d>55)d=55; if(d<-55)d=-55;
      unsigned hk=hash2(xx*17+seed*2,yy*13+seed*3);
      if((hk&31)==0) d=(hk&128)?-55:55;       /* rare strong speck */
      if(xx>x && d==prevd) d=(d>=0)?d+2:d-2;  /* never equal to the pixel at left */
      prevd=d;
      Uint32 base=texel_c(c,d);               /* multiplicative: dark blocks still textured */
      int e=0;
      if(xx==x)           e=-L/2;             /* left AO edge  */
      else if(xx==x+w-1)  e=L/3;              /* right sunlit   */
      setpix(xx,yy,shade_c(shade_c(base,band),e));
    }
  }
  /* full-width dark under-seam -> stacked cubes read as SEPARATE blocks */
  for(int xx=x;xx<x+w;xx++)
    setpix(xx,y+h-1,texel_c(mulc(c,34),mc_texel_pct((int)hash2(xx,seed*9+3))));
  /* bright top rim -> every cube catches the sun; keeps texel texture so the
     rim is bright BUT never a long flat run of identical pixels */
  for(int xx=x;xx<x+w;xx++) if((hash2(xx,seed*11+1)&3)!=0){
    Uint32 base=texel_c(c,mc_texel_pct((int)hash2(xx*5+seed,seed*17+3)));
    setpix(xx,y,shade_c(base,L+8));
  }
}
/* ---- grass-topped terrain strip (side view of MC grass blocks) ----------------
   Dirt is drawn as a stack of 6px dirt CUBES (each with lit top / textured body /
   shaded bottom + seam), capped by a jagged bright green sod band with sunlit
   blade tips - reads as stacked MC grass blocks with real depth. */
static void vox_ground(int x,int y,int w,int h,Uint32 dirt,Uint32 grass,int light,int seed){
  int bh=6;
  for(int yy=y;yy<y+h;yy+=bh){
    int hh=h-(yy-y); if(hh>bh)hh=bh;
    vox_cube(x,yy,w,hh,dirt,light,14,seed*31+yy);
  }
  /* green sod band: bright, textured, jagged blocky top + sunlit tips.
     Every sod texel gets a MULTIPLICATIVE MC tone + per-pixel dither and is
     forced to differ from its left neighbour -> fine visible pixels, never
     a long flat green run. */
  for(int xx=x;xx<x+w;xx++){
    int j=hash2(xx>>1,seed*13)%3;              /* jagged block tops 0..2 */
    int prevd=1000;
    for(int yy=y-j;yy<=y;yy++){
      int dd=(yy-(y-j))*3;
      int d=mc_texel_pct((int)hash2(xx*7+seed,yy*13+seed*5));
      d+=(int)(hash2(xx*3+seed,yy*5+seed)%3)-1;
      if(yy>y-j && d==prevd) d=(d>=0)?d+1:d-1;
      prevd=d;
      int lv=(dd)?(-dd):light;                  /* bright sod top fading to dirt */
      if(yy==y-j) lv=light+5;                   /* sunlit grass tips */
      setpix(xx,yy,shade_c(texel_c(grass,d),lv));
    }
    if(hash2(xx,seed)%3) setpix(xx,y-j-1,shade_c(grass,light+6)); /* raised blade */
  }
  /* dark seam where the sod meets the dirt */
  for(int xx=x;xx<x+w;xx++) if(hash2(xx,seed*9+1)%2==0) setpix(xx,y+1,mulc(dirt,72));
}
/* fully replace a rect with per-pixel dithered colour: every logic pixel gets
   its own tiny brightness offset, so no two pixels look identical (MC grass) */
static void noise_fill(int x,int y,int w,int h,Uint32 c,int amp,int seed){
  for(int yy=y;yy<y+h;yy++)for(int xx=x;xx<x+w;xx++){
    int v=(int)((hash2(xx*7+seed*131,yy*13+seed*7))%(2*amp+1))-amp;
    setpix(xx,yy,shade_c(c,v));
  }
}
/* sparse dither over already-drawn pixels: flips ~1/denom pixels by dv,
   keeps the base gradient underneath (MC water ripple / sun sparkle) */
static void dither_rect(int x,int y,int w,int h,int denom,int dv,int seed){
  for(int yy=y;yy<y+h;yy++)for(int xx=x;xx<x+w;xx++)
    if(hash2(xx*3+seed,yy*5+seed)%denom==0){
      Uint32 c=scr[(M2P(yy))*scrpitch+(M2P(xx))];
      setpix(xx,yy,shade_c(c,dv));
    }
}
/* Minecraft cube face shading on a flat cell:
   top strip (bright) + middle (base) + bottom (dark) - fake 3D volume */
static void vox_face(int x,int y,int w,int h,Uint32 c,int light){
  int th=(h*3+2)/5; if(th<1)th=1;
  int bh=h-th*2; if(bh<1)bh=1;
  noise_fill(x,y,w,th,shade_c(c,light),2,x*3);
  noise_fill(x,y+th,w,bh,c,2,x*3+1);
  noise_fill(x,y+th+bh,w,h-th-bh,shade_c(c,-light),2,x*3+2);
}
/* full Minecraft cube: alias of vox_cube (lit top / textured body / shaded
   bottom + seam + AO). amp = per-texel dither. */
static void vox_block(int x,int y,int w,int h,Uint32 c,int light,int amp,int seed){
  vox_cube(x,y,w,h,c,light,amp,seed);
}
/* wood plank cube: voxel board (lit top / body / shaded bottom + seams) plus
   horizontal grain streaks and nail dots (MC plank block look) */
static void vox_wood(int x,int y,int w,int h,Uint32 c,int light,int seed){
  vox_cube(x,y,w,h,c,light,18,seed);
  /* MC plank texture: horizontal grain - every 3rd row gets light + dark streaks.
     Each grain pixel gets its own tiny jitter so even a wood streak is a run of
     slightly-different pixels, never a flat line. */
  for(int yy=y+1;yy<y+h-1;yy++) if(hash2(seed*7,yy)%3==0)
    for(int xx=x+1;xx<x+w-1;xx++){
      int j=(int)(hash2(xx*3,yy*5)%3)-1;                 /* ±1 dither         */
      if(hash2(xx*3+seed,yy*5)%3==0) setpix(xx,yy,shade_c(shade_c(c,16),j*2));      /* light grain */
      else if(hash2(xx*7+seed,yy*11)%5==0) setpix(xx,yy,mulc(c,66-j*2));          /* dark grain  */
    }
  /* dark knot dots */
  for(int xx=x+3;xx<x+w-1;xx+=9) if(hash2(xx,seed)%3==0){
    setpix(xx,y+h/2-1,mulc(c,55)); setpix(xx+1,y+h/2-1,mulc(c,55));
    setpix(xx,y+h/2,  mulc(c,70)); setpix(xx+1,y+h/2,  mulc(c,70));
  }
  /* nail dots on the top edge */
  for(int xx=x+6;xx<x+w-2;xx+=15) if(hash2(xx,seed)%4==0){ setpix(xx,y+1,mulc(c,50)); setpix(xx+1,y+1,mulc(c,45)); }
}
/* Minecraft grass block: dithered dirt body + bright green sod + hanging
   grass blades + sunlit blade tips (MC grass block look) */
static void vox_grass(int x,int y,int w,int h,Uint32 dirt,Uint32 grass,int light,int seed){
  vox_ground(x,y,w,h,dirt,grass,light,seed);
}
/* per-pixel dither over an existing gradient (keeps the base colour under) */
static void grad_noise(int x,int y,int w,int h,int amp,int seed){
  for(int yy=y;yy<y+h;yy++)for(int xx=x;xx<x+w;xx++){
    int v=((int)(hash2(xx*7+seed,yy*13+seed*3))%(2*amp+1))-amp;
    Uint32 c=scr[(M2P(yy))*scrpitch+(M2P(xx))];
    setpix(xx,yy,shade_c(c,v));
  }
}
/* blit a 64x64 hard-pixel sprite from fish_sprites.h into fsp (optionally flipped) */
static void render_fish_mode(int sp,int mode,int flip){
  for(int y=0;y<64;y++)for(int x=0;x<64;x++){
    int sx=flip?63-x:x;
    unsigned char p=fsp_pix[sp][y*64+sx];
    /* FISHSPR_TRANSP(255)=mod系统透明标记; 15=原始精灵背景填充(洋红) */
    fsp[y*64+x]=(p==FISHSPR_TRANSP||p==15)?0:fish_mode_col(FSPR_PACK[sp][p],mode);
  }
}
static void render_fish(int sp,int dim){ render_fish_mode(sp,dim?1:0,0); }
/* fractional-scale nearest blit of fsp (transparent = 0) */
static void blit_fish_f(float dx,float dy,float scale){
  int step=(int)(64*scale);
  if(step<1)step=1;
  int ix=(int)dx,iy=(int)dy;
  for(int y=0;y<step;y++){
    int sy=(int)(y/scale);
    for(int x=0;x<step;x++){
      Uint32 c=fsp[sy*64+(int)(x/scale)];
      if(c) setpix(ix+x,iy+y,c);
    }
  }
}
#if 0
static void render_fish(int sp,int dim){
  for(int i=0;i<64*64;i++)fsp[i]=0;
  const FISH*F=&FISHES[sp];
  int cx=F->cx,rx=F->rx,ry=F->ry,cy=32;
  int snout=cx-rx+2, tail=cx+rx; if(tail>60)tail=60;
  Uint32 body=colr(F->body),belly=colr(F->belly),fin=colr(F->fin),acc=colr(F->acc);
  if(dim){ body=dimc(body);belly=dimc(belly);fin=dimc(fin);acc=dimc(acc); }
  Uint32 dark=packrgb(14,14,16),outline=mulc(body,60);
  Uint32 back=mix3(mulc(body,140),body,0.35f); /* darker back */
  Uint32 hi=mix3(body,packrgb(255,255,255),0.45f); /* back highlight */
  int eel=(F->pat==4);
  /* ---- body with vertical gradient + patterns ---- */
  for(int x=snout;x<=tail;x++){
    float hh = x<=cx ? 2+((x-snout)/(float)(cx-snout>0?cx-snout:1))*(ry-1)
                     : ry*(1-0.86f*((x-cx)/(float)(tail-cx>0?tail-cx:1)));
    int h=(int)hh; if(h<1)h=1;
    for(int y=cy-h;y<=cy+h;y++){
      float v=(float)(y-(cy-h))/(float)(2*h); /* 0 top .. 1 bottom */
      Uint32 c=mix3(back,body,v<0.35f?v/0.35f:1.0f);
      if(v>0.62f) c=mix3(body,belly,(v-0.62f)/0.38f);
      if(F->pat==1&&(x%5==0)&&(y%5==2)) c=mulc(c,150);
      else if(F->pat==2&&((x-snout)%8)<3) c=mulc(c,155);
      else if(F->pat==3){ int dx=x-14,dy=y-cy; if(dx*dx+dy*dy<12) c=acc; }
      else if(F->pat==4&&(x%11==0)&&(y%17==0)) c=acc;
      sp_put(x,y,c);
    }
    /* top+bottom outline */
    sp_put(x,cy-h-1,outline); sp_put(x,cy+h+1,outline);
    if(!eel&&(x-snout)%7==3) sp_put(x,cy-h,mulc(hi,190));
  }
  /* ---- overlapping scale arcs (skip eel) ---- */
  if(!eel){
    Uint32 scale=mix3(mulc(body,120),belly,0.25f);
    for(int gy=0;gy<6;gy++)for(int gx=0;gx<9;gx++){
      int x0=snout+3+gx*5+((gy&1)?3:0), y0=cy-8+gy*4;
      for(int k=-2;k<=2;k++){
        int x=x0+k, y=y0-(k*k)/2;
        int h=(int)(2+((x-snout)/(float)(cx-snout>0?cx-snout:1))*(ry-1));
        if(y>cy-h-1&&y<cy+h) sp_put(x,y,scale);
      }
    }
  }
  /* ---- gill cover arc ---- */
  int gx0=snout+(int)((cx-snout)*0.42f);
  for(int d=-1;d<=1;d++){
    int x=gx0+d*3;
    for(int y=cy-ry+3;y<=cy+ry-2;y++){
      int h=(int)(2+((x-snout)/(float)(cx-snout>0?cx-snout:1))*(ry-1));
      int dy=y-cy; if(dy<0)dy=-dy;
      if(dy<=h-1) sp_put(x+(dy>h/2?1:0),y,mulc(body,140));
    }
  }
  /* ---- lateral line ---- */
  for(int x=snout+4;x<=tail-4;x+=2){
    sp_put(x,cy-1,mulc(body,130)); sp_put(x+1,cy-1,mulc(body,130));
  }
  /* ---- tail: 3 shapes (fork/round/long-eel) ---- */
  int tfx=clampij(tail,0,57);
  if(eel){ /* ribbon tail */
    for(int k=1;k<=6;k++){ int w=(k<4)?2:1;
      sp_hline(tfx+k,tfx+k+w-1,cy-1,fin); sp_hline(tfx+k,tfx+k+w-1,cy,fin); }
  } else if(F->pat==2){ /* forked */
    for(int dy=0;dy<=7;dy++){ int w=5-dy/2; if(w<1)w=1;
      for(int k=1;k<=w;k++){ sp_put(clampij(tfx+k+dy/2,0,63),clampij(cy-dy,0,63),fin);
        sp_put(clampij(tfx+k+dy/2,0,63),clampij(cy+dy,0,63),fin); } }
    for(int k=1;k<=3;k++){sp_put(clampij(tfx+k,0,63),cy,fin);sp_put(clampij(tfx+k,0,63),clampij(cy-1,0,63),fin);}
  } else { /* round fan */
    for(int dy=-6;dy<=6;dy++){ int ad=dy<0?-dy:dy; int wid=6-ad/2; if(wid<1)wid=1;
      for(int k=1;k<=wid;k++) sp_put(clampij(tfx+k,0,63),clampij(cy+dy,0,63),fin); }
  }
  /* tail outline */
  for(int dy=-6;dy<=6;dy++) sp_put(clampij(tfx+6,0,63),clampij(cy+dy,0,63),mulc(fin,130));
  /* ---- dorsal fin (spiky for trout/salmon) ---- */
  int dfx=cx-11;
  for(int k=0;k<7;k++){ int dh=(F->pat==1||F->pat==3)?5-((k>3)?k-3:0):4-((k>2)?k-2:0);
    for(int d=1;d<=dh;d++) sp_put(clampij(dfx+k,0,63),clampij(cy-ry-d,0,63),fin); }
  /* dorsal highlight */
  for(int k=1;k<6;k++) sp_put(clampij(dfx+k,0,63),clampij(cy-ry-1,0,63),mix3(fin,packrgb(255,255,255),0.4f));
  /* ---- anal + pelvic fins ---- */
  for(int k=0;k<4;k++)for(int d=1;d<=3;d++) sp_put(clampij(tail-10+k,0,63),clampij(cy+ry+d,0,63),fin);
  for(int k=0;k<3;k++) sp_put(clampij(cx-13+k,0,63),clampij(cy+ry-1+k/2,0,63),mulc(fin,170));
  /* pectoral fin (drawn over body) */
  for(int k=0;k<5;k++) sp_put(clampij(cx-13+(k/2),0,63),clampij(cy+2+k,0,63),mulc(fin,185));
  /* ---- catfish whiskers ---- */
  if(F->pat==0){
    for(int k=0;k<7;k++){ sp_put(clampij(snout-1-k,0,63),clampij(cy+1+k/2,0,63),dark);
      sp_put(clampij(snout-1-k,0,63),clampij(cy-1-k/2,0,63),dark); }
  }
  /* ---- mouth ---- */
  sp_put(clampij(snout,0,63),cy,dark); sp_put(clampij(snout+1,0,63),clampij(cy+(F->pat==0||F->pat==3?1:0),0,63),dark);
  if(F->pat==0) sp_put(clampij(snout,0,63),clampij(cy-1,0,63),dark);
  /* ---- eye: socket, iris, pupil, 2 glints ---- */
  int ex=cx-rx/2,ey=cy-3;
  sp_put(clampij(ex-1,0,63),clampij(ey,0,63),mulc(body,90));
  sp_put(clampij(ex-1,0,63),clampij(ey-1,0,63),mulc(body,90));
  sp_put(clampij(ex,0,63),clampij(ey-1,0,63),packrgb(250,250,250));
  sp_put(clampij(ex+1,0,63),clampij(ey-1,0,63),packrgb(250,250,250));
  sp_put(clampij(ex,0,63),clampij(ey,0,63),packrgb(250,250,250));
  sp_put(clampij(ex+1,0,63),clampij(ey,0,63),mix3(packrgb(240,240,240),acc,0.5f));
  sp_put(clampij(ex+1,0,63),clampij(ey,0,63),dark);
  sp_put(clampij(ex,0,63),clampij(ey-1,0,63),packrgb(255,255,255));
}
#endif

/* ---------- CG backdrops (phys-blitted for 1080p speed) ---------- */
static Uint32 CG_PACK[4][96];
static void prep_cg(void){
  for(int s=0;s<4;s++)for(int i=0;i<96;i++)
    CG_PACK[s][i]=packrgb(CG_PAL[s][i][0],CG_PAL[s][i][1],CG_PAL[s][i][2]);
}
static void draw_cg_pan(int scene,int xoff){
  const unsigned char*img=CG_IMG[scene];
  const Uint32*pal=CG_PACK[scene];
  for(int y=0;y<CG_H;y++){
    int py0=M2P(y),py1=M2P(y+1);
    const unsigned char*src=img+(size_t)y*CG_W;
    for(int x=0;x<CG_W;x++){
      int col=x-xoff; if(col<0)col+=CG_W;
      Uint32 c=pal[src[col]];
      int px0=M2P(x),px1=M2P(x+1),pw=px1-px0;
      for(int py=py0;py<py1;py++){
        Uint32*row=scr+py*scrpitch+px0;
        for(int k=0;k<pw;k++)row[k]=c;
      }
    }
  }
}
static void darkall(int a){
  if(a>=255) { fill(0,0,IN_W,IN_H,PACKED[C_BLACK]); return; }
  if(a<=0) return;
  Uint32 am=(Uint32)(255-a);
  int n=scrpitch*PH_H;
  for(int i=0;i<n;i++){ Uint32 c=scr[i];
    int r=(c>>16&0xFF)*am/255,g=(c>>8&0xFF)*am/255,b=(c&0xFF)*am/255;
    scr[i]=0xFF000000u|((Uint32)r<<16)|((Uint32)g<<8)|(Uint32)b; }
}

/* ---------- game state ---------- */
enum{ST_TITLE,ST_CUSTOM,ST_INTRO,ST_PLAY,ST_REEL,ST_SHOP,ST_BAG,ST_LANMENU,ST_QUIT};

/* ---------- LAN multiplayer forward decls (defined later, after scenery) ---------- */
static void net_update(float dt);
static void net_draw(void);
static void net_report_catch(int fishIndex);
static int  net_in_room(void);
static int state=ST_TITLE;

/* ---------- character customization (Terraria-style) ---------- */
static const RGB3 SKIN_COLS[6]={{238,196,158},{214,166,124},{184,132,94},{142,98,66},{96,66,46},{248,224,196}};
static const RGB3 HAIR_COLS[6]={{62,46,32},{156,96,44},{224,202,104},{186,186,196},{96,64,146},{206,64,84}};
static const RGB3 SHIRT_COLS[8]={{74,124,196},{196,74,74},{86,164,86},{204,164,64},{136,84,164},{64,172,172},{232,232,232},{48,54,72}};
static const RGB3 PANTS_COLS[4]={{64,78,120},{92,72,54},{54,96,64},{110,70,110}};
static int cs_skin=0,cs_hair=0,cs_haircol=1,cs_shirt=0,cs_pants=0;
static int csSel=0; /* 0 skin 1 hair 2 haircol 3 shirt 4 pants 5 start */
static Uint32 cs_rgb(const RGB3*t,int i,int n){ return colr(t[i%n]); }
static Uint32 skin_c(void){ return cs_rgb(SKIN_COLS,cs_skin,6); }
static Uint32 hair_c(void){ return cs_rgb(HAIR_COLS,cs_haircol,6); }
static Uint32 shirt_c(void){ return cs_rgb(SHIRT_COLS,cs_shirt,8); }
static Uint32 pants_c(void){ return cs_rgb(PANTS_COLS,cs_pants,4); }
enum{PH_IDLE,PH_CAST,PH_WAIT,PH_NIBBLE,PH_MISS,PH_CATCHMSG};
static int phase=PH_IDLE;
enum{PA_IDLE,PA_WALK,PA_CAST,PA_WAIT,PA_REEL,PA_MISS};
static int pAnim=PA_IDLE;
static float pAnimT=0;

static int playerX=120; static int spot=0; static int plannedFish=-1;
static float castT,phaseT,bobX,bobY,nibbleDelay;
static float reelInd,reelTarget,reelZoneW,reelVel; static int reelGood,reelNeed,reelBad;
static int coins=8,xp=0,level=1,caughtToday=0,day=1;
static float timeH=7.0f;
static int rodLevel=0,lureLevel=0,boat=0,netLevel=0;
static int bagFill=0,bag[64]; static int caughtCount[NFISH];
static float toastT=-1; static char toast[48];
static float introT; static int slide=-1; static int menuSel=0;
/* mod multipliers (set by mods/*.mod, default 1x) */
static int modCoinMul=1, modXpMul=1;

typedef struct{Uint8 space,accept,back,up,down,e,b,left,right;}PRESS;
static PRESS press;

static int bagSize(void){ return 8+netLevel*4; }
static void add_toast(const char*t){ strncpy(toast,t,47);toast[47]=0;toastT=2.0f; }
static const char* spot_name(int r){ return r==0?T("SHORE","岸边"):(r==1?T("PIER","码头"):T("DEEP-WATER","深水")); }
static int is_night(void){ return (timeH<6||timeH>=19); }

/* ============================================================
   SAVE / LOAD  --  progress persists in the per-user app-data dir
   (Windows: %APPDATA%\PixelLakeHeart\PixelLakeHeart\save.dat
    Linux:   ~/.local/share/PixelLakeHeart/PixelLakeHeart/save.dat)
   ============================================================ */
static char save_fullpath[600];
static void save_path_init(void){
  const char*pref=SDL_GetPrefPath("PixelLakeHeart","PixelLakeHeart");
  if(pref) snprintf(save_fullpath,sizeof(save_fullpath),"%ssave.dat",pref);
  else    snprintf(save_fullpath,sizeof(save_fullpath),"save.dat");
}
static void save_game(void){
  save_path_init();
  FILE*f=fopen(save_fullpath,"wb");
  if(!f)return;
  char hdr[8]={'P','L','H','S','A','V','E',3};
  fwrite(hdr,1,8,f);
  fwrite(&coins,sizeof(int),1,f);
  fwrite(&xp,sizeof(int),1,f);
  fwrite(&level,sizeof(int),1,f);
  fwrite(&day,sizeof(int),1,f);
  fwrite(&caughtToday,sizeof(int),1,f);
  fwrite(&timeH,sizeof(float),1,f);
  fwrite(&rodLevel,sizeof(int),1,f);
  fwrite(&lureLevel,sizeof(int),1,f);
  fwrite(&boat,sizeof(int),1,f);
  fwrite(&netLevel,sizeof(int),1,f);
  fwrite(&bagFill,sizeof(int),1,f);
  fwrite(bag,sizeof(int),64,f);
  fwrite(caughtCount,sizeof(int),NFISH,f);
  fwrite(&cs_skin,sizeof(int),1,f);
  fwrite(&cs_hair,sizeof(int),1,f);
  fwrite(&cs_haircol,sizeof(int),1,f);
  fwrite(&cs_shirt,sizeof(int),1,f);
  fwrite(&cs_pants,sizeof(int),1,f);
  fwrite(&lang,sizeof(int),1,f);
  fclose(f);
}
static void load_game(void){
  save_path_init();
  FILE*f=fopen(save_fullpath,"rb");
  if(!f)return;                              /* first run -> keep defaults */
  char hdr[8];
  if(fread(hdr,1,8,f)!=8 || memcmp(hdr,"PLHSAVE",7)!=0){fclose(f);return;}
  fread(&coins,sizeof(int),1,f);
  fread(&xp,sizeof(int),1,f);
  fread(&level,sizeof(int),1,f);
  fread(&day,sizeof(int),1,f);
  fread(&caughtToday,sizeof(int),1,f);
  fread(&timeH,sizeof(float),1,f);
  fread(&rodLevel,sizeof(int),1,f);
  fread(&lureLevel,sizeof(int),1,f);
  fread(&boat,sizeof(int),1,f);
  fread(&netLevel,sizeof(int),1,f);
  fread(&bagFill,sizeof(int),1,f);
  fread(bag,sizeof(int),64,f);
  fread(caughtCount,sizeof(int),NFISH,f);
  fread(&cs_skin,sizeof(int),1,f);
  fread(&cs_hair,sizeof(int),1,f);
  fread(&cs_haircol,sizeof(int),1,f);
  fread(&cs_shirt,sizeof(int),1,f);
  fread(&cs_pants,sizeof(int),1,f);
  fread(&lang,sizeof(int),1,f);
  if(level<1)level=1;
  if(coins<0)coins=0;
  if(bagFill<0)bagFill=0; if(bagFill>64)bagFill=64;
  fclose(f);
}

/* ============================================================
   MOD SYSTEM  --  data-file mods, no recompile needed.
   Put .mod files next to the exe in a "mods/" folder.
   Syntax (UTF-8 text, '#' = comment):
     fish <0-9> value=.. exp=.. weight=.. diff=..   (override fish stats)
     mult coins=.. xp=..                            (global multipliers)
     shop <ROD|LURE|NET|BOAT> cost=..               (override shop price)
     sprite <0-9>                                   (next 64 lines replace
        ........                                       the fish's 64x64 pixels)
   ============================================================ */
static char* trim(char*s){
  while(*s==' '||*s=='\t'||*s=='\r')s++;
  char*e=s+strlen(s);
  while(e>s&&(e[-1]==' '||e[-1]=='\t'||e[-1]=='\r'||e[-1]=='\n'))e--;
  *e=0;
  return s;
}
static void strip_crlf(char*s){
  int n=(int)strlen(s);
  while(n>0&&(s[n-1]=='\n'||s[n-1]=='\r'))s[--n]=0;
}
static int clamp_i(int v,int lo,int hi){ return v<lo?lo:(v>hi?hi:v); }
/* shop item table (declared early so mods can override prices) */
typedef struct{const char*name;int cost;int maxown;int*var;const char*cname;}MENUITEM;
static MENUITEM SHOPITEMS[5]={
  {"SELL ALL",0,0,0,"全部出售"},
  {"ROD",30,3,0,"鱼竿"},
  {"LURE",40,3,0,"鱼饵"},
  {"NET",50,3,0,"渔网"},
  {"BOAT",150,1,0,"小船"},
};
#define NSHOP 5
static void mod_line_fish(char*tok){
  int idx=atoi(tok);
  if(idx<0||idx>=NFISH)return;
  tok=strtok(tok," \t");          /* first token = fish number (consumed by atoi) */
  tok=strtok(NULL," \t");         /* next token = first key=value */
  while(tok){
    char*eq=strchr(tok,'=');
    if(eq){ *eq=0; int v=atoi(eq+1); const char*k=tok;
      if(!strcmp(k,"value")) FISHES[idx].value=clamp_i(v,0,100000);
      else if(!strcmp(k,"exp")) FISHES[idx].exp=clamp_i(v,0,10000);
      else if(!strcmp(k,"weight")) FISHES[idx].weight=clamp_i(v,1,10000);
      else if(!strcmp(k,"diff")) FISHES[idx].diff=clamp_i(v,0,4);
    }
    tok=strtok(NULL," \t");
  }
}
static void mod_line_mult(char*tok){
  tok=strtok(tok," \t");
  while(tok){
    char*eq=strchr(tok,'=');
    if(eq){ *eq=0; int v=atoi(eq+1); const char*k=tok;
      if(!strcmp(k,"coins")) modCoinMul=clamp_i(v,1,1000);
      else if(!strcmp(k,"xp")) modXpMul=clamp_i(v,1,1000);
    }
    tok=strtok(NULL," \t");
  }
}
static void mod_line_shop(char*tok){
  char*name=strtok(tok," \t");
  if(!name)return;
  int idx=-1;
  for(int i=0;i<NSHOP;i++) if(!strcmp(SHOPITEMS[i].name,name)){idx=i;break;}
  if(idx<0)return;
  tok=strtok(NULL," \t");
  while(tok){
    char*eq=strchr(tok,'=');
    if(eq){ *eq=0; if(!strcmp(tok,"cost")) SHOPITEMS[idx].cost=clamp_i(atoi(eq+1),0,100000); }
    tok=strtok(NULL," \t");
  }
}
static void mod_line_palette(char*tok){
  int idx=atoi(tok);
  if(idx<0||idx>=NFISH)return;
  tok=strtok(tok," \t");          /* first token = fish number */
  tok=strtok(NULL," \t");         /* next token = color index 0..15 */
  int ci=atoi(tok?tok:"0");
  if(ci<0||ci>15)return;
  int r=-1,g=-1,b=-1;
  tok=strtok(NULL," \t");
  while(tok){
    char*eq=strchr(tok,'=');
    if(eq){ *eq=0; int v=atoi(eq+1); const char*k=tok;
      if(!strcmp(k,"r"))r=v; else if(!strcmp(k,"g"))g=v; else if(!strcmp(k,"b"))b=v;
    }
    tok=strtok(NULL," \t");
  }
  if(r>=0&&g>=0&&b>=0)
    FSPR_PACK[idx][ci]=packrgb(clamp_i(r,0,255),clamp_i(g,0,255),clamp_i(b,0,255));
}
static void mod_apply_sprite(int idx,FILE*f){
  char line[128];
  int rows=0;
  while(rows<64){
    if(!fgets(line,sizeof(line),f))break;
    strip_crlf(line);
    const char*s=line;
    while(*s==' '||*s=='\t')s++;           /* keep trailing spaces: they = transparent */
    if(!*s||*s=='#')continue;
    int len=(int)strlen(s);
    for(int x=0;x<64;x++){
      int v=255;
      if(x<len){
        char c=s[x];
        if(c>='0'&&c<='9')v=c-'0';
        else if(c>='a'&&c<='f')v=c-'a'+10;
        else if(c>='A'&&c<='F')v=c-'A'+10;
      }
      fsp_pix[idx][rows*64+x]=(unsigned char)v;
    }
    rows++;
  }
}
static void mod_load_file(const char*path){
  FILE*f=fopen(path,"r");
  if(!f)return;
  char line[256];
  while(fgets(line,sizeof(line),f)){
    char*s=trim(line);
    if(!*s||*s=='#')continue;
    if(strncmp(s,"fish",4)==0&&(s[4]==' '||s[4]=='\t')) mod_line_fish(s+4);
    else if(strncmp(s,"mult",4)==0&&(s[4]==' '||s[4]=='\t')) mod_line_mult(s+4);
    else if(strncmp(s,"shop",4)==0&&(s[4]==' '||s[4]=='\t')) mod_line_shop(s+4);
    else if(strncmp(s,"palette",7)==0&&(s[7]==' '||s[7]=='\t')) mod_line_palette(s+7);
    else if(strncmp(s,"sprite",6)==0&&(s[6]==' '||s[6]=='\t')){
      int idx=atoi(s+6);
      if(idx>=0&&idx<NFISH) mod_apply_sprite(idx,f);
    }
  }
  fclose(f);
}
static void mod_scan_and_load(void){
  const char*base=SDL_GetBasePath();
  if(!base)return;
  char dir[600]; snprintf(dir,sizeof(dir),"%smods",base);
  DIR*d=opendir(dir);
  if(!d)return;                              /* no mods/ -> nothing to apply */
  struct dirent*e;
  while((e=readdir(d))!=NULL){
    const char*nm=e->d_name;
    int len=(int)strlen(nm);
    if(len<4||strcmp(nm+len-4,".mod")!=0)continue;
    char fp[700]; snprintf(fp,sizeof(fp),"%s/%s",dir,nm);
    mod_load_file(fp);
  }
  closedir(d);
}

/* ---------- ambient fish schools: lake is alive before you cast ----------
   (Dave-style: you always SEE fish cruising under the surface) */
typedef struct{ int sp; float x,y,dir,spd,ph,sc; }AMBF;
#define NAMB 9
static AMBF AMB[NAMB];
static void ambient_init(void){
  for(int i=0;i<NAMB;i++){
    AMB[i].sp=rndi(10);
    AMB[i].x=rndf()*(IN_W+140)-70;
    AMB[i].y=163.0f+rndf()*92.0f;
    AMB[i].dir=(rndi(2))?1.0f:-1.0f;
    AMB[i].spd=9.0f+rndf()*20.0f;
    AMB[i].ph=rndf()*6.2832f;
    AMB[i].sc=(AMB[i].sp>=8)?0.6f:(0.4f+rndf()*0.2f); /* legends a bit bigger */
  }
}
static void ambient_update(float dt){
  for(int i=0;i<NAMB;i++){
    AMBF*a=&AMB[i];
    a->x+=a->dir*a->spd*dt;
    a->ph+=dt*1.5f;
    if(a->x<-80.0f)a->x=(float)IN_W+79.0f;
    if(a->x>(float)IN_W+80.0f)a->x=-79.0f;
  }
}
static void draw_ambient(void){
  int night=is_night();
  for(int i=0;i<NAMB;i++){
    AMBF*a=&AMB[i];
    float yy=a->y+sinf(a->ph)*3.5f;
    int moving_right=(a->dir>0.0f);
    int face_left=FSPR_FACE_LEFT[a->sp];
    int flip=(moving_right&&face_left)||(!moving_right&&!face_left);
    /* underwater tint; extra dark veil at night (only on fish pixels) */
    render_fish_mode(a->sp,2,flip);
    blit_fish_f(a->x-32*a->sc,yy-32*a->sc,a->sc);
    if(night){
      int step=(int)(64*a->sc);
      int bx0=(int)(a->x-32*a->sc), by0=(int)(yy-32*a->sc);
      for(int y=0;y<step;y++)for(int x=0;x<step;x++){
        Uint32 c=fsp[((int)(y/a->sc))*64+(int)(x/a->sc)];
        if(c&&((x+y)&3)==0) setpix(bx0+x,by0+y,PACKED[C_DEEPWATER]);
      }
    }
  }
}
/* Dave-style bite: the hooked fish visibly cruises toward the bobber */
static void draw_bite_fish(void){
  if(plannedFish<0)return;
  int sp=plannedFish;
  float t;
  if(phase==PH_WAIT){ t=phaseT/(nibbleDelay>0?nibbleDelay:1.0f); if(t>1)t=1; }
  else t=1.0f; /* nibble: right under the bobber */
  float side=-1.0f; /* approach from the left */
  float dist=(1.0f-t)*80.0f+10.0f;
  float fx=bobX+side*dist-14.0f;
  float fy=bobY+12.0f+sinf(phaseT*4.0f)*3.0f;
  int flip=0; /* swims right (head right) */
  if(FSPR_FACE_LEFT[sp]) flip=1;
  render_fish_mode(sp,2,flip);
  blit_fish_f(fx-16.0f,fy-16.0f,0.5f);
}

/* ---------- player figure (30-frame parametric) ---------- */
/* block-scaled rect with Minecraft volume: per-pixel texel texture + top row
   sunlit + bottom dark seam + left edge AO, so every body part reads as a
   separate lit 3D cube (MC player look). */
static void P(int ox,int oy,int sc,int x,int y,int w,int h,Uint32 c){
  for(int by=y;by<y+h;by++){
    int fr=by-y;
    int dv=(h>=3)?((fr>=h*3/5)?-16:((fr>=h*2/5)?2:16)):3;
    int top=(h>=2&&fr==0), bot=(h>=2&&fr==h-1);
    for(int a=0;a<sc;a++){
      int py0=oy+by*sc+a;
      int prevd=1000;
      for(int bx=x;bx<x+w;bx++){
        for(int b=0;b<sc;b++){
          int px0=ox+bx*sc+b;
          /* multiplicative MC tone: stays visible on dark colours (boots/legs),
             plus per-pixel dither and a guarantee it differs from the pixel
             to its left -> every pixel of the sprite is its own tiny dot */
          int d=mc_texel_pct((int)hash2(px0*7+(int)(c>>20),py0*13+((int)((c>>8)&15))));
          d+=(int)(hash2(px0*3+1,py0*5+7)%3)-1;
          if(prevd!=1000 && d==prevd) d=(d>=0)?d+1:d-1;
          prevd=d;
          Uint32 base=texel_c(c,d);
          Uint32 col=shade_c(base,dv);
          if(top) col=shade_c(col,16);        /* sunlit top face */
          if(bot) col=mulc(col,45);           /* shaded bottom face */
          if(bx==0) col=shade_c(col,-12);     /* left AO side */
          else if(bx==w-1) col=shade_c(col,9);/* right sunlit side */
          setpix(px0,py0,col);
        }
      }
    }
  }
}
static void Pline(int ox,int oy,int sc,int x0,int y0,int x1,int y1,Uint32 c){
  int dx=abs(x1-x0),sx=x0<x1?1:-1,dy=-abs(y1-y0),sy=y0<y1?1:-1,er=dx+dy;
  for(;;){ P(ox,oy,sc,x0,y0,1,1,c); if(x0==x1&&y0==y1)break; int e2=2*er; if(e2>=dy){er+=dy;x0+=sx;} if(e2<=dx){er+=dx;y0+=sy;} }
}
/* draw puppet (customizable appearance, Terraria-style); returns grip via hx,hy */
static void draw_person(int ox,int oy,int sc,int anim,float t,int* hx,int* hy){
  Uint32 skin=skin_c(),hair=hair_c(),cap=PACKED[C_CAP],shirt=shirt_c(),
        pants=pants_c(),boot=PACKED[C_BOOT],dark=PACKED[C_DARK];
  float ph=t*6.0f;
  int bob=0,lig=0,rig=0,larm=0,rarm=0;
  if(anim==PA_WALK){
    float s1=sinf(ph),s2=sinf(ph+3.14159f);
    lig=(int)(s1*3); rig=(int)(s2*3); larm=(int)(s2*2); rarm=(int)(s1*2);
    bob=(int)(fabsf(sinf(ph))*1.0f);
  } else if(anim==PA_IDLE){
    bob=(((int)(t*2))&1);
  } else if(anim==PA_REEL){ /* arm pumping */
    float s1=sinf(ph*2);
    larm=(int)(s1*2); rarm=(int)(s1*-2); bob=(int)((s1>0)?1:0);
  }
  int b=bob;
  /* ---- hair styles (drawn under cap) ---- */
  if(cs_hair==1){ /* long hair: crown + sides down to torso */
    P(ox,oy,sc, 3,1+b,9,2,hair);
    P(ox,oy,sc, 3,2+b,2,9,hair); P(ox,oy,sc, 10,2+b,2,9,hair);
    P(ox,oy,sc, 3,10+b,2,3,hair);
  } else if(cs_hair==2){ /* ponytail: crown + back tail swinging */
    P(ox,oy,sc, 3,1+b,9,2,hair);
    P(ox,oy,sc, 2,2+b,2,4,hair);
    int sw=(anim==PA_WALK)?(int)(sinf(ph)*2):0;
    P(ox,oy,sc, 1+sw,5+b,2,6,hair);
  } else if(cs_hair==3){ /* bald - just sideburns */
    P(ox,oy,sc, 4,5+b,1,2,mulc(hair,140)); P(ox,oy,sc, 10,5+b,1,2,mulc(hair,140));
  } else { /* short hair: crown + short sides */
    P(ox,oy,sc, 3,1+b,9,2,hair);
    P(ox,oy,sc, 4,2+b,1,2,hair); P(ox,oy,sc, 10,2+b,1,2,hair);
  }
  /* cap (fisherman's bucket hat; tilted, sits over hair) */
  if(cs_hair!=3){
    P(ox,oy,sc, 2,0+b,10,2,cap); P(ox,oy,sc, 1,1+b,13,1,cap);
    P(ox,oy,sc, 2,0+b,10,1,mulc(cap,180)); /* hat top highlight */
  } else { /* bald gets a headband */
    P(ox,oy,sc, 3,1+b,9,1,PACKED[C_WARN]);
  }
  /* face */
  P(ox,oy,sc, 4,2+b,7,5,skin);
  P(ox,oy,sc, 4,6+b,7,1,mulc(skin,200)); /* jaw shadow */
  /* eye (blink on idle every ~3s) */
  int blink= (anim==PA_IDLE && ((int)(t*1.0f))%3==0);
  if(!blink){ P(ox,oy,sc, 6,4+b,1,1,dark); P(ox,oy,sc, 6,4+b,1,1,dark); }
  /* torso */
  P(ox,oy,sc, 3,7+b,9,7,shirt);
  P(ox,oy,sc, 3,7+b,9,1,mulc(shirt,190)); /* collar highlight */
  P(ox,oy,sc, 7,8+b,1,6,mulc(shirt,170)); /* center seam */
  /* legs + boots */
  P(ox,oy,sc, 4+lig,14+b,3,6,pants);
  P(ox,oy,sc, 9+rig,14+b,3,6,pants);
  P(ox,oy,sc, 4+lig-1,19+b,4,2,boot);
  P(ox,oy,sc, 8+rig+1,19+b,5,2,boot);
  /* arms */
  int shl=5, shr=10;
  /* decide grip */
  int chx=9, chy=3+b; /* default: rod held up-forward with both hands */
  if(anim==PA_WALK||anim==PA_IDLE){ /* hands free */
    Pline(ox,oy,sc,shl,8+b, 4-larm,15, shirt);
    Pline(ox,oy,sc,shr,8+b, 11+rarm,15, shirt);
    if(hx){*hx=6;*hy=12;}
    return;
  }
  /* fishing poses: both hands grip rod */
  chx=9; chy=1+b;
  if(anim==PA_CAST){ float a=sinf(ph); chx=9+(int)(a*4); chy=0+((int)(a*3)); }
  Pline(ox,oy,sc,shl,8+b,chx,chy,shirt);
  Pline(ox,oy,sc,shr,8+b,chx,chy,shirt);
  if(hx){*hx=chx;*hy=chy;}
}

/* ---------- reel minigame (difficulty-driven moving green zone) ---------- */
static int reelDiff,reelTol;
static float zoneBase,zonePh,zoneAmp,zoneSpd,zoneDrift,zoneJumpT;
static void begin_reel(void){
  int d=(plannedFish>=0)?FISHES[plannedFish].diff:0;
  reelDiff=d;
  reelInd=20;
  reelVel=115.0f+d*46.0f+rodLevel*10.0f;
  reelNeed=3+d; if(reelNeed+(level/3)>8)reelNeed=8; else reelNeed+=level/3;
  reelGood=0; reelBad=0;
  reelTol=(d>=3)?2:3;
  reelZoneW=(float)(86-d*14)+rodLevel*7.0f; if(reelZoneW>100)reelZoneW=100;
  zoneBase=40.0f+rndf()*(IN_W-reelZoneW-80.0f);
  zonePh=rndf()*6.2832f;
  zoneAmp=18.0f+d*20.0f;
  zoneSpd=0.8f+d*0.8f;
  zoneDrift=0;
  zoneJumpT=1.1f+rndf()*1.4f;
  reelTarget=zoneBase;
  state=ST_REEL;
}
static void update_reel(float dt){
  /* --- moving green zone: sine sweep + damped random walk + struggle jumps --- */
  zonePh+=dt*zoneSpd;
  zoneDrift+=(rndf()-0.5f)*dt*(60.0f+reelDiff*90.0f);
  zoneDrift*=(1.0f-1.6f*dt);
  float t=zoneBase+sinf(zonePh)*zoneAmp+zoneDrift;
  if(reelDiff>=2){ /* strong fish struggle: sudden zone jumps */
    zoneJumpT-=dt*(0.55f+reelDiff*0.4f);
    if(zoneJumpT<=0){
      zoneJumpT=0.8f+rndf()*(1.7f-reelDiff*0.18f);
      zoneDrift+=(rndf()-0.5f)*(110.0f+reelDiff*100.0f);
    }
  }
  if(t<24){t=24;} if(t>IN_W-reelZoneW-24){t=IN_W-reelZoneW-24;}
  reelTarget=t;
  /* marker bounce */
  reelInd+=reelVel*dt;
  if(reelInd>IN_W-20){reelInd=IN_W-20;reelVel=-reelVel;}
  if(reelInd<20){reelInd=20;reelVel=-reelVel;}
  if(press.space){
    if(reelInd>=reelTarget&&reelInd<=reelTarget+reelZoneW){
      reelGood++;
      /* each successful hook makes the fish fight harder */
      zoneSpd+=0.18f; zoneAmp+=2.5f;
      zoneBase=30.0f+rndf()*(IN_W-reelZoneW-60.0f);
      zonePh=rndf()*6.2832f;
      if(reelGood>=reelNeed){
        int ok=(plannedFish>=0)?plannedFish:0;
        net_report_catch(ok);
        if(bagFill<bagSize()){bag[bagFill++]=ok;caughtCount[ok]++;xp+=FISHES[ok].exp*modXpMul;caughtToday++;}
        else add_toast(T("CHEST FULL - SELL IN SHOP","背包已满 - 去商店出售"));
        if(xp>=level*20){xp-=level*20;level++;add_toast(T("LEVEL UP!","升级了!"));}
        state=ST_PLAY;phase=PH_CATCHMSG;phaseT=0;
        save_game();
      }
    } else {
      reelBad++;
      if(reelBad>=reelTol){add_toast(T("LINE SNAPPED!","线断了!"));state=ST_PLAY;phase=PH_IDLE;}
    }
  }
}
static void draw_reel(void){
  fill(0,0,IN_W,IN_H,PACKED[C_DEEPWATER]);
  /* track */
  fill(20,92,IN_W-40,2,PACKED[C_SHADOW]);
  /* green zone: brighter core + soft edges */
  int zx=(int)reelTarget,zw=(int)reelZoneW;
  fill(zx-2,88,zw+4,10,mulc(PACKED[C_GREEN],120));
  fill(zx,90,zw,6,PACKED[C_GREEN]);
  fill(zx+2,91,zw-4,2,mix3(PACKED[C_GREEN],packrgb(255,255,255),0.5f));
  /* marker */
  fill((int)reelInd-2,84,4,18,PACKED[C_FLOATB]);
  fill((int)reelInd-1,86,2,14,packrgb(255,240,200));
  /* hook pips */
  for(int i=0;i<reelNeed;i++){Uint32 c=i<reelGood?PACKED[C_GREEN]:PACKED[C_GREY];
    setpix(24+i*16,22,c);setpix(25+i*16,22,PACKED[C_GREY]);}
  center_text(112,T("SPACE: HIT MARKER IN GREEN ZONE","按空格:在绿色区域内击中标记"),PACKED[C_WHITE],1);
  { char b[48]; snprintf(b,48,T("%d MISSES = LINE SNAPS","%d次失误 = 断线"),reelTol-reelBad);
    center_text(126,b,PACKED[C_SILVER],1); }
  center_text(140,T("REEL!","收竿!"),PACKED[C_GOLD],2);
  if(plannedFish>=0){
    int d=FISHES[plannedFish].diff;
    Uint32 now=SDL_GetTicks();
    /* fish struggles harder with difficulty */
    int shake=(int)(sinf(now*0.02f*(1+d))* (2+d*3));
    int jolt=(d>=2)?(int)(sinf(now*0.013f)*4):0;
    render_fish(plannedFish,0);
    blit_rgba((IN_W-64)/2+shake,30+jolt,1,fsp,64,64);
    center_text(166,T(FISHES[plannedFish].en,FISHES[plannedFish].cn),PACKED[C_CYAN],2);
    /* difficulty stars */
    char st[16]; int n=0;
    st[n++]=' '; st[n++]=' ';
    for(int i=0;i<5;i++) st[n++]=(i<=d)?'*':'.';
    st[n]=0;
    draw_text((IN_W+text_w(FISHES[plannedFish].en,2))/2+6,166+12,st,PACKED[C_GOLD],1);
  }
  char b[40]; snprintf(b,40,T("HOOKS %d/%d","收竿数 %d/%d"),reelGood,reelNeed);
  center_text(196,b,PACKED[C_CYAN],1);
}

/* ---------- play ---------- */
static int roll_fish(void){
#define MAXP 256
  int cand[MAXP],n=0;
  for(int i=0;i<NFISH;i++){ FISH f=FISHES[i];
    if(f.region>spot)continue;
    if(f.time==2&&!is_night())continue;
    if(f.time==1&&is_night())continue;
    if(f.region==2&&!boat)continue;
    int w=f.weight+(i>=3?lureLevel*6:0);
    for(int k=0;k<w&&n<MAXP;k++)cand[n++]=i;
  }
  if(n==0){cand[0]=0;n=1;}
  return cand[rndi(n)];
#undef MAXP
}
static void cast_line(void){
  spot=(playerX<170)?0:(playerX<340)?1:2;
  plannedFish=roll_fish();
  bobX=(float)playerX; bobY=170+rndf()*28+spot*8;
  phase=PH_CAST; phaseT=0; castT=0.5f;
  pAnim=PA_CAST;
}
static void update_play(float dt){
  const Uint8*keys=SDL_GetKeyboardState(NULL);
  int moving=0; int spd=95;
  if(keys[SDL_SCANCODE_LEFT]){playerX-=(int)(spd*dt);moving=1;}
  if(keys[SDL_SCANCODE_RIGHT]){playerX+=(int)(spd*dt);moving=1;}
  playerX=clampij(playerX,20,IN_W-20);
  timeH+=(dt/420.0f)*24.0f; if(timeH>=24){timeH-=24;day++;caughtToday=0;}
  pAnimT+=dt;
  ambient_update(dt);
  /* pick anim */
  int fishing=(phase==PH_CAST||phase==PH_WAIT||phase==PH_NIBBLE||phase==PH_MISS||phase==PH_CATCHMSG);
  if(fishing){ pAnim=(phase==PH_CAST)?PA_CAST:(phase==PH_MISS?PA_WAIT:PA_WAIT); }
  else if(moving){ pAnim=PA_WALK; }
  else pAnim=PA_IDLE;

  if(phase==PH_IDLE){ if(press.space){
      if(!is_night()) cast_line();
      else add_toast(T("CANNOT FISH AT NIGHT","夜晚不能钓鱼"));
  } }
  else if(phase==PH_CAST){ castT-=dt; if(castT<=0){phase=PH_WAIT;phaseT=0;pAnim=PA_WAIT;
      nibbleDelay=2.0f+rndf()*3.5f-rodLevel*0.5f; if(nibbleDelay<1.2f)nibbleDelay=1.2f;} }
  else if(phase==PH_WAIT){ phaseT+=dt; if(phaseT>=nibbleDelay){phase=PH_NIBBLE;phaseT=0;} }
  else if(phase==PH_NIBBLE){ phaseT+=dt;
    float win=1.1f+rodLevel*0.25f;
    if(press.space){begin_reel();return;}
    if(phaseT>=win){phase=PH_MISS;phaseT=0;add_toast(T("TOO SLOW!","太慢了!"));} }
  else if(phase==PH_MISS){ phaseT+=dt; if(phaseT>=1.0f)phase=PH_IDLE; }
  else if(phase==PH_CATCHMSG){ phaseT+=dt; if(phaseT>=1.8f)phase=PH_IDLE; }
}
/* ---------- Stardew-style scenery: sky, sun/moon arc, stars, clouds,
   layered hills, big tree, reeds, shimmering water, reflections ---------- */
static int sunX=430,sunY=30; /* shared with water reflections */
static const int STARS[24][2]={
 {12,9},{38,20},{60,6},{88,26},{104,12},{132,30},{150,8},{178,22},
 {206,34},{224,10},{250,26},{276,14},{300,32},{318,6},{342,24},{368,16},
 {390,34},{412,10},{438,26},{460,12},{482,30},{498,18},{26,36},{470,36}
};
static void cloud(int x,int y,int s,Uint32 c){
  /* cloud = a cluster of little voxel cubes (lit top / shaded underside), so it
     reads as a blocky MC cloud instead of a flat rectangle */
  static const int PX[7]={0,2,5,3,7,1,4}, PY[7]={0,-1,0,-2,0,1,1}, PW[7]={2,2,2,2,2,1,1};
  for(int i=0;i<7;i++){
    int cx=x+PX[i]*s, cy=y+PY[i]*s;
    vox_cube(cx,cy,PW[i]*s,s,c,10,2,i*11+x);
  }
  dither_rect(x,y-3,9*s,5,6,6,x*3);   /* bright rim speckle */
}
static void hills(Uint32 c,int amp,int base,float seed){
  for(int x=0;x<IN_W;x++){
    float fx=sinf(x*0.013f+seed)*amp+sinf(x*0.037f+seed*2.0f)*amp*0.4f;
    int h=base+(int)fx;
    int topY=(150-h)/4*4;              /* step the silhouette into 4px blocks */
    for(int y=topY;y<150;y++){
      int blk=(y-topY)/4;
      int d=mc_texel_pct((int)hash2(x*7,y*13+3));  /* MC clustered texels, % */
      d+=(int)(hash2(x*3+1,y*5+7)%3)-1;
      int add=(blk==0)?16:0;                      /* lit top of each block */
      if(((y-topY)%4)==3 && blk!=0) d-=40;        /* dark seam row (still textured) */
      setpix(x,y,shade_c(texel_c(c,d),add));
    }
    if(topY<150) setpix(x,topY,shade_c(texel_c(c,mc_texel_pct((int)hash2(x*5,(int)(seed*97.0f)))),20));
  }
}
static void draw_tree(int x,int ground,int night){
  Uint32 trunk=PACKED[night?C_TRUNK:C_DOCK2], leaf=PACKED[night?C_BUSH:C_GRASS];
  Uint32 leafdark=mulc(leaf,night?125:145), leafhi=mix3(leaf,PACKED[C_WHITE],night?0.12f:0.30f);
  /* trunk: stacked bark CUBES (lit top / shaded bottom / own texture + seams),
     three columns with LEFT-shaded / base / RIGHT-sunlit sides, fixed light
     from the upper-right, MC log look */
  for(int yy=ground-32;yy<ground-1;yy+=5){
    int hh=(ground-1)-yy; if(hh>5)hh=5;
    vox_cube(x-2,yy,2,hh,shade_c(trunk,-12),10,10,x*3+yy/2);
    vox_cube(x,  yy,2,hh,trunk,             10,10,x*3+yy/2+1);
    vox_cube(x+2,yy,2,hh,shade_c(trunk,10), 11,10,x*3+yy/2+2);
  }
  /* bark grain: MC log vertical streaks (light + dark) over the middle column */
  for(int yy=ground-31;yy<ground-1;yy++){
    if(hash2(x*3,yy)%3==0) setpix(x,yy,mulc(trunk,150));        /* dark streak */
    if(hash2(x*3+1,yy)%5==0) setpix(x+1,yy,shade_c(trunk,14));  /* light streak */
  }
  /* root flare cubes */
  vox_cube(x-4,ground-3,4,2,shade_c(trunk,-8),10,8,x*7);
  vox_cube(x+2,ground-3,4,2,shade_c(trunk,8), 10,8,x*7+1);
  fill(x-3,ground-1,8,2,mulc(trunk,50));    /* root AO seam */
  /* low branches: bark cubes */
  vox_cube(x-6,ground-30,3,2,shade_c(trunk,-6),10,8,x*3+4);
  vox_cube(x+4,ground-34,3,2,trunk,         10,8,x*3+5);
  /* leaf crown: a DENSE ball of stacked 5px leaf cubes. One coherent volume:
     light comes from the upper-right, so top rows + right columns are bright,
     bottom rows + left columns dark; every cube keeps its own 3-zone shading
     + seam so the crown reads as MANY separate leaf blocks (MC oak leaves) */
  int maxcol=4,maxrow=3;
  for(int row=-maxrow;row<=maxrow;row++){
    int hw=(int)(maxcol*sqrtf(1.0f-(float)(row*row)/(float)(maxrow*maxrow)));
    int yy=ground-46+row*5;
    for(int col=-hw;col<=hw;col++){
      int xx=x+col*5;
      Uint32 cc=leaf;
      int lv=-row*5+(col>0?4:(col<0?-4:0));  /* volume gradient */
      cc=shade_c(cc,lv);
      if(hash2(xx,yy)%7==0) cc=mulc(cc,150);           /* scattered dark leaf */
      if(hash2(xx+3,yy+1)%11==0) cc=mix3(cc,PACKED[C_WHITE],0.25f); /* bright speck */
      vox_cube(xx,yy,5,5,cc,18,16,xx);
    }
  }
  /* canopy scatter: sunlit specks on top, stray dark leaves at the rim */
  int toprow=-maxrow;
  int hwt=(int)(maxcol*sqrtf(1.0f-(float)(toprow*toprow)/(float)(maxrow*maxrow)));
  for(int xx=x-hwt*5;xx<=x+hwt*5;xx+=2)
    if(hash2(xx,ground-46+toprow*5)%3) setpix(xx,ground-46+toprow*5-1,leafhi);
  for(int row=-maxrow;row<=maxrow;row++){
    int hw=(int)(maxcol*sqrtf(1.0f-(float)(row*row)/(float)(maxrow*maxrow)));
    int yy=ground-46+row*5;
    int x0c=x-hw*5, x1c=x+hw*5;
    for(int k=0;k<4;k++){
      int den=hw*10+1;
      int lx=x0c+(hash2(k*7,row*13)%den);
      setpix(lx,yy+(hash2(k,row)%2),leafdark);
      setpix(x1c-(hash2(k*3,row)%2),yy+(hash2(k,row*2)%2),shade_c(leafdark,-6));
    }
  }
}
static void draw_sky(void){
  int night=is_night();
  Uint32 skytop,skybot;
  if(night){ skytop=packrgb(34,44,76); skybot=packrgb(64,86,132); }
  else if(timeH<8){ /* dawn: peach horizon */
    skytop=packrgb(64,110,168); skybot=packrgb(246,178,120);
  } else if(timeH>=17){ /* dusk: violet-orange */
    skytop=packrgb(72,52,112); skybot=packrgb(236,120,86);
  } else { skytop=packrgb(84,158,222); skybot=packrgb(168,224,238); }
  fill_grad(0,0,IN_W,150,skytop,skybot);
  grad_noise(0,0,IN_W,150,3,11);   /* per-pixel sky dither, no flat bands */
  dither_rect(0,0,IN_W,150,19,6,13); /* faint horizontal weave -> depth strata */
  Uint32 now=SDL_GetTicks();
  if(night){
    /* stars with twinkle */
    for(int i=0;i<24;i++){
      if((i+(int)(now/560))%6!=0)
        setpix(STARS[i][0],STARS[i][1],(i%3)?PACKED[C_WHITE]:packrgb(200,210,255));
      if(i%7==0) setpix(STARS[i][0]+1,STARS[i][1]+1,packrgb(120,130,180));
    }
    /* moon: blocky voxel disc + crescent bite + craters */
    float nf=(timeH>=19)?(timeH-19.0f)/10.0f:((timeH+5.0f)/10.0f);
    int mx=(int)(50+nf*(IN_W-110)), my=44;
    fill(mx-5,my-5,11,11,packrgb(228,232,242));
    fill(mx-3,my-6,7,13,packrgb(228,232,242));
    fill(mx-6,my-3,13,7,packrgb(228,232,242));
    fill(mx-4,my-4,8,8,packrgb(246,248,252));
    fill(mx-3,my-3,6,6,mulc(skytop,190)); /* crescent bite */
    fill(mx+3,my-5,3,3,packrgb(190,196,210)); /* craters */
    fill(mx-2,my+2,2,2,packrgb(190,196,210));
    grad_noise(mx-6,my-6,13,13,2,77);     /* per-pixel moon surface */
    sunX=mx; sunY=my;
  } else {
    /* sun arc: rises 6:00, sets 18:00 */
    float dayf=(timeH-6.0f)/12.0f;
    if(dayf<0){dayf=0;} if(dayf>1){dayf=1;}
    int sx=(int)(46+dayf*(IN_W-100));
    int sy=(int)(96-fabsf(dayf-0.5f)*2*78);
    Uint32 sc=(timeH<8||timeH>=17)?packrgb(250,150,80):packrgb(255,214,90);
    /* blocky voxel sun: glow rings + lit cube core + rays */
    fill(sx-6,sy-6,13,13,mulc(sc,120));
    fill(sx-4,sy-4,9,9,mulc(sc,170));
    vox_block(sx-3,sy-3,7,7,sc,18,2,5);
    noise_fill(sx-1,sy-1,3,3,mix3(sc,packrgb(255,255,240),0.6f),2,9);
    setpix(sx,sy-8,sc);setpix(sx,sy+8,sc);setpix(sx-8,sy,sc);setpix(sx+8,sy,sc);
    setpix(sx+1,sy-7,mulc(sc,190));setpix(sx-1,sy-7,mulc(sc,190));
    sunX=sx; sunY=sy;
    /* drifting clouds (3 parallax layers) */
    Uint32 cc=(timeH<8||timeH>=17)?packrgb(250,200,170):packrgb(250,252,255);
    int c1=((int)(now/340))%(IN_W+160)-80;
    cloud(c1,26,3,cc);
    int c2=((int)(now/260)+230)%(IN_W+160)-80;
    cloud(c2,58,2,mulc(cc,205));
    int c3=((int)(now/210)+420)%(IN_W+160)-80;
    cloud(c3,42,2,mulc(cc,225));
    /* birds (day only): little V flock */
    int bx=((int)(now/90))%(IN_W+60)-30;
    for(int k=0;k<3;k++){
      int bxx=bx+k*9, byy=20+((k%2)*3);
      int flap=((int)(now/220)+k)%2;
      setpix(bxx,byy,PACKED[C_DARK]); setpix(bxx+1,byy+(flap?1:-1),PACKED[C_DARK]);
      setpix(bxx+2,byy,PACKED[C_DARK]);
    }
  }
  /* distant hills: two layered silhouettes */
  hills(mix3(skybot,packrgb(40,70,90),0.45f),15,22,1.7f);
  hills(mix3(skybot,packrgb(24,50,66),0.62f),11,13,4.9f);
  /* haze band at horizon: textured depth strip, never a flat fill */
  for(int y=142;y<150;y++){
    Uint32 hc=mulc(skybot,(150-y)*16+92);
    int prevd=1000;
    for(int x=0;x<IN_W;x++){
      int d=mc_texel_pct((int)hash2(x*5+y,y*7+3));
      d+=(int)(hash2(x*3+1,y*5+7)%3)-1;
      if(d==prevd) d=(d>=0)?d+1:d-1;
      prevd=d;
      setpix(x,y,texel_c(hc,d));
    }
  }
}
static void draw_water(void){
  int night=is_night();
  Uint32 topc,botc;
  if(night){ topc=packrgb(46,72,128); botc=packrgb(16,30,70); }
  else if(timeH<8){ topc=packrgb(214,150,120); botc=packrgb(40,70,120); }
  else if(timeH>=17){ topc=packrgb(190,100,100); botc=packrgb(30,44,90); }
  else { topc=packrgb(84,176,220); botc=packrgb(20,84,150); }
  fill_grad(0,150,IN_W,138,topc,botc);
  Uint32 now=SDL_GetTicks();
  /* vertical depth table (surface light -> deep dark) */
  Uint32 wcol[IN_H];
  {
    int tr=topc>>16&0xFF,tg=topc>>8&0xFF,tb=topc&0xFF;
    int br=botc>>16&0xFF,bg=botc>>8&0xFF,bb=botc&0xFF;
    for(int yy=150;yy<IN_H;yy++){
      float t=(float)(yy-150)/(float)(IN_H-150);
      wcol[yy]=packrgb((int)(tr+(br-tr)*t),(int)(tg+(bg-tg)*t),(int)(tb+(bb-tb)*t));
    }
  }
  /* MC-style water: a grid of 4px water CUBES - each cube keeps its own
     crest/trough tone (drifting), a sunlit TOP row, a shaded BOTTOM row and a
     per-texel MC texture, so the lake reads as tiled animated water blocks
     instead of a flat gradient sheet. The texture runs ALL the way down so the
     deep water is never a flat fill. */
  for(int cy=150;cy<IN_H;cy+=4){
    float ph=((cy-150)/4)*0.55f+now*0.0011f;
    int drift=(int)(sinf(ph)*8.0f);
    int surf=(cy<182);                     /* only surface rows keep wave drift */
    for(int cx=0;cx<IN_W;cx+=4){
      int cre=((hash2(cx*7,cy*13+3)&1)?3:-3);   /* per-block crest/trough */
      for(int yy=cy;yy<cy+4&&yy<IN_H;yy++){
        int rowin=yy-cy;
        int band;
        if(surf) band=(rowin==0)?(15+drift):((rowin==3)?(-13+drift):cre);
        else     band=(rowin==0)?8:((rowin==3)?-10:cre);
        for(int xx=cx;xx<cx+4&&xx<IN_W;xx++){
          /* multiplicative MC water tone + dither, differs from left texel */
          int d=mc_texel_pct((int)hash2(xx*5+cy,yy*7+3));
          d+=(int)(hash2(xx*3,yy*11+cy)&3)-1;
          setpix(xx,yy,shade_c(texel_c(wcol[yy],d),band));
        }
      }
      /* dark under-seam of every water-block row -> the block grid is visible */
      if(cy+4<IN_H) for(int xx=cx;xx<cx+4&&xx<IN_W;xx++)
        if((hash2(xx,cy)&1)==0) setpix(xx,cy+3,mulc(wcol[cy+3],70));
    }
  }
  dither_rect(0,150,IN_W,138,7,20,37);   /* sunlit wave crest speckle */
  dither_rect(0,150,IN_W,138,11,-16,39); /* deep water patches */
  /* depth: darkest band only at the very bottom - textured, never a flat fill */
  for(int yy=IN_H-24;yy<IN_H;yy++){
    Uint32 bc=mulc(botc,140-(yy-(IN_H-24))*3);
    int prevd=1000;
    for(int xx=0;xx<IN_W;xx++){
      int d=mc_texel_pct((int)hash2(xx*5+yy,yy*7+3));
      d+=(int)(hash2(xx*3,yy*11)%3)-1;
      if(d==prevd) d=(d>=0)?d+1:d-1;
      prevd=d;
      setpix(xx,yy,texel_c(bc,d));
    }
  }
  /* shimmer: horizontal light streaks drifting */
  Uint32 shim=night?packrgb(70,110,190):mix3(topc,packrgb(255,255,255),0.55f);
  for(int i=0;i<70;i++){
    int seedx=(i*97+((int)(now/140)))%IN_W;
    int y=153+(i*37)%120;
    int w=3+(i%9);
    Uint32 c=mulc(shim,(i%3==0)?230:180);
    fill(seedx,y,w,1,c);
  }
  /* sun/moon reflection column (Stardew-style broken light path) */
  {
    Uint32 rc=night?mulc(packrgb(228,232,242),140):packrgb(255,236,150);
    for(int y=152;y<230;y+=2){
      int spread=2+(y-152)/22;
      int off=((y+(int)(now/190))%5)-2;
      fill(sunX-spread+off,y,spread*2+1,1,mulc(rc,(y<190)?190:130));
    }
  }
}
static void draw_dock(void){
  int night=is_night();
  Uint32 d=PACKED[night?C_BROWN:C_DOCK],d2=PACKED[night?C_TRUNK:C_DOCK2];
  Uint32 grass=PACKED[night?C_BUSH:C_GRASS], dirt=PACKED[night?C_TRUNK:C_DOCK2];
  /* left bank: Minecraft grass-block terrain (jagged blocky sod + dirt cubes) */
  vox_ground(0,150,54,20,dirt,grass,night?8:16,1);
  draw_tree(34,172,night);
  /* reeds swaying (pixel stalks with tuft) */
  Uint32 now=SDL_GetTicks();
  for(int i=0;i<5;i++){
    int rx=8+i*9;
    int sway=(int)(sinf(now*0.0015f+i)*2);
    Pline(0,0,1,rx,170,rx+sway,152,PACKED[night?C_TRUNK:C_DOCK2]);
    setpix(rx+sway,151,PACKED[night?C_BUSH:C_GRASS]);
    setpix(rx+sway+1,151,PACKED[night?C_BUSH:C_GRASS]);
  }
  /* flowers (distinct pixels + tiny ground shadow) */
  setpix(6,147,PACKED[C_PINK]); setpix(7,146,PACKED[C_PINK]); setpix(5,148,mulc(PACKED[C_PINK],150));
  setpix(44,148,PACKED[C_WHITE]); setpix(20,145,PACKED[C_GOLD]); setpix(43,149,mulc(PACKED[C_GOLD],150));
  if(!night){ setpix(48,150,PACKED[C_WHITE]); setpix(12,149,PACKED[C_PINK]); }
  /* dock: 16 voxel plank BOARDS - every board is its own MC plank cube with a
     lit top / textured body / shaded bottom, dark seams between boards, grain
     streaks and nail rows (MC plank blocks) */
  for(int p=0;p<16;p++){
    int x0=p*32;
    vox_wood(x0,170,32,13,d,night?8:16,p);
    if(p>0) fill(x0,170,1,13,mulc(d,38));   /* board seam */
    for(int xx=x0+8;xx<x0+32;xx+=24) if(hash2(xx,p)%3==0) setpix(xx,171,mulc(d,75)); /* nails */
  }
  /* railing posts as lit cubes */
  for(int x=8;x<IN_W;x+=24) vox_cube(x,164,2,6,d2,night?6:12,6,(x>>3)&255);
  /* water pillars: lit top cap + 4 textured cube columns with left-lit /
     right-shaded edges - EVERY column keeps its own per-pixel MC texture
     (the old overdraw flattened the two outer columns into solid bars) */
  for(int x=-30;x<IN_W;x+=64){
    int sx=(x>>4)&255;
    for(int yy=184;yy<IN_H;yy++){
      int prevd=1000;
      for(int k=0;k<4;k++){
        unsigned hc=hash2((k>>1)*13+sx,(yy>>1)*17+((yy/4)&31));
        int d=mc_texel_pct((int)hc);
        d+=(int)(hash2(k*5+sx,yy*3)%3)-1;
        if(k>0 && d==prevd) d=(d>=0)?d+1:d-1;
        prevd=d;
        int e=0;
        if(k==0) e=-10; else if(k==3) e=10;   /* AO left / sunlit right */
        setpix(x+34+k,yy,shade_c(texel_c(d2,d),e));
      }
    }
    vox_cube(x+32,180,8,4,shade_c(d2,12),10,8,sx+31);
  }
  /* post reflections */
  for(int x=-30;x<IN_W;x+=64){
    for(int y=183;y<200;y+=2){
      int wob=((y+(int)(now/230))%4)-2;
      fill(x+34+wob,y,4,1,mulc(d2,120));
    }
  }
}
/* draw player + rod + line + bobber */
static void draw_player(void){
  int px=playerX-16;
  int sc=2;
  int ox=px, oy=124;
  int hx,hy;
  int fishing=(phase==PH_CAST||phase==PH_WAIT||phase==PH_NIBBLE||phase==PH_MISS||phase==PH_CATCHMSG);
  int anim=(state==ST_REEL)?PA_REEL:pAnim;
  draw_person(ox,oy,sc,anim,pAnimT,&hx,&hy);
  /* rod from grip */
  if(fishing||state==ST_REEL){
    int gx=ox+hx*sc, gy=oy+hy*sc;
    float ang;
    int len;
    /* bob arc during cast */
    int drawB=(phase==PH_CAST);
    int bx,by;
    float cp=0.0f;
    if(drawB){ cp=1.0f-(castT/0.5f); if(cp<0)cp=0; if(cp>1)cp=1; bx=playerX+(int)((bobX-playerX)*cp); by=(int)bobY+(int)(sinf(3.14159f*cp)*-70.0f); }
    else { bx=(int)bobX; by=(int)bobY;
      if(phase==PH_NIBBLE){ /* Dave-style: bobber jiggles + dips when biting */
        by+=(int)(sinf(phaseT*26.0f)*2.0f)+1; bx+=(int)(sinf(phaseT*17.0f)*1.0f); } }
    if(state==ST_REEL){ ang=-0.6f; len=34; }
    else { ang=(phase==PH_CAST)?(1.2f-1.8f*cp):(-1.1f); /* wait:nibble: -1.05..-1.25 */
           if(phase==PH_NIBBLE){ ang=-1.05f-0.2f*sinf(phaseT*30); }
           len=36; }
    int tipx=gx+(int)(cosf(ang)*len*sc), tipy=gy+(int)(sinf(ang)*len*sc);
    Pline(ox,oy,sc,gx/sc,gy/sc,tipx/sc,tipy/sc,PACKED[C_ROD]);
    /* line from tip to bob */
    int ly0=(int)tipy, lx0=(int)tipx;
    for(int y=ly0;y<by;y++) setpix(lx0,y,PACKED[C_LINE]);
    /* bobber */
    fill(bx-3,by,7,3,PACKED[C_FLOATA]);
    fill(bx-2,by+1,5,1,PACKED[C_FLOATB]);
  } else {
    /* idle no rod, but still store */
    (void)hx;(void)hy;
  }
}
static void draw_play(void){
  draw_sky(); draw_water();
  draw_ambient();            /* fish are ALWAYS visible cruising the lake */
  draw_dock();
  int drawLine=(phase==PH_CAST||phase==PH_WAIT||phase==PH_NIBBLE||phase==PH_MISS||phase==PH_CATCHMSG);
  if(drawLine) draw_player();
  else draw_player();
  /* Dave-style: fish approaches bobber while waiting, strikes on nibble */
  if(phase==PH_WAIT||phase==PH_NIBBLE) draw_bite_fish();
  if(phase==PH_CATCHMSG&&plannedFish>=0){
    render_fish(plannedFish,0); blit_rgba((IN_W-64)/2,96,1,fsp,64,64);
  }
  char b[64];
  snprintf(b,64,"$%d",coins); draw_text(8,6,b,PACKED[C_YELLOW],1);
  snprintf(b,64,T("LV%d","等%d") ,level); { char bb[64]; snprintf(bb,64,"LV%d",level); draw_text(8,20,bb,PACKED[C_CYAN],1); }
  snprintf(b,64,"%02d:%02d %s",(int)timeH,(int)((timeH-(int)timeH)*60),is_night()?T("NIGHT","夜晚"):T("DAY","白天"));
  draw_text(IN_W-text_w(b,1)-8,6,b,is_night()?PACKED[C_YELLOW]:PACKED[C_WHITE],1);
  snprintf(b,64,T("SPOT:%s","位置:%s"),spot_name(spot)); draw_text(IN_W-text_w(b,1)-8,20,b,PACKED[C_SILVER],1);
  { const char*bs=T("B:SHOP","B:商店"); draw_text(IN_W-text_w(bs,1)-8,34,bs,PACKED[C_SILVER],1); }
  if(phase==PH_IDLE){
    if(is_night()) center_text(224,T("NIGHT: NO FISHING - WAIT FOR DAWN","夜晚不能钓鱼 天亮再来"),PACKED[C_WARN],1);
    else center_text(224,T("SPACE TO CAST","按空格抛竿"),PACKED[C_WHITE],1);
  }
  if(phase==PH_NIBBLE) center_text(224,T("!!! BITE !!!","!!! 鱼咬钩了 !!!"),PACKED[C_WARN],1);
  if(phase==PH_CATCHMSG){ if(plannedFish>=0){ char s[48]; snprintf(s,48,T("CAUGHT %s!","捕获 %s!"),lang?FISHES[plannedFish].cn:FISHES[plannedFish].en); center_text(164,s,PACKED[C_GOLD],2); } }
}

/* ============================================================
   LAN MULTIPLAYER  -- 同湖共钓 + 共享排行榜 (UDP 无头同步)
   ------------------------------------------------------------
   一人创建房间(HOST)，好友加入(JOIN)。所有人站在同一片湖岸
   各自钓鱼，实时看到彼此，钓到的鱼上报主机，主机汇总全房间
   累计价值并广播共享排行榜。
   协议（一行一条，逗号分列，\n 结尾）：
     客户端>主机  JOIN  skin,hair,hcol,shirt,pants,label
     客户端>主机  STATE x,anim,phase
     客户端>主机  CATCH fishIndex,value
     主机>客户端  YOU   id
     主机>客户端  PJOIN id,sk,h,hc,sh,p,label,x,anim,phase,total,count
     主机>客户端  PLEFT id
     主机>客户端  PSTATE id,x,anim,phase
     主机>客户端  SCORE id,total,count
   ============================================================ */
#define NET_PORT       3317u
#define NET_MAXPLAYERS 8
#define NET_TICK       100u   /* ms between state broadcasts */
#if defined(_WIN32)
typedef SOCKET NETFD;
#else
typedef int NETFD;
#endif
#ifndef INVALID_NETFD
#define INVALID_NETFD (-1)
#endif

typedef struct{
  int   active,isHost,myId;
  int   x,anim,phase;
  int   skin,hair,hcol,shirt,pants;
  char  label[16];
  int   total,count;
  Uint32 lastSeen;
}NETP;

static NETFD net_fd=(NETFD)INVALID_NETFD;
static int   net_role=0;        /* 0=none 1=host 2=client */
static int   net_me=-1;         /* my own player id (-1 = unknown) */
static NETP  net_players[NET_MAXPLAYERS];
static struct sockaddr_in net_peer;                /* client -> host addr */
static struct sockaddr_in net_cliaddr[NET_MAXPLAYERS]; /* host -> client addr */
static Uint32 net_last=0;
static int   net_connected=0;   /* client: YOU received */
/* room menu state */
static int   lanSel=0;          /* 0 create 1 join 2 back */
static int   lanEdit=0;         /* typing an IP */
static char  joinBuf[40];
static void net_close(void);
static void net_bcast_player2(int id,int skip);
static int  net_start_join(const char*ip);
static int  net_start_host(void);

static int net_addr_eq(const struct sockaddr_in*a,const struct sockaddr_in*b){
  return a->sin_port==b->sin_port && a->sin_addr.s_addr==b->sin_addr.s_addr;
}
static void net_sendto(const struct sockaddr_in*dst,const char*line){
  if(net_fd==INVALID_NETFD||!dst)return;
  size_t n=strlen(line); char buf[300];
  if(n+2>sizeof(buf))return;
  memcpy(buf,line,n); buf[n]='\n'; buf[n+1]=0;
  sendto(net_fd,buf,(int)n+1,0,(const struct sockaddr*)dst,sizeof(*dst));
}
#if defined(_WIN32)
static void net_cleanup_wsa(void){ WSACleanup(); }
#endif
static int net_open(unsigned port){
#if defined(_WIN32)
  WSADATA wd; if(WSAStartup(MAKEWORD(2,2),&wd)!=0)return 0;
#endif
  net_fd=(NETFD)socket(AF_INET,SOCK_DGRAM,0);
  if(net_fd==(NETFD)INVALID_NETFD){
#if defined(_WIN32)
    WSACleanup();
#endif
    return 0;
  }
  int one=1; setsockopt(net_fd,SOL_SOCKET,SO_REUSEADDR,(const char*)&one,sizeof(one));
  struct sockaddr_in a; memset(&a,0,sizeof(a));
  a.sin_family=AF_INET; a.sin_addr.s_addr=htonl(INADDR_ANY); a.sin_port=htons(port);
  if(bind(net_fd,(struct sockaddr*)&a,sizeof(a))!=0){ net_close(); return 0; }
#if defined(_WIN32)
  u_long nb=1; ioctlsocket(net_fd,FIONBIO,&nb);
#else
  { int fl=fcntl(net_fd,F_GETFL,0); fcntl(net_fd,F_SETFL,fl|O_NONBLOCK); }
#endif
  return 1;
}
static void net_close(void){
  if(net_fd!=(NETFD)INVALID_NETFD){
#if defined(_WIN32)
    closesocket(net_fd); WSACleanup();
#else
    close(net_fd);
#endif
    net_fd=(NETFD)INVALID_NETFD;
  }
  memset(net_players,0,sizeof(net_players));
  net_role=0; net_connected=0; net_me=-1;
}

/* ---- host helpers ---- */
static void net_bcast(const char*line){
  for(int i=1;i<NET_MAXPLAYERS;i++)
    if(net_players[i].active) net_sendto(&net_cliaddr[i],line);
}
static void net_send_player_to(const struct sockaddr_in*dst,int id){
  NETP*p=&net_players[id]; char b[300];
  snprintf(b,sizeof(b),"PJOIN %d,%d,%d,%d,%d,%d,%s,%d,%d,%d,%d,%d",
     p->myId,p->skin,p->hair,p->hcol,p->shirt,p->pants,p->label,
     p->x,p->anim,p->phase,p->total,p->count);
  net_sendto(dst,b);
}
static void net_bcast_player(int id){ net_bcast_player2(id,0); }
static void net_bcast_player2(int id,int skip){
  char b[300]; NETP*p=&net_players[id];
  snprintf(b,sizeof(b),"PJOIN %d,%d,%d,%d,%d,%d,%s,%d,%d,%d,%d,%d",
     p->myId,p->skin,p->hair,p->hcol,p->shirt,p->pants,p->label,
     p->x,p->anim,p->phase,p->total,p->count);
  for(int i=1;i<NET_MAXPLAYERS;i++)
    if(i!=skip&&net_players[i].active) net_sendto(&net_cliaddr[i],b);
}
static void net_bcast_scores(void){
  for(int i=0;i<NET_MAXPLAYERS;i++) if(net_players[i].active){
    char b[48]; snprintf(b,sizeof(b),"SCORE %d,%d,%d",net_players[i].myId,net_players[i].total,net_players[i].count);
    net_bcast(b);
  }
}
static void net_bcast_pleft(int id){
  char b[24]; snprintf(b,sizeof(b),"PLEFT %d",id); net_bcast(b);
}
static int net_slot_by_addr(const struct sockaddr_in*from){
  for(int i=1;i<NET_MAXPLAYERS;i++)
    if(net_players[i].active && net_addr_eq(&net_cliaddr[i],from)) return i;
  return -1;
}

/* ---- host message handlers ---- */
static void net_host_join(const struct sockaddr_in*from,const char*arg){
  int id=-1; for(int i=1;i<NET_MAXPLAYERS;i++) if(!net_players[i].active){id=i;break;}
  if(id<0)return;                         /* room full */
  int skin=0,hair=0,hcol=1,shirt=0,pants=0; char lab[16]="P";
  sscanf(arg,"%d,%d,%d,%d,%d,%15s",&skin,&hair,&hcol,&shirt,&pants,lab);
  NETP*p=&net_players[id]; memset(p,0,sizeof(*p));
  p->active=1;p->myId=id;p->isHost=0;p->skin=skin;p->hair=hair;
  p->hcol=hcol;p->shirt=shirt;p->pants=pants;
  snprintf(p->label,16,"%s",lab);
  p->x=20+rndi(IN_W-40); p->anim=0; p->phase=0;
  net_cliaddr[id]=*from; p->lastSeen=SDL_GetTicks();
  char b[24]; snprintf(b,sizeof(b),"YOU %d",id); net_sendto(from,b);
  net_bcast_player2(id,id);               /* tell the others about the newcomer */
  for(int j=0;j<NET_MAXPLAYERS;j++)        /* tell the newcomer about everyone */
    if(j!=id&&net_players[j].active) net_send_player_to(from,j);
  net_bcast_scores();
}
static void net_host_state(const struct sockaddr_in*from,const char*arg){
  int i=net_slot_by_addr(from); if(i<0)return;
  int x=net_players[i].x,a=net_players[i].anim,ph=net_players[i].phase;
  sscanf(arg,"%d,%d,%d",&x,&a,&ph);
  net_players[i].x=x; net_players[i].anim=a; net_players[i].phase=ph;
  net_players[i].lastSeen=SDL_GetTicks();
}
static void net_host_catch(const struct sockaddr_in*from,const char*arg){
  int i=net_slot_by_addr(from); if(i<0)return;
  int fi=0,v=0; sscanf(arg,"%d,%d",&fi,&v);
  net_players[i].total+=v; net_players[i].count++;
  net_bcast_scores();
}
static void net_host_tick(void){
  net_players[0].x=playerX; net_players[0].anim=pAnim; net_players[0].phase=phase;
  for(int c=1;c<NET_MAXPLAYERS;c++) if(net_players[c].active){
    struct sockaddr_in*dst=&net_cliaddr[c];
    for(int j=0;j<NET_MAXPLAYERS;j++)
      if(net_players[j].active && j!=c){
        char b[48]; snprintf(b,sizeof(b),"PSTATE %d,%d,%d,%d",
           net_players[j].myId,net_players[j].x,net_players[j].anim,net_players[j].phase);
        net_sendto(dst,b);
      }
  }
  net_bcast_scores();
}
static void net_host_timeout(void){
  Uint32 now=SDL_GetTicks();
  for(int i=1;i<NET_MAXPLAYERS;i++)
    if(net_players[i].active && now-net_players[i].lastSeen>3000u){
      net_bcast_pleft(i); net_players[i].active=0;
      add_toast(T("PLAYER LEFT ROOM","有玩家离开了房间"));
    }
}

/* ---- client handlers ---- */
static void net_cli_you(char*arg){ int id=atoi(arg);
  if(id>=0&&id<NET_MAXPLAYERS){
    memset(&net_players[id],0,sizeof(net_players[id]));
    net_players[id].active=1; net_players[id].myId=id;
    net_players[id].skin=cs_skin; net_players[id].hair=cs_hair;
    net_players[id].hcol=cs_haircol; net_players[id].shirt=cs_shirt;
    net_players[id].pants=cs_pants; strcpy(net_players[id].label,"ME");
    net_players[id].x=playerX;
    net_connected=1; net_me=id;
    add_toast(T("JOINED ROOM!","已加入房间！"));
  }
}
/* ---- client + room-menu layer (was lost in a corrupted edit, re-added) ---- */
static const char* net_next(const char*t){
  const char*c=strchr(t,',');
  return c?c+1:t+(int)strlen(t);
}
static void net_cli_pleft(char*arg){
  int id=atoi(arg);
  if(id>=0&&id<NET_MAXPLAYERS){
    net_players[id].active=0;
    add_toast(T("PLAYER LEFT ROOM","有玩家离开了房间"));
  }
}
static void net_cli_pjoin(char*arg){
  int id=atoi(arg);
  NETP np; memset(&np,0,sizeof(np));
  np.active=1; np.myId=id; np.isHost=(id==0);
  np.skin=0; np.hair=0; np.hcol=1; np.shirt=0; np.pants=0;
  strcpy(np.label,"P"); np.x=-999; np.anim=0; np.phase=0; np.total=0; np.count=0;
  const char*t=net_next(arg);
  np.skin=atoi(t); t=net_next(t);
  np.hair=atoi(t); t=net_next(t);
  np.hcol=atoi(t); t=net_next(t);
  np.shirt=atoi(t); t=net_next(t);
  np.pants=atoi(t); t=net_next(t);
  { const char*c=strchr(t,','); int ln=c?(int)(c-t):(int)strlen(t);
    if(ln>15)ln=15; memcpy(np.label,t,ln); np.label[ln]=0;
    if(c){ t=c+1;
      np.x=atoi(t); t=net_next(t);
      np.anim=atoi(t); t=net_next(t);
      np.phase=atoi(t); t=net_next(t);
      np.total=atoi(t); t=net_next(t);
      np.count=atoi(t);
    } }
  if(id>=0&&id<NET_MAXPLAYERS) net_players[id]=np;
}
static void net_cli_pstate(char*arg){
  int id=atoi(arg);
  if(id<0||id>=NET_MAXPLAYERS||!net_players[id].active||id==net_me)return;
  const char*t=net_next(arg);
  net_players[id].x=atoi(t); t=net_next(t);
  net_players[id].anim=atoi(t); t=net_next(t);
  net_players[id].phase=atoi(t);
}
static void net_cli_score(char*arg){
  int id=atoi(arg);
  if(id<0||id>=NET_MAXPLAYERS)return;
  const char*t=net_next(arg);
  net_players[id].total=atoi(t); t=net_next(t);
  net_players[id].count=atoi(t);
}
static void net_cli_tick(void){
  char b[48]; snprintf(b,sizeof(b),"STATE %d,%d,%d",playerX,pAnim,phase);
  net_sendto(&net_peer,b);
}

/* ---- message dispatch + receive ---- */
static void net_handle_line(const struct sockaddr_in*from,char*line){
  char*e=strchr(line,'\n'); if(e)*e=0;
  char*cmd=line; char*arg=cmd;
  while(*arg&&*arg!=' '&&*arg!='\t')arg++;
  if(*arg){*arg=0;arg++;} else arg="";
  if(net_role==1){
    if(!strcmp(cmd,"JOIN")) net_host_join(from,arg);
    else if(!strcmp(cmd,"STATE")) net_host_state(from,arg);
    else if(!strcmp(cmd,"CATCH")) net_host_catch(from,arg);
  } else if(net_role==2){
    if(!strcmp(cmd,"YOU")) net_cli_you(arg);
    else if(!strcmp(cmd,"PJOIN")) net_cli_pjoin(arg);
    else if(!strcmp(cmd,"PLEFT")) net_cli_pleft(arg);
    else if(!strcmp(cmd,"PSTATE")) net_cli_pstate(arg);
    else if(!strcmp(cmd,"SCORE")) net_cli_score(arg);
  }
}
static void net_recv(void){
  char buf[320];
  for(int k=0;k<32;k++){
    struct sockaddr_in from; socklen_t fl=sizeof(from);
    int n=(int)recvfrom(net_fd,buf,sizeof(buf)-1,0,(struct sockaddr*)&from,&fl);
    if(n<=0)break;
    buf[n]=0; net_handle_line(&from,buf);
  }
}
static void net_update(float dt){
  (void)dt;
  if(net_role==0)return;
  net_recv();
  Uint32 now=SDL_GetTicks();
  if(now-net_last>=NET_TICK){
    net_last=now;
    if(net_role==1) net_host_tick();
    else if(net_role==2&&net_connected) net_cli_tick();
  }
  if(net_role==1) net_host_timeout();
}

/* ---- room start ---- */
static int net_start_host(void){
  if(!net_open(NET_PORT))return 0;
  net_role=1; net_me=0;
  memset(net_players,0,sizeof(net_players));
  NETP*me=&net_players[0];
  me->active=1; me->isHost=1; me->myId=0;
  me->skin=cs_skin; me->hair=cs_hair; me->hcol=cs_haircol;
  me->shirt=cs_shirt; me->pants=cs_pants;
  snprintf(me->label,16,"HOST");
  me->x=playerX; me->lastSeen=SDL_GetTicks();
  net_last=SDL_GetTicks();
  add_toast(T("ROOM OPEN!","房间已开启！"));
  return 1;
}
static int net_start_join(const char*ip){
  unsigned long a=inet_addr(ip);
  if(a==(unsigned long)INADDR_NONE)return 0;
  if(!net_open(NET_PORT))return 0;
  memset(&net_peer,0,sizeof(net_peer));
  net_peer.sin_family=AF_INET; net_peer.sin_port=htons(NET_PORT);
  net_peer.sin_addr.s_addr=a;
  net_role=2; net_me=-1;
  memset(net_players,0,sizeof(net_players));
  net_connected=0; net_last=SDL_GetTicks();
  char b[128]; snprintf(b,sizeof(b),"JOIN %d,%d,%d,%d,%d,ME",
     cs_skin,cs_hair,cs_haircol,cs_shirt,cs_pants);
  net_sendto(&net_peer,b);
  return 1;
}
static int net_in_room(void){ return net_role!=0; }
static void net_report_catch(int fi){
  if(fi<0||fi>=NFISH)return;
  if(net_role==1){
    net_players[0].total+=FISHES[fi].value; net_players[0].count++;
    net_bcast_scores();
  } else if(net_role==2&&net_connected){
    char b[48]; snprintf(b,sizeof(b),"CATCH %d,%d",fi,FISHES[fi].value);
    net_sendto(&net_peer,b);
  }
}

/* ---- draw remote players + shared leaderboard ---- */
static void net_draw(void){
  if(net_role==0)return;
  int me=(net_role==1)?0:net_me;
  if(me>=0) net_players[me].x=playerX;
  for(int i=0;i<NET_MAXPLAYERS;i++){
    if(!net_players[i].active||i==me)continue;
    if(net_players[i].x<-900)continue;
    int s_sk=cs_skin,s_h=cs_hair,s_hc=cs_haircol,s_sh=cs_shirt,s_pa=cs_pants;
    cs_skin=net_players[i].skin; cs_hair=net_players[i].hair;
    cs_haircol=net_players[i].hcol; cs_shirt=net_players[i].shirt;
    cs_pants=net_players[i].pants;
    int ox=net_players[i].x-16; int hx,hy;
    draw_person(ox,124,2,net_players[i].anim,(float)SDL_GetTicks()/1000.0f,&hx,&hy);
    const char*lbl=net_players[i].label;
    draw_text(net_players[i].x-text_w(lbl,1)/2,110,lbl,PACKED[C_YELLOW],1);
    cs_skin=s_sk; cs_hair=s_h; cs_haircol=s_hc; cs_shirt=s_sh; cs_pants=s_pa;
  }
  NETP*ord[NET_MAXPLAYERS]; int n=0;
  for(int i=0;i<NET_MAXPLAYERS;i++) if(net_players[i].active) ord[n++]=&net_players[i];
  for(int a=1;a<n;a++){ NETP*k=ord[a]; int b=a-1;
    while(b>=0&&ord[b]->total<k->total){ ord[b+1]=ord[b]; b--; } ord[b+1]=k; }
  int yy=244; int cnt=0;
  for(int k=0;k<n&&cnt<3;k++){
    char b[64]; snprintf(b,64,"%s $%d x%d",ord[k]->label,ord[k]->total,ord[k]->count);
    draw_text(8,yy,b,(me>=0&&ord[k]==&net_players[me])?PACKED[C_GOLD]:PACKED[C_WHITE],1);
    yy+=13; cnt++;
  }
  if(n>3){ char b[24]; snprintf(b,24,"+%d MORE",n-3); draw_text(8,yy,b,PACKED[C_SILVER],1); }
  char s[32]; snprintf(s,32,"LAN %s (port %u)",net_role==1?"HOST":"CLIENT",NET_PORT);
  draw_text(IN_W-text_w(s,1)-8,50,s,PACKED[C_CYAN],1);
}

/* ---- room menu ---- */
static void lan_menu_open(void){ lanSel=0; lanEdit=0; joinBuf[0]=0; state=ST_LANMENU; net_close(); }
static void update_lanmenu(void){
  if(lanEdit)return;                 /* text input handled in event loop */
  if(press.up&&lanSel>0)lanSel--;
  if(press.down&&lanSel<2)lanSel++;
  if(press.accept){
    if(lanSel==0){ if(net_start_host()){ state=ST_PLAY; phase=PH_IDLE; phaseT=0; } }
    else if(lanSel==1){ lanEdit=1; joinBuf[0]=0; }
    else state=ST_TITLE;
  }
  if(press.back)state=ST_TITLE;
}
static void draw_lanmenu(void){
  draw_cg_pan(1,((int)(SDL_GetTicks()/80))%IN_W);
  darkall(120);
  center_text(12,T("LAN MULTIPLAYER","局域网联机"),PACKED[C_GOLD],2);
  center_text(28,T("SAME LAKE, LIVE TOGETHER","同湖共钓 实时在线"),PACKED[C_CYAN],1);
  if(lanEdit){
    center_text(70,T("ENTER HOST IP","输入主机IP"),PACKED[C_SILVER],1);
    { int w=text_w(joinBuf,2); int bx=(IN_W-w)/2;
      draw_text(bx,90,joinBuf,PACKED[C_WHITE],2);
      if((SDL_GetTicks()/400)%2) draw_text(bx+w,90,"_",PACKED[C_GOLD],2); }
    center_text(140,T("ENTER JOIN  ESC BACK   NUMBERS & .","回车加入  退出返回  数字和点"),PACKED[C_SILVER],1);
    return;
  }
  const char*op[3]={ T("CREATE ROOM","创建房间"),T("JOIN ROOM","加入房间"),T("BACK","返回") };
  int y=78;
  for(int i=0;i<3;i++){
    Uint32 c=(i==lanSel)?PACKED[C_GOLD]:PACKED[C_WHITE];
    int w=text_w(op[i],1); draw_text((IN_W-w)/2,y,op[i],c,2);
    if(i==lanSel) draw_text((IN_W-w)/2-26,y,">",PACKED[C_GOLD],2);
    y+=34;
  }
  center_text(222,T("UP/DOWN SELECT  SPACE OK","上下选择 空格确认"),PACKED[C_SILVER],1);
  char pb[24]; snprintf(pb,24,T("PORT %u","端口 %u"),NET_PORT);
  center_text(236,pb,PACKED[C_SILVER],1);
}

/* ---------- shop ---------- */
static void shop_links(void){
  SHOPITEMS[1].var=&rodLevel;SHOPITEMS[2].var=&lureLevel;
  SHOPITEMS[3].var=&netLevel;SHOPITEMS[4].var=&boat;
}
static void update_shop(void){
  if(press.up&&menuSel>0)menuSel--;
  if(press.down&&menuSel<NSHOP-1)menuSel++;
  if(press.accept){ MENUITEM*m=&SHOPITEMS[menuSel];
    if(menuSel==0){ if(bagFill>0){int sum=0;for(int i=0;i<bagFill;i++)sum+=FISHES[bag[i]].value;coins+=sum*modCoinMul;bagFill=0;add_toast(T("SOLD","已出售"));save_game();}
      else add_toast(T("NOTHING TO SELL","没有可卖的")); }
    else if(*m->var<m->maxown){ if(coins>=m->cost){coins-=m->cost;(*m->var)++;add_toast(T("PURCHASED","已购买"));save_game();} else add_toast(T("NOT ENOUGH COINS","金币不足")); }
    else add_toast(T("ALREADY MAX","已是最大")); }
  if(press.back)state=ST_PLAY;
}
/* ---------- shopkeeper: Dave-style boss sprite + counter + chatter ---------- */
static int keeperLine=-1;
static const char* KEEPER_EN[4]={"FRESH CATCH? GOOD PRICE!","THE LAKE KEEPS SECRETS...","TRY DEEP WATER, KID.","STORM BREWING TONIGHT."};
static const char* KEEPER_CN[4]={"有新鲜的鱼吗？好价钱！","这片湖藏着秘密……","去深水试试吧，孩子。","今晚有暴风雨。"};
static void draw_shopkeeper(int ox,int oy,int sc,float t){
  int br=(int)(sinf(t*1.6f)*1.0f);            /* breathing */
  int blink=(((int)(t*0.9f))%5)==0;
  int talk=(((int)(t*2.2f))%2)==0;
  Uint32 skin=packrgb(228,182,142), beard=packrgb(214,214,222), shirt=packrgb(96,104,118),
        cap=packrgb(196,70,58), apron=packrgb(76,116,186), dark=packrgb(16,16,20),
        wood=PACKED[C_DOCK], wood2=PACKED[C_DOCK2];
  /* beret */
  P(ox,oy,sc, 8,1+br,26,4,cap); P(ox,oy,sc, 6,4+br,30,2,cap);
  P(ox,oy,sc, 8,1+br,26,1,mulc(cap,190));
  P(ox,oy,sc, 20,0+br,2,2,cap); /* beret stem */
  /* face */
  P(ox,oy,sc, 12,5+br,18,12,skin);
  P(ox,oy,sc, 12,5+br,18,1,mulc(skin,215));
  /* bushy brows */
  P(ox,oy,sc, 15,8+br,6,2,beard); P(ox,oy,sc, 22,8+br,6,2,beard);
  /* eyes (blink) */
  if(!blink){
    P(ox,oy,sc, 16,11+br,4,3,packrgb(252,252,252));
    P(ox,oy,sc, 23,11+br,4,3,packrgb(252,252,252));
    P(ox,oy,sc, 17,12+br,2,2,dark); P(ox,oy,sc, 24,12+br,2,2,dark);
  }else{
    P(ox,oy,sc, 16,12+br,4,1,dark); P(ox,oy,sc, 23,12+br,4,1,dark);
  }
  /* nose */
  P(ox,oy,sc, 20,11+br,2,3,mulc(skin,178));
  /* huge beard hiding mouth (moves when talking) */
  P(ox,oy,sc, 12,15+br,18,6,beard);
  P(ox,oy,sc, 14,14+br,14,2,beard);
  P(ox,oy,sc, 12,20+br,18,2,mulc(beard,150));
  if(talk) P(ox,oy,sc, 20,13+br,2,1,dark);
  /* rotund torso */
  P(ox,oy,sc, 8,21+br,26,20,shirt);
  P(ox,oy,sc, 8,21+br,26,1,mulc(shirt,200));
  /* apron with pocket */
  P(ox,oy,sc, 13,23+br,16,17,apron);
  P(ox,oy,sc, 15,25+br,12,2,mulc(apron,185));
  P(ox,oy,sc, 16,32+br,10,6,mulc(apron,140));
  /* arms resting on counter */
  P(ox,oy,sc, 4,26+br,5,11,shirt); P(ox,oy,sc, 33,26+br,5,11,shirt);
  P(ox,oy,sc, 4,36+br,5,3,skin);   P(ox,oy,sc, 33,36+br,5,3,skin);
  /* counter with wood grain + display fish */
  P(ox,oy,sc, 2,40,40,3,wood);
  P(ox,oy,sc, 2,40,40,1,mulc(wood,185));
  for(int k=0;k<10;k++) setpix(ox+(4+k*4)*sc/2,oy+41*sc/2,wood2);
  P(ox,oy,sc, 3,43,3,10,wood2); P(ox,oy,sc, 38,43,3,10,wood2);
  /* little fish on counter */
  fill(ox+14*sc,oy+38*sc,10*sc/2,3*sc/2,mulc(PACKED[C_CYAN],200));
  setpix(ox+13*sc,oy+39*sc,PACKED[C_WHITE]);
}
static void draw_shop(void){
  fill(0,0,IN_W,IN_H,PACKED[C_DIALOG]);
  /* warm interior backdrop */
  fill_grad(0,0,IN_W,60,packrgb(90,70,52),packrgb(60,46,36));
  for(int x=16;x<IN_W-10;x+=48){ /* wall planks */
    fill(x,10,2,50,mulc(packrgb(70,54,40),160));
  }
  center_text(8,T("PIER SUPPLIES","码头商店"),PACKED[C_GOLD],2);
  /* shopkeeper + counter on the left */
  float t=(float)SDL_GetTicks()/1000.0f;
  draw_shopkeeper(30,92,2,t);
  if(keeperLine<0)keeperLine=rndi(4);
  /* speech bubble */
  {
    const char*line=T(KEEPER_EN[keeperLine],KEEPER_CN[keeperLine]);
    int bw=text_w(line,1)+14;
    int bx=118,by=116;
    fill(bx,by,bw,20,packrgb(250,248,240));
    fill(bx,by,bw,1,packrgb(180,176,164)); fill(bx,by+19,bw,1,packrgb(180,176,164));
    fill(bx,by,1,20,packrgb(180,176,164)); fill(bx+bw-1,by,1,20,packrgb(180,176,164));
    draw_text(bx+7,by+6,line,PACKED[C_DARK],1);
    /* bubble tail */
    fill(bx-4,by+12,4,4,packrgb(250,248,240));
    setpix(bx-6,by+16,packrgb(250,248,240));
  }
  char c[32]; snprintf(c,32,T("COINS %d","金币 %d"),coins); draw_text(28,30,c,PACKED[C_YELLOW],1);
  /* menu on the right */
  for(int i=0;i<NSHOP;i++){ MENUITEM*m=&SHOPITEMS[i];
    int y=50+i*22; int maxed=(m->var&&*m->var>=m->maxown); int sel=(i==menuSel);
    Uint32 col=maxed?PACKED[C_GREY]:(sel?PACKED[C_GOLD]:PACKED[C_WHITE]);
    if(sel) fill(216,y-2,IN_W-216-8,16,mulc(PACKED[C_BLACK],150));
    draw_text(222,y,lang?m->cname:m->name,col,1);
    if(m->var){char b[24];if(maxed)snprintf(b,24,"%s",T("MAX","已满"));else snprintf(b,24,"$%d",m->cost);draw_text(340,y,b,col,1);}
    if(sel)draw_text(210,y+1,">",PACKED[C_GOLD],1);
  }
  center_text(198,T("UP/DOWN SELECT  SPACE BUY  ESC BACK","上/下选择  空格购买  退出返回"),PACKED[C_SILVER],1);
  center_text(214,T("SELL FISH FIRST.  BUY BOAT >> DEEP LEGENDS","先卖鱼。 买船后可钓深水传说"),PACKED[C_SILVER],1);
}

/* ---------- bag ---------- */
static void update_bag(void){ if(press.back||press.accept)state=ST_PLAY; }
static void draw_bag(void){
  fill(0,0,IN_W,IN_H,PACKED[C_DIALOG]);
  center_text(6,T("FISHER'S LOG","渔夫日志"),PACKED[C_GOLD],2);
  char b[96];
  snprintf(b,96,T("COINS %d  LEVEL %d  EXP %d/%d","金币 %d  等级 %d  经验 %d/%d"),coins,level,xp,level*20);
  draw_text(24,20,b,PACKED[C_YELLOW],1);
  snprintf(b,96,T("DAY %d  CHEST %d/%d  ROD%d LURE%d NET%d  BOAT:%s",
      "第%d天  渔获 %d/%d  竿%d 饵%d 网%d  船:%s"),day,bagFill,bagSize(),rodLevel,lureLevel,netLevel,boat?"Y":"N");
  draw_text(24,34,b,lang?PACKED[C_CYAN]:PACKED[C_WHITE],1);
  /* 64x64 grid */
  int tile=64, gap=12;
  int x0=(IN_W-(5*tile+4*gap))/2;
  for(int i=0;i<NFISH;i++){
    int colx=i%5, rowy=i/5;
    int tx=x0+colx*(tile+gap), ty=58+rowy*(tile+34);
    int caught=caughtCount[i]>0;
    render_fish(i,caught?0:1);
    blit_rgba(tx,ty,1,fsp,64,64);
    if(!caught) dim_overlay(tx-1,ty-1,tile+2,tile+2);
    Uint32 namec=caught?PACKED[C_WHITE]:PACKED[C_GREY];
    draw_text(tx+ (tile-text_w(FISHES[i].cn,1))/2, ty+tile+2, FISHES[i].cn, namec,1);
    { char v[12]; snprintf(v,12,"$%d",FISHES[i].value);
      draw_text((tx+tile)/2-text_w(v,1)/2, ty+tile+10, v, caught?PACKED[C_YELLOW]:PACKED[C_GREY],1); }
  }
  center_text(266,T("ESC: BACK TO LAKE","返回:回到湖边"),PACKED[C_SILVER],1);
}

/* ---------- intro / CG (+title lang) ---------- */
static const char* IN_EN[5]={
  "A STORM TOOK GRANDPA'S BOAT...",
  "GRANDPA ALWAYS SAID: 'THE LAKE KEEPS SECRETS.'",
  "HE LEFT YOU THE OLD BAMBOO ROD...",
  "AND ONE LAST PROMISE: 'GUARD THIS LAKE.'",
  "DAY 1 - YOUR LEGACY BEGINS"};
static const char* IN_CN[5]={
  "一场风暴卷走了爷爷的小船……",
  "爷爷常说:\"这片湖藏着秘密。\"",
  "他留给你那根旧竹竿……",
  "还有最后一个承诺:\"守护这片湖。\"",
  "第1天——你的传奇从此开始"};
#define SLIDE_T 4.0f
static int scene_for_slide(int s){ return s<2?0:(s==2?2:(s==3?3:1)); }
static void draw_rain(float t){
  Uint32 rain2=mulc(PACKED[C_CYAN],110);
  for(int i=0;i<40;i++){ int x=(i*47+(int)(t*90))%IN_W; int y=(i*29+(int)(t*300))%170;
    setpix(x,y,PACKED[C_CYAN]); setpix(x+1,y+1,rain2); }
}
static void draw_lightning_rnd(float t){
  int flash=((int)(t*1.7f))%6;
  if(flash==3||flash==5){ int x=120+(rnd()%40); setpix(x,70,PACKED[C_WHITE]); int y=70; int px=x;
    while(y<178){ if(rndi(2))setpix(px,y,PACKED[C_WHITE]); px+=rndi(5)-2; y+=6; for(int k=-1;k<=1;k++)setpix(px+k,y,PACKED[C_WHITE]);} }
}
static void update_intro(float dt){
  introT+=dt;
  slide=(int)(introT/SLIDE_T);
  if(slide>4||press.accept){ add_toast(lang?T("MOVE:ARROWS CAST:SPACE","移动:方向键  抛竿:空格"):T("MOVE:ARROWS CAST:SPACE","移动:方向键  抛竿:空格"));
    state=ST_PLAY;phase=PH_IDLE;phaseT=0; }
}
static void draw_intro(void){
  int s=clampij(slide,0,4);
  float prog=introT-s*SLIDE_T;
  int scene=scene_for_slide(s);
  int xoff=(s<2)?((int)(introT*90)%20):(s==4?((int)(introT*20)%8):0);
  draw_cg_pan(scene,xoff);
  /* fade in + fade out at slide end (film-style transition) */
  float fade=prog/0.6f; if(fade<1) darkall((int)((1-fade)*255));
  float rem=SLIDE_T-prog; if(rem<0.45f) darkall((int)((0.45f-rem)*2.2f*230));
  /* storm fx */
  if(s<2){
    if(((int)(introT*1.2f))%4==3&&((s==1)?((int)introT)%12>6:1)){ fill_grad(0,0,IN_W,110,PACKED[C_WHITE],PACKED[C_FLOATB]); }
    draw_lightning_rnd(introT);
    draw_rain(introT);
  }
  /* silhouette overlays */
  if(s==1){ /* grandfather on pier */
    fill(170,132,26,30,PACKED[C_BLACK]); fill(160,140,18,14,PACKED[C_BLACK]);
    fill(196,134,28,2,PACKED[C_BLACK]); fill(196,137,20,1,PACKED[C_BLACK]);
  }
  else if(s==2){ /* grandpa at sunset on pier */
    fill(60,130,24,30,PACKED[C_BLACK]); fill(50,138,16,14,PACKED[C_BLACK]);
    fill(84,132,30,2,PACKED[C_BLACK]); fill(84,135,22,1,PACKED[C_BLACK]);
  }
  else if(s==4){ /* hero + rod on dock */
    fill(150,150,10,10,PACKED[C_BLACK]);
  }
  fill(0,220,IN_W,64,PACKED[C_DIALOG]);
  if(s<4){ center_text(246,T(IN_EN[s],IN_CN[s]),PACKED[C_WHITE],1); }
  else{ center_text(236,T(IN_EN[s],IN_CN[s]),PACKED[C_GOLD],2);
        center_text(254,T("PRESS SPACE","按空格继续"),PACKED[C_SILVER],1); }
}
/* ---------- title with language select ---------- */
static void update_title(void){
  if(press.up)lang=0;
  if(press.down)lang=1;
  if(press.accept){csSel=0;state=ST_CUSTOM;}
}
/* ---------- character customization screen (Terraria-style) ---------- */
static const char* HAIR_EN[4]={"SHORT","LONG","PONYTAIL","BALD"};
static const char* HAIR_CN[4]={"短发","长发","马尾","光头"};
static void update_custom(void){
  if(press.up&&csSel>0)csSel--;
  if(press.down&&csSel<5)csSel++;
  if(press.left){
    switch(csSel){case 0:cs_skin=(cs_skin+5)%6;break;case 1:cs_hair=(cs_hair+3)%4;break;
      case 2:cs_haircol=(cs_haircol+5)%6;break;case 3:cs_shirt=(cs_shirt+7)%8;break;
      case 4:cs_pants=(cs_pants+3)%4;break;default:break;}
  }
  if(press.right){
    switch(csSel){case 0:cs_skin=(cs_skin+1)%6;break;case 1:cs_hair=(cs_hair+1)%4;break;
      case 2:cs_haircol=(cs_haircol+1)%6;break;case 3:cs_shirt=(cs_shirt+1)%8;break;
      case 4:cs_pants=(cs_pants+1)%4;break;default:break;}
  }
  if(press.accept){
    if(csSel==5){slide=0;introT=0;state=ST_INTRO;}
    else csSel=5;
  }
  if(press.back)state=ST_TITLE;
}
static void draw_custom(void){
  draw_cg_pan(1,(SDL_GetTicks()/80)%IN_W);
  darkall(110);
  fill(0,0,IN_W,IN_H,PACKED[C_DIALOG]);
  darkall(30);
  center_text(10,T("CREATE YOUR ANGLER","打造你的钓手"),PACKED[C_GOLD],2);
  /* live preview: walking puppet at 4x, mirrored panels */
  int pvx=(IN_W/4)*3-40, pvy=90;
  fill(pvx-14,pvy-16,96,116,mulc(PACKED[C_BLACK],170));
  float t=(float)SDL_GetTicks()/1000.0f;
  draw_person(pvx,pvy,4,PA_WALK,t,NULL,NULL);
  draw_person(pvx-120,pvy,4,PA_IDLE,t,NULL,NULL);
  draw_person(pvx+120,pvy,4,PA_CAST,t,NULL,NULL);
  /* option rows */
  const char* names[6];
  int vals[6],maxs[6];
  names[0]=T("SKIN","肤色"); vals[0]=cs_skin; maxs[0]=6;
  names[1]=T("HAIR","发型"); vals[1]=cs_hair; maxs[1]=4;
  names[2]=T("HAIR COLOR","发色"); vals[2]=cs_haircol; maxs[2]=6;
  names[3]=T("SHIRT","上衣"); vals[3]=cs_shirt; maxs[3]=8;
  names[4]=T("PANTS","裤子"); vals[4]=cs_pants; maxs[4]=4;
  names[5]=T("BEGIN ADVENTURE","开始冒险"); vals[5]=-1; maxs[5]=0;
  int ry=64;
  for(int i=0;i<6;i++){
    Uint32 col=(i==csSel)?PACKED[C_GOLD]:PACKED[C_WHITE];
    draw_text(40,ry+i*22,names[i],col,1);
    if(i==csSel) draw_text(26,ry+i*22+1,">",PACKED[C_GOLD],1);
    if(vals[i]>=0){
      /* swatch */
      Uint32 sw;
      switch(i){case 0:sw=skin_c();break;case 1:sw=PACKED[C_GREY];break;
        case 2:sw=hair_c();break;case 3:sw=shirt_c();break;default:sw=pants_c();}
      fill(150,ry+i*22,10,10,sw);
      fill(150,ry+i*22,10,1,mulc(sw,200)); fill(150,ry+i*22+9,10,1,mulc(sw,120));
      /* value text: hair shows style name */
      if(i==1) draw_text(166,ry+i*22,T(HAIR_EN[cs_hair],HAIR_CN[cs_hair]),col,1);
      else{ char b[16]; snprintf(b,16,"%d/%d",vals[i]+1,maxs[i]); draw_text(166,ry+i*22,b,col,1); }
      draw_text(140,ry+i*22,"<",PACKED[C_SILVER],1);
      draw_text(200,ry+i*22,">",PACKED[C_SILVER],1);
    }
  }
  center_text(214,T("UP/DOWN SELECT  LEFT/RIGHT CHANGE  SPACE OK","上下选择 左右更换 空格确认"),PACKED[C_SILVER],1);
  center_text(228,T("ESC BACK TO TITLE","退出返回标题"),PACKED[C_SILVER],1);
}
static void draw_title(void){
  int xoff=((int)(SDL_GetTicks()/40))%IN_W;
  draw_cg_pan(1,xoff); /* dawn */
  darkall(60);
  draw_text(46,16,T("PIXEL LAKE HEART","像素湖心"),PACKED[C_WHITE],3);
  draw_text(46,56,T("A LOW-END FISHING ADVENTURE","一款低配钓鱼冒险"),PACKED[C_CYAN],1);
  /* language menu */
  int lx= (IN_W-76)/2;
  int ly=120;
  const char* op0= T("ENGLISH  [1]","英文  [1]");
  const char* op1= T("CHINESE  [2]","中文  [2]");
  int w0=text_w(op0,1), w1=text_w(op1,1);
  draw_text(lx+ (76-w0)/2, ly, op0, lang==0?PACKED[C_GOLD]:PACKED[C_WHITE],1);
  draw_text(lx+ (76-w1)/2, ly+16, op1, lang==1?PACKED[C_GOLD]:PACKED[C_WHITE],1);
  if(lang==0) draw_text(lx-10,ly+3,">",PACKED[C_GOLD],1); else draw_text(lx-10,ly+19,">",PACKED[C_GOLD],1);
  center_text(176,(SDL_GetTicks()/320)%2? T("PRESS SPACE / [1][2]","按空格 或 [1][2]"):T("PRESS SPACE / [1][2]","按空格 或 [1][2]"),PACKED[C_GOLD],2);
  center_text(200,T("ARROWS: SELECT LANGUAGE","方向键 选择语言"),PACKED[C_SILVER],1);
  center_text(214,T("WIN7-WIN10 X64  PIXEL MODE","WIN7-WIN10 64位 像素模式"),PACKED[C_SILVER],1);
  center_text(228,T("F: FULLSCREEN  ARROWS: MOVE","F键全屏  方向键移动"),PACKED[C_SILVER],1);
}

/* ---------- dispatchers ---------- */
static void update_top(float dt){
  if(toastT>0){ toastT-=dt; if(toastT<0)toastT=0; }
  net_update(dt);
  if(state==ST_PLAY){
    if(press.e){state=ST_BAG;menuSel=0;}
    if(press.b){state=ST_SHOP;menuSel=0;keeperLine=-1;}
  }
  switch(state){
    case ST_TITLE: update_title(); break;
    case ST_CUSTOM: update_custom(); break;
    case ST_INTRO: update_intro(dt); break;
    case ST_PLAY: update_play(dt); break;
    case ST_REEL: update_reel(dt); break;
    case ST_SHOP: update_shop(); break;
    case ST_BAG: update_bag(); break;
    case ST_LANMENU: update_lanmenu(); break;
    default: break;
  }
}
static void draw_top(void){
  g_nightlift=is_night()?45:0;   /* brighter night so scene is never muddy/dark */
  switch(state){
    case ST_TITLE: draw_title(); break;
    case ST_CUSTOM: draw_custom(); break;
    case ST_INTRO: draw_intro(); break;
    case ST_PLAY: draw_play(); net_draw(); break;
    case ST_REEL: draw_reel(); break;
    case ST_SHOP: draw_shop(); break;
    case ST_BAG: draw_bag(); break;
    case ST_LANMENU: draw_lanmenu(); break;
    default: break;
  }
  if(toastT>0){fill(0,264,IN_W,22,PACKED[C_BLACK]);center_text(272,toast,PACKED[C_YELLOW],1);}
}

/* ---------- automated visual self-test (SELFTEST builds only) ---------- */
#ifdef SELFTEST
#ifdef _WIN32
#include <direct.h>
static int st_mkdir(const char*p){ return _mkdir(p); }
#else
#include <sys/stat.h>
static int st_mkdir(const char*p){ return mkdir(p,0755); }
#endif
static void st_render(SDL_Renderer*ren,SDL_Texture*tex){
  SDL_LockSurface(screen);
  scr=(Uint32*)screen->pixels;scrpitch=screen->pitch/4;
  memset(scr,0,(size_t)(scrpitch*PH_H)*sizeof(Uint32));
  draw_top();
  SDL_UnlockSurface(screen);
  SDL_UpdateTexture(tex,NULL,screen->pixels,screen->pitch);
  SDL_SetRenderDrawColor(ren,0,0,0,255);
  SDL_RenderClear(ren);
  SDL_RenderCopy(ren,tex,NULL,NULL);
  SDL_RenderPresent(ren);
}
static void st_shot(const char*name){
  char p[160]; snprintf(p,160,"shots/%s.bmp",name);
  if(SDL_SaveBMP(screen,p)!=0) printf("SAVE FAIL %s: %s\n",p,SDL_GetError());
  else printf("shot %s\n",p);
}
static void st_wait(SDL_Renderer*ren,SDL_Texture*tex,float sec){
  Uint32 t0=SDL_GetTicks();
  while(SDL_GetTicks()-t0<(Uint32)(sec*1000.0f)){ st_render(ren,tex); SDL_Delay(16); }
}
static void selftest(SDL_Window*win,SDL_Renderer*ren,SDL_Texture*tex){
  st_mkdir("shots");
  printf("physical canvas: %dx%d\n",PH_W,PH_H);
  /* 1) title EN + CN */
  state=ST_TITLE; lang=0; st_wait(ren,tex,0.4f); st_shot("01_title_en");
  lang=1; st_wait(ren,tex,0.4f); st_shot("02_title_cn");
  /* 1b) character customization (Terraria-style) */
  state=ST_CUSTOM; lang=0; csSel=1; cs_skin=2; cs_hair=2; cs_haircol=4; cs_shirt=3; cs_pants=1;
  st_wait(ren,tex,0.3f); st_shot("03_custom_en");
  lang=1; st_wait(ren,tex,0.3f); st_shot("03b_custom_cn");
  /* 2) intro slides (CG + storm fx) */
  state=ST_INTRO; lang=0;
  for(int s=0;s<5;s++){ slide=s; introT=s*4.0f+2.0f; st_wait(ren,tex,0.3f);
    char nm[32]; snprintf(nm,32,"intro_%d",s); st_shot(nm); }
  lang=1; slide=0; introT=2.0f; st_wait(ren,tex,0.3f); st_shot("03c_intro0_cn");
  /* 3) play day / idle (Stardew-style scenery) */
  state=ST_PLAY; phase=PH_IDLE; pAnim=PA_IDLE; pAnimT=0.5f;
  timeH=14.0f; playerX=120; st_wait(ren,tex,0.3f); st_shot("04_play_day");
  /* 3b) dawn + dusk lighting */
  timeH=6.5f; st_wait(ren,tex,0.3f); st_shot("04b_play_dawn");
  timeH=17.5f; st_wait(ren,tex,0.3f); st_shot("04c_play_dusk");
  /* 4) play night */
  timeH=21.0f; st_wait(ren,tex,0.3f); st_shot("05_play_night");
  /* 5) casting arc (mid-cast) */
  timeH=14.0f; phase=PH_CAST; pAnim=PA_CAST; castT=0.25f; bobX=340; bobY=190;
  st_wait(ren,tex,0.1f); st_shot("06_casting");
  /* 6) waiting: Dave-style fish approaching the bobber */
  phase=PH_WAIT; pAnim=PA_WAIT; phaseT=1.2f; bobX=330; bobY=185;
  plannedFish=6; nibbleDelay=3.0f;
  st_wait(ren,tex,0.3f); st_shot("07_waiting");
  /* 7) nibble: fish strikes, bobber jiggles */
  phase=PH_NIBBLE; phaseT=0.3f; st_wait(ren,tex,0.15f); st_shot("08_nibble");
  /* 8) catch message w/ 64x64 fish */
  phase=PH_CATCHMSG; plannedFish=7; phaseT=0.5f;
  st_wait(ren,tex,0.3f); st_shot("09_catch");
  lang=1; st_wait(ren,tex,0.2f); st_shot("10_catch_cn");
  /* 9a) reel: weak fish (big slow zone) */
  state=ST_REEL; pAnim=PA_REEL; plannedFish=0; reelDiff=0; reelTol=3;
  reelInd=120; reelTarget=150; reelZoneW=86; reelGood=1; reelNeed=3; reelBad=0;
  st_wait(ren,tex,0.3f); st_shot("11a_reel_easy");
  /* 9b) reel: boss fish (tiny fast zone) */
  plannedFish=9; reelDiff=4; reelTol=2;
  reelInd=300; reelTarget=340; reelZoneW=30; reelGood=2; reelNeed=7; reelBad=1;
  st_wait(ren,tex,0.3f); st_shot("11b_reel_hard");
  /* 10) walk anim */
  state=ST_PLAY; phase=PH_IDLE; pAnim=PA_WALK; pAnimT=0.42f;
  st_wait(ren,tex,0.1f); st_shot("12_walk");
  /* 11) shop EN/CN with Dave-style keeper */
  state=ST_SHOP; menuSel=1; lang=0; st_wait(ren,tex,0.3f); st_shot("13_shop_en");
  lang=1; st_wait(ren,tex,0.3f); st_shot("14_shop_cn");
  /* 12) bag with all 10 fish (64x64 art grid) */
  state=ST_BAG; bagFill=10;
  for(int i=0;i<10;i++){bag[i]=i;caughtCount[i]=1;}
  lang=0; st_wait(ren,tex,0.3f); st_shot("15_bag_en");
  lang=1; st_wait(ren,tex,0.3f); st_shot("16_bag_cn");
  /* 13) save/load + mod round-trip self-check (SELFTEST builds only) */
  printf("== SAVE/MOD CHECK ==\n");
  coins=1234; xp=77; level=5; day=12; cs_skin=3; bagFill=2; bag[0]=1; bag[1]=9;
  save_game();
  coins=0; xp=0; level=1; day=1; cs_skin=0; bagFill=0;
  load_game();
  printf("save/load: coins=%d xp=%d level=%d day=%d skin=%d bag=%d (expect 1234 77 5 12 3 2)\n",
         coins,xp,level,day,cs_skin,bagFill);
  { FILE*mf=fopen("/tmp/_selftest.mod","w");
    fprintf(mf,"# test\nfish 0 value=99 exp=7 diff=1\nmult coins=10 xp=7\nshop ROD cost=5\n");
    fclose(mf);
    int v0=FISHES[0].value,d0=FISHES[0].diff,cm=modCoinMul,xm=modXpMul,rod=SHOPITEMS[1].cost;
    mod_load_file("/tmp/_selftest.mod");
    printf("mod: fish0 value %d->%d | diff %d->%d | mult coins %d->%d xp %d->%d | ROD %d->%d (expect 3->99 0->1 1->10 1->7 30->5)\n",
           v0,FISHES[0].value,d0,FISHES[0].diff,cm,modCoinMul,xm,modXpMul,rod,SHOPITEMS[1].cost);
    remove("/tmp/_selftest.mod");
    FILE*spr=fopen("/tmp/_selftest_sprite.mod","w");
    fprintf(spr,"sprite 0\n");
    for(int r=0;r<64;r++){ for(int c=0;c<64;c++) fputc(r==0?'1':'.',spr); fputc('\n',spr); }
    fclose(spr);
    unsigned char b0=fsp_pix[0][0];
    mod_load_file("/tmp/_selftest_sprite.mod");
    printf("sprite: fsp_pix[0][0] %d->%d | fsp_pix[0][64]=%d (expect ->1 .. 255)\n",
           b0,fsp_pix[0][0],(int)fsp_pix[0][64]);
    remove("/tmp/_selftest_sprite.mod");
    /* restore defaults (mods are re-applied on every real launch, not persisted) */
    coins=1234; xp=77; level=5; day=12; cs_skin=3;
  }
  printf("SELFTEST DONE\n");
  (void)win;
}
#endif

int main(int argc,char*argv[]){
  (void)argc;(void)argv;
  if(SDL_Init(SDL_INIT_VIDEO)<0)return 1;
  SDL_Window*win=SDL_CreateWindow("PIXEL LAKE HEART - A Low-end Fishing Game",
      SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,WIND_W,WIND_H,SDL_WINDOW_RESIZABLE);
  if(!win){SDL_Quit();return 1;}
  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY,"0");
  SDL_Renderer*ren=SDL_CreateRenderer(win,-1,0);
  if(!ren){SDL_Surface*ws=SDL_GetWindowSurface(win);ren=SDL_CreateSoftwareRenderer(ws);}
  if(!ren){SDL_DestroyWindow(win);SDL_Quit();return 1;}
  screen=SDL_CreateRGBSurfaceWithFormat(0,PH_W,PH_H,32,SDL_PIXELFORMAT_ARGB8888);
  if(!screen){SDL_DestroyRenderer(ren);SDL_DestroyWindow(win);SDL_Quit();return 1;}
  SDL_Texture*tex=SDL_CreateTexture(ren,SDL_PIXELFORMAT_ARGB8888,SDL_TEXTUREACCESS_STREAMING,PH_W,PH_H);
  for(int i=0;i<NPAL;i++)PACKED[i]=SDL_MapRGBA(screen->format,PALRGB[i].r,PALRGB[i].g,PALRGB[i].b,255);
  prep_cg();
  prep_fishspr();
  seed_rng((unsigned)SDL_GetTicks()^((unsigned)time(NULL)<<8));
  ambient_init();
  shop_links();
  state=ST_TITLE;menuSel=0;slide=-1;introT=0;lang=0;
#ifdef SELFTEST
  selftest(win,ren,tex);
  SDL_DestroyTexture(tex);SDL_FreeSurface(screen);
  SDL_DestroyRenderer(ren);SDL_DestroyWindow(win);SDL_Quit();
  return 0;
#endif

  load_game();
  mod_scan_and_load();

  int running=1;Uint32 tprev=SDL_GetTicks();
  while(running){
    Uint32 tnow=SDL_GetTicks();
    float dt=(tnow-tprev)/1000.0f; if(dt>0.1f)dt=0.1f; tprev=tnow;
    memset(&press,0,sizeof(press));
    SDL_Event ev;
    while(SDL_PollEvent(&ev)){
      if(ev.type==SDL_QUIT)running=0;
      else if(ev.type==SDL_KEYDOWN){
        if(state==ST_LANMENU&&lanEdit){
          SDL_Scancode sc=ev.key.keysym.scancode;
          SDL_Keycode sy=ev.key.keysym.sym;
          if(sc==SDL_SCANCODE_ESCAPE) lanEdit=0;
          else if(sc==SDL_SCANCODE_RETURN||sc==SDL_SCANCODE_KP_ENTER||sc==SDL_SCANCODE_SPACE){
            if(joinBuf[0]&&net_start_join(joinBuf)){ state=ST_PLAY; phase=PH_IDLE; phaseT=0; } }
          else if(sc==SDL_SCANCODE_BACKSPACE){ int l=(int)strlen(joinBuf); if(l>0)joinBuf[l-1]=0; }
          else if(sy>=SDLK_0&&sy<=SDLK_9){ if(strlen(joinBuf)<15){ int c=sy; char s[2]={(char)c,0}; strcat(joinBuf,s);} }
          else if(sy==SDLK_PERIOD||sy==SDLK_KP_PERIOD){ if(strlen(joinBuf)<15)strcat(joinBuf,"."); }
          break; /* consume */
        }
        switch(ev.key.keysym.scancode){
          case SDL_SCANCODE_SPACE: case SDL_SCANCODE_RETURN: press.space=1;press.accept=1;break;
          case SDL_SCANCODE_ESCAPE: press.back=1;break;
          case SDL_SCANCODE_W: case SDL_SCANCODE_UP: press.up=1;break;
          case SDL_SCANCODE_S: case SDL_SCANCODE_DOWN: press.down=1;break;
          case SDL_SCANCODE_E: press.e=1;break;
          case SDL_SCANCODE_B: press.b=1;break;
          case SDL_SCANCODE_M: if(state==ST_TITLE) lan_menu_open(); break;
          case SDL_SCANCODE_1: if(state==ST_TITLE)lang=0;break;
          case SDL_SCANCODE_2: if(state==ST_TITLE)lang=1;break;
          case SDL_SCANCODE_LEFT: press.left=1;break;
          case SDL_SCANCODE_RIGHT: press.right=1;break;
          case SDL_SCANCODE_F:
            if(SDL_GetWindowFlags(win)&SDL_WINDOW_FULLSCREEN_DESKTOP)SDL_SetWindowFullscreen(win,0);
            else SDL_SetWindowFullscreen(win,SDL_WINDOW_FULLSCREEN_DESKTOP);
            break;
          default:break;
        }
      }
      else if(ev.type==SDL_MOUSEBUTTONDOWN&&ev.button.button==SDL_BUTTON_LEFT){press.space=1;press.accept=1;}
    }
    update_top(dt);
    SDL_LockSurface(screen);
    scr=(Uint32*)screen->pixels;scrpitch=screen->pitch/4;
    memset(scr,0,(size_t)(scrpitch*PH_H)*sizeof(Uint32));
    draw_top();
    SDL_UnlockSurface(screen);
    SDL_UpdateTexture(tex,NULL,screen->pixels,screen->pitch);
    /* ---- screen adaptation: crisp integer-scale letterbox ----
       1) big ~16:9 window (>=1080p): fill whole window (1:1 at 1920x1080,
          uniform logic-blocks otherwise) - no wasted pixels in fullscreen;
       2) any other window: integer multiple of the 512x288 logic grid,
          centered with black bars (never a non-integer blurry scale). */
    int ww,wh;SDL_GetWindowSize(win,&ww,&wh);
    SDL_Rect dst;
    float aspect=(float)ww/(float)wh;
    if(ww>=PH_W&&wh>=PH_H&&fabsf(aspect-16.0f/9.0f)<0.04f){
      dst.x=0;dst.y=0;dst.w=ww;dst.h=wh;
    }else{
      int k=(int)fminf(ww/(float)IN_W,wh/(float)IN_H);
      if(k<1)k=1;
      dst.w=IN_W*k;dst.h=IN_H*k;
      dst.x=(ww-dst.w)/2;dst.y=(wh-dst.h)/2;
    }
    SDL_SetRenderDrawColor(ren,0,0,0,255);
    SDL_RenderClear(ren);
    SDL_RenderCopy(ren,tex,NULL,&dst);
    SDL_RenderPresent(ren);
    int wait=(int)(FRAME_MS-(int)(SDL_GetTicks()-tnow)); if(wait>0)SDL_Delay((Uint32)wait);
  }
  save_game();
  net_close();
  if(tex)SDL_DestroyTexture(tex);
  SDL_FreeSurface(screen);SDL_DestroyRenderer(ren);SDL_DestroyWindow(win);SDL_Quit();
  return 0;
}

/* Windows GUI-subsystem entry wrapper (SDL_MAIN_HANDLED: main stays main) */
#ifdef _WIN32
#include <windows.h>
int WINAPI WinMain(HINSTANCE hI,HINSTANCE hP,LPSTR cmd,int nShow){
  (void)hI;(void)hP;(void)cmd;(void)nShow;
  SDL_SetMainReady();
  return main(__argc,(char**)__argv);
}
#endif