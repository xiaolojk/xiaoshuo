/* ============================================================
   PIXEL LAKE HEART  -  a low-end fishing game for Win7/Win10 x64
   Engine: SDL2, pure software pixel-art
   v2: CG backdrops (Seedream) + 64x64 fish art + 30-frame
   character animation + fishing motions + EN/CN bilingual
   + 32x32 CJK font
   ============================================================ */
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "cjk_font_32x32.h"
#include "cg_scenes.h"

#define IN_W 512
#define IN_H 288
#define FPS 60
#define FRAME_MS (1000/FPS)

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
static int WIND_W=1280,WIND_H=720;

static SDL_Surface*screen; static Uint32*scr; static int scrpitch;

static inline void setpix(int x,int y,Uint32 c){
  if(x>=0&&x<IN_W&&y>=0&&y<IN_H) scr[y*scrpitch+x]=c;
}
static void fill(int x,int y,int w,int h,Uint32 c){
  int x0=x<0?0:x,x1=x+w>IN_W?IN_W:x+w,y0=y<0?0:y,y1=y+h>IN_H?IN_H:y+h;
  for(int yy=y0;yy<y1;yy++){Uint32*row=scr+yy*scrpitch;for(int xx=x0;xx<x1;xx++)row[xx]=c;}
}
static void fill_grad(int x,int y,int w,int h,Uint32 c0,Uint32 c1){
  int r0=c0>>16&0xFF,g0=c0>>8&0xFF,b0=c0&0xFF;
  int r1=c1>>16&0xFF,g1=c1>>8&0xFF,b1=c1&0xFF;
  for(int yy=0;yy<h;yy++){
    float t=h<=1?1:(float)yy/(h-1);
    Uint32 col=((Uint32)(r0+(r1-r0)*t)<<16)|((Uint32)(g0+(g1-g0)*t)<<8)|((Uint32)(b0+(b1-b0)*t));
    for(int xx=0;xx<w;xx++) setpix(x+xx,y+yy,col);
  }
}
static Uint32 packrgb(int r,int g,int b){ return 0xFF000000u|((Uint32)r<<16)|((Uint32)g<<8)|(Uint32)b; }
static void blit_spr(int dx,int dy,int scale,const Uint32*packed,const Uint8*d,int w,int h){
  for(int sy=0;sy<h;sy++)for(int sx=0;sx<w;sx++){
    Uint8 idx=d[sy*w+sx]; if(idx==255) continue;
    Uint32 c=packed[idx];
    for(int a=0;a<scale;a++)for(int b=0;b<scale;b++) setpix(dx+sx*scale+b,dy+sy*scale+a,c);
  }
}
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
static unsigned long rngstate;
static void seed_rng(unsigned s){ rngstate=(unsigned long)s*2654435761u+0x9E3779B9UL; }
static unsigned rnd(void){ rngstate=rngstate*6364136223846793005UL+1; return (unsigned)(rngstate>>32); }
static int rndi(int n){ return n<=0?0:(int)(rnd()%n); }
static float rndf(void){ return (float)((double)(rnd()%1000000)/1000000.0); }

/* ---------- fish table + 64x64 art ---------- */
static Uint32 colr(RGB3 c){ return packrgb(c.r,c.g,c.b); }
static Uint32 mulc(Uint32 c,int p){ return packrgb((c>>16&0xFF)*p/255,(c>>8&0xFF)*p/255,(c&0xFF)*p/255); }
static Uint32 dimc(Uint32 c){ return packrgb((c>>16&0xFF)*4/5,(c>>8&0xFF)*4/5,(c&0xFF)*4/5); }

