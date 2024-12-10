/*
 * ブロックスデュオの手を記録する
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_SIZE 5 /* ペントミノまで */
#define MAX_PIECE 21
#define MAX_X 14 /* ボードは14x14 */
#define MAX_Y 14

char const piece_name[MAX_PIECE]="abcdefghijklmnopqrstu";
struct Piece {
  int size;
  int x[MAX_SIZE-1],y[MAX_SIZE-1];
} const piece[MAX_PIECE]={
  { 1 }, /* a */
  { 2,  0, 0, 0, 0,  1, 0, 0, 0 }, /* b */
  { 3,  0, 0, 0, 0, -1, 1, 0, 0 }, /* c */
  { 3,  0, 1, 0, 0, -1, 0, 0, 0 }, /* d */
  { 4,  0, 0, 0, 0, -1, 1, 2, 0 }, /* e */
  { 4,  0, 0,-1, 0, -1, 1, 1, 0 }, /* f */
  { 4,  0, 0, 1, 0, -1, 1, 0, 0 }, /* g */
  { 4,  1, 0, 1, 0,  0, 1, 1, 0 }, /* h */
  { 4, -1, 0, 1, 0,  0, 1, 1, 0 }, /* i */
  { 5,  0, 0, 0, 0, -2,-1, 1, 2 }, /* j */
  { 5,  0, 0, 0,-1, -2,-1, 1, 1 }, /* k */
  { 5,  0, 0,-1,-1, -2,-1, 0, 1 }, /* l */
  { 5,  0, 0,-1,-1, -1, 1, 0, 1 }, /* m */
  { 5,  0, 0,-1,-1, -1, 1,-1, 1 }, /* n */
  { 5,  0, 0, 0, 1, -1, 1, 2, 0 }, /* o */
  { 5,  0, 0,-1, 1, -1, 1, 1, 1 }, /* p */
  { 5,  0, 0, 1, 2, -2,-1, 0, 0 }, /* q */
  { 5, -1, 0, 1, 1, -1,-1, 0, 1 }, /* r */
  { 5, -1,-1, 1, 1, -1, 0, 0, 1 }, /* s */
  { 5, -1,-1, 1, 0, -1, 0, 0, 1 }, /* t */
  { 5,  0,-1, 1, 0, -1, 0, 0, 1 }, /* u */
};

/* ボードは1次元配列で、壁2列、最初の列、…、最後の列、壁2列の順に並べる */
/* 各列は桁数+1の長さで、最後の桁は壁とする */
#define FENCE_LEN (MAX_X*2+2)	/* 壁2列の長さ */
#define X 1			/* X方向の添字は1ずつ進む */
#define Y (MAX_X+1)		/* Y方向の添字は桁数+1ずつ進む */
int const dir_x[]={ X,-X, Y, Y,-X, X,-Y,-Y };
int const dir_y[]={ Y, Y,-X, X,-Y,-Y, X,-X };
unsigned char board[FENCE_LEN+MAX_Y*Y+FENCE_LEN]; /* ボード(0で初期化) */
#define PASS 0 /* 位置0はパスを表す */
#define START_P1 (FENCE_LEN+4*X+4*Y) /* 先手のスタートポイント */
#define START_P2 (FENCE_LEN+9*X+9*Y) /* 後手のスタートポイント */
/* 0は空、1,2は先手と後手のピース */
#define FENCE 255 /* 壁 */
unsigned char used[2][MAX_PIECE]; /* ピースを使ったか */
struct Move {
  int p;
  unsigned char n,d;
} history[MAX_PIECE*2];
int history_len=0,history_max=0;

/* ボードに壁を設置する */
void init_board(void) {
  int i;
  for(i=0;i<FENCE_LEN;i++) { board[i]=FENCE; board[FENCE_LEN+MAX_Y*Y+i]=FENCE; }
  for(i=FENCE_LEN;i<FENCE_LEN+MAX_Y*Y;i+=Y) board[i+MAX_X]=FENCE;
}

/* ボードを表示する */
void print_board(void) {
  int x,y;
  int p=FENCE_LEN;
  int p1=0,p2=0;
  int t,n;
  printf("  ");
  for(x=0;x<MAX_X;x++) printf(" %X",x+1);
  putchar('\n');
  for(y=0;y<MAX_Y;y++) {
    printf("%X ",y+1);
    for(x=0;x<MAX_X;x++) {
      if(board[p]==1) { printf("■"); p1++; }
      else if(board[p]==2) { printf("□"); p2++; }
      else {
        if(p==START_P1 || p==START_P2) printf("○");
        else printf("・");
      }
      p++;
    }
    putchar('\n');
    p++;
  }
  for(t=1;t<=2;t++) {
    printf("P%d:%2d ",t,t==1 ? p1 : p2);
    for(n=0;n<MAX_PIECE;n++) if(!used[t-1][n]) printf(" %c",piece_name[n]);
    putchar('\n');
  }
}

