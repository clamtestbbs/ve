/*-------------------------------------------------------*/
/* edit.c       ( NTHU CS MapleBBS Ver 2.36 )            */
/*-------------------------------------------------------*/
/* target : simple ANSI/Chinese editor                   */
/* create : 1995/03/29                                   */
/* update : 2009/12/18                 Dopin: ¬å±¼¤£¥Îªº */
/*-------------------------------------------------------*/

#include <stdlib.h>
#include <sys/param.h>
#include "bbs.h"

#define KEEP_EDITING    -2
#define SCR_WIDTH       80
#define BACKUP_LIMIT    100

textline *firstline = NULL;
textline *lastline = NULL;
textline *currline = NULL;
textline *blockline = NULL;
textline *top_of_win = NULL;
textline *deleted_lines = NULL;

extern int KEY_ESC_arg;
extern int local_article;

char line[WRAPMARGIN + 2];
int currpnt, currln, totaln;
int curr_window_line;
int redraw_everything;
int insert_character;
int my_ansimode;
int raw_mode;
int edit_margin;
int phone_mode;
int blockln  = -1;
int blockpnt;
int prevln = -1;
int prevpnt;
int modified;
int indent_mode  = 1;
int insert_c = ' ';
struct stat st0;
char reset_color[4] = "[m";
char* FPath;

char fp_bak[] = "bak";
char *my_edit_mode[2] =
{"¨ú¥N", "´¡¤J"};

char save_title[STRLEN];

/* ----------------------------------------------------- */
/* °O¾ÐÅéºÞ²z»P½s¿è³B²z                                  */
/* ----------------------------------------------------- */

static void indigestion(i)
{
  fprintf(stderr, "ÄY­«¤º¶Ë %d\n", i);
}

/* ----------------------------------------------------- */
/* Thor: ansi ®y¼ÐÂà´«  for color ½s¿è¼Ò¦¡               */
/* ----------------------------------------------------- */

static int ansi2n(int ansix, textline * line)
{
  register char *data, *tmp;
  register char ch;

  data = tmp = line->data;

  while(*tmp) {
    if (*tmp == KEY_ESC)
    {
      while((ch = *tmp) && !isalpha(ch))
        tmp++;
      if (ch)
        tmp++;
      continue;
    }
    if (ansix <= 0) break;
    tmp++;
    ansix--;
  }
  return tmp - data;
}

static int n2ansi(int nx, textline * line)
{
  register ansix = 0;
  register char *tmp,*nxp;
  register char ch;

  tmp = nxp = line->data;
  nxp += nx;

  while(*tmp){
    if (*tmp == KEY_ESC)
    {
      while((ch = *tmp) && !isalpha(ch))
        tmp++;
      if (ch)
        tmp++;
      continue;
    }
    if (tmp >= nxp) break;
    tmp++;
    ansix++;
  }
  return ansix;
}

/* ----------------------------------------------------- */
/* ¿Ã¹õ³B²z¡G»²§U°T®§¡BÅã¥Ü½s¿è¤º®e                      */
/* ----------------------------------------------------- */

static void edit_msg(void)
{
  static char *edit_mode[2] = {"¨ú¥N", "´¡¤J"};
  register int n = currpnt;

  if (my_ansimode)                      /* Thor: §@ ansi ½s¿è */
    n = n2ansi(n, currline);
  n++;
  move(b_lines, 0);
  clrtoeol();
  prints("\033[%sm  ½s¿è¤å³¹  \033[31;47m  (Ctrl-G)\033[30m ½u¤W»²§U»¡©ú  \033[31m(^X,^Q)\033[30m ÀÉ®×³B²z ùø%s¢x%c%c%c%cùø %3d:%3d  \033[m",
    (modified == -1) ? "34;46" : "37;44",
    edit_mode[insert_character],
    my_ansimode ? 'A' : 'a', indent_mode ? 'I' : 'i',
    phone_mode ? 'P' : 'p', raw_mode ? 'R' : 'r',
    currln + 1, n);
}

static textline * back_line(pos, num)
  register textline *pos;
  int num;
{
  while (num-- > 0)
  {
    register textline *item;

    if (pos && (item = pos->prev))
    {
      pos = item;
      currln--;
    }
  }
  return pos;
}


static textline *forward_line(pos, num)
  register textline *pos;
  int num;
{
  while (num-- > 0)
  {
    register textline *item;

    if (pos && (item = pos->next))
    {
      pos = item;
      currln++;
    }
  }
  return pos;
}

static int getlineno()
{
  int cnt = 0;
  textline *p = currline;

  while (p && (p != top_of_win))
  {
    cnt++;
    p = p->prev;
  }
  return cnt;
}

static char *killsp(s)
  char *s;
{
  while (*s == ' ')
    s++;
  return s;
}

static textline *alloc_line()
{
  extern void *malloc();
  register textline *p;

  if (p = (textline *) malloc(sizeof(textline)))
  {
    memset(p, 0, sizeof(textline));
    return p;
  }

  indigestion(13);
  abort_bbs();
}

/* ----------------------------------------------------- */
/* append p after line in list. keeps up with last line  */
/* ----------------------------------------------------- */

static void append(p, line)
  register textline *p, *line;
{
  register textline *n;

  if (p->next = n = line->next)
    n->prev = p;
  else
    lastline = p;
  line->next = p;
  p->prev = line;
}


/* ----------------------------------------------------- */
/* delete_line deletes 'line' from the list,             */
/* and maintains the lastline, and firstline pointers.   */
/* ----------------------------------------------------- */

static void delete_line(line)
  register textline *line;
{
  register textline *p = line->prev;
  register textline *n = line->next;

  if (!p && !n)
  {
    line->data[0] = line->len = 0;
    return;
  }
  if (n)
    n->prev = p;
  else
    lastline = p;
  if (p)
    p->next = n;
  else
    firstline = n;
  strcat(line->data, "\n");
  line->prev = deleted_lines;
  deleted_lines = line;
  totaln--;
  modified = -2;
}

/* woju */

undelete_line()
{
   textline* p = deleted_lines;

   textline* currline0 = currline;
   textline* top_of_win0 = top_of_win;
   int currpnt0 = currpnt;
   int currln0 = currln;
   int curr_window_line0 = curr_window_line;
   int indent_mode0 = indent_mode;


   if (!deleted_lines)
      return 0;

   indent_mode = 0;
   insert_string(deleted_lines->data);
   indent_mode = indent_mode0;
   deleted_lines = deleted_lines->prev;
   free(p);

   currline = currline0;
   top_of_win = top_of_win0;
   currpnt = currpnt0;
   currln = currln0;
   curr_window_line = curr_window_line0;
   modified = -2;
}

