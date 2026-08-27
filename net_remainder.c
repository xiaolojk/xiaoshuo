/* ---- client + room-menu layer (was lost in a corrupted edit, re-added) ---- */
static int net_me=-1;                      /* my own player id (-1 = unknown) */

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
  center_text(222,T("UP/DOWN SELECT  SPACE OK  M MENU SAYS HELLO","上下选择 空格确认"),PACKED[C_SILVER],1);
  char pb[24]; snprintf(pb,24,T("PORT %u","端口 %u"),NET_PORT);
  center_text(236,pb,PACKED[C_SILVER],1);
}