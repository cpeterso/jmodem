/****************************************************************************/
/*   FILE JMODEM.H                                                          */
/*   Created 11-JAN-1990                Richard B. Johnson                  */
/*                                      405 Broughton Drive                 */
/*                                      Beverly, Massachusetts 01915        */
/*                                      BBS (508) 922-3166                  */
/*                                                                          */
/*                  Parameters that are specific to JMODEM                  */
/****************************************************************************/
#define VERS            "VERSION 3.03"  /* Version number               */
#define _8K             0x2000          /* 4096 bytes                   */
#define BLK_SIZ         0x200           /* Starting block size          */
#define OVRHD           0x06            /* Private, JMODEM overhead     */
#define DAT_LEN         _8K + 1024      /* Data buffer length           */
#define DAT_MAX         _8K             /* Max block length             */
#define OPEN_READ       0x01            /* Private OPEN file function   */
#define CREATE          0x02            /* Private CREATE file function */
#define WRITE           0x03            /* Private WRITE file function  */
#define CLOSE           0x04            /* Private CLOSE file function  */
#define DELETE          0x05            /* Private DELETE file function */
#define READ            0x06            /* Private READ file function   */
#define GET_CRC         0x00            /* Private Get CRC function     */
#define SET_CRC         0x01            /* Private Set CRC function     */
#define NORM            0x01            /* Private, show normal data    */
#define COMP            0x02            /* Private, show compressed     */
#define EOF_            0x04            /* Private, show end of file    */
#define TIMOUT          0x5A            /* Timeout (ticks) for read     */
#define EOT             0x04            /* "D" - 64                     */
#define ACK             0x06            /* "F" - 64                     */
#define NAK             0x15            /* "U" - 64                     */
#define SYN             0x16            /* "V" - 64                     */
#define CAN             0x18            /* "X" - 64                     */
#define SCR_SGN         0x01            /* Signon screen                */
#define SCR_BOX         0x02            /* Write box on the screen      */
#define SCR_TXT         0x03            /* Write text to x/y address    */
#define SCR_STA         0x04            /* Write status box             */
#define SCR_FIL         0x05            /* Write open file box          */
#define SCR_FOK         0x06            /* File open okay               */
#define SCR_FNF         0x07            /* File not found               */
#define SCR_FCR         0x08            /* Can't create the file        */
#define SCR_FRN         0x09            /* Renamed the file             */
#define SCR_SYS         0x0A            /* Show system parameters       */
#define SCR_SYT         0x0B            /* Show trans synchronization   */
#define SCR_SYR         0x0C            /* Show Receive synchronization */
#define SCR_END         0x0D            /* Exit all screens             */
#define JM_NRM          0x00            /* Normal exit                  */
#define JM_FNF          0x01            /* File not found               */
#define JM_REN          0x02            /* Can't rename the file        */
#define JM_CRE          0x03            /* Can't create the file        */
#define JM_MEM          0x04            /* No memory available          */
#define JM_CAR          0x05            /* Modem carrier failed         */
#define JM_SYN          0x06            /* Can't synchronize            */
#define JM_ABT          0x07            /* Aborted                      */
#define JM_CMD          0x08            /* Command-line error           */
#define JM_TIM          0x09            /* Time-out                     */
#define JM_FAT          0x0A            /* Fatal error                  */
#define JM_MAX          0xFFFF          /* Maximum buffer space exceeded*/

/****************************************************************************/
/*                     Structures and templates                             */
/****************************************************************************/
typedef struct
        {                               /* Structure for JMODEM status  */
        unsigned short  s_blk;          /* Block being sent / received  */
        unsigned short  s_len;          /* Length of the block          */
        unsigned long   s_byt;          /* Bytes so far                 */
        unsigned short  s_cps;          /* Speed, characters / second   */
        unsigned char  *s_sta;          /* Pointer to current status    */
        } SYS;

typedef struct                          /* JMODEM block header structure */
        {
        unsigned short len;             /* Block length                  */
        unsigned char blk_typ;          /* Kind of block EOF, COMP, etc  */
        unsigned char blk_num;          /* Block number (starts at 1 )   */
        unsigned char blk_dat;          /* First data character          */
        } JBUF;

/****************************************************************************/
/*                   External function prototypes                           */
/****************************************************************************/
extern short screen(
             short,                     /* Function (in SCREEN.H)       */
             SYS *,                     /* Pointer to status block      */
             char * );                  /* Text                         */
extern unsigned short
       open_chan(unsigned short);       /* Open communications channel  */
extern unsigned short
       close_chan(unsigned short);      /* Close communications channel */
extern unsigned short
       read_chan(unsigned short,
       unsigned char *);                /* Write communications channel */
extern unsigned short
       write_chan(unsigned short,
       unsigned char *);                /* Read communications channel  */
extern unsigned short
       send_blk(unsigned short,
                SYS *,
       unsigned char *);                /* Send JMODEM block            */
extern unsigned short
       recv_blk(unsigned short *,
       unsigned char *);                /* Receive JMODEM block         */
extern unsigned short
       encode(unsigned short,
       unsigned char *,
       unsigned char *);                /* Data compression routine     */
extern unsigned short
       decode(unsigned short,
       unsigned char *,
       unsigned char *);                /* Data expansion routine       */
extern unsigned short
       calc_crc(unsigned short,
       unsigned short,
       unsigned char *);                /* Calculate CRC                */
extern unsigned short
       file_io(unsigned short,
       short *,
       unsigned char **,
       unsigned short);                 /* File I/O                     */
extern char
       *get_inp(unsigned short,         /* Get input file-name          */
       char **);
extern char *get_fun(unsigned short,    /* Get function (S or R)        */
       char **);
extern char *get_prt(unsigned short,
       char **);                        /* Get port (1 - 4)             */
extern void
       disp(void);                      /* Display 'usage' message      */
extern unsigned short
       get_port(char);                  /* Convert port number          */
extern unsigned char
      *allocate_memory(unsigned short); /* Allocate memory              */
extern void flush (void);               /* Flush the interrupt buffer   */
extern unsigned short tx_sync(void);    /* Synchronize during transmit  */
extern unsigned short rx_sync(void);    /* Synchronize during receive   */
extern unsigned short port;             /* Port number                  */
extern unsigned short timer;            /* Global timer                 */
extern SYS syst;                        /* In JMODEM_A.C                */
#ifdef NOENV                            /* If a compiler command        */
void _setenvp(void);                    /* Dummy routine prototype      */
void _setenvp()                         /* Dummy routine                */
{}
#endif
/****************************************************************************/
/*********************** E N D  OF  M O D U L E *****************************/