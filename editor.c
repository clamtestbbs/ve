/*
    Pirate Bulletin Board System
    Copyright (C) 1990, Edward Luke, lush@Athena.EE.MsState.EDU
    Eagles Bulletin Board System
    Copyright (C) 1992, Raymond Rocker, rocker@rock.b11.ingr.com
                        Guy Vega, gtvega@seabass.st.usm.edu
                        Dominic Tynes, dbtynes@seabass.st.usm.edu

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 1, or (at your option)
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "bbs.h"

#define gtty(fd,data)  tcgetattr(fd,data)
#define stty(fd,data)  tcsetattr(fd,TCSETS,data)

struct userec currentuser ;
jmp_buf byebye ;
char	genbuf[ 1024 ];
int	showansi = 1;
int	local_article = 0;

void bell(void)
{
    fprintf(stderr,"%c",0x7) ;
}

void abort_bbs(void)
{
    fprintf(stderr,"fatal error has occured\n") ;
    reset_tty() ;
    exit(-1) ;
}

int main(argc,argv)
int argc ;
char *argv[] ;
{
    char *term, *getenv() ;
    extern int dumb_term ;    /* this comes in from term_init */

    if(argc < 2) {
        fprintf(stderr,"Usage: %s <filename>\n", argv[0]) ;
        exit(-1) ;
    }
    get_tty() ;               /* get tty port mode settings */
    init_tty() ;              /* set up mode for NOECHO and RAW */
    if(setjmp(byebye)) {
        fprintf(stderr,"Goodbye\n") ;
        reset_tty() ;
        exit(-1) ;
    }
#if 0
    term = getenv("TERM") ;   /* Get term type from unix environment */
#endif
    term = "vt100";
    if(term == NULL) {
        fprintf(stderr,"No Terminal environment type!\n") ;
        reset_tty() ;
        exit(-1) ;
    }
    term_init(term) ; /* Load up strings used to control terminal type 'term'*/
    if(dumb_term) {
        fprintf(stderr,"Terminal type not smart enough to support editor\n") ;
        reset_tty() ;
        exit(-1) ;
    }
    initscr();			/* Initialize screen interface */
    vedit(argv[argc-1],NA);	/* Start editor on file, do not save header */
    clear();			/* clear screen */
    redoscr();			/* make clear() happen */
    reset_tty();    
    exit(0);
}

