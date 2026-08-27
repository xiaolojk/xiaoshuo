/* ============================================================
   PIXEL LAKE HEART  -  a low-end fishing game for Win7/Win10 x64
   Engine: SDL2, pure software pixel-art
   v2: CG backdrops (Seedream) + 64x64 fish art + 30-frame
   character animation + fishing motions + EN/CN bilingual
   + 32x32 CJK font
   ============================================================ */
#define SDL_MAIN_HANDLED  /* we provide our own entry point on Windows */
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "cjk_font_32x32.h"
#include "cg_scenes.h"

#define IN_W 512     /* logic grid (all layout code) */
#define IN_H 288
#define PH_W 1920    /* true physical canvas - native 1080p */
#define PH_H 1080
#define FPS 60
#define FRAME_MS (1000/FPS)
/* logic->phys exact grid map: 1920/512 = 1080/288 = 15/4.
   A logic pixel (x,y) covers phys [x*15/4,(x+1)*15/4) - seamless, no drift. */
#define M2P(v) (((v)*15)>>2)

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
static int WIND_W=1920,WIND_H=1080;

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
/* ---------- CJK 32x32 ---------- */
static int draw_cjk_char(int x,int y,unsigned cp,Uint32 col,int scale){
  int idx=cjk_find(cp);
  if(idx<0){for(int r=0;r<24;r++)for(int b=0;b<24;b++){
    int on=(r==0||r==23||b==0||b==23);
    if(on)for(int a=0;a<scale;a++)for(int e=0;e<scale;e++)setpix(x+b*scale+e,y+r*scale+a,col);
    }return 25*scale;}
  for(int r=0;r<32;r++){const unsigned char*row=CJK_BITS[idx][r];
    for(int b=0;b<32;b++)if(row[b])for(int a=0;a<scale;a++)for(int e=0;e<scale;e++)setpix(x+b*scale+e,y+r*scale+a,col);}
  return 33*scale;
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
    if(is_utf8_cjk(c)){ n+=(cjk_find(utf8_cp((const unsigned char*)s))>=0?33:25)*scale; s+=3; }
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
  RGB3 body,belly,fin,acc; int pat; int cx,rx,ry;
}FISH;
static const FISH FISHES[10]={
 /*en            cn       val exp wt reg t dif body         belly        fin          acc             pat cx rx ry*/
 {"CARP","鲤鱼",    3,1,52,0,0,0,{214,150,60},{246,214,120},{150,105,55},{110,80,40},      1,38,26,18},
 {"PERCH","鲈鱼",   5,1,46,0,0,0,{120,170,92},{232,222,158},{92,132,70},{58,88,45},       2,38,26,17},
 {"SILVER CARP","银鲤",9,2,34,1,0,1,{192,212,226},{238,246,250},{152,172,192},{112,132,152},1,38,26,17},
 {"CATFISH","鲶鱼",12,2,28,1,2,1,{124,112,100},{212,202,186},{92,86,78},{66,60,54},       0,38,26,16},
 {"LARGEMOUTH","大口鲈",14,2,26,1,1,2,{92,152,96},{228,226,180},{72,122,82},{40,92,56},   2,38,26,16},
 {"BLUEGILL","蓝鳃鱼",22,3,16,1,1,2,{122,162,122},{232,152,90},{102,142,112},{72,112,82}, 0,36,24,15},
 {"SALMON","鲑鱼", 30,4,18,2,2,3,{202,192,182},{238,152,142},{182,172,166},{212,92,92},    1,38,26,16},
 {"GOLD TROUT","金鳟",40,4,12,2,1,3,{236,182,72},{246,212,132},{202,132,52},{222,70,60},   3,38,26,15},
 {"ELECTRIC EEL","电鳗",55,5,9,2,2,4,{72,82,122},{112,127,172},{52,62,96},{240,230,80},    4,30,33,10},
 {"LAKE DRAGON","湖龙",150,8,5,2,2,4,{62,152,162},{142,202,202},{122,72,202},{62,240,214}, 4,34,30,16},
};
#define NFISH 10
static Uint32 fsp[64*64];
static void sp_put(int x,int y,Uint32 c){ if(x>=0&&x<64&&y>=0&&y<64)fsp[y*64+x]=c; }
static void sp_hline(int x0,int x1,int y,Uint32 c){ for(int x=x0;x<=x1;x++)sp_put(x,y,c); }

