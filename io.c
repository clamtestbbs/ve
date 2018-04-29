/*-------------------------------------------------------*/
/* io.c         ( NTHU CS MapleBBS Ver 2.36 )            */
/*-------------------------------------------------------*/
/* target : basic console/screen/keyboard I/O routines   */
/* create : 1995/02/28                                   */
/* update : 2009/12/18               Dopin: 不用的砍光光 */
/*-------------------------------------------------------*/

#include "bbs.h"

#ifdef AIX
#include <sys/select.h>
#endif

#ifdef  Linux
#define OBUFSIZE  (2048)
#define IBUFSIZE  (128)
#else
#define OBUFSIZE  (4096)
#define IBUFSIZE  (256)
#endif

#define INPUT_ACTIVE    0
#define INPUT_IDLE      1

int KEY_ESC_arg;

static char outbuf[OBUFSIZE];
static int obufsize = 0;

static char inbuf[IBUFSIZE];
static int ibufsize = 0;
static int icurrchar = 0;

static int i_mode = INPUT_ACTIVE;

extern int dumb_term;

/* ----------------------------------------------------- */
/* output routines                                       */
/* ----------------------------------------------------- */

void
oflush()
{
  if (obufsize)
  {
    write(1, outbuf, obufsize);
    obufsize = 0;
  }
}

void
output(s, len)
  char *s;
{
  /* Invalid if len >= OBUFSIZE */

  if (obufsize + len > OBUFSIZE)
  {
    write(1, outbuf, obufsize);
    obufsize = 0;
  }
  memcpy(outbuf + obufsize, s, len);
  obufsize += len;
}

void ochar(c)
{
  if (obufsize > OBUFSIZE - 1)
  {
    write(1, outbuf, obufsize);
    obufsize = 0;
  }
  outbuf[obufsize++] = c;
}

/* ----------------------------------------------------- */
/* input routines                                        */
/* ----------------------------------------------------- */

static int i_newfd = 0;
static struct timeval i_to, *i_top = NULL;
static int (*flushf) () = NULL;

int num_in_buf()
{
  return icurrchar - ibufsize;
}

int igetch()
{
  int ch;

  for (;;)
  {
    if (ibufsize == icurrchar)
    {
      fd_set readfds;
      struct timeval to;

      to.tv_sec = to.tv_usec = 0;
      FD_ZERO(&readfds);
      FD_SET(0, &readfds);
      if (i_newfd)
        FD_SET(i_newfd, &readfds);
      if ((ch = select(FD_SETSIZE, &readfds, NULL, NULL, &to)) <= 0)
      {
        if (flushf)
          (*flushf) ();

        if (dumb_term)
          oflush();
        else
          refresh();

        FD_ZERO(&readfds);
        FD_SET(0, &readfds);
        if (i_newfd)
          FD_SET(i_newfd, &readfds);

        while ((ch = select(FD_SETSIZE, &readfds, NULL, NULL, i_top)) < 0)
        {
          if (errno == EINTR)
            continue;
          else
          {
            perror("select");
            return -1;
          }
        }
        if (ch == 0)
          return I_TIMEOUT;
      }
      if (i_newfd && FD_ISSET(i_newfd, &readfds))
        return I_OTHERDATA;

      while ((ibufsize = read(0, inbuf, IBUFSIZE)) <= 0)
      {
        if (ibufsize == 0)
          longjmp(byebye, -1);
        if (ibufsize < 0 && errno != EINTR)
          longjmp(byebye, -1);
      }
      icurrchar = 0;
    }

    i_mode = INPUT_ACTIVE;
    ch = inbuf[icurrchar++];
    if (ch != Ctrl('L'))
      return (ch);

    redoscr();
  }
}

