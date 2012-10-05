/****************************************************************************/
/*   FILE JMODEM_F.C                                                        */
/*   Created 11-JAN-1990                    Richard B. Johnson              */
/*                                          405 Broughton Drive             */
/*                                          Beverly, Massachusetts 01925    */
/*                                          BBS (508) 922-3166              */
/*                                                                          */
/*   screen();         All screen output procedures. Uses INT 10H under     */
/*                     MS-DOS.                                              */
/*                                                                          */
/*   These routines are absolutely not necessary and could be replaced      */
/*   with _printf() statements. They are used to make the pretty screens    */
/*   and overlapping windows that the PC community has grown to expect.     */
/*   I didn't spend a lot of time on documentation.  _malloc() is used      */
/*   to obtain memory for saving the screen-content. A failure to obtain    */
/*   sufficient memory will not abort the program, just mess up the screen. */
/*                                                                          */
/****************************************************************************/

#include <stdio.h>
#include <dos.h> 
#include <stdlib.h>
#include <malloc.h>
#include "screen.h"
#include "jmodem.h"
#pragma check_stack(off)
short sav_par[56];                               /* Save row/columns       */
short *buffer[6];                                /* 6 pointers for 6 boxes */
short last_box;                                  /* Last box on the screen */
short next_line;                                 /* Next line to print     */
short start_txt;
short start_row;
short start_col;
short end_row;
short end_col;

char  *signon[] = {
                 "     J M O D E M ",
                 "File transfer protocol",
                 "     "VERS
                 };

char *sta_blk[] = {
                  "   Block : " ,
                  "  Length : " ,
                  "   Bytes : " ,
                  "Rate CPS : " ,
                  "  Status : " ,
                  "Synchronizing ...", 
                  "  Receiving file ",
                  "Transmitting file"
                  };

char *fil_blk[] = {
                  "Opening file ",
                  "Can't open the file!",
                  "Open okay",
                  "File exists, renaming to ",
                  "Can't create the file!",
                  };

