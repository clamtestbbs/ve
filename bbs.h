/*-------------------------------------------------------*/
/* bbs.h        ( NTHU CS MapleBBS Ver 2.36 )            */
/*-------------------------------------------------------*/
/* target : all header files                             */
/* create : 1995/03/29                                   */
/* update : 2009/12/18                                   */
/*-------------------------------------------------------*/

#ifndef _BBS_H_
#define _BBS_H_

#define BIT8

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <setjmp.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <termios.h>

#ifdef  SYSV

#ifndef LOCK_EX
#define LOCK_EX         F_LOCK
#define LOCK_UN         F_ULOCK
#endif

#define getdtablesize()         (64)

#define usleep(usec)            {               \
    struct timeval t;                           \
    t.tv_sec = usec / 1000000;                  \
    t.tv_usec = usec % 1000000;                 \
    select( 0, NULL, NULL, NULL, &t);           \
}

#else                           /* SYSV */
   #ifndef MIN
      #define   MIN(a,b)        ((a<b)?a:b)
   #endif
   #ifndef MAX
      #define   MAX(a,b)        ((a>b)?a:b)
   #endif
#endif                          /* SYSV */


#define YEA (1)                 /* Booleans  (Yep, for true and false) */
#define NA  (0)

#define NOECHO (0)
#define DOECHO (1)              /* Flags to getdata input function */
#define LCECHO (2)

#define I_TIMEOUT   (-2)        /* Used for the getchar routine select call */
#define I_OTHERDATA (-333)      /* interface, (-3) will conflict with chinese */


#include "global.h"             /* global variable & definition */
#include "struct.h"             /* data structure */
#endif                          /* _BBS_H_ */