char *phone_char(char c) {
   switch (c) {
          case '1':
             return "ㄅ";
          case 'q':
             return "ㄆ";
          case 'a':
             return "ㄇ";
          case 'z':
             return "ㄈ";
          case '2':
             return "ㄉ";
          case 'w':
             return "ㄊ";
          case 's':
             return "ㄋ";
          case 'x':
             return "ㄌ";
          case 'e':
             return "ㄍ";
          case 'd':
             return "ㄎ";
          case 'c':
             return "ㄏ";
          case 'r':
             return "ㄐ";
          case 'f':
             return "ㄑ";
          case 'v':
             return "ㄒ";
          case '5':
             return "ㄓ";
          case 't':
             return "ㄔ";
          case 'g':
             return "ㄕ";
          case 'b':
             return "ㄖ";
          case 'y':
             return "ㄗ";
          case 'h':
             return "ㄘ";
          case 'n':
             return "ㄙ";
          case 'u':
             return "ㄧ";
          case 'j':
             return "ㄨ";
          case 'm':
             return "ㄩ";
          case '8':
             return "ㄚ";
             break;
          case 'i':
             return "ㄛ";
          case 'k':
             return "ㄜ";
          case ',':
             return "ㄝ";
          case '9':
             return "ㄞ";
          case 'o':
             return "ㄟ";
          case 'l':
             return "ㄠ";
          case '.':
             return "ㄡ";
          case '0':
             return "ㄢ";
          case 'p':
             return "ㄣ";
          case ';':
             return "ㄤ";
          case '/':
             return "ㄥ";
          case '-':
             return "ㄦ";
          case '6':
             return "ˊ";
          case '3':
             return "ˇ";
          case '4':
             return "ˋ";
          case '7':
             return "˙";
   }
  return 0;
}

int getdata(line, col, prompt, buf, len, echo)
  int line, col;
  char *prompt, *buf;
  int len, echo;
{
  register int ch;
  int clen;
  int x, y;
  extern unsigned char scr_cols;
#define MAXLASTCMD 6
  static char lastcmd[MAXLASTCMD][80];

  if (prompt)
  {
    move(line, col);
    clrtoeol();
    edit_outs(prompt);
  }
  else
    clrtoeol();

  if (dumb_term || !echo)
  {
    len--;
    clen = 0;
    while ((ch = igetch()) != '\r')
    {
      if (ch == '\n')
        break;
      if (ch == '\177' || ch == Ctrl('H'))
      {
        if (!clen)
        {
          bell();
          continue;
        }
        clen--;
        if (echo)
        {
          ochar(Ctrl('H'));
          ochar(' ');
          ochar(Ctrl('H'));
        }
        continue;
      }

#ifdef BIT8
      if (!isprint2(ch))
#else
      if (!isprint(ch))
#endif

      {
        if (echo)
          bell();
        continue;
      }
      if (clen >= len)
      {
        if (echo)
          bell();
        continue;
      }
      buf[clen++] = ch;
      if (echo)
        ochar(ch);
    }
    buf[clen] = '\0';
    outc('\n');
    oflush();
  }
  else
  {
   int cmdpos = -1;
   int currchar = 0;
   int phone_mode = 0;
   char* pstr;

    getyx(&y, &x);
    standout();
    for (clen = len--; clen; clen--)
      outc(' ');
    standend();
    memset(buf, 0, len);

    while (move(y, x + currchar), (ch = igetkey()) != '\r')
    {
/*
woju
*/
       switch (ch) {
       case KEY_DOWN:
       case Ctrl('N'):
          cmdpos += MAXLASTCMD - 2;
       case Ctrl('P'):
       case KEY_UP: {
          int i;

          cmdpos++;
          cmdpos %= MAXLASTCMD;
          strncpy(buf, lastcmd[cmdpos], len);
          buf[len] = 0;

          move(y, x);                   /* clrtoeof */
          for (i = 0; i <= clen; i++)
             outc(' ');
          move(y, x);

          edit_outs(buf);
          clen = currchar = strlen(buf);
          continue;
       }
       case KEY_LEFT:
          if (currchar)
             --currchar;
          continue;
       case KEY_RIGHT:
          if (buf[currchar])
             ++currchar;
          continue;
       case KEY_ESC:
           if (KEY_ESC_arg == 'p')
              phone_mode ^= 1;
           continue;
       }

      if (ch == '\n' || ch == '\r')
         break;
      if (ch == '\177' || ch == Ctrl('H'))
      {
        if (currchar) {
           int i;

           currchar--;
           clen--;
           for (i = currchar; i <= clen; i++)
              buf[i] = buf[i + 1];
           move(y, x + clen);
           outc(' ');
           move(y, x);
           edit_outs(buf);
        }

        continue;
      }
      if (ch == Ctrl('Y')) {
         int i;

         buf[0] = 0;
         currchar = 0;
         move(y, x);                    /* clrtoeof */
         for (i = 0; i < clen; i++)
            outc(' ');
         clen = 0;
         continue;
      }
      if (ch == Ctrl('S')) {
         buf[0] = 'S';
         clen = 1;
         echo = 0;
         break;
      }
      if (ch == Ctrl('D')) {
        if (buf[currchar]) {
           int i;

           clen--;
           for (i = currchar; i <= clen; i++)
              buf[i] = buf[i + 1];
           move(y, x + clen);
           outc(' ');
           move(y, x);
           edit_outs(buf);
        }
        continue;
      }
      if (ch == Ctrl('K')) {
         int i;

         buf[currchar] = 0;
         move(y, x + currchar);
         for (i = currchar; i < clen; i++)
            outc(' ');
         clen = currchar;
         continue;
      }
      if (ch == Ctrl('A')) {
         currchar = 0;
         continue;
      }
      if (ch == Ctrl('E')) {
         currchar = clen;
         continue;
      }


      if (!(phone_mode && (pstr = phone_char(ch)) || isprint2(ch) || ch == Ctrl('U')))
      {
        continue;
      }
      if (clen + (phone_mode && pstr) >= len || x + clen >= scr_cols)
      {
        continue;
      }

/* woju */

      if (buf[currchar]) {               /* insert */
         int i;

         for (i = currchar; buf[i] && i + (phone_mode && pstr) < len && i + (phone_mode && pstr) < 80; i++)
            ;
         buf[i + 1 + (phone_mode && pstr)] = 0;
         for (; i > currchar; i--)
            buf[i + (phone_mode && pstr)] = buf[i - 1];
      }
      else                              /* append */
         buf[currchar + 1 + (phone_mode && pstr)] = '\0';
      if (ch == Ctrl('U'))
         ch = KEY_ESC;
      if (phone_mode && pstr) {
          buf[currchar] = pstr[0];
          buf[currchar + 1] = pstr[1];
      }
      else
         buf[currchar] = ch;
      move(y, x + currchar);
      edit_outs(buf + currchar);
      currchar++;
      currchar += phone_mode && pstr;
      clen++;
      clen += phone_mode && pstr;
    }
    buf[clen] = '\0';
    if (clen > 1) {
       for (cmdpos = MAXLASTCMD - 1; cmdpos; cmdpos--)
          strcpy(lastcmd[cmdpos], lastcmd[cmdpos - 1]);
       strncpy(lastcmd[0], buf, len);
    }
    if (echo)
      outc('\n');
    refresh();
  }
  if ((echo == LCECHO) && ((ch = buf[0]) >= 'A') && (ch <= 'Z'))
    buf[0] = ch | 32;
  return clen;
}