/* 4文字コードを文字列で返す */
/* 文字列は静的な配列で、次回上書きされる可能性がある */
char const *string_code(struct Move const *h) {
  if(h->p==PASS) return "----";
  else {
    static char code[5];
    int p=h->p-FENCE_LEN;
    sprintf(code,"%X%X%c%d",p%Y+1,p/Y+1,piece_name[h->n],h->d);
    return code;
  }
}

/* 4文字コードを表示する */
void print_code(struct Move const *h) {
  puts(string_code(h));
}

/* 置けるか調べる(使ったピースかはチェックしない) */
/* tに手番、pに場所、nに種類、dに向き */
int check(int t,int p,int n,int d) {
  struct Piece const *q=&piece[n];
  int dx=dir_x[d],dy=dir_y[d];
  int i;
  int f;
  if(t==1 && board[START_P1]==0) {
    if(p==START_P1) return 1;
    for(i=0;i<q->size-1;i++) if(p+q->x[i]*dx+q->y[i]*dy==START_P1) return 1;
    return 0;
  }
  if(t==2 && board[START_P2]==0) {
    if(p==START_P2) return 1;
    for(i=0;i<q->size-1;i++) if(p+q->x[i]*dx+q->y[i]*dy==START_P2) return 1;
    return 0;
  }
  if(board[p]!=0) return 0;
  for(i=0;i<q->size-1;i++) if(board[p+q->x[i]*dx+q->y[i]*dy]!=0) return 0;
  if(board[p-X]==t || board[p+X]==t || board[p-Y]==t || board[p+Y]==t) return 0;
  f=board[p-X-Y]==t || board[p+X-Y]==t || board[p-X+Y]==t || board[p+X+Y]==t;
  for(i=0;i<q->size-1;i++) {
    int p1=p+q->x[i]*dx+q->y[i]*dy;
    if(board[p1-X]==t || board[p1+X]==t || board[p1-Y]==t || board[p1+Y]==t) return 0;
    f|=board[p1-X-Y]==t || board[p1+X-Y]==t || board[p1-X+Y]==t || board[p1+X+Y]==t;
  }
  return f;
}

/* パスかどうか調べる */
int nomove(int t) {
  int x,y,n,d;
  int p=FENCE_LEN;
  for(y=0;y<MAX_Y;y++) {
    for(x=0;x<MAX_X;x++) {
      for(n=0;n<MAX_PIECE;n++) if(!used[t-1][n]) {
        for(d=0;d<8;d++) if(check(t,p,n,d)) return 0;
      }
      p++;
    }
    p++;
  }
  return 1;
}

/* ピースを置く */
/* tに手番、pに場所、nに種類、dに向き */
void put(int t,int p,int n,int d) {
  struct Move *h=&history[history_len++];
  if(p!=PASS) {
    struct Piece const *q=&piece[n];
    int dx=dir_x[d],dy=dir_y[d];
    int i,j;
    int pa[MAX_SIZE];
    pa[0]=p; board[p]=t;
    for(i=0;i<q->size-1;i++) { int p1=p+q->x[i]*dx+q->y[i]*dy; pa[i+1]=p1; board[p1]=t; }
    for(d=0;;d++) {
      int dx=dir_x[d],dy=dir_y[d];
      for(i=0;i<q->size;i++) {
        p=pa[i];
        if(board[p]!=t) goto next;
        for(j=0;j<q->size-1;j++) if(board[p+q->x[j]*dx+q->y[j]*dy]!=t) goto next;
        goto found;
      next:;
      }
    }
  found:
    used[t-1][n]=1;
  }
  h->p=p; h->n=n; h->d=d;
  if(history_len>history_max) history_max=history_len;
  print_code(h);
}

/* ピースを取り除く */
void undo(int t) {
  struct Move const *h=&history[--history_len];
  int p=h->p,n=h->n,d=h->d;
  if(p!=PASS) {
    struct Piece const *q=&piece[n];
    int dx=dir_x[d],dy=dir_y[d];
    int i;
    board[p]=0;
    for(i=0;i<q->size-1;i++) board[p+q->x[i]*dx+q->y[i]*dy]=0;
    used[t-1][n]=0;
  }
}