typedef struct{
  const char*en; const char*cn; int value,exp,weight,region,time;
  RGB3 body,belly,fin,acc; int pat; int cx,rx,ry;
}FISH;
static const FISH FISHES[10]={
 /*en            cn       val exp wt reg t  body         belly        fin          acc             pat cx rx ry*/
 {"CARP","鲤鱼",    3,1,52,0,0,{214,150,60},{246,214,120},{150,105,55},{110,80,40},      1,38,26,18},
 {"PERCH","鲈鱼",   5,1,46,0,0,{120,170,92},{232,222,158},{92,132,70},{58,88,45},       2,38,26,17},
 {"SILVER CARP","银鲤",9,2,34,1,0,{192,212,226},{238,246,250},{152,172,192},{112,132,152},1,38,26,17},
 {"CATFISH","鲶鱼",12,2,28,1,2,{124,112,100},{212,202,186},{92,86,78},{66,60,54},       0,38,26,16},
 {"LARGEMOUTH","大口鲈",14,2,26,1,1,{92,152,96},{228,226,180},{72,122,82},{40,92,56},   2,38,26,16},
 {"BLUEGILL","蓝鳃鱼",22,3,16,1,1,{122,162,122},{232,152,90},{102,142,112},{72,112,82}, 0,36,24,15},
 {"SALMON","鲑鱼", 30,4,18,2,2,{202,192,182},{238,152,142},{182,172,166},{212,92,92},    1,38,26,16},
 {"GOLD TROUT","金鳟",40,4,12,2,1,{236,182,72},{246,212,132},{202,132,52},{222,70,60},   3,38,26,15},
 {"ELECTRIC EEL","电鳗",55,5,9,2,2,{72,82,122},{112,127,172},{52,62,96},{240,230,80},    4,30,33,10},
 {"LAKE DRAGON","湖龙",150,8,5,2,2,{62,152,162},{142,202,202},{122,72,202},{62,240,214}, 4,34,30,16},
};
#define NFISH 10
static Uint32 fsp[64*64];
static void sp_put(int x,int y,Uint32 c){ if(x>=0&&x<64&&y>=0&&y<64)fsp[y*64+x]=c; }
static void render_fish(int sp,int dim){
  for(int i=0;i<64*64;i++)fsp[i]=0;
  const FISH*F=&FISHES[sp];
  int cx=F->cx,rx=F->rx,ry=F->ry,cy=32;
  int snout=cx-rx+2, tail=cx+rx; if(tail>63)tail=63;
  Uint32 body=colr(F->body),belly=colr(F->belly),fin=colr(F->fin),acc=colr(F->acc);
  if(dim){ body=dimc(body);belly=dimc(belly);fin=dimc(fin);acc=dimc(acc); }
  Uint32 dark=packrgb(12,12,12);
  for(int x=snout;x<=tail;x++){
    float hh = x<=cx ? 2+((x-snout)/(float)(cx-snout))*(ry-1)
                     : ry*(1-0.86f*((x-cx)/(float)(tail-cx)));
    int h=(int)hh;
    for(int y=cy-h;y<=cy+h;y++){
      Uint32 c=body;
      if(y>cy+h/3) c=belly;
      if(F->pat==1&&(x%5==0)&&(y%5==2)) c=mulc(body,150);
      else if(F->pat==2&&((x-snout)%8)<3) c=mulc(body,155);
      else if(F->pat==3){ int dx=x-12,dy=y-cy; if(dx*dx+dy*dy<10) c=acc; }
      else if(F->pat==4&&(x%11==0)&&(y%17==0)) c=acc;
      sp_put(x,y,c);
    }
  }
  /* tail fin fan */
  int tfx=clampij(tail,0,58);
  for(int dy=-5;dy<=5;dy++){ int ad=dy<0?-dy:dy; int wid=5-ad/2; if(wid<1)wid=1;
    for(int k=1;k<=wid;k++) sp_put(clampij(tfx+k,0,63),clampij(cy+dy,0,63),fin); }
  /* dorsal fin */
  int dfx=cx-9;
  for(int k=0;k<5;k++)for(int d=1;d<=4;d++) sp_put(clampij(dfx+k,0,63),clampij(cy-ry-d,0,63),fin);
  /* pectoral fin */
  for(int k=0;k<6;k++) sp_put(clampij(cx-12+(k/2),0,63),clampij(cy+3+k,0,63),fin);
  /* mouth */
  sp_put(clampij(snout,0,63),cy,dark); sp_put(clampij(snout+1,0,63),clampij(cy+(F->pat==0||F->pat==3?1:0),0,63),dark);
  /* eye */
  int ex=cx-rx/2,ey=cy-3;
  sp_put(clampij(ex,0,63),clampij(ey,0,63),dark);
  sp_put(clampij(ex+1,0,63),clampij(ey-1,0,63),packrgb(255,255,255));
  sp_put(clampij(ex+2,0,63),clampij(ey,0,63),packrgb(240,240,240));
}

