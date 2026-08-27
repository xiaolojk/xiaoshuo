/* ============================================================
   PIXEL LAKE HEART  -  a low-end fishing game for Win7/Win10 x64
   Engine: SDL2, pure software pixel-art rendering
   ============================================================ */
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ---- Internal render resolution (the "画面") ---- */
#define IN_W 512
#define IN_H 288

/* ---- Tunables ---- */
#define FPS     60
#define FRAME_MS (1000/FPS)

/* ---------- palette (r,g,b) ---------- */
typedef struct { int r,g,b; } RGB3;
static const int NPAL = 40;
static const RGB3 PALRGB[40] = {
  {120,190,235},{60,140,190},{40,110,170},{230,210,160},{150,110,60},
  {110,80,45},{90,170,70},{90,60,40},{60,130,60},{10,10,10},
  {220,70,50},{255,255,255},{245,245,245},{15,15,20},{120,120,120},
  {210,55,45},{70,180,90},{245,205,60},{60,110,200},{140,90,50},
  {235,140,40},{235,150,180},{25,30,45},{250,190,50},{20,60,110},
  {30,40,60},{20,30,45},{235,190,150},{70,48,30},{205,60,45},
  {60,120,190},{80,60,45},{90,70,60},{150,110,70},{150,80,200},
  {60,200,200},{185,195,205},{10,15,35},{235,140,40},{70,200,110}
};
static Uint32 PACKED[40];   /* precomputed ARGB for each palette index */
/* fixed palette indices */
enum {
  C_SKY=0, C_WATER=1, C_WATER2=2, C_SAND=3, C_DOCK=4, C_DOCK2=5,
  C_GRASS=6, C_TRUNK=7, C_BUSH=8, C_LINE=9, C_FLOATA=10, C_FLOATB=11,
  C_WHITE=12, C_BLACK=13, C_GREY=14, C_RED=15, C_GREEN=16, C_YELLOW=17,
  C_BLUE=18, C_BROWN=19, C_ORANGE=20, C_PINK=21, C_DARK=22, C_GOLD=23,
  C_DEEPWATER=24, C_SHADOW=25, C_DIALOG=26, C_SKIN=27, C_HAIR=28, C_CAP=29,
  C_SHIRT=30, C_PANTS=31, C_BOOT=32, C_ROD=33, C_PURPLE=34, C_CYAN=35,
  C_SILVER=36, C_NIGHT=37, C_WARN=38, C_OK=39
};

static int WIND_W = 1280, WIND_H = 720;

/* ================= pixel / draw helpers ================= */
static SDL_Surface *screen;      /* internal ARGB8888 surface */
static Uint32 *scr;              /* locked pixels */
static int scrpitch;             /* pitch/4 */