/****************************************************************************/
short screen (function,sys, text )
short function;
SYS  *sys;
char *text;
{
    short port=1;
    union REGS bios;                                /* For int 10H         */
    char string[80];                                /* Messages in windows */
    short page;
    unsigned screen;
    short i;

    bios.x.ax =                                        /* Initialize       */
    bios.x.bx =
    bios.x.cx =
    bios.x.dx = 0;

    switch (function)
    {
        case SCR_SGN:
        {
            page=0;
            kill_curs(&bios);
            screen    = attribute[page].win | 0x20;
            start_row = box_loc [(page * 4 ) + 0];
            start_col = box_loc [(page * 4 ) + 1];
            end_row   = box_loc [(page * 4 ) + 2];
            end_col   = box_loc [(page * 4 ) + 3];
            start_txt = start_row + 1;
            set_box (page     ,                        /* Page             */
                     screen   ,                        /* Screen attribute */
                     start_row,                        /* Start row        */
                     start_col,                        /* Start column     */
                     end_row  ,                        /* End row          */
                     end_col  ,                        /* End column       */
                     sav_par  ,
                     buffer   ,
                     &bios  );
            for (i = 0; i<3; i++)
            {
                set_curs (start_txt+i,start_col + 4,&bios);
                sprintf(string,"%s",signon[i]);
                write_str(string,attribute[page].txt,&bios);
            }
            last_box = page;
            break;
        }
    case SCR_FIL:
        {
            page=1;
            screen    = attribute[page].win | 0x20;
            start_row = box_loc [(page * 4 ) + 0];
            start_col = box_loc [(page * 4 ) + 1];
            end_row   = box_loc [(page * 4 ) + 2];
            end_col   = box_loc [(page * 4 ) + 3];
            start_txt = start_row + 1;
            set_box (page     ,                        /* Page             */
                     screen   ,                        /* Screen attribute */
                     start_row,                        /* Start row        */
                     start_col,                        /* Start column     */
                     end_row  ,                        /* End row          */
                     end_col  ,                        /* End column       */
                     sav_par  ,
                     buffer   ,
                     &bios  );
            set_curs (start_txt++,start_col + 4,&bios);
            write_str(fil_blk[0],attribute[page].txt,&bios);
            write_str(text,attribute[page].txt,&bios);
            last_box=page;
            break;
        }
    case SCR_FNF:
        {
            page = last_box;
            set_curs (start_txt++ ,start_col + 4, &bios);
            write_str(fil_blk[1],attribute[page].txt,&bios);
            break;
        }
    case SCR_FOK:
        {
            page = last_box;
            set_curs (start_txt++, start_col + 4, &bios);
            write_str(fil_blk[2],attribute[page].txt,&bios);
            if (sys != NULL)
            {
                sprintf(string,"File Size = %-9lu",sys->s_byt);
                set_curs (start_txt-1, start_col + 38, &bios);
                write_str(string,attribute[page].txt,&bios);
            }
            break;
        }
    case SCR_STA:
        {
            page = 2;
            screen    = attribute[page].win | 0x20;
            start_row = box_loc [(page * 4 ) + 0];
            start_col = box_loc [(page * 4 ) + 1];
            end_row   = box_loc [(page * 4 ) + 2];
            end_col   = box_loc [(page * 4 ) + 3];
            start_txt = start_row + 1;
            set_box (page     ,                        /* Page             */
                     screen   ,                        /* Screen attribute */
                     start_row,                        /* Start row        */
                     start_col,                        /* Start column     */
                     end_row  ,                        /* End row          */
                     end_col  ,                        /* End column       */
                     sav_par  ,
                     buffer   ,
                     &bios  );

            for (i=0; i<6; i++)
            {
                set_curs (start_txt+i,start_col + 4,&bios);
                write_str(sta_blk[i],attribute[page].txt,&bios);
            }
            last_box = page;
            break;
        }
   case SCR_FRN:
        {
            page = last_box;
            set_curs (start_txt++, start_col + 4, &bios);
            write_str(fil_blk[3],attribute[page].txt,&bios);
            write_str(text,attribute[page].txt,&bios);
            break;
        }
   case SCR_FCR:
        {
            page = last_box;
            set_curs (start_txt++, start_col + 4, &bios);
            write_str(fil_blk[4],attribute[page].txt,&bios);
            break;
        }
   case SCR_SYR:
        {
            page = last_box;
            start_txt = start_row + 1;
            set_curs (start_txt + 5, start_col + 4, &bios);
            write_str(sta_blk[6],attribute[page].txt | 0x8000 ,&bios);
            break;
        }
   case SCR_SYT:
        {
            page = last_box;
            start_txt = start_row + 1;
            set_curs (start_txt + 5, start_col + 4, &bios);
            write_str(sta_blk[7],attribute[page].txt | 0x8000,& bios);
            break;
        }
    case SCR_SYS:
        {
        page = last_box;
        sprintf(string,"%-6d",sys->s_blk);
        set_curs (start_txt, start_col + 15, &bios);
        write_str(string,attribute[page].txt,& bios);

        sprintf(string,"%-4d",sys->s_len);
        set_curs (start_txt + 1, start_col + 15, &bios);
        write_str(string,attribute[page].txt,& bios);

        sprintf(string,"%-9lu",sys->s_byt);
        set_curs (start_txt + 2, start_col + 15, &bios);
        write_str(string,attribute[page].txt,& bios);

        sprintf(string,"%-4d",sys->s_cps);
        set_curs (start_txt + 3, start_col + 15, &bios);
        write_str(string,attribute[page].txt,& bios);

        sprintf(string,"%s",sys->s_sta);
        set_curs (start_txt + 4, start_col + 15, &bios);
        write_str(string,attribute[page].txt,& bios);
        break;
        }
    case SCR_END:
        {
        for (page = last_box; page >=0; page--)
            {
            end_box (page,sav_par,buffer,&bios);
            }
        restore_curs(&bios);
        break;
        }
    }
    return 0;
}
/****************************************************************************
   Save screen contents in a buffer obtained from _malloc. Write a border and
   screen attributes to saved screen location. Record the address of the
   buffer so the screen contents may be restored. Global *buffer[] is used
   to save the pointers.
*/
short set_box (page  ,                           /* Box number             */
             screen,                             /* Screen attribute       */
             start_row,                          /* Start row of border    */
             start_col,                          /* Start column of border */
             end_row,                            /* End row of border      */
             end_col,                            /* End column of border   */
             sav_par,
             buffer,
             bios)
