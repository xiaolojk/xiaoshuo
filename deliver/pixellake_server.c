/*
 * PixelLakeHeart — 局域网联机 服务端（无头主机）
 *
 * 兼容客户端：PixelLakeHeart.exe（Windows 版钓鱼游戏）的"加入房间"协议。
 * 玩家在游戏里选「局域网联机」->「加入房间」-> 输入本服务端所在机器的 IP 即可进入。
 *
 * 原理：完全复用游戏自带的 HOST 逻辑（UDP 3317, 逗号分隔, 无 SDL 依赖）。
 *   客户端 -> 服务端:  JOIN skin,hair,hcol,shirt,pants,label
 *                      STATE x,anim,phase
 *                      CATCH fi,value
 *   服务端 -> 客户端:  YOU id
 *                      PJOIN id,skin,hair,hcol,shirt,pants,label,x,anim,phase,total,count
 *                      PLEFT id
 *                      PSTATE id,x,anim,phase
 *                      SCORE id,total,count
 *
 * 编译（Linux）:
 *   gcc -O2 -o pixellake_server server.c
 *
 * 运行:
 *   ./pixellake_server [端口] [房间名]
 *   (默认端口 3317)
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define NET_PORT        3317u
#define NET_MAXPLAYERS  8

/* 玩家表项：与服务端维护的共享状态一一对应 */
typedef struct{
    int active;
    int myId;
    int x, anim, phase;
    int skin,hair,hcol,shirt,pants;
    char label[16];
    int total, count;
    uint32_t lastSeen;
    struct sockaddr_in addr;   /* client->server 地址 */
}PLAYER;

static int   fd = -1;
static PLAYER P[NET_MAXPLAYERS];
static uint32_t now_millis(void){ struct timespec ts; clock_gettime(CLOCK_MONOTONIC,&ts); return (uint32_t)(ts.tv_sec*1000 + ts.tv_nsec/1000000); }

static int addr_eq(const struct sockaddr_in*a,const struct sockaddr_in*b){
    return a->sin_port==b->sin_port && a->sin_addr.s_addr==b->sin_addr.s_addr;
}

static void send_line(const struct sockaddr_in*dst,const char*fmt,...){
    char buf[320]; va_list ap; va_start(ap,fmt); vsnprintf(buf,sizeof(buf),fmt,ap); va_end(ap);
    size_t n=strlen(buf); if(n+2>sizeof(buf))return;
    buf[n]='\n'; buf[n+1]=0;
    sendto(fd,buf,(int)n+1,0,(const struct sockaddr*)dst,sizeof(*dst));
}

static int net_open(unsigned port){
    fd=socket(AF_INET,SOCK_DGRAM,0);
    if(fd<0){ perror("socket"); return 0; }
    int one=1; setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,&one,sizeof(one));
    struct sockaddr_in a; memset(&a,0,sizeof(a));
    a.sin_family=AF_INET; a.sin_addr.s_addr=htonl(INADDR_ANY); a.sin_port=htons((uint16_t)port);
    if(bind(fd,(struct sockaddr*)&a,sizeof(a))!=0){ perror("bind"); close(fd); return 0; }
    int fl=fcntl(fd,F_GETFL,0); fcntl(fd,F_SETFL,fl|O_NONBLOCK);
    return 1;
}

static void bcast(const char*fmt,...){
    char buf[320]; va_list ap; va_start(ap,fmt); vsnprintf(buf,sizeof(buf),fmt,ap); va_end(ap);
    size_t n=strlen(buf); char out[322]; memcpy(out,buf,n); out[n]='\n'; out[n+1]=0;
    for(int i=0;i<NET_MAXPLAYERS;i++)
        if(P[i].active) sendto(fd,out,(int)n+1,0,(const struct sockaddr*)&P[i].addr,sizeof(P[i].addr));
}

static void send_player_to(const struct sockaddr_in*dst,int id){
    PLAYER*p=&P[id];
    send_line(dst,"PJOIN %d,%d,%d,%d,%d,%d,%s,%d,%d,%d,%d,%d",
        p->myId,p->skin,p->hair,p->hcol,p->shirt,p->pants,p->label,
        p->x,p->anim,p->phase,p->total,p->count);
}
static void bcast_scores(void){
    for(int i=0;i<NET_MAXPLAYERS;i++) if(P[i].active){
        char b[48]; snprintf(b,sizeof(b),"SCORE %d,%d,%d",P[i].myId,P[i].total,P[i].count);
        bcast("%s",b);
    }
}

static int slot_by_addr(const struct sockaddr_in*from){
    for(int i=0;i<NET_MAXPLAYERS;i++)
        if(P[i].active && addr_eq(&P[i].addr,from)) return i;
    return -1;
}

static int active_count(void){
    int c=0; for(int i=1;i<NET_MAXPLAYERS;i++) if(P[i].active) c++;
    return c;
}

