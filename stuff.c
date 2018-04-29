/*-------------------------------------------------------*/
/* stuff.c      ( NTHU CS MapleBBS Ver 2.36 )            */
/*-------------------------------------------------------*/
/* target : utility routines                             */
/* create : 1995/03/29                                   */
/* update : 2009/12/18 from sob_pack Dopin: ���Ϊ������ */
/*-------------------------------------------------------*/

#include <sys/param.h>
#include <string.h>

#include "bbs.h"

/* ----------------------------------------------------- */
/* �r���ഫ�ˬd���                                      */
/* ----------------------------------------------------- */

/* Case Independent strncmp */

void str_lower(t, s)
  char *t, *s;
{
  register uschar ch;

  do
  {
    ch = *s++;
    *t++ = char_lower(ch);
  } while (ch);
}

void trim(buf)                       /* remove trailing space */
  char *buf;
{
  char *p = buf;

  while (*p)
    p++;
  while (--p >= buf)
  {
    if (*p == ' ')
      *p = '\0';
    else
      break;
  }
}

/* ----------------------------------------------------- */
/* �r���ˬd��ơG�^��B�Ʀr�B�ɦW�BE-mail address        */
/* ----------------------------------------------------- */

#ifdef BIT8
int isprint2(ch)
  char ch;
{
  return ((ch & 0x80) ? 1 : isprint(ch));
}
#endif

int not_alpha(ch)
  register char ch;
{
  return (ch < 'A' || (ch > 'Z' && ch < 'a') || ch > 'z');
}

/* ----------------------------------------------------- */
/* �ɮ��ˬd��ơG�ɮסB�ؿ��B�ݩ�                        */
/* ----------------------------------------------------- */

int dashf(fname)
  char *fname;
{
  struct stat st;

  return (stat(fname, &st) == 0 && S_ISREG(st.st_mode));
}

int dashd(fname)
  char *fname;
{
  struct stat st;

  return (stat(fname, &st) == 0 && S_ISDIR(st.st_mode));
}

void pressanykey()
{
  int ch;

  outmsg("[37;45;1m                        �� �Ы� [33m(Space/Return)"
         "[37m �~�� ��                       [0m");

  do {
    ch = igetkey();
  } while ((ch != ' ') && (ch != KEY_LEFT) && (ch != '\r') && (ch != '\n'));

  move(b_lines, 0);
  clrtoeol();

  refresh();
}

void stand_title(title)
  char *title;
{
  clear();
  prints("[1;37;46m�i %s �j[0m\n", title);
}

/* opus : cursor position */

more(char* fpath, int i) {
  FILE* fp;
  char buf[100];

  clear();

  if(fp = fopen(fpath, "r")) {
    for(i = 0; i < 20 && fgets(buf, 100, fp); i++) outs(buf);

    fclose(fp);
  }
}

setuserfile(char* fpath, char* fname) {
  char *getenv();
  char *dir = getenv("HOME");

  dir = dir ? dir : "/tmp";

  sprintf(fpath,  "%s/.ve_%s", dir, fname);
}