/* Deterministic per-fish helper: shade body column top->bottom */
static Uint32 mix3(Uint32 a,Uint32 b,float t){
  int r0=a>>16&0xFF,g0=a>>8&0xFF,b0=a&0xFF,r1=b>>16&0xFF,g1=b>>8&0xFF,b1=b&0xFF;
  return packrgb((int)(r0+(r1-r0)*t),(int)(g0+(g1-g0)*t),(int)(b0+(b1-b0)*t));
}
/* Exquisite 64x64 fish art: gradient body, overlapping scales, gill cover,
   lateral line, 3 tail shapes, whiskers (catfish), dorsal/anal/pelvic fins. */
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
enum{ST_TITLE,ST_CUSTOM,ST_INTRO,ST_PLAY,ST_REEL,ST_SHOP,ST_BAG,ST_QUIT};
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
static int rodLevel=0,lureLevel=0,boat=0,lantern=0,netLevel=0;
static int bagFill=0,bag[64]; static int caughtCount[NFISH];
static float toastT=-1; static char toast[48];
static float introT; static int slide=-1; static int menuSel=0;

typedef struct{Uint8 space,accept,back,up,down,e,b,left,right;}PRESS;
static PRESS press;

static int bagSize(void){ return 8+netLevel*4; }
static void add_toast(const char*t){ strncpy(toast,t,47);toast[47]=0;toastT=2.0f; }
static const char* spot_name(int r){ return r==0?T("SHORE","岸边"):(r==1?T("PIER","码头"):T("DEEP-WATER","深水")); }
static int is_night(void){ return (timeH<6||timeH>=19); }