/* 文字を36進数までの桁と見たときの数値を返す。 */
/* 桁になれない文字のときは0を返す。 */
/* 英字はコード順に並んでいるとは限らないので、まじめに変換する。 */
int ctoi(char c) {
  char buf[2];
  buf[0]=c; buf[1]=0;
  return (int)strtol(buf,NULL,36);
}

/* 4文字コードを読んでピースを置く */
int input_move(int t) {
  char buf[6];
  int c;
  int x,y,p,n,d;
  int f=nomove(t) ? nomove(3-t) ? 2 : 1 : 0;
retry:
  if(f<2) printf("%d-P%d %s ",history_len+1,t,f==0 ? "4文字コード:置く" : "----:パス" );
  printf("E:記録終了 ");
  if(history_len>0) printf("B:一手戻る ");
  if(history_len<history_max) {
    struct Move const *h=&history[history_len];
    printf("F:一手進む(%s) ",string_code(h));
    p=h->p; n=h->n; d=h->d;
  }
  putchar('?');
  if(fgets(buf,sizeof buf,stdin)==NULL) exit(1);
  if(buf[0]==0 || buf[0]=='\n') goto retry;
  if(buf[1]=='\n') {
    c=toupper((unsigned char)buf[0]);
    if(c=='E') return 1;
    if(history_len>0 && c=='B') { undo(3-t); return 0; }
    if(history_len<history_max && c=='F') {
      if(p!=PASS) goto put;
      if(f==1) { put(t,0,0,0); return 0; } else goto bad;
    }
    goto bad;
  }
  if(f==2) goto bad;
  if(f==1 && buf[0]=='-' && buf[1]=='-' &&
     buf[2]=='-' && buf[3]=='-' && buf[4]=='\n') { put(t,0,0,0); return 0; }
  x=ctoi(buf[0])-1;
  if(x<0 || x>=MAX_X) goto bad;
  y=ctoi(buf[1])-1;
  if(y<0 || y>=MAX_Y) goto bad;
  n=ctoi(buf[2])-10;
  if(n<0 || n>=MAX_PIECE) goto bad;
  d=ctoi(buf[3]);
  if(d<0 || d>=8) goto bad;
  if(buf[4]!='\n') goto bad;
  p=FENCE_LEN+x*X+y*Y;
put:
  if(used[t-1][n]) { puts("使用済み"); goto retry; }
  if(!check(t,p,n,d)) { puts("置けません"); goto retry; }
  put(t,p,n,d);
  return 0;
bad:
  puts("入力誤り");
  if(strchr(buf,'\n')==NULL) {
    while((c=getchar())!='\n') if(c==EOF) exit(1);
  }
  goto retry;
}

/* HTML出力の補助関数 */
static int write_beg_font(FILE *f,int t) {
  static char const *a[]={"sp","p1","p2"};
  return fprintf(f,"<FONT CLASS=%s>",a[t])<0;
}
static int write_end_font(FILE *f) {
  return fputs("</FONT>",f)==EOF;
}