#define TRAP_ESC
#ifdef  TRAP_ESC

int igetkey()
{
  int mode;
  int ch, last;

  mode = last = 0;
  while (1)
  {
    ch = igetch();
    if (mode == 0)
    {
      if (ch == KEY_ESC)
        mode = 1;
      else
        return ch;              /* Normal Key */
    }
    else if (mode == 1)
    {                           /* Escape sequence */
      if (ch == '[' || ch == 'O')
        mode = 2;
      else if (ch == '1' || ch == '4')
        mode = 3;
      else
      {
        KEY_ESC_arg = ch;
        return KEY_ESC;
      }
    }
    else if (mode == 2)
    {                           /* Cursor key */
      if (ch >= 'A' && ch <= 'D')
        return KEY_UP + (ch - 'A');
      else if (ch >= '1' && ch <= '6')
        mode = 3;
      else
        return ch;
    }
    else if (mode == 3)
    {                           /* Ins Del Home End PgUp PgDn */
      if (ch == '~')
        return KEY_HOME + (last - '1');
      else
        return ch;
    }
    last = ch;
  }
}

#else                           /* TRAP_ESC */

int igetkey(void) {
  int mode;
  int ch, last;

  mode = last = 0;
  while (1)
  {
    ch = igetch();
    if (ch == KEY_ESC)
      mode = 1;
    else if (mode == 0)         /* Normal Key */
      return ch;
    else if (mode == 1)
    {                           /* Escape sequence */
      if (ch == '[' || ch == 'O')
        mode = 2;
      else if (ch == '1' || ch == '4')
        mode = 3;
      else
        return ch;
    }
    else if (mode == 2)
    {                           /* Cursor key */
      if (ch >= 'A' && ch <= 'D')
        return KEY_UP + (ch - 'A');
      else if (ch >= '1' && ch <= '6')
        mode = 3;
      else
        return ch;
    }
    else if (mode == 3)
    {                           /* Ins Del Home End PgUp PgDn */
      if (ch == '~')
        return KEY_HOME + (last - '1');
      else
        return ch;
    }
    last = ch;
  }
}
#endif                          /* TRAP_ESC */

int ask (prompt)
     char *prompt;
{
  int ch;

  move (0, 0);
  clrtoeol ();
  standout ();
  prints ("%s", prompt);
  standend ();
  ch = igetkey ();
  move (0, 0);
  clrtoeol ();
  return (ch);
}