short page;
unsigned short screen;
short start_row;
short start_col;
short end_row;
short end_col;
short sav_par[];
short *buffer[];
union REGS *bios;
{
    unsigned short putscr;
    short sav_col;
    short sav_row;
    short row;
    short col;
    get_curs(bios);                                 /* Get cursor position  */
    sav_row = (short) bios->h.dh;                   /* Save cursor row      */
    sav_col = (short) bios->h.dl;                   /* Save cursor column   */
    sav_par[(page * 7) + 0] = (short) screen;
    sav_par[(page * 7) + 1] = start_row;
    sav_par[(page * 7) + 2] = start_col;
    sav_par[(page * 7) + 3] = end_row;
    sav_par[(page * 7) + 4] = end_col;
    sav_par[(page * 7) + 5] = sav_row;
    sav_par[(page * 7) + 6] = sav_col;

    buffer[page] = (short*) malloc (     /* Get pointer to memory and save */
                   ((end_row - start_row)
                  * (end_col - start_col))
                  *  sizeof (short) );

    if (buffer[page] == NULL)
        {
        puts("\nMemory allocation failed!");
        return(1);
        }
   for (row= start_row; row < end_row; row++)
        {
        for (col=start_col; col < end_col; col++)
            {
            set_curs (row,col,bios);                    /* Set cursor pos */
            *buffer[page] = get_char_atr(bios);         /* Save char/attr */
            buffer[page]++;                             /* Bump pointer   */
            putscr = screen;                            /* default (else) */
            if (row == start_row || row == end_row-1)   /* Top and bottom */
            putscr = (screen & 0x0FF00) | 205;
            if (col == start_col || col == end_col-1)   /* Right and left */
            putscr = (screen & 0x0FF00) | 186;
            if (row == start_row && col == start_col)   /* NW corner      */
            putscr = (screen & 0x0FF00) | 201;
            if (row == start_row && col == end_col-1)   /* NE corner      */
            putscr = (screen & 0x0FF00) | 187;
            if (row == end_row-1 && col == start_col)   /* SW corner      */
            putscr = (screen & 0x0FF00) | 200;
            if (row == end_row -1 && col == end_col-1)  /* SE corner      */
            putscr = (screen & 0x0FF00) | 188;
            set_char_atr (putscr,bios);                 /* Write to screen */
            }
        }
    return(0);
}
/****************************************************************************
/* Restore the screen contents saved in memory pointed to by global *buffer[].
*/
short end_box (page,sav_par,buffer,bios)
short page;
short sav_par[];
short *buffer[];
union REGS *bios;
{
    unsigned screen;
    short start_row;
    short start_col;
    short end_row;
    short end_col;
    short row;
    short col;
    short sav_row;
    short sav_col;

    screen    = (unsigned) sav_par[(page * 7) + 0];  /* Restore from array */
    start_row = sav_par[(page * 7) + 1];
    start_col = sav_par[(page * 7) + 2];
    end_row   = sav_par[(page * 7) + 3];
    end_col   = sav_par[(page * 7) + 4];
    sav_row   = sav_par[(page * 7) + 5];
    sav_col   = sav_par[(page * 7) + 6];

    buffer[page] -= ( (end_row - start_row)             /* Get buffer start */
               * (end_col - start_col) );