static inline void setpix(int x,int y,Uint32 c){
  if(x>=0&&x<IN_W&&y>=0&&y<IN_H) scr[y*scrpitch+x]=c;
}
static void fill(int x,int y,int w,int h,Uint32 c){
  int x0=x<0?0:x, x1=x+w>IN_W?IN_W:x+w;
  int y0=y<0?0:y, y1=y+h>IN_H?IN_H:y+h;
  for(int yy=y0;yy<y1;yy++){ Uint32*row=scr+yy*scrpitch; for(int xx=x0;xx<x1;xx++) row[xx]=c; }
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
/* blit a small pixel-sprite scaled nearest; 255=transparent */
static void blit_spr(int dx,int dy,int scale,const Uint32*packed,
                     const Uint8*d,int w,int h){
  for(int sy=0;sy<h;sy++)for(int sx=0;sx<w;sx++){
    Uint8 idx=d[sy*w+sx]; if(idx==255) continue;
    Uint32 c=packed[idx];
    for(int a=0;a<scale;a++)for(int b=0;b<scale;b++)
      setpix(dx+sx*scale+b, dy+sy*scale+a, c);
  }
}
/* ================= 8x8 uppercase pixel font ================= */
static const Uint8 F_A[8]={0x00,0x3C,0x66,0x66,0x7E,0x66,0x66,0x00};
static const Uint8 F_B[8]={0x00,0x7C,0x66,0x7C,0x66,0x66,0x7C,0x00};
static const Uint8 F_C[8]={0x00,0x3C,0x66,0x60,0x60,0x66,0x3C,0x00};
static const Uint8 F_D[8]={0x00,0x78,0x6C,0x66,0x66,0x6C,0x78,0x00};
static const Uint8 F_E[8]={0x00,0x7E,0x60,0x7C,0x60,0x60,0x7E,0x00};
static const Uint8 F_F[8]={0x00,0x7E,0x60,0x7C,0x60,0x60,0x60,0x00};
static const Uint8 F_G[8]={0x00,0x3C,0x66,0x60,0x6E,0x66,0x3E,0x00};
static const Uint8 F_H[8]={0x00,0x66,0x66,0x7E,0x66,0x66,0x66,0x00};
static const Uint8 F_I[8]={0x00,0x3C,0x18,0x18,0x18,0x18,0x3C,0x00};
static const Uint8 F_J[8]={0x00,0x06,0x06,0x06,0x06,0x66,0x3C,0x00};
static const Uint8 F_K[8]={0x00,0x66,0x6C,0x78,0x6C,0x66,0x66,0x00};
static const Uint8 F_L[8]={0x00,0x60,0x60,0x60,0x60,0x60,0x7E,0x00};
static const Uint8 F_M[8]={0x00,0x63,0x77,0x7F,0x6B,0x63,0x63,0x00};
static const Uint8 F_N[8]={0x00,0x66,0x76,0x7E,0x6E,0x66,0x66,0x00};
static const Uint8 F_O[8]={0x00,0x3C,0x66,0x66,0x66,0x66,0x3C,0x00};
static const Uint8 F_P[8]={0x00,0x7C,0x66,0x66,0x7C,0x60,0x60,0x00};
static const Uint8 F_Q[8]={0x00,0x3C,0x66,0x66,0x6E,0x3C,0x07,0x00};
static const Uint8 F_R[8]={0x00,0x7C,0x66,0x66,0x7C,0x6C,0x66,0x00};
static const Uint8 F_S[8]={0x00,0x3C,0x66,0x38,0x0C,0x66,0x3C,0x00};
static const Uint8 F_T[8]={0x00,0x7E,0x18,0x18,0x18,0x18,0x18,0x00};
static const Uint8 F_U[8]={0x00,0x66,0x66,0x66,0x66,0x66,0x3C,0x00};
static const Uint8 F_V[8]={0x00,0x66,0x66,0x66,0x66,0x3C,0x18,0x00};
static const Uint8 F_W[8]={0x00,0x63,0x63,0x6B,0x7F,0x77,0x63,0x00};
static const Uint8 F_X[8]={0x00,0x66,0x66,0x3C,0x66,0x66,0x66,0x00};
static const Uint8 F_Y[8]={0x00,0x66,0x66,0x3C,0x18,0x18,0x18,0x00};
static const Uint8 F_Z[8]={0x00,0x7E,0x06,0x0C,0x30,0x60,0x7E,0x00};
static const Uint8 F_0[8]={0x00,0x3C,0x66,0x6E,0x76,0x66,0x3C,0x00};
static const Uint8 F_1[8]={0x00,0x18,0x38,0x18,0x18,0x18,0x7E,0x00};
static const Uint8 F_2[8]={0x00,0x3C,0x66,0x0C,0x18,0x30,0x7E,0x00};
static const Uint8 F_3[8]={0x00,0x3C,0x66,0x0C,0x06,0x66,0x3C,0x00};
static const Uint8 F_4[8]={0x00,0x0C,0x1C,0x3C,0x6C,0x7E,0x0C,0x00};
static const Uint8 F_5[8]={0x00,0x7E,0x60,0x7C,0x06,0x66,0x3C,0x00};
static const Uint8 F_6[8]={0x00,0x3C,0x60,0x7C,0x66,0x66,0x3C,0x00};
static const Uint8 F_7[8]={0x00,0x7E,0x06,0x0C,0x18,0x30,0x30,0x00};
static const Uint8 F_8[8]={0x00,0x3C,0x66,0x3C,0x66,0x66,0x3C,0x00};
static const Uint8 F_9[8]={0x00,0x3C,0x66,0x3E,0x06,0x66,0x3C,0x00};
static const Uint8 F_SP[8]={0,0,0,0,0,0,0,0};
static const Uint8 F_DOT[8]={0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x00};
static const Uint8 F_COMMA[8]={0x00,0x00,0x00,0x00,0x00,0x18,0x08,0x10};
static const Uint8 F_BANG[8]={0x00,0x18,0x18,0x18,0x18,0x00,0x18,0x00};
static const Uint8 F_QUES[8]={0x00,0x3C,0x66,0x0C,0x18,0x00,0x18,0x00};
static const Uint8 F_COLON[8]={0x00,0x00,0x18,0x00,0x00,0x18,0x00,0x00};
static const Uint8 F_DASH[8]={0x00,0x00,0x00,0x7E,0x00,0x00,0x00,0x00};
static const Uint8 F_SLASH[8]={0x00,0x02,0x06,0x0C,0x18,0x30,0x60,0x00};
static const Uint8 F_PCT[8]={0x00,0x63,0x66,0x0C,0x18,0x33,0x63,0x00};
static const Uint8 F_DOLLAR[8]={0x18,0x7E,0x60,0x7C,0x06,0x7E,0x18,0x00};
static const Uint8 F_QUOTE[8]={0x00,0x36,0x36,0x00,0x00,0x00,0x00,0x00};
static const Uint8 F_PLUS[8]={0x00,0x18,0x18,0x7E,0x18,0x18,0x00,0x00};
static const Uint8 F_UNDER[8]={0x00,0x00,0x00,0x00,0x00,0x00,0x7E,0x00};

static const Uint8* glyph_of(char c){
  if(c>='A'&&c<='Z'){ static const Uint8* const L[]={F_A,F_B,F_C,F_D,F_E,F_F,F_G,F_H,F_I,F_J,F_K,F_L,F_M,F_N,F_O,F_P,F_Q,F_R,F_S,F_T,F_U,F_V,F_W,F_X,F_Y,F_Z}; return L[c-'A']; }
  if(c>='0'&&c<='9'){ static const Uint8* const D[]={F_0,F_1,F_2,F_3,F_4,F_5,F_6,F_7,F_8,F_9}; return D[c-'0']; }
  switch(c){
    case ' ':return F_SP; case '.':return F_DOT; case ',':return F_COMMA;
    case '!':return F_BANG; case '?':return F_QUES; case ':':return F_COLON;
    case '-':return F_DASH; case '/':return F_SLASH; case '%':return F_PCT;
    case '$':return F_DOLLAR; case '\'':return F_QUOTE; case '+':return F_PLUS;
    case '_':return F_UNDER;
  }
  return F_SP;
}
static int draw_char(int x,int y,char c,Uint32 col,int scale){
  const Uint8*g=glyph_of(c);
  for(int r=0;r<8;r++){
    Uint8 row=g[r];
    for(int b=0;b<8;b++){
      if((row>>(7-b))&1){ for(int a=0;a<scale;a++)for(int e=0;e<scale;e++) setpix(x+b*scale+e,y+r*scale+a,col); }
    }
  }
  return 8*scale;
}
static void draw_text(int x,int y,const char*s,Uint32 col,int scale){
  while(*s){ char c=*s; if(c>='a'&&c<='z') c-=32; x+=draw_char(x,y,c,col,scale); s++; }
}
static int text_w(const char*s,int scale){ int n=0; while(*s){char c=*s;if(c>='a'&&c<='z')c-=32; n+=8*scale; s++;} return n; }
static void center_text(int cy,const char*s,Uint32 col,int scale){
  draw_text((IN_W-text_w(s,scale))/2, cy, s, col, scale);
}
static inline int clamp(int v,int lo,int hi){ return v<lo?lo:(v>hi?hi:v); }

/* ================= sprites ================= */
typedef struct { int w,h; int npal; const int* enums; const Uint8* data; Uint32 pack[16]; } SPRITE;
static const int PL_PAL[7]={C_SKIN,C_HAIR,C_CAP,C_DARK,C_SHIRT,C_PANTS,C_BOOT};
static const Uint8 PL[18*16]={
  0,0,2,2,2,2,2,0,0,0,0,0,0,0,0,0,
  0,2,2,2,2,2,2,2,0,0,0,0,0,0,0,0,
  0,2,2,3,3,2,2,2,0,0,0,0,0,0,0,0,
  0,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,
  1,1,0,0,0,0,0,1,1,0,0,0,0,0,0,0,
  1,0,3,0,0,0,3,0,1,0,0,0,0,0,0,0,
  1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,
  0,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,
  4,4,4,4,4,4,4,4,4,4,0,0,0,0,0,0,
  4,4,0,4,4,4,4,4,4,4,0,0,0,0,0,0,
  4,4,0,4,4,4,4,4,4,0,0,0,0,0,0,0,
  4,0,4,4,4,4,4,4,4,0,0,0,0,0,0,0,
  4,0,0,4,4,4,4,4,0,0,0,0,0,0,0,0,
  5,5,5,5,5,5,5,5,5,5,0,0,0,0,0,0,
  5,5,5,5,0,0,5,5,5,5,0,0,0,0,0,0,
  5,5,5,0,0,0,0,5,5,5,0,0,0,0,0,0,
  6,6,6,0,0,0,0,6,6,6,0,0,0,0,0,0,
  6,6,6,0,0,0,0,6,6,6,0,0,0,0,0,0
};
static void prep_sprite(SPRITE*s){
  s->pack[255]=0;
  for(int i=0;i<s->npal;i++) s->pack[i]=PACKED[s->enums[i]];
}

/* ================= fish table ================= */
#define NFISH 10
typedef struct { const char*name; int value; int exp; int weight;
                 int region; int time; } FISH;
static const FISH FISHES[NFISH]={
  {"CARP",3,1,52,0,0},
  {"PERCH",5,1,46,0,0},
  {"SILVER CARP",9,2,34,1,0},
  {"CATFISH",12,2,28,1,2},
  {"LARGEMOUTH",14,2,26,1,1},
  {"BLUEGILL",22,3,16,1,1},
  {"SALMON",30,4,18,2,2},
  {"GOLD TROUT",40,4,12,2,1},
  {"ELECTRIC EEL",55,5,9,2,2},
  {"LAKE DRAGON",150,8,5,2,2}
};

/* ================= game state ================= */
enum { ST_TITLE, ST_INTRO, ST_PLAY, ST_REEL, ST_SHOP, ST_BAG, ST_QUIT };
static int state = ST_TITLE;
enum { PH_IDLE, PH_CAST, PH_WAIT, PH_NIBBLE, PH_MISS, PH_CATCHMSG };
static int phase = PH_IDLE;

static int playerX = 120;
static int spot = 0;
static int plannedFish=-1;
static float castT, phaseT;
static float bobX, bobY;
static float nibbleDelay;
static float reelInd, reelTarget, reelZoneW, reelVel;
static int reelGood, reelNeed, reelBad;

static int coins=8, xp=0, level=1, caughtToday=0, day=1;
static float timeH=7.0f;
static int rodLevel=0, lureLevel=0, boat=0, lantern=0, netLevel=0;
static int bagFill=0, bag[64];
static int caughtCount[NFISH];

static float toastT=-1; static char toast[48];
static float introT; static int slide=-1;

static int menuSel=0;

typedef struct { Uint8 space,accept,back,up,down,e,b; } PRESS;
static PRESS press;

static unsigned long rngstate;
static unsigned rnd(void){ rngstate=rngstate*6364136223846793005UL+1; return (unsigned)(rngstate>>32); }
static int rndi(int n){ return n<=0?0:(int)(rnd()%n); }
static float rndf(void){ return (float)((double)(rnd()%1000000)/1000000.0); }

static int bagSize(void){ return 8 + netLevel*4; }
static void add_toast(const char*t){ strncpy(toast,t,47); toast[47]=0; toastT=2.0f; }
static const char* spot_name(int r){ return r==0?"SHORE":(r==1?"PIER":"DEEP-WATER"); }
static int is_night(void){ return (timeH<6||timeH>=19); }

/* ================= reel ----------------------------- */
static void begin_reel(void){
  reelInd=20; reelVel=120.0f+rodLevel*14.0f;
  reelNeed=6+(level/2); reelGood=0; reelBad=0;
  reelZoneW=46.0f+rodLevel*10.0f;
  reelTarget=40+rndf()*(IN_W-reelZoneW-60);
  state=ST_REEL;
}
static void update_reel(float dt){
  reelInd+=reelVel*dt;
  if(reelInd>IN_W-20){ reelInd=IN_W-20; reelVel=-reelVel; }
  if(reelInd<20){ reelInd=20; reelVel=-reelVel; }
  if(press.space){
    if(reelInd>=reelTarget && reelInd<=reelTarget+reelZoneW){
      reelGood++;
      reelTarget=20+rndf()*(IN_W-reelZoneW-40);
      if(reelGood>=reelNeed){
        int ok=(plannedFish>=0)?plannedFish:0;
        if(bagFill<bagSize()){ bag[bagFill++]=ok; caughtCount[ok]++; xp+=FISHES[ok].exp; caughtToday++; }
        else add_toast("CHEST FULL - SELL IN SHOP");
        if(xp>=level*20){ xp-=level*20; level++; add_toast("LEVEL UP"); }
        state=ST_PLAY; phase=PH_CATCHMSG; phaseT=0;
      }
    } else {
      reelBad++;
      if(reelBad>=3){ add_toast("LINE SNAPPED!"); state=ST_PLAY; phase=PH_IDLE; }
    }
  }
}
static void draw_reel(void){
  fill(0,0,IN_W,IN_H,PACKED[C_DEEPWATER]);
  fill((int)reelTarget,90,(int)reelZoneW,6,PACKED[C_GREEN]);
  fill((int)reelInd-2,84,4,18,PACKED[C_FLOATB]);
  for(int i=0;i<reelNeed;i++){ setpix(24+i*16,22, i<reelGood?PACKED[C_GREEN]:PACKED[C_GREY]); setpix(25+i*16,22,PACKED[C_GREY]); }
  center_text(120,"SPACE: HOLD MARKER IN GREEN ZONE",PACKED[C_WHITE],1);
  center_text(134,"3 MISSES = LINE SNAPS",PACKED[C_SILVER],1);
  char b[40]; snprintf(b,40,"HOOKS %d/%d",reelGood,reelNeed);
  center_text(150,b,PACKED[C_CYAN],1);
}
/* ================= play ----------------------------- */
static int roll_fish(void){
#define MAXP 256
  int cand[MAXP], n=0;
  for(int i=0;i<NFISH;i++){
    FISH f=FISHES[i];
    if(f.region>spot) continue;
    if(f.time==2 && !is_night()) continue;
    if(f.time==1 && is_night()) continue;
    if(f.region==2 && !boat) continue;
    int w=f.weight + (i>=3? lureLevel*6:0);
    for(int k=0;k<w && n<MAXP;k++) cand[n++]=i;
  }
  if(n==0){ cand[0]=0; n=1; }
  return cand[rndi(n)];
#undef MAXP
}
static void cast_line(void){
  spot = (playerX<170)?0 : (playerX<340)?1:2;
  plannedFish=roll_fish();
  bobX=(float)playerX;
  bobY=170+ rndf()*28 + spot*8;
  phase=PH_CAST; phaseT=0; castT=0.35f;
}
static void update_play(float dt){
  const Uint8*keys=SDL_GetKeyboardState(NULL);
  int spd=90;
  if(keys[SDL_SCANCODE_LEFT]) playerX-=(int)(spd*dt);
  if(keys[SDL_SCANCODE_RIGHT]) playerX+=(int)(spd*dt);
  playerX=clamp(playerX,20,IN_W-20);

  timeH+=(dt/420.0f)*24.0f; if(timeH>=24){ timeH-=24; day++; caughtToday=0; }

  if(phase==PH_IDLE){
    if(press.space){
      if(!is_night() || (lantern&&boat)) cast_line();
      else add_toast(is_night()?"NIGHT - BUY LANTERN":"GET GEAR AT B:SHOP");
    }
  }
  else if(phase==PH_CAST){
    castT-=dt; if(castT<=0){ phase=PH_WAIT; phaseT=0;
      nibbleDelay=2.0f+rndf()*3.5f-rodLevel*0.5f; if(nibbleDelay<1.2f)nibbleDelay=1.2f; }
  }
  else if(phase==PH_WAIT){
    phaseT+=dt;
    if(phaseT>=nibbleDelay){ phase=PH_NIBBLE; phaseT=0; }
  }
  else if(phase==PH_NIBBLE){
    phaseT+=dt;
    float win=1.1f+rodLevel*0.25f;
    if(press.space){ begin_reel(); return; }
    if(phaseT>=win){ phase=PH_MISS; phaseT=0; add_toast("TOO SLOW!"); }
  }
  else if(phase==PH_MISS){ phaseT+=dt; if(phaseT>=1.0f) phase=PH_IDLE; }
  else if(phase==PH_CATCHMSG){ phaseT+=dt; if(phaseT>=1.6f) phase=PH_IDLE; }
}
static void draw_water(void){
  fill_grad(0,150,IN_W,120,PACKED[C_WATER],PACKED[C_DEEPWATER]);
  int night=is_night();
  for(int x=0;x<IN_W;x+=4){ setpix(x,156+ (rndi(3)), night?PACKED[C_CYAN]:PACKED[C_WATER2]); }
}
static void draw_sky(void){
  int night=is_night();
  fill_grad(0,0,IN_W,150, night?PACKED[C_DEEPWATER]:PACKED[C_SKY], night?PACKED[C_NIGHT]:PACKED[C_WATER]);
  if(!night){ setpix(430,30,PACKED[C_YELLOW]);setpix(431,30,PACKED[C_YELLOW]);setpix(430,31,PACKED[C_YELLOW]);setpix(431,31,PACKED[C_YELLOW]); }
  else { setpix(430,26,PACKED[C_FLOATB]);setpix(429,27,PACKED[C_FLOATB]);setpix(430,27,PACKED[C_FLOATB]);setpix(431,27,PACKED[C_FLOATB]);setpix(430,28,PACKED[C_FLOATB]); }
  fill(0,132,IN_W,20,PACKED[C_NIGHT]);
}
static void draw_dock(void){
  int night=is_night();
  Uint32 d=PACKED[night?C_BROWN:C_DOCK], d2=PACKED[night?C_TRUNK:C_DOCK2];
  fill(0,170,IN_W,13,d);
  for(int x=0;x<IN_W;x+=64){ setpix(x,170,d2);setpix(x+1,170,d2);setpix(x,171,d2); }
  for(int x=-30;x<IN_W;x+=64){ fill(x+34,183,4,IN_H-183,d2); }
  for(int x=8;x<IN_W;x+=24){ fill(x,164,2,4,d2); }
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
static void draw_player(void){
  static SPRITE pl; static int inited=0;
  if(!inited){ pl.w=16; pl.h=18; pl.npal=7; pl.enums=PL_PAL; pl.data=PL; prep_sprite(&pl); inited=1; }
  int sc=2;
  blit_spr(playerX-(pl.w*sc)/2, 168-pl.h*sc, sc, pl.pack, pl.data, pl.w, pl.h);
  if(phase!=PH_IDLE){
    int handX=playerX+10, handY=156;
    fill(handX,handY,20,1,PACKED[C_ROD]);
    fill(handX+20,handY+1,2,1,PACKED[C_ROD]);
    int ly0=handY+1, lx=handX+22;
    for(int y=ly0;y<=(int)bobY;y++) setpix(lx,y,PACKED[C_LINE]);
  }
}
static void draw_play(void){
  draw_sky();
  draw_water();
  draw_dock();
  int drawLine = (phase==PH_CAST||phase==PH_WAIT||phase==PH_NIBBLE||phase==PH_MISS||phase==PH_CATCHMSG);
  if(drawLine) draw_bobber();
  if(phase==PH_WAIT){ int fx=90+(int)(phaseT*50)%(IN_W-160); draw_fish_shape(fx,(int)bobY+14,20,PACKED[C_CYAN]); }
  draw_player();
  char b[64];
  snprintf(b,64,"$%d",coins); draw_text(8,6,b,PACKED[C_YELLOW],1);
  snprintf(b,64,"LV%d  D%d",level,day); draw_text(8,20,b,PACKED[C_CYAN],1);
  snprintf(b,64,"%02d:%02d %s",(int)timeH,(int)((timeH-(int)timeH)*60),is_night()?"NIGHT":"DAY");
  draw_text(IN_W-text_w(b,1)-8,6,b,is_night()?PACKED[C_YELLOW]:PACKED[C_WHITE],1);
  snprintf(b,64,"SPOT:%s",spot_name(spot)); draw_text(IN_W-text_w(b,1)-8,20,b,PACKED[C_SILVER],1);
  snprintf(b,64,"B:SHOP E:LOG"); draw_text(IN_W-text_w(b,1)-8,34,b,PACKED[C_SILVER],1);
  if(phase==PH_IDLE){
    if(is_night()&&!(lantern&&boat)) center_text(224,"NIGHT - BUY LANTERN AT SHOP",PACKED[C_WARN],1);
    else center_text(224,"SPACE TO CAST",PACKED[C_WHITE],1);
  }
  if(phase==PH_NIBBLE) center_text(224,"!!! BITE - SPACE TO SET HOOK !!!",PACKED[C_WARN],1);
  if(phase==PH_CATCHMSG){ char s[48]; snprintf(s,48,"CAUGHT %s!",FISHES[plannedFish>=0?plannedFish:0].name); center_text(240,s,PACKED[C_GOLD],2); }
}
/* ================= shop / bag ----------------------- */
typedef struct { const char*name; int cost; int maxown; int*var; } MENUITEM;
static MENUITEM SHOPITEMS[6]={
  {"SELL ALL",0,0,0},
  {"ROD",30,3,0},
  {"LURE",40,3,0},
  {"NET",50,3,0},
  {"BOAT",150,1,0},
  {"LANTERN",45,1,0},
};
#define NSHOP 6
static void shop_links(void){
  SHOPITEMS[1].var=&rodLevel; SHOPITEMS[2].var=&lureLevel;
  SHOPITEMS[3].var=&netLevel; SHOPITEMS[4].var=&boat; SHOPITEMS[5].var=&lantern;
}
static void update_shop(void){
  if(press.up && menuSel>0) menuSel--;
  if(press.down && menuSel<NSHOP-1) menuSel++;
  if(press.accept){
    MENUITEM*m=&SHOPITEMS[menuSel];
    if(menuSel==0){
      if(bagFill>0){ int sum=0; for(int i=0;i<bagFill;i++) sum+=FISHES[bag[i]].value; coins+=sum; bagFill=0; add_toast("SOLD"); }
      else add_toast("NOTHING TO SELL");
    }
    else if(*m->var < m->maxown){ if(coins>=m->cost){ coins-=m->cost; (*m->var)++; add_toast("PURCHASED"); }
      else add_toast("NOT ENOUGH COINS"); }
    else add_toast("ALREADY MAX");
  }
  if(press.back){ state=ST_PLAY; }
}
static void draw_shop(void){
  fill(0,0,IN_W,IN_H,PACKED[C_DIALOG]);
  center_text(8,"PIER SUPPLIES",PACKED[C_GOLD],2);
  char c[32]; snprintf(c,32,"COINS %d",coins); draw_text(28,30,c,PACKED[C_YELLOW],1);
  for(int i=0;i<NSHOP;i++){
    MENUITEM*m=&SHOPITEMS[i];
    int y=50+i*22;
    int maxed=(m->var && *m->var>=m->maxown);
    int sel=(i==menuSel);
    Uint32 col= maxed?PACKED[C_GREY]:(sel?PACKED[C_GOLD]:PACKED[C_WHITE]);
    draw_text(34,y,m->name,col,1);
    if(m->var){
      char b[24]; if(maxed) snprintf(b,24,"MAX"); else snprintf(b,24,"$%d",m->cost);
      draw_text(120,y,b,col,1);
    }
    if(sel) draw_text(22,y+1,">",PACKED[C_GOLD],1);
  }
  center_text(198,"UP/DOWN SELECT  SPACE BUY  ESC BACK",PACKED[C_SILVER],1);
  center_text(212,"SELL FISH FIRST.  BUY BOAT >> DEEP LEGENDS",PACKED[C_SILVER],1);
}
static void update_bag(void){
  if(press.back||press.accept){ state=ST_PLAY; }
}
static void draw_bag(void){
  fill(0,0,IN_W,IN_H,PACKED[C_DIALOG]);
  center_text(6,"FISHER'S LOG",PACKED[C_GOLD],2);
  char b[64];
  snprintf(b,64,"COINS %d   LEVEL %d   EXP %d/%d",coins,level,xp,level*20); draw_text(24,22,b,PACKED[C_YELLOW],1);
  snprintf(b,64,"DAY %d   CHEST %d/%d",day,bagFill,bagSize()); draw_text(24,36,b,PACKED[C_WHITE],1);
  snprintf(b,64,"ROD%d LURE%d NET%d  BOAT:%s LANTERN:%s",rodLevel,lureLevel,netLevel,boat?"Y":"N",lantern?"Y":"N"); draw_text(24,50,b,PACKED[C_WHITE],1);
  center_text(68,"-  LAKE FIELD GUIDE  -",PACKED[C_CYAN],2);
  for(int i=0;i<NFISH;i++){
    int y=90+i*17;
    Uint32 col=caughtCount[i]>0?PACKED[C_WHITE]:PACKED[C_GREY];
    snprintf(b,32,"%-14s x%d",FISHES[i].name,caughtCount[i]);
    draw_text(30,y,b,col,1);
    snprintf(b,16,"$%d",FISHES[i].value); draw_text(210,y,b,caughtCount[i]>0?PACKED[C_YELLOW]:PACKED[C_GREY],1);
  }
  center_text(266,"ESC: BACK TO LAKE",PACKED[C_SILVER],1);
}
/* ================= intro / title -------------------- */
static const char* INTROCAP[5]={
  "A STORM TOOK GRANDPA'S BOAT...",
  "GRANDPA ALWAYS SAID: 'THE LAKE KEEPS SECRETS.'",
  "HE LEFT YOU THE OLD BAMBOO ROD...",
  "AND ONE LAST PROMISE: 'GUARD THIS LAKE.'",
  "DAY 1 - YOUR LEGACY BEGINS"
};
static void update_intro(float dt){
  introT+=dt;
  slide=(int)(introT/3.6f);
  if(slide>4 || press.accept){ add_toast("MOVE:ARROWS  CAST:SPACE"); state=ST_PLAY; phase=PH_IDLE; phaseT=0; }
}
static void draw_intro(void){
  int s=slide; if(s<0)s=0; if(s>4)s=4;
  int night=(s<2);
  if(night){
    fill_grad(0,0,IN_W,IN_H,PACKED[C_NIGHT],PACKED[C_DEEPWATER]);
    if(((int)introT)%4==3) fill(0,0,IN_W,IN_H,PACKED[C_FLOATB]); /* lightning */
    fill(0,170,IN_W,13,PACKED[C_BROWN]);
    if(s==1){ /* grandfather on pier */
      fill(170,132,26,34,PACKED[C_BLACK]);
      fill(160,140,18,16,PACKED[C_BLACK]);
      fill(196,135,26,2,PACKED[C_BLACK]); /* rod */
    }
  } else {
    fill_grad(0,0,IN_W,IN_H,PACKED[C_SKY],PACKED[C_WATER]);
    fill(0,170,IN_W,13,PACKED[C_DOCK]);
    /* grandpa walking into sun */
    fill(300,122,22,26,PACKED[C_BLACK]);
    fill(318,130,14,12,PACKED[C_BLACK]);
  }
  fill(0,220,IN_W,64,PACKED[C_DIALOG]);
  if(s<4) center_text(246,INTROCAP[s],PACKED[C_WHITE],1);
  else { center_text(236,INTROCAP[s],PACKED[C_GOLD],2); center_text(252,"PRESS SPACE",PACKED[C_SILVER],1); }
}
static void draw_title(void){
  fill(0,0,IN_W,IN_H,PACKED[C_DEEPWATER]);
  draw_water(); fill_grad(0,60,IN_W,28,PACKED[C_WATER],PACKED[C_BLACK]);
  draw_text(64,20,"PIXEL LAKE HEART",PACKED[C_WHITE],3);
  draw_text(70,70,"A LOW-END FISHING ADVENTURE",PACKED[C_CYAN],1);
  static float t=0; t+=0.02f;
  draw_fish_shape(70+(int)(t*40)%(IN_W-160),120,30,PACKED[C_SILVER]);
  center_text(180,(SDL_GetTicks()/300)%2? "PRESS SPACE TO BEGIN":"PRESS SPACE TO BEGIN",PACKED[C_GOLD],2);
  center_text(206,"WIN7-WIN10 X64  PIXEL MODE  ~512MB",PACKED[C_SILVER],1);
  center_text(220,"MIN: 1ST GEN CORE i3 + ANY DGPU",PACKED[C_SILVER],1);
  center_text(234,"F11 FULLSCREEN  ARROWS MOVE",PACKED[C_SILVER],1);
}
/* ================= dispatchers ---------------------- */
static void update_top(float dt){
  if(toastT>0) toastT-=dt; if(toastT<0) toastT=0;
  if(state==ST_PLAY){
    if(press.e){ state=ST_BAG; menuSel=0; }
    if(press.b){ state=ST_SHOP; menuSel=0; }
  }
  switch(state){
    case ST_TITLE: if(press.accept){ slide=0; introT=0; state=ST_INTRO; } break;
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
  if(toastT>0){ fill(0,264,IN_W,22,PACKED[C_BLACK]); center_text(272,toast,PACKED[C_YELLOW],1); }
}
/* ================= main ----------------------------- */
int main(int argc,char *argv[]){
  (void)argc; (void)argv;
  if(SDL_Init(SDL_INIT_VIDEO)<0) return 1;
  SDL_Window*win=SDL_CreateWindow("PIXEL LAKE HEART - A Low-end Fishing Game",
      SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,WIND_W,WIND_H,SDL_WINDOW_RESIZABLE);
  if(!win){ SDL_Quit(); return 1; }
  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY,"0");
  SDL_Renderer*ren=SDL_CreateRenderer(win,-1,0);
  if(!ren){ SDL_Surface*ws=SDL_GetWindowSurface(win); ren=SDL_CreateSoftwareRenderer(ws); }
  if(!ren){ SDL_DestroyWindow(win); SDL_Quit(); return 1; }
  screen=SDL_CreateRGBSurfaceWithFormat(0,IN_W,IN_H,32,SDL_PIXELFORMAT_ARGB8888);
  if(!screen){ SDL_DestroyRenderer(ren); SDL_DestroyWindow(win); SDL_Quit(); return 1; }
  SDL_Texture*tex=SDL_CreateTexture(ren,SDL_PIXELFORMAT_ARGB8888,SDL_TEXTUREACCESS_STREAMING,IN_W,IN_H);
  for(int i=0;i<NPAL;i++) PACKED[i]=SDL_MapRGBA(screen->format,PALRGB[i].r,PALRGB[i].g,PALRGB[i].b,255);
  rngstate=(unsigned long)SDL_GetTicks()^((unsigned long)time(NULL)<<8)^0x9E3779B9UL;
  shop_links();
  state=ST_TITLE; menuSel=0; slide=-1; introT=0;

  int running=1; Uint32 tprev=SDL_GetTicks();
  while(running){
    Uint32 tnow=SDL_GetTicks();
    float dt=(tnow-tprev)/1000.0f; if(dt>0.1f)dt=0.1f; tprev=tnow;

    memset(&press,0,sizeof(press));
    SDL_Event ev;
    while(SDL_PollEvent(&ev)){
      if(ev.type==SDL_QUIT) running=0;
      else if(ev.type==SDL_KEYDOWN){
        switch(ev.key.keysym.scancode){
          case SDL_SCANCODE_SPACE: case SDL_SCANCODE_RETURN: press.space=1; press.accept=1; break;
          case SDL_SCANCODE_ESCAPE: press.back=1; break;
          case SDL_SCANCODE_W: case SDL_SCANCODE_UP: press.up=1; break;
          case SDL_SCANCODE_S: case SDL_SCANCODE_DOWN: press.down=1; break;
          case SDL_SCANCODE_E: press.e=1; break;
          case SDL_SCANCODE_B: press.b=1; break;
          case SDL_SCANCODE_F:
            if(SDL_GetWindowFlags(win)&SDL_WINDOW_FULLSCREEN_DESKTOP) SDL_SetWindowFullscreen(win,0);
            else SDL_SetWindowFullscreen(win,SDL_WINDOW_FULLSCREEN_DESKTOP);
            break;
          default: break;
        }
      }
      else if(ev.type==SDL_MOUSEBUTTONDOWN && ev.button.button==SDL_BUTTON_LEFT){ press.space=1; press.accept=1; }
    }

    update_top(dt);

    SDL_LockSurface(screen);
    scr=(Uint32*)screen->pixels; scrpitch=screen->pitch/4;
    memset(scr,0,(size_t)(scrpitch*IN_H)*sizeof(Uint32));
    draw_top();
    SDL_UnlockSurface(screen);

    SDL_UpdateTexture(tex,NULL,screen->pixels,screen->pitch);
    int ww,wh; SDL_GetWindowSize(win,&ww,&wh);
    float sc=(float)(ww<wh?ww/IN_W:wh/IN_H); if(sc<0.5f)sc=0.5f;
    SDL_Rect dst={ (ww-(int)(IN_W*sc))/2,(wh-(int)(IN_H*sc))/2,(int)(IN_W*sc),(int)(IN_H*sc) };
    SDL_SetRenderDrawColor(ren,0,0,0,255);
    SDL_RenderClear(ren);
    SDL_RenderCopy(ren,tex,NULL,&dst);
    SDL_RenderPresent(ren);

    int wait=(int)(FRAME_MS-(int)(SDL_GetTicks()-tnow)); if(wait>0) SDL_Delay((Uint32)wait);
  }
  if(tex) SDL_DestroyTexture(tex);
  SDL_FreeSurface(screen); SDL_DestroyRenderer(ren); SDL_DestroyWindow(win); SDL_Quit();
  return 0;
}