int indent_spcs()
{
   textline* p;
   int spcs;

   if (!indent_mode)
      return 0;

   for (p = currline; p; p = p->prev) {
      for (spcs = 0; p->data[spcs] == ' '; ++spcs)
         ;
      if (p->data[spcs])
         return spcs;
   }
   return 0;
}

/* ----------------------------------------------------- */
/* split 'line' right before the character pos           */
/* ----------------------------------------------------- */

static void split(line, pos)
  register textline *line;
  register int pos;
{
  if (pos <= line->len)
  {
    register textline *p = alloc_line();
    register char *ptr;
    int spcs = indent_spcs();

    totaln++;

    p->len = line->len - pos + spcs;
    line->len = pos;

    memset(p->data, ' ', spcs);
    p->data[spcs] = 0;
    strcat(p->data, (ptr = line->data + pos));
    ptr[0] = '\0';
    append(p, line);
    if (line == currline && pos <= currpnt)
    {
      currline = p;
      if (pos == currpnt)
         currpnt = spcs;
      else
         currpnt -= pos;
      curr_window_line++;
      currln++;
    }
    redraw_everything = YEA;
    modified = -2;
  }
}

/* ----------------------------------------------------- */
/* 1) lines were joined and one was deleted              */
/* 2) lines could not be joined                          */
/* 3) next line is empty                                 */
/* returns false if:                                     */
/* 1) Some of the joined line wrapped                    */
/* ----------------------------------------------------- */

static int join(line)
  register textline *line;
{
  register textline *n;
  register int ovfl;

  if (!(n = line->next))
    return YEA;
  if (!*killsp(n->data))
    return YEA;

  modified = -2;
  ovfl = line->len + n->len - WRAPMARGIN;
  if (ovfl < 0)
  {
    strcat(line->data, n->data);
    line->len += n->len;
    delete_line(n);
    return YEA;
  }
  else
  {
    register char *s;

    s = n->data + n->len - ovfl - 1;
    while (s != n->data && *s == ' ')
      s--;
    while (s != n->data && *s != ' ')
      s--;
    if (s == n->data)
      return YEA;
    split(n, (s - n->data) + 1);
    if (line->len + n->len >= WRAPMARGIN)
    {
      indigestion(0);
      return YEA;
    }
    join(line);
    n = line->next;
    ovfl = n->len - 1;
    if (ovfl >= 0 && ovfl < WRAPMARGIN - 2)
    {
      s = &(n->data[ovfl]);
      if (*s != ' ')
      {
        strcpy(s, " ");
        n->len++;
      }
    }
    return NA;
  }
}

static void insert_char(ch)
  register int ch;
{
  register textline *p = currline;
  register int i = p->len;
  register char *s;
  int wordwrap = YEA;

  if (currpnt > i)
  {
    indigestion(1);
    return;
  }
  if (currpnt < i && !insert_character)
  {
    p->data[currpnt++] = ch;
    if (my_ansimode) /* Thor: ansi ½s¿è, ¥i¥Hoverwrite, ¤£»\¨ì ansi code */
       currpnt = ansi2n(n2ansi(currpnt, p),p);
  }
  else
  {
    while (i >= currpnt)
    {
      p->data[i + 1] = p->data[i];
      i--;
    }
    p->data[currpnt++] = ch;
    i = ++(p->len);
  }
  if (i < WRAPMARGIN)
    return;
  split(p, WRAPMARGIN / 2);
}

insert_string(str)
  char *str;
{
  int ch;

  while (ch = *str++)
  {

#ifdef BIT8
    if (isprint2(ch) || ch == '')
#else
    if (isprint(ch))
#endif

    {
      insert_char(ch);
    }
    else if (ch == '\t')
    {
      do
      {
        insert_char(' ');
      } while (currpnt & 0x7);
    }
    else if (ch == '\n')
      split(currline, currpnt);
  }
}

static void delete_char()
{
  register int len;

  if (len = currline->len)
  {
    register int i;
    register char *s;

    if (currpnt >= len)
    {
      indigestion(1);
      return;
    }
    for (i = currpnt, s = currline->data + i; i != len; i++, s++)
      s[0] = s[1];
    currline->len--;
  }
}

static void load_file(fp)
  FILE *fp;
{
  int indent_mode0 = indent_mode;

  indent_mode = 0;
  while (fgets(line, WRAPMARGIN + 2, fp))
    insert_string(line);
  fclose(fp);
  indent_mode = indent_mode0;
}


/* ----------------------------------------------------- */
/* ¼È¦sÀÉ                                                */
/* ----------------------------------------------------- */

static char *ask_tmpbuf()
{
  static char fp_buf[10] = "buf.0";
  static char msg[] = "½Ð¿ï¾Ü¼È¦sÀÉ (0-9)[0]: ";

  msg[19] = fp_buf[4];
  do
  {
    if (!getdata(3, 0, msg, fp_buf + 4, 4, DOECHO))
      fp_buf[4] = msg[19];
  } while (fp_buf[4] < '0' || fp_buf[4] > '9');
  return fp_buf;
}

static void read_tmpbuf(int n)
{
  FILE *fp;
  char fp_tmpbuf[80];
  char tmpfname[] = "buf.0";
  char *tmpf;
  char ans[4] = "y";

  if (0 <= n && n <= 9) {
     tmpfname[4] = '0' + n;
     tmpf = tmpfname;
  }
  else {
     tmpf = ask_tmpbuf();
     n = tmpf[4] - '0';
  }

  setuserfile(fp_tmpbuf, tmpf);

/* woju */
  if (n != 0 && n != 5 && more(fp_tmpbuf, NA) != -1)
     getdata(b_lines - 1, 0, "½T©wÅª¤J¶Ü(Y/N)?[Y]",  ans, 4, LCECHO);
  if (*ans != 'n' && (fp = fopen(fp_tmpbuf, "r")))
  {
    modified = -2;
    prevln = currln;
    prevpnt = currpnt;
    load_file(fp);
    while (curr_window_line >= b_lines)
    {
      curr_window_line--;
      top_of_win = top_of_win->next;
    }
  }
}

static void write_tmpbuf()
{
  FILE *fp;
  char fp_tmpbuf[80], ans[4];
  textline *p;

  setuserfile(fp_tmpbuf, ask_tmpbuf());
  if (dashf(fp_tmpbuf))
  {
    more(fp_tmpbuf, NA);
    getdata(b_lines - 1, 0, "¼È¦sÀÉ¤w¦³¸ê®Æ (A)ªþ¥[ (W)ÂÐ¼g (Q)¨ú®ø¡H[A] ",
      ans, 4, LCECHO);

    if (ans[0] == 'q')
      return;
  }

  fp = fopen(fp_tmpbuf, (ans[0] == 'w' ? "w" : "a+"));
  for (p = firstline; p; p = p->next)
  {
    if (p->next || p->data[0])
      fprintf(fp, "%s\n", p->data);
  }
  fclose(fp);
}