/* ---- host 消息处理 ---- */
static void host_join(const struct sockaddr_in*from,const char*arg){
    int id=-1; for(int i=1;i<NET_MAXPLAYERS;i++) if(!P[i].active){id=i;break;}
    if(id<0)return;                       /* room full -> silently ignored */
    int skin=0,hair=0,hcol=1,shirt=0,pants=0; char lab[16]="P";
    sscanf(arg,"%d,%d,%d,%d,%d,%15s",&skin,&hair,&hcol,&shirt,&pants,lab);
    PLAYER*p=&P[id]; memset(p,0,sizeof(*p));
    p->active=1;p->myId=id;p->skin=skin;p->hair=hair;
    p->hcol=hcol;p->shirt=shirt;p->pants=pants;
    snprintf(p->label,16,"%s",lab);
    p->x=40; p->anim=0; p->phase=0;
    p->addr=*from; p->lastSeen=now_millis();
    send_line(from,"YOU %d",id);
    /* 告诉其它人来了个新玩家 */
    for(int j=1;j<NET_MAXPLAYERS;j++) if(j!=id && P[j].active){
        send_line(&P[j].addr,"PJOIN %d,%d,%d,%d,%d,%d,%s,%d,%d,%d,%d,%d",
            p->myId,p->skin,p->hair,p->hcol,p->shirt,p->pants,p->label,
            p->x,p->anim,p->phase,p->total,p->count);
    }
    /* 把房间里的人告诉新来的 */
    for(int j=1;j<NET_MAXPLAYERS;j++) if(j!=id && P[j].active) send_player_to(from,j);
    bcast_scores();
    fprintf(stderr,"[net] %s 加入房间 (id=%d)  在线 %d\n",lab,id,active_count());
}
static void host_state(const struct sockaddr_in*from,const char*arg){
    int i=slot_by_addr(from); if(i<0)return;
    int x=P[i].x,a=P[i].anim,ph=P[i].phase;
    sscanf(arg,"%d,%d,%d",&x,&a,&ph);
    P[i].x=x; P[i].anim=a; P[i].phase=ph; P[i].lastSeen=now_millis();
}
static void host_catch(const struct sockaddr_in*from,const char*arg){
    int i=slot_by_addr(from); if(i<0)return;
    int fi=0,v=0; sscanf(arg,"%d,%d",&fi,&v);
    P[i].total+=v; P[i].count++;
    bcast_scores();
}

/* 周期性向每个客户端广播其它玩家的位置 */
static void host_tick(void){
    for(int c=1;c<NET_MAXPLAYERS;c++) if(P[c].active){
        for(int j=1;j<NET_MAXPLAYERS;j++) if(P[j].active && j!=c){
            send_line(&P[c].addr,"PSTATE %d,%d,%d,%d",P[j].myId,P[j].x,P[j].anim,P[j].phase);
        }
    }
}
static void host_timeout(void){
    uint32_t now=now_millis();
    for(int i=1;i<NET_MAXPLAYERS;i++)
        if(P[i].active && now-P[i].lastSeen>8000u){
            bcast("PLEFT %d",i);
            fprintf(stderr,"[net] #%d %s 超时离开\n",i,P[i].label);
            P[i].active=0;
        }
}

static void handle_line(const struct sockaddr_in*from,char*line){
    char*e=strchr(line,'\n'); if(e)*e=0;
    char*cmd=line; char*arg=cmd;
    while(*arg && *arg!=' ' && *arg!='\t') arg++;
    if(*arg){*arg=0;arg++;} else arg="";
    if(!strcmp(cmd,"JOIN"))  host_join(from,arg);
    else if(!strcmp(cmd,"STATE")) host_state(from,arg);
    else if(!strcmp(cmd,"CATCH")) host_catch(from,arg);
}

static void recv_packets(void){
    char buf[320];
    for(int k=0;k<64;k++){
        struct sockaddr_in from; socklen_t fl=sizeof(from);
        int n=(int)recvfrom(fd,buf,sizeof(buf)-1,0,(struct sockaddr*)&from,&fl);
        if(n<=0) break;
        buf[n]=0;
        handle_line(&from,buf);
    }
}

int main(int argc,char**argv){
    setvbuf(stdout,NULL,_IOLBF,0);   /* 行缓冲：确保后台运行时日志实时可见 */
    unsigned port=NET_PORT;
    if(argc>=2) port=(unsigned)strtoul(argv[1],NULL,10);
    if(!net_open(port)){ fprintf(stderr,"无法监听 UDP %u\n",port); return 1; }

    printf("PixelLakeHeart 联机服务端已启动\n");
    printf("  监听 UDP 端口: %u\n",port);
    printf("  最大玩家数:    %d\n",NET_MAXPLAYERS-1);
    printf("  连接方式:      游戏内选「局域网联机」->「加入房间」-> 输入本机IP\n");
    printf("  本机 IP:\n");
    { /* 打印本机 IPv4 */
        FILE*fp=popen("hostname -I 2>/dev/null || ip -4 addr show 2>/dev/null","r");
        if(fp){ char line[256]; while(fgets(line,sizeof(line),fp)) printf("    %s",line); pclose(fp); }
    }
    printf("  按 Ctrl+C 停止。\n");

    uint32_t last_tick=now_millis();
    while(1){
        recv_packets();
        uint32_t now=now_millis();
        if(now-last_tick>=200){ last_tick=now; host_tick(); host_timeout(); }
        usleep(2000);
    }
    return 0;
}