    for (row= start_row; row < end_row; row++)
        {
        for (col=start_col; col < end_col; col++)
            {
            set_curs (row,col,bios);                    /* Set cursor pos  */
            set_char_atr(*buffer[page],bios);           /* Restore screen  */
            buffer[page]++;                             /* Bump pointer    */
            }
        }
    free(buffer[page]);                                 /* Free alloc mem  */
    set_curs (sav_row,sav_col,bios);                    /* Restore cursor  */
    return(0);
}
/****************************************************************************/
                        /* Set Cursor to row, column */
void set_curs (row,col,bios)
short row;
short col;
union REGS *bios;
{
    bios->h.ah=(unsigned char)2;         /* Set cursor function  */
    bios->h.dh=(unsigned char)row;       /* Row                  */
    bios->h.dl=(unsigned char)col;       /* Column               */
    bios->h.bh=(unsigned char)0;         /* Page 0               */
    int86(VIDEO, bios, bios);            /* Use for in and out   */
    return;
}
/****************************************************************************/
void get_curs(bios)
union REGS *bios;
{
    bios->h.ah=(unsigned char)3;         /* Get cursor function  */
    bios->h.bh=(unsigned char)0;         /* Page 0               */
    int86(VIDEO, bios, bios);            /* Use for in and out   */
    return;
}
/****************************************************************************/
void kill_curs(bios)
union REGS *bios;
{
    bios->h.ah=(unsigned char)1;         /* Set cursor function  */
    bios->h.bh=(unsigned char)0;         /* Page 0               */
    bios->x.cx=(unsigned short) 0xFFFF;  /* Kill the cursor      */
    int86(VIDEO, bios, bios);            /* Use for in and out   */
    return;
}
/****************************************************************************/
void restore_curs(bios)
union REGS *bios;
{
    bios->h.ah=(unsigned char)1;         /* Get cursor function  */
    bios->h.bh=(unsigned char)0;         /* Page 0               */
    bios->x.cx=(unsigned short) 0x0607;  /* Restore the cursor   */
    int86(VIDEO, bios, bios);            /* Use for in and out   */
    return;
}
/****************************************************************************/
                /* Write char /attr at cursor position */
void set_char_atr(atr_chr,bios)
unsigned atr_chr;
union REGS *bios;
{
    bios->h.ah=(unsigned char)9;                 /* Write char function  */
    bios->h.al=(unsigned char) (atr_chr & 0xFF); /* Character            */
    bios->x.cx=1;                                /* One character        */
    bios->h.bl=(unsigned char) (atr_chr >> 8);   /* Attribute            */
    bios->h.bh=(unsigned char)0;                 /* Page 0               */
    int86(VIDEO, bios, bios);                    /* Use for in and out   */
    return;
}
/****************************************************************************/
                /* Get char /attr at cursor position */
unsigned get_char_atr(bios)
union REGS *bios;
{
    bios->h.ah=(unsigned char)8;                 /* Read char function   */
    bios->h.bh=(unsigned char)0;                 /* Page 0               */
    int86(VIDEO, bios, bios);                    /* Use for in and out   */
    return (bios->x.ax);                         /* Its in the AX regis  */
 }
/****************************************************************************/
void write_str(string,atr,bios)
char string[];
unsigned atr;
union REGS *bios;

{
    short i= 0;
    short row;
    short col;
    while (string[i])                            /* Until the NULL      */
    {
    set_char_atr(atr|(short) string[i++],bios);  /* Write char and attr */
    get_curs(bios);                              /* Get cursor position */
    row = (short) bios->h.dh;                    /* Get row             */
    col = (short) bios->h.dl;                    /* Get column          */
    col++;                                       /* Next column         */
    set_curs (row,col,bios);                     /* Set column          */
    }
    return;
}
/****************************************************************************/
/*                       E N D  O F  M O D U L E                            */
/****************************************************************************/