static void erase_tmpbuf()
{
  char fp_tmpbuf[80];
  char ans[4] = "n";

  setuserfile(fp_tmpbuf, ask_tmpbuf());
/* woju */
  if (more(fp_tmpbuf, NA) != -1)
     getdata(b_lines - 1, 0, "½T©w§R°£¶Ü(Y/N)?[N]",  ans, 4, LCECHO);
  if (*ans == 'y')
     unlink(fp_tmpbuf);
}

/* ----------------------------------------------------- */
/* ÀÉ®×³B²z¡GÅªÀÉ¡B¦sÀÉ¡B¼ÐÃD¡BÃ±¦WÀÉ                    */
/* ----------------------------------------------------- */

static void read_file(fpath)
  char *fpath;
{
  FILE *fp;

  if ((fp = fopen(fpath, "r")) == NULL)
    return;
  load_file(fp);
}

static int write_file(fpath, saveheader)
  char *fpath;
  int saveheader;
{
  FILE *fp;
  textline *p, *v;
  char ans[TTLEN], *msg;
  int aborted = 0;

  if (!saveheader) {
     stand_title("ÀÉ®×³B²z");
     getdata(1, 0, "[S]Àx¦s (A)©ñ±ó (E)Ä~Äò (R/W/D)Åª¼g§R¼È¦sÀÉ¡H", ans, 4, LCECHO);
  }
  else
     *ans = 's';

  switch (ans[0])
  {
  case 'a':
    outs("¤å³¹[1m ¨S¦³ [0m¦s¤J");
    aborted = -1;
    break;

  case 'r':
    read_tmpbuf(-1);

  case 'e':
    return KEEP_EDITING;

  case 'w':
    write_tmpbuf();
    return KEEP_EDITING;

  case 'd':
    erase_tmpbuf();
    return KEEP_EDITING;

  case 'S':
     saveheader = YEA;
  case 's':
    local_article = 0;
    break;
  }

  if (!aborted) {
     struct stat st;

     if (stat(FPath, &st) != -1 && st.st_mtime != st0.st_mtime) {
        char buf[200];

        sprintf(buf, "¼g¤JÄµ§i: %s ¤w³Q¨ä¥¦µ{¦¡§ó°Ê¹L", FPath);
        stand_title(buf);
        getdata(1, 0, "½T©wÀx¦s(Y/N)¡H[N]", ans, 4, LCECHO);
        if (*ans != 'y')
           return KEEP_EDITING;
     }
     if ((fp = fopen(fpath, "w")) == NULL) {
        stand_title("¼g¤J¥¢±Ñ: §ï¼g¨ì¼È¦sÀÉ");
        write_tmpbuf();
        return KEEP_EDITING;
     }
  }

  for (p = firstline; p; p = v)
  {
    v = p->next;
    if (!aborted)
    {
      strncpy(line, p->data, WRAPMARGIN - 1);
      if (v || line[0])
      {
        trim(line);
        fprintf(fp, "%s\n", line);
      }
    }
    if (!saveheader)
       free(p);
  }
  if (!saveheader)
     currline = NULL;

  if (!aborted) {
     fclose(fp);
     stat(FPath, &st0);
     modified = -1;
  }

  return saveheader ? KEEP_EDITING : aborted;
}

edit_outs(text)
  char *text;
{
  register int column = 0;
  register char ch;

  while ((ch = *text++) && (++column < SCR_WIDTH))
  {
    outch(ch == 27 ? '*' : ch);
  }
}

block_outs(char* text, int column)
{
  register char ch;

  while ((ch = *text++) && (++column < SCR_WIDTH))
  {
    outch(ch == 27 ? '*' : ch);
  }
}

int outs();

static void display_buffer()
{
  register textline *p;
  register int i;
  int inblock;
  char buf[WRAPMARGIN + 2];
  int min, max;
  int (*OUTS)();

  if (currpnt > blockpnt) {
     min = blockpnt;
     max = currpnt;
  }
  else {
     min = currpnt;
     max = blockpnt;
  }

  OUTS = my_ansimode ? outs : edit_outs;

  for (p = top_of_win, i = 0; i < b_lines; i++)
  {

    move(i, 0);
    clrtoeol();
/*
woju
*/
    if (blockln >= 0
        && (blockln <= currln
              && blockln <= (currln - curr_window_line + i)
              &&            (currln - curr_window_line + i) <= currln
           ||
                 currln  <= (currln - curr_window_line + i)
              &&            (currln - curr_window_line + i) <= blockln)) {
       outs("[7m");
       inblock = 1;
    }
    else
       inblock = 0;
    if (p)
    {
      if (currln == blockln && p == currline && max > min) {
         if (min > edit_margin) {
            outs("[0m");
            strncpy(buf, p->data + edit_margin, min - edit_margin);
            buf[min - edit_margin] = 0;
            block_outs(buf, 0);
         }
         outs("[7m");
         if (min > edit_margin) {
            strncpy(buf, p->data + min, max - min);
            buf[max - min] = 0;
            block_outs(buf, min - edit_margin);
         }
         else {
            strncpy(buf, p->data + edit_margin, max - edit_margin);
            buf[max - edit_margin] = 0;
            block_outs(buf, 0);
         }
         outs("[0m");
         block_outs(p->data + max, max - edit_margin);
      }
      else
         OUTS(&p->data[edit_margin]);
      p = p->next;
      if (inblock)
         outs("[0m");
    }
    else
      outch('~');
  }
  edit_msg();
}

