/****************************************************************************/
/*   FILE SCREEN.H                                                          */
/*   Created  11-JAN-1990               Richard B. Johnson                  */
/*                                      405 Broughton Drive                 */
/*                                      Beverly, Massachusetts 01915        */
/*                                      BBS (508) 922-3166                  */
/*                                                                          */
/*                             JMODEM screen header                         */
/*                                                                          */
/****************************************************************************/

#define VIDEO 0x10                              /* Video BIOS interrupt */
#define BNK  128  << 8                          /* Video attributes     */
#define RD_BG 64  << 8
#define GN_BG 32  << 8
#define BL_BG 16  << 8
#define HI_FG  8  << 8
#define RD_FG  4  << 8
#define GN_FG  2  << 8
#define BL_FG  1  << 8
#define BK_BG  7  << 8
#define WH_FG  RD_FG|BL_FG|GN_FG
#define boxes 4                                 /* Number of windows        */
#define lines 6                                 /* Max lines of text in box */
                        /* Function prototypes  */
void get_curs(union REGS *);                    /* Get cursor position      */
void kill_curs(union REGS *);                   /* Kill the cursor          */
void restore_curs(union REGS *);                /* Restore the cursor       */
void set_curs(short,short,union REGS *);        /* Set cursor position      */
void set_char_atr(unsigned,union REGS *);       /* Set char and attribute   */
void write_str(char*,unsigned,union REGS *);    /* write string/attr        */
unsigned get_char_atr(union REGS *);            /* Get char and attribute   */
short end_box(short,short*,short**,union REGS *); /* Restore window        */
short set_box ( short   ,                       /* Set window function      */
                unsigned short,
                short   ,
                short   ,
                short   ,
                short   ,
                short * ,
                short **,
                union REGS *);

short box_loc[] = {                             /* Coordinates of the boxes */
/* start   start   end    end
   row     col     row    col */
   5    ,  4    ,  13   , 34    ,       /* Box 0        */
   9    ,  12   ,  14   , 72    ,       /* Box 1        */
   12   ,  25   ,  20   , 52    ,       /* Box 2        */
   };

typedef struct {
            unsigned short win;
            unsigned short txt;
                } ATTRIB;

ATTRIB attribute[]= {                   /* Attributes for each box      */
    {
    GN_BG|RD_FG|HI_FG,                  /* GRN/WHT Border (box 0)       */
    GN_BG|WH_FG|HI_FG                   /* GRN/RED Text                 */
    },
    {
    BL_BG|WH_FG|HI_FG,                  /* BLU/WHT Border (box 1)        */
    BL_BG|RD_FG|GN_FG|HI_FG             /* BLU/YEL Text                  */
    },
    {
    BL_BG|GN_BG|RD_FG|HI_FG,            /* RED/CYN Border (box 2)         */
    BL_BG|GN_BG|RD_FG|GN_FG|HI_FG       /* YEL/CYN Text                   */
    }
    };
/****************************************************************************/
/***********************  E N D  O F  M O D U L E  **************************/
