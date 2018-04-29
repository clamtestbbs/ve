/*-------------------------------------------------------*/
/* global.h     ( NTHU CS MapleBBS Ver 2.36 )            */
/*-------------------------------------------------------*/
/* target : global definitions & variables               */
/* create : 1995/03/29                                   */
/* update : 2009/12/18             Dopin: 不需要的砍光光 */
/*-------------------------------------------------------*/

#ifndef _GLOBAL_H_
#define _GLOBAL_H_

/* ----------------------------------------------------- */
/* GLOBAL DEFINITION                                     */
/* ----------------------------------------------------- */

/* 鍵盤設定 */

#ifndef EXTEND_KEY
#define EXTEND_KEY
#define KEY_TAB         9
#define KEY_ESC         27
#define KEY_UP          0x0101
#define KEY_DOWN        0x0102
#define KEY_RIGHT       0x0103
#define KEY_LEFT        0x0104
#define KEY_HOME        0x0201
#define KEY_INS         0x0202
#define KEY_DEL         0x0203
#define KEY_END         0x0204
#define KEY_PGUP        0x0205
#define KEY_PGDN        0x0206
#endif

#define Ctrl(c)         ( c & 037 )

#ifdef SYSV
#undef CTRL                     /* SVR4 CTRL macro is hokey */
#define CTRL(c) ('c'&037)       /* This gives ESIX a warning...ignore it! */
#endif

#define chartoupper(c)  ((c >= 'a' && c <= 'z') ? c+'A'-'a' : c)
#define char_lower(c)  ((c >= 'A' && c <= 'Z') ? c|32 : c)

/* ----------------------------------------------------- */
/* 訊息字串：獨立出來，以利支援各種語言                  */
/* ----------------------------------------------------- */

#define STR_CURSOR      "●"

#define STR_UNCUR       "  "

/* ----------------------------------------------------- */
/* GLOBAL VARIABLE                                       */
/* ----------------------------------------------------- */

extern char genbuf[1024];

extern int showansi;

/* global string variable */

extern jmp_buf byebye;          // for exception condition like I/O error

extern int dumb_term;
extern int t_lines, t_columns;  // Screen size / width
extern int b_lines;             // Screen bottom line number: t_lines-1
extern int p_lines;             // a Page of Screen line numbers: tlines-4

#endif                          /* _GLOBAL_H_ */