static char *vedithelp[] =
{
  "\01¤@¯ë«ü¥O",
  "^X       ÀÉ®×³B²z       ^L             ­«·sÅã¥Üµe­±",
  "^V       ¤Á´«ANSI¦â±m   ^G             Åã¥Ü¥»¨D§Uµe­±",
  "\01´å¼Ð²¾°Ê«ü¥O",
  "¡ö       ©¹«á²¾°Ê¤@®æ   ^A,Home        ²¾¨ì¦¹¦æ¶}ÀY",
  "¡÷       ©¹«e²¾°Ê¤@®æ   ^E,End         ²¾¨ì¦¹¦æµ²§À",
  "¡ô,^P    ©¹¤W²¾°Ê¤@¦æ   (ESC-,)        ²¾¨ìÀÉ®×¶}ÀY",
  "¡õ,^N    ©¹¤U²¾°Ê¤@¦æ   (ESC-.),^T     ²¾¨ìÀÉ®×µ²§À",
  "^B,PgUp  ©¹¤W²¾°Ê¤@­¶   ^F,PgDn        ©¹¤U²¾°Ê¤@­¶",
  "\01§R°£´¡¤J«ü¥O",
  "^O,Ins   §ïÅÜ¼Ò¦¡¬° ´¡¤J/ÂÐ»\\[m",
  "^H,BS    §R°£«e¤@­Ó¦r¤¸",
  "(ESC-l)  ¼Ð°O°Ï¶ô",
  "^D,Del   §R°£¥Ø«eªº¦r¤¸         ANSI¢x¶Â¬õºñ¶ÀÂÅµµÀQ¥Õ",
  "^K       §R°£´å¼Ð¤§«á¦Ü¦æ§À     «e´º¢x[47;30;1m30[31m31[32m32[33m33[34m34[35m35[36m36[37m37[0m",
  "^Y       §R°£¥Ø«e³o¦æ           ­I´º¢x[40;33;1m40[41m41[42m42[43m43[44m44[45m45[46m46[47m47[0m",
  "\01¯S®í«ü¥O",
  "^U       ¿é¤J ESC ½X(¥H * ªí¥Ü)  ^C    ÁÙ­ì/³]©w ANSI ¦â±m",
  "§ó¸Ô²Óªº»¡©ú: ve.hlp",
NULL};

void
show_help(helptext)
  char *helptext[];
{
  char *str;
  int i;
  char resolvedname[MAXPATHLEN], *rp;

  clear();
  for (i = 0; (str = helptext[i]); i++)
  {
    if (*str == '\0')
      prints("[1m¡i %s ¡j[0m\n", str + 1);
    else if (*str == '\01')
      prints("\n[36m¡i %s ¡j[m\n", str + 1);
    else
      prints("        %s\n", str);
  }
  move(0, 0);
  prints((rp = realpath(FPath, resolvedname)) ? resolvedname : FPath);

  pressanykey();
}

/*
woju
lino == 0 for prompt
*/

goto_line(int lino)
{
   char buf[10];

   if (lino > 0 || getdata(b_lines - 1, 0, "¸õ¦Ü²Ä´X¦æ:", buf, 10, DOECHO)
       && sscanf(buf, "%d", &lino) && lino > 0) {
      textline* p;

      prevln = currln;
      prevpnt = currpnt;
      p = firstline;
      currln = lino - 1;

      while (--lino && p->next)
         p = p->next;

      if (p)
         currline = p;
      else {
         currln = totaln;
         currline = lastline;
      }
      currpnt = 0;
      if (currln < 11) {
         top_of_win = firstline;
         curr_window_line = currln;
      }
      else {
         int i;
         curr_window_line = 11;

         for (i = curr_window_line; i; i--)
            p = p->prev;
            top_of_win = p;
      }
   }
   redraw_everything = YEA;
}


char *strcasestr(const char* big, const char* little)
{
   char* ans = (char*)big;
   int len = strlen(little);
   char* endptr = (char*)big + strlen(big) - len;

   while (ans <= endptr)
      if (!strncasecmp(ans, little, len))
         return ans;
      else
         ans++;
   return 0;
}


/*
woju
mode:
    0: prompt
    1: forward
    -1: backward
*/
search_str(int mode)
{
   static char str[80];
   char buf[100];
   typedef char* (*FPTR)();
   static FPTR fptr;
   char ans[4] = "n";

   if (!mode) {
      sprintf(buf, "´M§ä¦r¦ê[%s]", str);
      if (getdata(b_lines - 1, 0, buf, buf, 40, DOECHO))
         strcpy(str, buf);
      if (*str)
         if (getdata(b_lines - 1, 0, "°Ï¤À¤j¤p¼g(Y/N/Q)? [N] ", ans, 4, LCECHO)
             && *ans == 'y')
            fptr = strstr;
         else
            fptr = strcasestr;
   }

   if (*str && *ans != 'q') {
      textline* p;
      char* pos  = 0;
      int lino;

      if (mode >= 0) {
         for (lino = currln, p = currline; p; p = p->next, lino++) {
            if (*str == KEY_ESC) {
               if (!strncmp(str + 1, p->data, strlen(str + 1))
                   && (lino != currln || currpnt)) {
                  pos = p->data;
                  break;
               }
            }
            else if ((pos =
                      fptr(p->data + (lino == currln ? currpnt + 1 : 0), str))
                && (lino != currln || pos - p->data != currpnt))
               break;
         }
      }
      else {
         for (lino = currln, p = currline; p; p = p->prev, lino--) {
            if (*str == KEY_ESC) {
               if (!strncmp(str + 1, p->data, strlen(str + 1))
                   && (lino != currln || currpnt)) {
                  pos = p->data;
                  break;
               }
            }
            if ((pos = fptr(p->data, str))
                && (lino != currln || pos - p->data != currpnt))
               break;
            else
               pos = 0;
         }
      }
      if (pos) {
         prevln = currln;
         prevpnt = currpnt;
         currline = p;
         currln = lino;
         currpnt = pos - p->data;
         if (lino < 11) {
            top_of_win = firstline;
            curr_window_line = currln;
         }
         else {
            int i;
            curr_window_line = 11;

            for (i = curr_window_line; i; i--)
               p = p->prev;
            top_of_win = p;
         }
         redraw_everything = YEA;
      }
   }
   if (!mode)
      redraw_everything = YEA;
}