/* ---------- CG backdrops ---------- */
static void draw_cg_pan(int scene,int xoff){
  int ncol=CG_PALCNT[scene];
  const unsigned char*img=CG_IMG[scene];
  for(int y=0;y<CG_H;y++){
    for(int x=0;x<CG_W;x++){
      int col=x-xoff; if(col<0)col+=CG_W;
      const unsigned char*c=CG_PAL[scene][img[y*CG_W+col]];
      setpix(x,y,packrgb(c[0],c[1],c[2]));
    }
  }
  (void)ncol;
}
static void darkall(int a){
  if(a>=255) { fill(0,0,IN_W,IN_H,PACKED[C_BLACK]); return; }
  if(a<=0) return;
  for(int i=0;i<scrpitch*IN_H;i++){ Uint32 c=scr[i];
    int r=c>>16&0xFF,g=c>>8&0xFF,b=c&0xFF;
    r=r*(255-a)/255; g=g*(255-a)/255; b=b*(255-a)/255;
    scr[i]=0xFF000000u|((Uint32)r<<16)|((Uint32)g<<8)|(Uint32)b; }
}

/* ---------- sprite palette (kept for any index-blits) ---------- */
typedef struct{int w,h,npal;const int*enums;const Uint8*data;Uint32 pack[16];}SPRITE;
static void prep_sprite(SPRITE*s){ for(int i=0;i<s->npal;i++) s->pack[i]=PACKED[s->enums[i]]; }