/* ファイル名を読んでHTMLファイルを出力する */
void write_html(void) {
  char const *head=
    "<HTML>\n"
    "<HEAD>\n"
    "<STYLE TYPE=\"text/css\">\n"
    "<!--\n"
    "FONT.sp { background-color: silver }\n"
    "FONT.p1 { color: black; background-color: #FF9900 }\n"
    "FONT.p2 { color: white; background-color: #9900CC }\n"
    "-->\n"
    "</STYLE>\n"
    "</HEAD>\n"
    "<BODY>\n"
    "<TABLE><TR VALIGN=TOP>\n"
    "<TD>\n"
    "<PRE STYLE=\"line-height: 100%\">\n"
    "<FONT CLASS=p1>先手</FONT>:P1\n"
    "<FONT CLASS=p2>後手</FONT>:P2\n";
  char const *middle=
    "</PRE>\n"
    "<TD>\n"
    "<PRE STYLE=\"line-height: 100%\">\n";
  char const *tail=
    "</PRE>\n"
    "</TR></TABLE>\n"
    "</BODY>\n"
    "</HTML>\n";
  int b[MAX_Y*Y];
  int i,k,p,x,y;
  int p1=0,p2=0;
  FILE *f;
  char buf[256];
  char *bufp;
  int c;
  printf("HTMLファイル名?");
  if(fgets(buf,sizeof buf,stdin)==NULL) exit(1);
  if(buf[0]==0 || buf[0]=='\n') return;
  if((bufp=strchr(buf,'\n'))==NULL) {
    puts("長過ぎます");
    while((c=getchar())!='\n') if(c==EOF) exit(1);
    return;
  }
  *bufp=0;
  if((f=fopen(buf,"r"))!=NULL) {
    char buf1[3];
    fclose(f);
    printf("%sを上書きしてよいですか(Y/N)?",buf);
    if(fgets(buf1,sizeof buf1,stdin)==NULL) exit(1);
    if(strchr(buf1,'\n')==NULL) {
      while((c=getchar())!='\n') if(c==EOF) exit(1);
    }
    if(toupper((unsigned char)buf1[0])!='Y') return;
  }
  if((f=fopen(buf,"w"))==NULL) goto error0;
  if(fputs(head,f)==EOF) goto error;
  for(i=0;i<MAX_Y*Y;i++) b[i]=0;
  for(k=0;k<history_len;k++) {
    struct Move const *h=&history[k];
    int p=h->p,n=h->n,d=h->d;
    if(p!=PASS) {
      struct Piece const *q=&piece[n];
      int dx=dir_x[d],dy=dir_y[d];
      int i;
      p-=FENCE_LEN;
      b[p]=k+1;
      for(i=0;i<q->size-1;i++) b[p+q->x[i]*dx+q->y[i]*dy]=k+1;
      if((k&1)==0) p1+=q->size; else p2+=q->size;
    }
  }
  if(fputs("\n  ",f)==EOF) goto error;
  for(x=0;x<MAX_X;x++) if(fprintf(f," %X",x+1)<0) goto error;
  if(fputs("  \n",f)==EOF) goto error;
  p=0;
  for(y=0;y<MAX_Y;y++) {
    int t0,t;
    if(fprintf(f,"%X ",y+1)<0) goto error;
    for(x=0;x<MAX_X;x++) {
      k=b[p++];
      t=k==0 ? 0 : k&1 ? 1 : 2;
      if(x==0) {
        if(write_beg_font(f,t)) goto error;
      } else if(t!=t0) {
        if(write_end_font(f)) goto error;
        if(write_beg_font(f,t)) goto error;
      }
      if(k==0) {
        if(fputs("  ",f)==EOF) goto error;
      } else {
        if(fprintf(f,"%2d",k)<0) goto error;
      }
      t0=t;
    }
    if(write_end_font(f)) goto error;
    if(fputc('\n',f)==EOF) goto error;
    p++;
  }
  if(fprintf(f,"\nP1:<FONT CLASS=p1>%3d</FONT>\n"
                 "P2:<FONT CLASS=p2>%3d</FONT>\n",p1,p2)<0) goto error;
  if(fputs(middle,f)==EOF) goto error;
  for(k=0;k<history_len;k++) {
    if(fprintf(f,"<FONT CLASS=p%d>%2d</FONT>:",(k&1)+1,k+1)<0) goto error;
    if(fputs(string_code(&history[k]),f)==EOF) goto error;
    if(fputc('\n',f)==EOF) goto error;
  }
  if(fputs(tail,f)==EOF) goto error;
  if(fclose(f)==EOF) goto error;
  printf("%sに出力しました\n",buf);
  return;
error:
  fclose(f);
error0:
  puts("書き込みエラーです");
}

/* コマンド入力ループ */
void command_loop(void) {
  int t=1;
  char buf[3];
  int c;
record:
  for(;;) {
    print_board();
    if(input_move(t)) break;
    t=3-t;
  }
command:
  printf("P:4文字コード表示 H:HTML出力 R:記録再開 Q:終了 ?");
  /* 4文字コードのファイル読み書き機能は未実装 */
  if(fgets(buf,sizeof buf,stdin)==NULL) exit(1);
  c=toupper((unsigned char)buf[0]);
  if(c==0 || c=='\n') goto command;
  if(buf[1]!='\n') goto bad;
  if(c=='P') {
    int k;
    for(k=0;k<history_len;k++) print_code(&history[k]);
    goto command;
  }
  if(c=='H') { write_html(); goto command; }
  if(c=='R') goto record;
  if(c=='Q') return;
bad:
  puts("入力誤り");
  if(strchr(buf,'\n')==NULL) {
    while((c=getchar())!='\n') if(c==EOF) exit(1);
  }
  goto command;
}

int main() {
  init_board();
  command_loop();
  return 0;
}