match_paren()
{
   static char parens[] = "()[]{}<>";
   int type, mode;
   int parenum = 0;
   char *ptype;
   textline* p;
   int lino;
   int c, i;

   if (!(ptype = strchr(parens, currline->data[currpnt])))
      return;

   type = (ptype - parens) / 2;
   parenum += ((ptype - parens) % 2) ? -1 : 1;


   if (parenum > 0) {
     for (lino = currln, p = currline; p; p = p->next, lino++) {
         lino = lino;
         for (i = (lino == currln) ? currpnt + 1 : 0; i < strlen(p->data); i++)
            if (p->data[i] == '/' && p->data[++i] == '*') {
               ++i;
               while (1) {
                  while(i < strlen(p->data) - 1
                      && !(p->data[i] == '*' && p->data[i + 1] == '/'))
                     i++;
                  if (i >= strlen(p->data) - 1 && p->next) {
                     p = p->next;
                     ++lino;
                     i = 0;
                  }
                  else
                     break;
               }
            }
            else if ((c = p->data[i]) == '\'' || c == '"') {
               while (1) {
                  while (i < (int)(strlen(p->data) - 1))
                     if (p->data[++i] == '\\' && i < strlen(p->data) - 2)
                        ++i;
                     else if (p->data[i] == c)
                        goto end_quote;
                  if (i >= strlen(p->data) - 1 && p->next) {
                     p = p->next;
                     ++lino;
                     i = -1;
                  }
                  else
                     break;
               }
end_quote:
               ;
            }
            else if ((ptype = strchr(parens, p->data[i]))
                     && (ptype - parens) / 2 == type)
               if (!(parenum += ((ptype - parens) % 2) ? -1 : 1))
                  goto p_outscan;
     }
   }
   else {
      for (lino = currln, p = currline; p; p = p->prev, lino--)
         for (i = (lino == currln) ? currpnt - 1 : strlen(p->data) - 1;
              i >= 0; i--)
            if (p->data[i] == '/' && p->data[--i] == '*' && i >= 0) {
               --i;
               while (1) {
                  while(i > 0
                      && !(p->data[i] == '*' && p->data[i - 1] == '/'))
                     i--;
                  if (i <= 0 && p->prev) {
                     p = p->prev;
                     --lino;
                     i = strlen(p->data) - 1;
                  }
                  else
                     break;
               }
            }
            else if ((c = p->data[i]) == '\'' || c == '"') {
               while (1) {
                  while (i > 0)
                     if (i > 1 && p->data[i - 2] == '\\')
                        i -= 2;
                     else if ((p->data[--i]) == c)
                        goto begin_quote;
                  if (i <= 0 && p->prev) {
                     p = p->prev;
                     --lino;
                     i = strlen(p->data);
                  }
                  else
                     break;
               }
begin_quote:
               ;
            }
            else if ((ptype = strchr(parens, p->data[i]))
                     && (ptype - parens) / 2 == type)
               if (!(parenum += ((ptype - parens) % 2) ? -1 : 1))
                  goto p_outscan;
   }

p_outscan:

   if (!parenum) {
      int top = currln - curr_window_line;
      int bottom = currln - curr_window_line + b_lines - 1;

      currpnt = i;
      currline = p;
      curr_window_line += lino - currln;
      currln = lino;

      if (lino < top || lino > bottom) {
         if (lino < 11) {
            top_of_win = firstline;
            curr_window_line = currln;
         }
         else {
            int i;
            curr_window_line = 11;

            for (i = curr_window_line; i; i--)
               p = p->prev;
            top_of_win = p;
         }
         redraw_everything = YEA;
      }
   }
}


block_del(int hide)
{
   if (blockln < 0) {
      blockln = currln;
      blockpnt = currpnt;
      blockline = currline;
   }
   else {
      char fp_tmpbuf[80];
      FILE* fp;
      textline *begin, *end, *p;
      int lines;
      char tmpfname[10] = "buf.0";
      char ans[6] = "w+n";

      move(b_lines - 1, 0);
      clrtoeol();
      if (hide == 1)
         tmpfname[4] = 'q';
      else if (!hide && !getdata(b_lines - 1, 0, "§â°Ï¶ô²¾¦Ü¼È¦sÀÉ (0:Cut, 5:Copy, 6-9, q: Cancel)[0] ",  tmpfname + 4, 4, LCECHO))
         tmpfname[4] = '0';
      if (tmpfname[4] < '0' || tmpfname[4] > '9')
         tmpfname[4] = 'q';
      if ('1' <= tmpfname[4] && tmpfname[4] <= '9') {
         setuserfile(fp_tmpbuf, tmpfname);
         if (tmpfname[4] != '5' && dashf(fp_tmpbuf)) {
            more(fp_tmpbuf, NA);
            getdata(b_lines - 1, 0, "¼È¦sÀÉ¤w¦³¸ê®Æ (A)ªþ¥[ (W)ÂÐ¼g (Q)¨ú®ø¡H[W] ", ans, 4, LCECHO);
            if (*ans == 'q')
               tmpfname[4] = 'q';
            else if (*ans != 'a')
               *ans = 'w';
          }
          if (tmpfname[4] != '5') {
             getdata(b_lines - 1, 0, "§R°£°Ï¶ô(Y/N)?[N] ", ans + 2, 4, LCECHO);
             if (ans[2] != 'y')
                ans[2] = 'n';
          }
      }
      else if (hide != 3)
         ans[2] = 'y';

      tmpfname[5] = ans[1] = ans[3] = 0;
      if (tmpfname[4] != 'q') {
         if (currln >= blockln) {
            begin = blockline;
            end = currline;
         }
         else {
            begin = currline;
            end = blockline;
         }

         if (ans[2] == 'y' && !(begin == end && currpnt != blockpnt)) {
            if (currln > blockln) {
               curr_window_line -= (currln - blockln);
               currln = blockln;
               if (curr_window_line < 0) {
                  curr_window_line = 0;
                  if (end->next)
                     top_of_win = end->next;
                  else
                     top_of_win = begin->prev;
               }
            }
            if (!curr_window_line)
               if (end->next)
                  top_of_win = end->next;
               else
                  top_of_win = begin->prev;

            if (begin->prev)
               begin->prev->next = end->next;
            else if (end->next)
               top_of_win = firstline = end->next;
            else {
               currline = top_of_win = firstline = lastline = alloc_line();

               currln = curr_window_line = edit_margin = 0;
            }

            if (end->next)
               (currline = end->next)->prev = begin->prev;
            else if (begin->prev) {
               currline = (lastline = begin->prev);
               currln--;
               if (curr_window_line > 0)
                  curr_window_line--;
            }
         }

         setuserfile(fp_tmpbuf, tmpfname);
         if (fp = fopen(fp_tmpbuf, ans)) {
            if (begin == end && currpnt != blockpnt) {
               char buf[WRAPMARGIN + 2];

               if (currpnt > blockpnt) {
                  strcpy(buf, begin->data + blockpnt);
                  buf[currpnt - blockpnt] = 0;
               }
               else {
                  strcpy(buf, begin->data + currpnt);
                  buf[blockpnt - currpnt] = 0;
               }
               fputs(buf, fp);
            }
            else {
               for (p = begin; p != end; p = p->next)
                  fprintf(fp, "%s\n", p->data);
               fprintf(fp, "%s\n", end->data);
            }
            fclose(fp);
         }

         if (ans[2] == 'y') {
            modified = -2;
            if (begin == end && currpnt != blockpnt) {
               int min, max;

               if (currpnt > blockpnt) {
                  min = blockpnt;
                  max = currpnt;
               }
               else {
                  min = currpnt;
                  max = blockpnt;
               }
               strcpy(begin->data + min, begin->data + max);
               begin->len -= max - min;
               currpnt = min;
            }
            else {
               for (p = begin; p != end; totaln--)
                  free((p = p->next)->prev);
               free(end);
               totaln--;
               currpnt = 0;
            }
         }
      }
      blockln = -1;
      redraw_everything = YEA;
   }
}