/* ---------- player figure (30-frame parametric) ---------- */
static void P(int ox,int oy,int sc,int x,int y,int w,int h,Uint32 c){
  for(int yy=y;yy<y+h;yy++)for(int xx=x;xx<x+w;xx++)
    for(int a=0;a<sc;a++)for(int b=0;b<sc;b++)setpix(ox+xx*sc+b,oy+yy*sc+a,c);
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
        if(bagFill<bagSize()){bag[bagFill++]=ok;caughtCount[ok]++;xp+=FISHES[ok].exp;caughtToday++;}
        else add_toast(T("CHEST FULL - SELL IN SHOP","背包已满 - 去商店出售"));
        if(xp>=level*20){xp-=level*20;level++;add_toast(T("LEVEL UP!","升级了!"));}
        state=ST_PLAY;phase=PH_CATCHMSG;phaseT=0;
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
  /* pick anim */
  int fishing=(phase==PH_CAST||phase==PH_WAIT||phase==PH_NIBBLE||phase==PH_MISS||phase==PH_CATCHMSG);
  if(fishing){ pAnim=(phase==PH_CAST)?PA_CAST:(phase==PH_MISS?PA_WAIT:PA_WAIT); }
  else if(moving){ pAnim=PA_WALK; }
  else pAnim=PA_IDLE;

  if(phase==PH_IDLE){ if(press.space){
      if(!is_night()||(lantern&&boat)) cast_line();
      else add_toast(is_night()?T("NIGHT - BUY LANTERN","夜晚 - 购买灯笼"):T("GET GEAR AT B:SHOP","请在商店购买装备"));
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
  fill(x,y,7*s,3,c);
  fill(x+s,y-1,3*s,2,c); fill(x+2*s,y-2,2*s,1,c);
  fill(x+s+4*s,y-1,2*s,2,c);
  fill(x+s,y+2,5*s,1,mulc(c,150));
}
static void hills(Uint32 c,int amp,int base,float seed){
  for(int x=0;x<IN_W;x++){
    int h=base+(int)(sinf(x*0.013f+seed)*amp+sinf(x*0.037f+seed*2.0f)*amp*0.4f);
    fill(x,150-h,1,h,c);
  }
}
static void draw_tree(int x,int ground,int night){
  Uint32 trunk=PACKED[night?C_TRUNK:C_DOCK2], leaf=PACKED[night?C_BUSH:C_GRASS];
  fill(x-2,ground-36,5,36,trunk);
  fill(x+1,ground-32,2,28,mulc(trunk,160));
  /* branch */
  fill(x-6,ground-30,4,2,trunk); fill(x+4,ground-34,4,2,trunk);
  /* 3 foliage clusters with highlights */
  int cs[3][3]={{x-10,ground-46,13},{x+2,ground-55,16},{x+12,ground-44,11}};
  for(int i=0;i<3;i++){
    int cx=cs[i][0],cy=cs[i][1],r=cs[i][2];
    fill(cx-r/2,cy-r/2,r,r,leaf);
    fill(cx-r/2+2,cy-r/2-2,r-4,3,mulc(leaf,185));
    fill(cx-r/2,cy+r/2-2,r,2,mulc(leaf,130));
  }
}
static void draw_sky(void){
  int night=is_night();
  Uint32 skytop,skybot;
  if(night){ skytop=packrgb(6,10,30); skybot=packrgb(26,38,78); }
  else if(timeH<8){ /* dawn: peach horizon */
    skytop=packrgb(64,110,168); skybot=packrgb(246,178,120);
  } else if(timeH>=17){ /* dusk: violet-orange */
    skytop=packrgb(72,52,112); skybot=packrgb(236,120,86);
  } else { skytop=packrgb(84,158,222); skybot=packrgb(168,224,238); }
  fill_grad(0,0,IN_W,150,skytop,skybot);
  Uint32 now=SDL_GetTicks();
  if(night){
    /* stars with twinkle */
    for(int i=0;i<24;i++){
      if((i+(int)(now/560))%6!=0)
        setpix(STARS[i][0],STARS[i][1],(i%3)?PACKED[C_WHITE]:packrgb(200,210,255));
      if(i%7==0) setpix(STARS[i][0]+1,STARS[i][1]+1,packrgb(120,130,180));
    }
    /* moon: disc + crescent bite */
    float nf=(timeH>=19)?(timeH-19.0f)/10.0f:((timeH+5.0f)/10.0f);
    int mx=(int)(50+nf*(IN_W-110)), my=44;
    fill(mx-5,my-5,11,11,packrgb(228,232,242));
    fill(mx-3,my-6,7,13,packrgb(228,232,242));
    fill(mx-6,my-3,13,7,packrgb(228,232,242));
    fill(mx-4,my-4,8,8,packrgb(246,248,252));
    fill(mx-3,my-3,6,6,mulc(skytop,190)); /* crescent bite */
    fill(mx+3,my-5,3,3,packrgb(190,196,210)); /* craters */
    fill(mx-2,my+2,2,2,packrgb(190,196,210));
    sunX=mx; sunY=my;
  } else {
    /* sun arc: rises 6:00, sets 18:00 */
    float dayf=(timeH-6.0f)/12.0f;
    if(dayf<0){dayf=0;} if(dayf>1){dayf=1;}
    int sx=(int)(46+dayf*(IN_W-100));
    int sy=(int)(96-fabsf(dayf-0.5f)*2*78);
    Uint32 sc=(timeH<8||timeH>=17)?packrgb(250,150,80):packrgb(255,214,90);
    /* glow rings */
    fill(sx-6,sy-6,13,13,mulc(sc,120));
    fill(sx-4,sy-4,9,9,mulc(sc,170));
    fill(sx-3,sy-3,7,7,sc);
    fill(sx-1,sy-1,3,3,mix3(sc,packrgb(255,255,240),0.6f));
    /* rays */
    setpix(sx,sy-8,sc);setpix(sx,sy+8,sc);setpix(sx-8,sy,sc);setpix(sx+8,sy,sc);
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
  /* haze band at horizon */
  fill(0,142,IN_W,8,mulc(skybot,160));
}
static void draw_water(void){
  int night=is_night();
  Uint32 topc,botc;
  if(night){ topc=packrgb(22,40,86); botc=packrgb(8,14,40); }
  else if(timeH<8){ topc=packrgb(214,150,120); botc=packrgb(40,70,120); }
  else if(timeH>=17){ topc=packrgb(190,100,100); botc=packrgb(30,44,90); }
  else { topc=packrgb(84,176,220); botc=packrgb(20,84,150); }
  fill_grad(0,150,IN_W,120,topc,botc);
  Uint32 now=SDL_GetTicks();
  /* shimmer: horizontal light streaks drifting */
  Uint32 shim=night?packrgb(70,110,190):mix3(topc,packrgb(255,255,255),0.55f);
  for(int i=0;i<70;i++){
    int seedx=(i*97+((int)(now/140)))%IN_W;
    int y=153+(i*37)%114;
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
  /* left bank: grass, tree, reeds, flowers */
  fill(0,150,54,22,PACKED[night?C_BUSH:C_GRASS]);
  fill(0,150,54,2,mulc(PACKED[night?C_BUSH:C_GRASS],185));
  draw_tree(34,172,night);
  /* reeds swaying */
  Uint32 now=SDL_GetTicks();
  for(int i=0;i<5;i++){
    int rx=8+i*9;
    int sway=(int)(sinf(now*0.0015f+i)*2);
    Pline(0,0,1,rx,170,rx+sway,152,PACKED[night?C_TRUNK:C_DOCK2]);
    setpix(rx+sway,151,PACKED[night?C_BUSH:C_GRASS]);
    setpix(rx+sway+1,151,PACKED[night?C_BUSH:C_GRASS]);
  }
  /* flowers */
  setpix(6,147,PACKED[C_PINK]); setpix(7,146,PACKED[C_PINK]);
  setpix(44,148,PACKED[C_WHITE]); setpix(20,145,PACKED[C_GOLD]);
  if(!night){ setpix(48,150,PACKED[C_WHITE]); setpix(12,149,PACKED[C_PINK]); }
  /* dock planks */
  fill(0,170,IN_W,13,d);
  fill(0,170,IN_W,1,mulc(d,185));
  for(int x=0;x<IN_W;x+=64){setpix(x,170,d2);setpix(x+1,170,d2);setpix(x,171,d2);}
  for(int x=-30;x<IN_W;x+=64)fill(x+34,183,4,IN_H-183,d2);
  for(int x=8;x<IN_W;x+=24)fill(x,164,2,4,d2);
  /* post reflections */
  for(int x=-30;x<IN_W;x+=64){
    for(int y=183;y<200;y+=2){
      int wob=((y+(int)(now/230))%4)-2;
      fill(x+34+wob,y,4,1,mulc(d2,120));
    }
  }
}
static void draw_fish_shape(int x,int y,int len,Uint32 color){
  int h=(len/3)+3;
  fill(x,y-len/2,len,h,color);
  fill(x-4,y-h/2,4,h,PACKED[C_SHADOW]);
  setpix(x,y,PACKED[C_WHITE]);
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
    else { bx=(int)bobX; by=(int)bobY; }
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
  draw_sky(); draw_water(); draw_dock();
  int drawLine=(phase==PH_CAST||phase==PH_WAIT||phase==PH_NIBBLE||phase==PH_MISS||phase==PH_CATCHMSG);
  if(drawLine) draw_player();
  else draw_player();
  if(phase==PH_WAIT){ int fx=90+((int)(phaseT*60))%(IN_W-160); draw_fish_shape(fx,(int)bobY+14,18,PACKED[C_CYAN]); }
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
    if(is_night()&&!(lantern&&boat)) center_text(224,T("NIGHT - BUY LANTERN","夜晚 - 请购买灯笼"),PACKED[C_WARN],1);
    else center_text(224,T("SPACE TO CAST","按空格抛竿"),PACKED[C_WHITE],1);
  }
  if(phase==PH_NIBBLE) center_text(224,T("!!! BITE !!!","!!! 鱼咬钩了 !!!"),PACKED[C_WARN],1);
  if(phase==PH_CATCHMSG){ if(plannedFish>=0){ char s[48]; snprintf(s,48,T("CAUGHT %s!","捕获 %s!"),lang?FISHES[plannedFish].cn:FISHES[plannedFish].en); center_text(164,s,PACKED[C_GOLD],2); } }
}

/* ---------- shop ---------- */
typedef struct{const char*name;int cost;int maxown;int*var;const char*cname;}MENUITEM;
static MENUITEM SHOPITEMS[6]={
  {"SELL ALL",0,0,0,"全部出售"},
  {"ROD",30,3,0,"鱼竿"},
  {"LURE",40,3,0,"鱼饵"},
  {"NET",50,3,0,"渔网"},
  {"BOAT",150,1,0,"小船"},
  {"LANTERN",45,1,0,"灯笼"},
};
#define NSHOP 6
static void shop_links(void){
  SHOPITEMS[1].var=&rodLevel;SHOPITEMS[2].var=&lureLevel;
  SHOPITEMS[3].var=&netLevel;SHOPITEMS[4].var=&boat;SHOPITEMS[5].var=&lantern;
}
static void update_shop(void){
  if(press.up&&menuSel>0)menuSel--;
  if(press.down&&menuSel<NSHOP-1)menuSel++;
  if(press.accept){ MENUITEM*m=&SHOPITEMS[menuSel];
    if(menuSel==0){ if(bagFill>0){int sum=0;for(int i=0;i<bagFill;i++)sum+=FISHES[bag[i]].value;coins+=sum;bagFill=0;add_toast(T("SOLD","已出售"));}
      else add_toast(T("NOTHING TO SELL","没有可卖的")); }
    else if(*m->var<m->maxown){ if(coins>=m->cost){coins-=m->cost;(*m->var)++;add_toast(T("PURCHASED","已购买"));} else add_toast(T("NOT ENOUGH COINS","金币不足")); }
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
  snprintf(b,96,T("DAY %d  CHEST %d/%d  ROD%d LURE%d NET%d  BOAT:%s  LANTERN:%s",
      "第%d天  渔获 %d/%d  竿%d 饵%d 网%d  船:%s  灯:%s"),day,bagFill,bagSize(),rodLevel,lureLevel,netLevel,boat?"Y":"N",lantern?"Y":"N");
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
  center_text(200,T("ARROWS ?? SELECT LANGUAGE","方向键 选择语言"),PACKED[C_SILVER],1);
  center_text(214,T("WIN7-WIN10 X64  PIXEL MODE","WIN7-WIN10 64位 像素模式"),PACKED[C_SILVER],1);
  center_text(228,T("F11 FULLSCREEN  ARROWS MOVE","F11全屏  方向键移动"),PACKED[C_SILVER],1);
}

/* ---------- dispatchers ---------- */
static void update_top(float dt){
  if(toastT>0){ toastT-=dt; if(toastT<0)toastT=0; }
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
    default: break;
  }
}
static void draw_top(void){
  switch(state){
    case ST_TITLE: draw_title(); break;
    case ST_CUSTOM: draw_custom(); break;
    case ST_INTRO: draw_intro(); break;
    case ST_PLAY: draw_play(); break;
    case ST_REEL: draw_reel(); break;
    case ST_SHOP: draw_shop(); break;
    case ST_BAG: draw_bag(); break;
    default: break;
  }
  if(toastT>0){fill(0,264,IN_W,22,PACKED[C_BLACK]);center_text(272,toast,PACKED[C_YELLOW],1);}
}

/* ---------- automated visual self-test (SELFTEST builds only) ---------- */
#ifdef SELFTEST
#include <sys/stat.h>
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
  mkdir("shots",0755);
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
  /* 6) waiting + fish shadow */
  phase=PH_WAIT; pAnim=PA_WAIT; phaseT=1.2f; bobX=330; bobY=185;
  st_wait(ren,tex,0.3f); st_shot("07_waiting");
  /* 7) nibble */
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
  seed_rng((unsigned)SDL_GetTicks()^((unsigned)time(NULL)<<8));
  shop_links();
  state=ST_TITLE;menuSel=0;slide=-1;introT=0;lang=0;
#ifdef SELFTEST
  selftest(win,ren,tex);
  SDL_DestroyTexture(tex);SDL_FreeSurface(screen);
  SDL_DestroyRenderer(ren);SDL_DestroyWindow(win);SDL_Quit();
  return 0;
#endif

  int running=1;Uint32 tprev=SDL_GetTicks();
  while(running){
    Uint32 tnow=SDL_GetTicks();
    float dt=(tnow-tprev)/1000.0f; if(dt>0.1f)dt=0.1f; tprev=tnow;
    memset(&press,0,sizeof(press));
    SDL_Event ev;
    while(SDL_PollEvent(&ev)){
      if(ev.type==SDL_QUIT)running=0;
      else if(ev.type==SDL_KEYDOWN){
        switch(ev.key.keysym.scancode){
          case SDL_SCANCODE_SPACE: case SDL_SCANCODE_RETURN: press.space=1;press.accept=1;break;
          case SDL_SCANCODE_ESCAPE: press.back=1;break;
          case SDL_SCANCODE_W: case SDL_SCANCODE_UP: press.up=1;break;
          case SDL_SCANCODE_S: case SDL_SCANCODE_DOWN: press.down=1;break;
          case SDL_SCANCODE_E: press.e=1;break;
          case SDL_SCANCODE_B: press.b=1;break;
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
    int ww,wh;SDL_GetWindowSize(win,&ww,&wh);
    float sc=(float)(ww<wh?ww/IN_W:wh/IN_H); if(sc<0.5f)sc=0.5f;
    SDL_Rect dst={(ww-(int)(IN_W*sc))/2,(wh-(int)(IN_H*sc))/2,(int)(IN_W*sc),(int)(IN_H*sc)};
    SDL_SetRenderDrawColor(ren,0,0,0,255);
    SDL_RenderClear(ren);
    SDL_RenderCopy(ren,tex,NULL,&dst);
    SDL_RenderPresent(ren);
    int wait=(int)(FRAME_MS-(int)(SDL_GetTicks()-tnow)); if(wait>0)SDL_Delay((Uint32)wait);
  }
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