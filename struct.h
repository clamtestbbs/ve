/*-------------------------------------------------------*/
/* struct.h     ( NTHU CS MapleBBS Ver 2.36 )            */
/*-------------------------------------------------------*/
/* target : all definitions about data structure         */
/* create : 1995/03/29                                   */
/* update : 2009/12/18                Dopin:不用的砍光光 */
/*-------------------------------------------------------*/

#ifndef _STRUCT_H_
#define _STRUCT_H_

#define STRLEN   80             /* Length of most string data */
#define BTLEN    48             /* Length of board title */
#define TTLEN    72             /* Length of title */
#define NAMELEN  40             /* Length of username/realname */
#define FNLEN    33             /* Length of filename  */
#define IDLEN    12             /* Length of bid/uid */
#define PASSLEN  14             /* Length of encrypted passwd field */

typedef unsigned char uschar;   /* length = 1 */
typedef unsigned int usint;     /* length = 4 */

/* these are flags in userec.uflag */
#define SIG_FLAG        0x3     /* signature number, 2 bits */
#define PAGER_FLAG      0x4     /* true if pager was OFF last session */
#define CLOAK_FLAG      0x8     /* true if cloak was ON last session */
#define DUMMY_FLAG      0x10    /* not used */
#define BRDSORT_FLAG    0x20    /* true if the boards sorted alphabetical */
#define MOVIE_FLAG      0x40    /* true if show movie */
#define COLOR_FLAG      0x80    /* true if the color mode open */

struct userec {
  char userid[IDLEN + 1];
  char realname[20];
  char username[24];
  char passwd[PASSLEN];
  uschar uflag;
  usint userlevel;
  ushort numlogins;
  ushort numposts;
  time_t firstlogin;
  time_t lastlogin;
  char lasthost[16];
  char termtype[8];
  char email[50];
  char address[50];
  char justify[44];
  };
typedef struct userec userec;

/* ----------------------------------------------------- */
/* screen.c 中運用的資料結構                             */
/* ----------------------------------------------------- */

#define ANSILINELEN (255)       /* Maximum Screen width in chars */

/* Line buffer modes */
#define MODIFIED (1)            /* if line has been modifed, screen output */
#define STANDOUT (2)            /* if this line has a standout region */

struct screenline {
  uschar oldlen;                /* previous line length */
  uschar len;                   /* current length of line */
  uschar mode;                  /* status of line, as far as update */
  uschar smod;                  /* start of modified data */
  uschar emod;                  /* end of modified data */
  uschar sso;                   /* start stand out */
  uschar eso;                   /* end stand out */
  unsigned char data[ANSILINELEN + 1];
};
typedef struct screenline screenline;

/* ----------------------------------------------------- */
/* edit.c 中運用的資料結構                               */
/* ----------------------------------------------------- */

#define WRAPMARGIN (159)

struct textline {
  struct textline *prev;
  struct textline *next;
  int len;
  char data[WRAPMARGIN + 1];
};
typedef struct textline textline;

#endif                          /* _STRUCT_H_ */