block_shift_left()
{
   textline *begin, *end, *p;

   if (currln >= blockln) {
      begin = blockline;
      end = currline;
   }
   else {
      begin = currline;
      end = blockline;
   }
   p = begin;
   while (1) {
      if (p->len) {
         strcpy(p->data, p->data + 1);
         --p->len;
      }
      if (p == end)
         break;
      else
         p = p->next;
   }
   if (currpnt > currline->len)
      currpnt = currline->len;
   modified = -2;
   redraw_everything = YEA;
}

block_shift_right()
{
   textline *begin, *end, *p;

   if (currln >= blockln) {
      begin = blockline;
      end = currline;
   }
   else {
      begin = currline;
      end = blockline;
   }
   p = begin;
   while (1) {
      if (p->len < WRAPMARGIN) {
         int i = p->len + 1;

         while (i--)
            p->data[i + 1] = p->data[i];
         p->data[0] = insert_character ? ' ' : insert_c;
         ++p->len;
      }
      if (p == end)
         break;
      else
         p = p->next;
   }
   if (currpnt > currline->len)
      currpnt = currline->len;
   modified = -2;
   redraw_everything = YEA;
}


transform_to_color(char* line)
{
   while (line[0] && line[1])
      if (line[0] == '*' && line[1] == '[') {
         modified = -2;
         line[0] = KEY_ESC;
         line += 2;
      }
      else
         ++line;
}


block_color()
{
   textline *begin, *end, *p;

   if (currln >= blockln) {
      begin = blockline;
      end = currline;
   }
   else {
      begin = currline;
      end = blockline;
   }
   p = begin;
   while (1) {
      transform_to_color(p->data);
      if (p == end)
         break;
      else
         p = p->next;
   }
   block_del(1);
}


/* ----------------------------------------------------- */
/* ½s¿è³B²z¡G¥Dµ{¦¡¡BÁä½L³B²z                            */
/* ----------------------------------------------------- */

extern int my_write();