/* ---------- game state ---------- */
enum{ST_TITLE,ST_INTRO,ST_PLAY,ST_REEL,ST_SHOP,ST_BAG,ST_QUIT};
static int state=ST_TITLE;
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
  int dx=abs(x1-x0),sx=x0<x1?1:-1,dy=-(y1<y0?y1-y0:y0-y1),sy=y0<y1?1:-1,er=dx+dy;
  for(;;){ P(ox,oy,sc,x0,y0,1,1,c); if(x0==x1&&y0==y1)break; int e2=2*er; if(e2>=dy){er+=dy;x0+=sx;} if(e2<=dx){er+=dx;y0+=sy;} }
}
/* draw puppet; returns hand position (grip) via hx,hy (art coords) */
static void draw_person(int ox,int oy,int sc,int anim,float t,int* hx,int* hy){
  Uint32 skin=PACKED[C_SKIN],hair=PACKED[C_HAIR],cap=PACKED[C_CAP],shirt=PACKED[C_SHIRT],
        pants=PACKED[C_PANTS],boot=PACKED[C_BOOT],dark=PACKED[C_DARK];
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
  /* cap */
  P(ox,oy,sc, 2,0,10,3,cap); P(ox,oy,sc, 1,1,13,1,cap);
  /* face */
  P(ox,oy,sc, 4,2+b,7,5,skin);
  /* hair sides */
  P(ox,oy,sc, 4,2+b,1,2,hair); P(ox,oy,sc,10,2+b,1,2,hair);
  /* eye (blink on idle every ~1.5s) */
  int blink= (anim==PA_IDLE && ((int)(t*1.0f))%3==0);
  if(!blink) P(ox,oy,sc, 6,4+b,1,1,dark);
  /* torso */
  P(ox,oy,sc, 3,7+b,9,7,shirt);
  P(ox,oy,sc, 3,7+b,9,1,mulc(shirt,180)); /* collar highlight */
  /* legs + boots */
  P(ox,oy,sc, 4+lig,14+b,3,6,pants);
  P(ox,oy,sc, 9+rig,14+b,3,6,pants);
  P(ox,oy,sc, 4+lig-1,19+b,4,2,boot);
  P(ox,oy,sc, 8+rig+1,19+b,5,2,boot);
  int ex=oy; (void)ex;
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

/* ---------- reel ---------- */
static void begin_reel(void){
  reelInd=20; reelVel=120.0f+rodLevel*14.0f;
  reelNeed=6+(level/2); reelGood=0; reelBad=0;
  reelZoneW=46.0f+rodLevel*10.0f;
  reelTarget=40+rndf()*(IN_W-reelZoneW-60);
  state=ST_REEL;
}
static void update_reel(float dt){
  reelInd+=reelVel*dt;
  if(reelInd>IN_W-20){reelInd=IN_W-20;reelVel=-reelVel;}
  if(reelInd<20){reelInd=20;reelVel=-reelVel;}
  if(press.space){
    if(reelInd>=reelTarget&&reelInd<=reelTarget+reelZoneW){
      reelGood++;
      reelTarget=20+rndf()*(IN_W-reelZoneW-40);
      if(reelGood>=reelNeed){
        int ok=(plannedFish>=0)?plannedFish:0;
        if(bagFill<bagSize()){bag[bagFill++]=ok;caughtCount[ok]++;xp+=FISHES[ok].exp;caughtToday++;}
        else add_toast(T("CHEST FULL - SELL IN SHOP","背包已满 - 去商店出售"));
        if(xp>=level*20){xp-=level*20;level++;add_toast(T("LEVEL UP!","升级了!"));}
        state=ST_PLAY;phase=PH_CATCHMSG;phaseT=0;
      }
    } else {
      reelBad++;
      if(reelBad>=3){add_toast(T("LINE SNAPPED!","线断了!"));state=ST_PLAY;phase=PH_IDLE;}
    }
  }
}
static void draw_reel(void){
  fill(0,0,IN_W,IN_H,PACKED[C_DEEPWATER]);
  fill((int)reelTarget,90,(int)reelZoneW,6,PACKED[C_GREEN]);
  fill((int)reelInd-2,84,4,18,PACKED[C_FLOATB]);
  for(int i=0;i<reelNeed;i++){Uint32 c=i<reelGood?PACKED[C_GREEN]:PACKED[C_GREY];
    setpix(24+i*16,22,c);setpix(25+i*16,22,PACKED[C_GREY]);}
  center_text(112,T("SPACE: HOLD MARKER IN GREEN ZONE","按空格:停在绿色区域内"),PACKED[C_WHITE],1);
  center_text(126,T("3 MISSES = LINE SNAPS","3次失误 = 断线"),PACKED[C_SILVER],1);
  center_text(140,T("REEL!","收竿!"),PACKED[C_GOLD],2);
  if(plannedFish>=0){ render_fish(plannedFish,0); blit_rgba((IN_W-64)/2,30,1,fsp,64,64);
    char s[40]; snprintf(s,40,"%s",FISHES[plannedFish].name); center_text(166,T(s, FISHES[plannedFish].cn),PACKED[C_CYAN],2); }
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
static void draw_water(void){
  fill_grad(0,150,IN_W,120,PACKED[C_WATER],PACKED[C_DEEPWATER]);
  int night=is_night();
  for(int x=0;x<IN_W;x+=4)setpix(x,156+(rndi(3)),night?PACKED[C_CYAN]:PACKED[C_WATER2]);
}
static void draw_sky(void){
  int night=is_night();
  fill_grad(0,0,IN_W,150,night?PACKED[C_DEEPWATER]:PACKED[C_SKY],night?PACKED[C_NIGHT]:PACKED[C_WATER]);
  if(!night){setpix(430,30,PACKED[C_YELLOW]);setpix(431,30,PACKED[C_YELLOW]);setpix(430,31,PACKED[C_YELLOW]);setpix(431,31,PACKED[C_YELLOW]);}
  else{setpix(430,26,PACKED[C_FLOATB]);setpix(429,27,PACKED[C_FLOATB]);setpix(430,27,PACKED[C_FLOATB]);setpix(431,27,PACKED[C_FLOATB]);setpix(430,28,PACKED[C_FLOATB]);}
  fill(0,132,IN_W,20,PACKED[C_NIGHT]);
}
static void draw_dock(void){
  int night=is_night();
  Uint32 d=PACKED[night?C_BROWN:C_DOCK],d2=PACKED[night?C_TRUNK:C_DOCK2];
  fill(0,170,IN_W,13,d);
  for(int x=0;x<IN_W;x+=64){setpix(x,170,d2);setpix(x+1,170,d2);setpix(x,171,d2);}
  for(int x=-30;x<IN_W;x+=64)fill(x+34,183,4,IN_H-183,d2);
  for(int x=8;x<IN_W;x+=24)fill(x,164,2,4,d2);
}
static void draw_bobber(void){
  fill((int)bobX-3,(int)bobY,7,3,PACKED[C_FLOATA]);
  fill((int)bobX-2,(int)bobY+1,5,1,PACKED[C_FLOATB]);
}
static void draw_fish_shape(int x,int y,int len,Uint32 color){
  int h=(len/3)+3;
  fill(x,y-len/2,len,h,color);
  fill(x-4,y-h/2,4,h,PACKED[C_SHADOW]);
  setpix(x,y,PACKED[C_WHITE]);
}
/* draw player + rod + line + bobber */
static void draw_player(void){
  int px=playerX-16, py=168-48; /* art origin at scale 2, feet near y=24 -> 168 */
  /* anchor: art feet row = 21; we want feet ~168, sc=2 => oy=168-21*2=126, but draw below */
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
    if(drawB){ float p=1.0f-(castT/0.5f); bx=playerX+(int)((bobX-playerX)*p); by=168+(int)(sinf(p*3.14159f)*-90.0f)*(p); by=(int)bobY+(int)(sinf(3.14159f*p)* -70.0f); }
    else { bx=(int)bobX; by=(int)bobY; }
    if(state==ST_REEL){ ang=-0.6f; len=34; }
    else { ang=(phase==PH_CAST)?(1.2f-1.8f*p):(-1.1f); /* wait:nibble: -1.05..-1.25 */ 
           if(phase==PH_NIBBLE) ang=-1.05f-0.2f*sinf(phaseT*30); len=36; }
    int tipx=gx+(int)(cosf(ang)*len*sc), tipy=gy+(int)(sinf(ang)*len*sc);
    Pline(ox,oy,sc,gx/sc,gy/sc,tipx/sc,tipy/sc,PACKED[C_ROD]);
    int tx=ox+(tipx==0?0:tipx);
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
  snprintf(b,64,T("B:%s","%s"),T("SHOP","商店")); draw_text(IN_W-8-text_w(b,1)-6,34,T("B:商店",b),PACKED[C_SILVER],1);
  if(phase==PH_IDLE){
    if(is_night()&&!(lantern&&boat)) center_text(224,T("NIGHT - BUY LANTERN","夜晚 - 请购买灯笼"),PACKED[C_WARN],1);
    else center_text(224,T("SPACE TO CAST","按空格抛竿"),PACKED[C_WHITE],1);
  }
  if(phase==PH_NIBBLE) center_text(224,T("!!! BITE !!!","!!! 鱼咬钩了 !!!"),PACKED[C_WARN],1);
  if(phase==PH_CATCHMSG){ if(plannedFish>=0){ char s[48]; snprintf(s,48,T("CAUGHT %s!","捕获 %s!"),FISHES[plannedFish].name); center_text(164,s,PACKED[C_GOLD],2); } }
}

/* ---------- shop ---------- */
typedef struct{const char*name;int cost;int maxown;int*var;const char*cname;}MENUITEM;
static MENUITEM SHOPITEMS[6]={
  {T("SELL ALL","全部出售"),0,0,0,"全部出售"},
  {T("ROD","鱼竿"),30,3,0,"鱼竿"},
  {T("LURE","鱼饵"),40,3,0,"鱼饵"},
  {T("NET","渔网"),50,3,0,"渔网"},
  {T("BOAT","小船"),150,1,0,"小船"},
  {T("LANTERN","灯笼"),45,1,0,"灯笼"},
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
static void draw_shop(void){
  fill(0,0,IN_W,IN_H,PACKED[C_DIALOG]);
  center_text(8,T("PIER SUPPLIES","码头商店"),PACKED[C_GOLD],2);
  char c[32]; snprintf(c,32,T("COINS %d","金币 %d"),coins); draw_text(28,30,c,PACKED[C_YELLOW],1);
  for(int i=0;i<NSHOP;i++){ MENUITEM*m=&SHOPITEMS[i];
    int y=50+i*22; int maxed=(m->var&&*m->var>=m->maxown); int sel=(i==menuSel);
    Uint32 col=maxed?PACKED[C_GREY]:(sel?PACKED[C_GOLD]:PACKED[C_WHITE]);
    draw_text(34,y,lang?m->cname:m->name,col,1);
    if(m->var){char b[24];if(maxed)snprintf(b,24,T("MAX","已满"));else snprintf(b,24,"$%d",m->cost);draw_text(150,y,b,col,1);}
    if(sel)draw_text(22,y+1,">",PACKED[C_GOLD],1);
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
  for(int i=0;i<40;i++){ int x=(i*47+(int)(t*90))%IN_W; int y=(i*29+(int)(t*300))%170;
    setpix(x,y,PACKED[C_CYAN]); setpix(x+1,y+1,PACKED[C_SECOND_RAIN]); }
}
static void draw_lightning(void){
  int x=120,y=20;
  while(y<150){ setpix(x,y,PACKED[C_WHITE]); setpix(x-2,y,PACKED[C_WHITE]);setpix(x+2,y,PACKED[C_WHITE]);
    y+=8; x+=rndi(7)-3; }
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
  /* fade in */
  float fade=prog/0.6f; if(fade<1) darkall((int)((1-fade)*255));
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
static void draw_lightning_rnd(float t){
  int flash=((int)(t*1.7f))%6;
  if(flash==3||flash==5){ int x=120+(rnd()%40); setpix(x,70,PACKED[C_WHITE]); int y=70; int px=x;
    while(y<178){ if(rndi(2))setpix(px,y,PACKED[C_WHITE]); px+=rndi(5)-2; y+=6; for(int k=-1;k<=1;k++)setpix(px+k,y,PACKED[C_WHITE]);} }
}

/* ---------- title with language select ---------- */
static void update_title(void){
  if(press.up)lang=0;
  if(press.down)lang=1;
  if(press.accept){slide=0;introT=0;state=ST_INTRO;}
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
  if(toastT>0)toastT-=dt; if(toastT<0)toastT=0;
  if(state==ST_PLAY){
    if(press.e){state=ST_BAG;menuSel=0;}
    if(press.b){state=ST_SHOP;menuSel=0;}
  }
  switch(state){
    case ST_TITLE: update_title(); break;
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
    case ST_INTRO: draw_intro(); break;
    case ST_PLAY: draw_play(); break;
    case ST_REEL: draw_reel(); break;
    case ST_SHOP: draw_shop(); break;
    case ST_BAG: draw_bag(); break;
    default: break;
  }
  if(toastT>0){fill(0,264,IN_W,22,PACKED[C_BLACK]);center_text(272,toast,PACKED[C_YELLOW],1);}
}

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
  screen=SDL_CreateRGBSurfaceWithFormat(0,IN_W,IN_H,32,SDL_PIXELFORMAT_ARGB8888);
  if(!screen){SDL_DestroyRenderer(ren);SDL_DestroyWindow(win);SDL_Quit();return 1;}
  SDL_Texture*tex=SDL_CreateTexture(ren,SDL_PIXELFORMAT_ARGB8888,SDL_TEXTUREACCESS_STREAMING,IN_W,IN_H);
  for(int i=0;i<NPAL;i++)PACKED[i]=SDL_MapRGBA(screen->format,PALRGB[i].r,PALRGB[i].g,PALRGB[i].b,255);
  seed_rng((unsigned)SDL_GetTicks()^((unsigned)time(NULL)<<8));
  shop_links();
  state=ST_TITLE;menuSel=0;slide=-1;introT=0;lang=0;

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
    memset(scr,0,(size_t)(scrpitch*IN_H)*sizeof(Uint32));
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