int vedit(fpath, saveheader)
  char *fpath;
  int saveheader;
{
  int ch, foo;
  int lastindent = -1;
  int last_margin;
  int line_dirty = 0;
  char* pstr;
  extern char* phone_char();

  FPath = fpath;
  stat(FPath, &st0);

  insert_character = redraw_everything = 1;

  currpnt = totaln = my_ansimode = 0;
  if (currline == NULL)
     currline = top_of_win = firstline = lastline = alloc_line();

  if (*fpath)
  {
    read_file(fpath);
  }

  currline = firstline;
  currpnt = currln = curr_window_line = edit_margin = last_margin = 0;
  modified = -1;

  while (1)
  {
    if (redraw_everything || blockln >=0)
    {
      display_buffer();
      redraw_everything = NA;
    }
    if (my_ansimode)
       ch = n2ansi(currpnt, currline);
    else
       ch = currpnt - edit_margin;
    move(curr_window_line, ch);
    if (!line_dirty && strcmp(line, currline->data))
       strcpy(line, currline->data);
    ch = igetkey();

    if (raw_mode)
       switch (ch) {
       case Ctrl('S'):
       case Ctrl('Q'):
       case Ctrl('T'):
          continue;
          break;
       }

/*
    if (ch == Ctrl('J') && !raw_mode)
       goto_line(0);
*/
    else if (phone_mode && (pstr = phone_char(ch)) || ch < 0x100 && isprint2(ch))
    {
      if (phone_mode && pstr)
         insert_string(pstr);
      else
         insert_char(ch);
      lastindent = -1;
      if (modified > -2)
         modified = currln;
      line_dirty = 1;
    }
    else
    {
      if (ch == Ctrl('P') || ch == KEY_UP || ch == KEY_DOWN || ch == Ctrl('N'))
      {
        if (lastindent == -1)
          lastindent = currpnt;
      }
      else
        lastindent = -1;

/*
woju
*/
      if (ch == KEY_ESC)
         switch (KEY_ESC_arg) {
         case ',':
            ch = Ctrl(']');
            break;
         case '.':
            ch = Ctrl('T');
            break;
         case 'v':
            ch = KEY_PGUP;
            break;
         case 'a':
         case 'A':
            ch = Ctrl('V');
            break;
         case 'X':
            ch = Ctrl('X');
            break;
         case 'q':
            ch = Ctrl('Q');
            break;
         case 'o':
            ch = Ctrl('O');
            break;
         case '-':
            ch = Ctrl('_');
            break;
         case 's':
            ch = Ctrl('S');
            break;
         }

      switch (ch)
      {
      case Ctrl('X'):           /* Save and exit */
        foo = write_file(fpath, saveheader);
        if (foo != KEEP_EDITING)
        {
          return foo;
        }
        line_dirty = 1;
        redraw_everything = YEA;
        break;

      case Ctrl('W'):
         if (blockln >= 0)
            block_del(2);
         line_dirty = 1;
         break;

      case Ctrl('Q'):           /* Quit without saving */
        ch = ask("µ²§ô¦ý¤£Àx¦s (Y/N)? [N]: ");
        if (ch == 'y' || ch == 'Y') {
           clear();
           return 0;
        }
        line_dirty = 1;
        redraw_everything = YEA;
        break;

      case Ctrl('C'):
        ch = insert_character;
        insert_character = redraw_everything = YEA;

        if (!my_ansimode)
        {
          insert_string(reset_color);
         if (modified > -2)
             modified = currln;
        }
        else
        {
          char ans[4];
          move(b_lines - 2, 55);
          outs("\033[1;33;40mB\033[41mR\033[42mG\033[43mY\033[44mL\033[45mP\033[46mC\033[47mW\033[m");
          if (getdata(b_lines - 1, 0, "½Ð¿é¤J  «G«×/«e´º/­I´º[¥¿±`¥Õ¦r¶Â©³][0wb]¡G", ans, 4, LCECHO))
          {
            char t[] = "BRGYLPCW";
            char color[15];
            char *tmp, *apos = ans;
            int fg, bg;

            if (!strchr(ans, 'q')) {
               strcpy(color, "\033[");
               if (isdigit(*apos))
               {
                 sprintf(color, "%s%c", color, *(apos++));
                 if (*apos)
                   sprintf(color, "%s;", color);
               }
               if (*apos)
               {
                 if (tmp = strchr(t, toupper(*(apos++))))
                   fg = tmp - t + 30;
                 else
                   fg = 37;
                 sprintf(color, "%s%d", color, fg);
               }
               if (*apos)
               {
                 if (tmp = strchr(t, toupper(*(apos++))))
                   bg = tmp - t + 40;
                 else
                   bg = 40;
                 sprintf(color, "%s;%d", color, bg);
               }
               sprintf(color, "%sm", color);
               insert_string(color);
               if (modified > -2)
                  modified = currln;
            }
          }
          else {
            insert_string(reset_color);
            if (modified > -2)
               modified = currln;
          }
        }
        insert_character = ch;
        line_dirty = 1;
        break;

      case KEY_ESC:
         line_dirty = 0;
         switch (KEY_ESC_arg) {
         case 'w':
            ch = ask("½T©w¦sÀÉ (Y/N)? [Y]: ");
            if (!(ch == 'n' || ch == 'N'))
               write_file(fpath, YEA);
            line_dirty = 1;
            redraw_everything = YEA;
            break;
         case 'x':
            if (prevln >= 0) {
               int prevln0 = prevln;
               int prevpnt0 = prevpnt;
               prevln = currln;
               prevpnt = currpnt;
               goto_line(prevln0 + 1);
               currpnt = prevpnt0;
            }
            break;
         case 'n':
            search_str(1);
            edit_margin = currpnt < SCR_WIDTH - 1 ? 0 : currpnt / 72 * 72;
            if (edit_margin != last_margin)
               redraw_everything = NA;
            break;
         case 'p':
            search_str(-1);
            edit_margin = currpnt < SCR_WIDTH - 1 ? 0 : currpnt / 72 * 72;
            if (edit_margin != last_margin)
               redraw_everything = NA;
            break;
         case 'J':
         case 'L':
         case 'g':
            goto_line(0);
            break;
         case ']':
            match_paren();
            break;
         case '0':
         case '5':
         case '6':
         case '7':
         case '8':
         case '9':
         {
            int currln0 = currln;
            int currpnt0 = currpnt;

            read_tmpbuf(KEY_ESC_arg - '0');
            redraw_everything = (currln0 == currln && currpnt0 != currpnt) ? NA : YEA;
         }
            break;
         case 'l':                       /* block delete */
         case ' ':
            block_del(0);
            line_dirty = 1;
            break;
         case 'u':
            if (blockln >= 0)
               block_del(1);
            line_dirty = 1;
            break;
         case 'c':
            if (blockln >= 0)
               block_del(3);
            line_dirty = 1;
            break;
         case 'y':
            undelete_line();
            break;
         case 'P':
            phone_mode ^= 1;
            line_dirty = 1;
            break;
         case 'R':
            raw_mode ^= 1;
            line_dirty = 1;
            break;
         case 'I':
            indent_mode ^= 1;
            line_dirty = 1;
            break;
         case 'j':
            if (blockln >= 0)
               block_shift_left();
            else if (currline->len) {
               int currpnt0 = currpnt;
               if (modified > -2)
                  modified = currln;
               currpnt = 0;
               delete_char();
               currpnt = (currpnt0 <= currline->len) ? currpnt0 : currpnt0 - 1;
               if (my_ansimode)
                   currpnt = ansi2n(n2ansi(currpnt, currline),currline);
            }
            line_dirty = 1;
            break;
         case 'k':
            if (blockln >= 0)
               block_shift_right();
            else {
               int currpnt0 = currpnt;
              if (modified > -2)
                  modified = currln;
               currpnt = 0;
               insert_char(' ');
               currpnt = currpnt0;
            }
            line_dirty = 1;
            break;
         case 'f':
            while (currpnt < currline->len && isalnum(currline->data[++currpnt]))
               ;
            while (currpnt < currline->len && isspace(currline->data[++currpnt]))
               ;
            line_dirty = 1;
            break;
         case 'b':
            while (currpnt && isalnum(currline->data[--currpnt]))
               ;
            while (currpnt && isspace(currline->data[--currpnt]))
               ;
            line_dirty = 1;
            break;
         case 'd':
            if (modified > -2)
               modified = currln;
            while (currpnt < currline->len) {
               delete_char();
               if (!isalnum(currline->data[currpnt]))
                  break;
            }
            while (currpnt < currline->len && isspace(currline->data[currpnt]))
               delete_char();
            line_dirty = 1;
            break;
         default:
            line_dirty = 1;
         }
         break;
      case Ctrl('S'):
         search_str(0);
         edit_margin = currpnt < SCR_WIDTH - 1 ? 0 : currpnt / 72 * 72;
         if (edit_margin != last_margin)
            redraw_everything = NA;
         break;
      case Ctrl('_'):
         if (strcmp(line, currline->data)) {
            char buf[WRAPMARGIN];

            strcpy(buf, currline->data);
            strcpy(currline->data, line);
            strcpy(line, buf);
            currline->len = strlen(currline->data);
            currpnt = 0;
            line_dirty = 1;
            if (modified == currln)
               modified = -1;
         }
         break;
      case Ctrl('U'):
        if (modified > -2)
           modified = currln;
        insert_char('');
        line_dirty = 1;
        break;

      case Ctrl('V'):                   /* Toggle ANSI color */
         my_ansimode ^= 1;
         if (my_ansimode && blockln >= 0)
            block_color();
         clear();
         redraw_everything = YEA;
         line_dirty = 1;
         break;

      case Ctrl('I'):
        if (modified > -2)
           modified = currln;
        do
        {
          insert_char(' ');
        }
        while (currpnt & 0x7);
        line_dirty = 1;
        break;

      case '\r':
      case '\n':
        split(currline, currpnt);
        line_dirty = 0;
        break;
      case Ctrl('Z'):
        clear();
        refresh();
        reset_tty();
        system(getenv("SHELL"));
        restore_tty();
        clear();
        redraw_everything = YEA;
        line_dirty = 1;
        break;
      case Ctrl('L'):
        clear();
        redraw_everything = YEA;
        line_dirty = 1;
        break;
      case Ctrl('G'):
        show_help(vedithelp);
        redraw_everything = YEA;
        line_dirty = 1;
        break;

      case KEY_LEFT:
        if (currpnt) {
          if (my_ansimode)
             currpnt = n2ansi(currpnt, currline);
          currpnt--;
          if (my_ansimode)
             currpnt = ansi2n(currpnt, currline);
          line_dirty = 1;
        }
        else if (currline->prev)
        {
          curr_window_line--;
          currln--;
          currline = currline->prev;
          currpnt = currline->len;
          line_dirty = 0;
        }
        break;

      case KEY_RIGHT:
        if (currline->len != currpnt) {
          if (my_ansimode)
             currpnt = n2ansi(currpnt, currline);
          currpnt++;
          if (my_ansimode)
             currpnt = ansi2n(currpnt, currline);
          line_dirty = 1;
        }
        else if (currline->next)
        {
          currpnt = 0;
          curr_window_line++;
          currln++;
          currline = currline->next;
          line_dirty = 0;
        }
        break;

      case KEY_UP:
      case Ctrl('P'):
        if (currline->prev)
        {
          if (my_ansimode)
             ch = n2ansi(currpnt,currline);
          curr_window_line--;
          currln--;
          currline = currline->prev;
          if (my_ansimode)
             currpnt = ansi2n(ch , currline);
          else
             currpnt = (currline->len > lastindent) ? lastindent : currline->len;
          line_dirty = 0;
        }
        break;

      case KEY_DOWN:
      case Ctrl('N'):
        if (currline->next)
        {
          if (my_ansimode)
             ch = n2ansi(currpnt,currline);
          currline = currline->next;
          curr_window_line++;
          currln++;
          if (my_ansimode)
             currpnt = ansi2n(ch , currline);
          else
             currpnt = (currline->len > lastindent) ? lastindent : currline->len;
          line_dirty = 0;
        }
        break;
      case Ctrl('B'):
      case KEY_PGUP:
        redraw_everything = currln;
        top_of_win = back_line(top_of_win, 22);
        currln = redraw_everything;
        currline = back_line(currline, 22);
        curr_window_line = getlineno();
        if (currpnt > currline->len)
          currpnt = currline->len;
        redraw_everything = YEA;
        line_dirty = 0;
        break;

      case KEY_PGDN:
      case Ctrl('F'):
        redraw_everything = currln;
        top_of_win = forward_line(top_of_win, 22);
        currln = redraw_everything;
        currline = forward_line(currline, 22);
        curr_window_line = getlineno();
        if (currpnt > currline->len)
          currpnt = currline->len;
        redraw_everything = YEA;
        line_dirty = 0;
        break;

      case KEY_END:
      case Ctrl('E'):
        trim(currline->data);
        currpnt = currline->len = strlen(currline->data);
        line_dirty = 1;
        break;

      case Ctrl(']'):   /* start of file */
        prevln = currln;
        prevpnt = currpnt;
        currline = top_of_win = firstline;
        currpnt = currln = curr_window_line = 0;
        redraw_everything = YEA;
        line_dirty = 0;
        break;

      case Ctrl('T'):           /* tail of file */
        prevln = currln;
        prevpnt = currpnt;
        top_of_win = back_line(lastline, 23);
        currline = lastline;
        curr_window_line = getlineno();
        currln = totaln;
        redraw_everything = YEA;
        currpnt = 0;
        line_dirty = 0;
        break;

      case KEY_HOME:
      case Ctrl('A'):
        currpnt = 0;
        line_dirty = 1;
        break;

      case KEY_INS:             /* Toggle insert/overwrite */
      case Ctrl('O'):
        if (blockln >= 0 && insert_character) {
           char ans[4];

           getdata(b_lines - 1, 0, "°Ï¶ô·L½Õ¥k²¾´¡¤J¦r¤¸(¹w³]¬°ªÅ¥Õ¦r¤¸)", ans, 4);
           insert_c = (*ans) ? *ans : ' ';
        }
        insert_character ^= 1;
        line_dirty = 1;
        break;

      case Ctrl('H'):
      case '\177':              /* backspace */
        line_dirty = 1;
        if (my_ansimode) {
           my_ansimode = 0;
           clear();
           redraw_everything = YEA;
        }
        else {
           if (currpnt == 0) {
              textline *p;

              if (!currline->prev)
              {
                break;
              }
              line_dirty = 0;
              curr_window_line--;
              currln--;
              currline = currline->prev;
              currpnt = currline->len;
              redraw_everything = YEA;
              if (*killsp(currline->next->data) == '\0')
              {
                delete_line(currline->next);
                break;
              }
              p = currline;
              while (!join(p))
              {
                p = p->next;
                if (p == NULL)
                {
                  indigestion(2);
                  abort_bbs();
                }
              }
              break;
           }
           if (modified > -2)
              modified = currln;
           currpnt--;
           delete_char();
        }
        break;

      case Ctrl('D'):
      case KEY_DEL:             /* delete current character */
        line_dirty = 1;
        if (currline->len == currpnt)
        {
          textline *p = currline;

          while (!join(p))
          {
            p = p->next;
            if (p == NULL)
            {
              indigestion(2);
              abort_bbs();
            }
          }
          line_dirty = 0;
          redraw_everything = YEA;
        }
        else {
           delete_char();
           if (modified > -2)
              modified = currln;
           if (my_ansimode)
              currpnt = ansi2n(n2ansi(currpnt, currline),currline);
        }
        break;

      case Ctrl('Y'):           /* delete current line */
        modified = -2;
        currline->len = currpnt = 0;

      case Ctrl('K'):           /* delete to end of line */
        if (currline->len == 0)
        {
          textline *p = currline->next;

          if (!p)
          {
            p = currline->prev;
            if (!p)
            {
              break;
            }
            if (curr_window_line > 0)
            {
              curr_window_line--;
              currln--;
            }
          }
          if (currline == top_of_win)
            top_of_win = p;
          delete_line(currline);
          currline = p;
          redraw_everything = YEA;
          modified = -2;
          line_dirty = 0;
          break;
        }

        if (currline->len == currpnt)
        {
          textline *p = currline;

          while (!join(p))
          {
            p = p->next;
            if (p == NULL)
            {
              indigestion(2);
              abort_bbs();
            }
          }
          redraw_everything = YEA;
          line_dirty = 0;
          break;
        }
        currline->len = currpnt;
        currline->data[currpnt] = '\0';
        line_dirty = 1;
        if (modified > -2)
           modified = currln;
        break;
      }

      if (currln < 0)
        currln = 0;
      if (curr_window_line < 0)
      {
        curr_window_line = 0;
        if (!top_of_win->prev)
        {
          indigestion(6);
        }
        else
        {
          top_of_win = top_of_win->prev;
          rscroll();
        }
      }
      if (curr_window_line == b_lines)
      {
        curr_window_line = t_lines - 2;
        if (!top_of_win->next)
        {
          indigestion(7);
        }
        else
        {
          top_of_win = top_of_win->next;
          move(b_lines, 0);
          clrtoeol();
          scroll();
        }
      }
    }
    edit_margin = currpnt < SCR_WIDTH - 1 ? 0 : currpnt / 72 * 72;

    if (!redraw_everything)
    {
      if (edit_margin != last_margin)
      {
        last_margin = edit_margin;
        redraw_everything = YEA;
      }
      else
      {
         move(curr_window_line, 0);
         clrtoeol();
         if (my_ansimode)
           outs(currline->data);
         else
            edit_outs(&currline->data[edit_margin]);
         edit_msg();
      }
    }
  }
}
