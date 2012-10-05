/****************************************************************************/
/*    FILE JMODEM_A.C                                                       */
/*                                                                          */
/*    The JMODEM protocol            MicroSoft (r)  'C' V5.1                */
/*    Created 03-FEB-1990            Richard B. Johnson                     */
/*                                   405 Broughton Drive                    */
/*                                   Beverly, Massachusetts 01915           */
/*                                   BBS (508) 922-3166                     */
/*                                                                          */
/*    An external protocol for high-speed data transmission.                */
/*                                                                          */
/*    This is the MAIN module                                               */
/*    The required modules are:                                             */
/*    JMODEM.H    (function prototypes and structures)                      */
/*    UART.H      (8250 UART parameters)                                    */
/*    SCREEN.H    (function protypes and structures for the screen)         */
/*    JMODEM_A.C  (this module)                                             */
/*    JMODEM_B.C  (memory allocation and input parsing)                     */
/*    JMODEM_C.C  (all file I/O)                                            */
/*    JMODEM_D.C  (encode/decode and CRC routines)                          */
/*    JMODEM_E.C  (communications I/O routines)                             */
/*    JMODEM_F.C  (the screen I/O routines)                                 */
/*    JMODEM.     (The MAKE file )                                          */
/*                                                                          */
/*    This program requires about 67k of free RAM to execute properly.      */
/*    If you have 66k or less, it will execute, but the screens will        */
/*    not be written or replaced properly. If you have only 64k, the        */
/*    program will exit with an error message.                              */
/*                                                                          */
/*    Revision History:                                                     */
/*    V3.00                Beta test  11-FEB-1990                           */
/*    V3.01                First release 18-FEB-1990                        */
/*    V3.01   Revised 19-FEB-1990               Richard B. Johnson          */
/*                                                                          */
/*      (1)   A bug in MicroSoft _calloc()  allocates overlapping           */
/*            buffers so data files were getting corrupted. I had           */
/*            used both _calloc() and _malloc() at the same time and        */
/*            they didn't like it. I changed the memory allocation          */
/*            to _malloc() only and it seems to work okay.                  */
/*                                                                          */
/*      (2)   While debugging, I found some structures I didn't need and    */
/*            removed them. Changed some code to accommodate.               */
/*                                                                          */
/*      (3)   Added a file-size during downloads.                           */
/*                                                                          */
/*      (4)   Changed code in the data encoding (compression) routine       */
/*            in an attempt to speed it up.                                 */
/*                                                                          */
/****************************************************************************/
#include <stdlib.h>                     /* Used for _free()                */
#include <stdio.h>                      /* Used for NULL value             */
#include <string.h>                     /* Used for _memcpy()              */
#include <time.h>                       /* Used for absolute time          */
#include "jmodem.h"                     /* JMODEM primatives               */
#pragma intrinsic (memcpy)              /* In-line code                    */
/****************************************************************************/
/*                   Global pointers and allocation                         */
/****************************************************************************/
short  handle;                          /* For file I/O                     */
SYS syst;                               /* Structure for JMODEM status      */
unsigned short user_abort = 0;          /* User abort flag                  */
unsigned char *in_buffer;               /* Pointer to input buffer          */
unsigned char *out_buffer;              /* Pointer to output buffer         */
unsigned char *comp_buffer;             /* Pointer to compression buffer    */
unsigned char *file_buffer;             /* Pointer to file buffer           */
unsigned char *int_buffer;              /* Pointer to interrupt buffer      */
unsigned long start;                    /* Start time                       */
unsigned long finish;                   /* End time                         */
JBUF *buff;                             /* A pointer for the JMODEM block   */
char abrt[]  = "Aborted!";              /* Four messages                    */
char okay[]  = "Okay    ";
char retry[] = "Retry   ";
char done[]  = "Done!   ";
/****************************************************************************/
/*                               C O D E                                    */
/****************************************************************************/
short main (short argc, char *argv[])
{
    unsigned short status=0;            /* TX and RX status                 */
    unsigned short tries;               /* Attempts to send a file          */
    unsigned short cmp_size;            /* Size after compression           */
    unsigned short data_written;        /* Data written to the file         */
    unsigned short data_read;           /* Data read from the file          */
    char *file_name;                    /* filename                         */
    char *function;                     /* Receive, Transmit                */
    char *com_port;                     /* Communications adapter port      */

    file_name = get_inp (argc, argv);   /* Get file name                    */
    if (file_name == NULL )
    {
        disp();                         /* Display usage message            */
        return JM_FNF;
    }
    function  = get_fun (argc, argv);   /* Get function 'R' or 'S'          */
    if (function == NULL)
    {
        disp();                         /* Display usage message            */
        return JM_CMD;
    }
    com_port  = get_prt (argc, argv);   /* Get port '1 to 4 '               */
    if (com_port == NULL)
    {
        disp();                         /* Display usage message            */
        return JM_CMD;
    }
    port = get_port(*com_port);         /* Convert port to an offset        */
/****************************************************************************/
/*                          Allocate buffers                                */
/****************************************************************************/
    in_buffer = allocate_memory(DAT_LEN);  /* Get some memory for input     */
    if (in_buffer == NULL)
        return JM_MEM;                     /* No memory available           */
    out_buffer = allocate_memory(DAT_LEN); /* Get some memory for output    */
    if (out_buffer == NULL)
        return JM_MEM;                     /* No memory available           */
    comp_buffer=allocate_memory(DAT_LEN);  /* Get memory for compression    */
    if (comp_buffer == NULL)
        return JM_MEM;                     /* No memory available           */
    file_buffer=allocate_memory(DAT_LEN);  /* Get memory for file buffer    */
    if (file_buffer == NULL)
        return JM_MEM;                     /* No memory available           */
    int_buffer =allocate_memory(DAT_LEN);  /* Memory for interrupt buffer   */
    if (int_buffer == NULL)
        return JM_MEM;                     /* No memory available           */
/****************************************************************************/

    screen (SCR_SGN,NULL,NULL);            /* Write signon screen           */
    syst.s_len = BLK_SIZ;
    syst.s_byt = 0;
    syst.s_blk = 0;
    syst.s_sta = okay;
    switch(*function)                     /* Functions are TX and RX       */
    {
/****************************************************************************/
/*                          Receive JMODEM file                             */
/****************************************************************************/
    case 'R':
        {
            if (!file_io(CREATE, &handle, &file_name, NULL) )
            {
                buff = (JBUF *) in_buffer;            /* Assign type JBUF   */
                open_chan(port);                      /* Open com channel   */
                screen (SCR_STA,NULL,NULL);           /* Write status block */
                status = rx_sync();                   /* Synchronize        */
                if (!status)
                    screen (SCR_SYR,NULL,NULL);
                data_written = 0xFFFF;
                tries = 10;			    /* Attempts to receive */
                while (    (data_written)             /* Write file okay   */
                        && (!user_abort )             /* No break key      */
                        && (!status     )             /* Recev block okay  */
                        && (tries--)    )             /* 10 retries        */
                {
                    time(&start);                     /* Get starting time */
		    screen (SCR_SYS,&syst,NULL);      /* Show status block */
                    status = recv_blk (               /* Receive data-block*/
                             &syst.s_len,             /* Block length      */
                             in_buffer);              /* Input buffer      */
                    if (status)                       /* If bad            */
                        break;                        /* Abort the WHILE   */
                    if( (!(calc_crc(GET_CRC,          /* Calculate CRC     */
                          syst.s_len,                 /* Amount to check   */
                          in_buffer) ))               /* Receiver buffer   */
		      && ( buff->blk_num ==           /* Check block also  */
                         (unsigned char)
                         (syst.s_blk +1)))            /* Block number      */
                    {
                        syst.s_sta = okay;            /* Text pointer      */
                        tries=10;                     /* Reset count       */
                        syst.s_len -= OVRHD;          /* Subtract overhead */
                        *out_buffer = ACK;            /* Good              */
                        write_chan(1,out_buffer);     /* Send the ACK      */

                        /* If data was compressed                          */
			if ( (buff->blk_typ & COMP) == COMP)
                        {
                             syst.s_len = decode (    /* Decode the data   */
                                      syst.s_len,     /* Data-block length */
				     &buff->blk_dat,  /* Where to start    */
                                     file_buffer);    /* Where to put data */
                        }
                        else
                        /* Data was normal (not compressed, just copy )    */
                        {
                            memcpy (file_buffer,&buff->blk_dat,syst.s_len);
                        }
                        /* Write to the file                                */
                        data_written = file_io( WRITE ,  /* Function        */
                                         &handle,        /* File handle     */
                                         &file_buffer ,  /* Where data is   */
                                         syst.s_len );   /* Amount to write */
                        syst.s_byt += data_written;      /* Total bytes     */
                        syst.s_blk++;                    /* Block number    */
                        time(&finish);                   /* Get end time    */
                        if (finish - start)              /* Check div/0     */
                            syst.s_cps = (short)         /* Calc Block CPS  */
                            (data_written / (finish - start) );

                            /* Check for end-of-file                        */
                        if ( (buff->blk_typ & EOF_) == EOF_)
                        {                       /* This was the end of file */
                            file_io(CLOSE,               /* Function        */
                                   &handle,              /* Open handle     */
                                   &file_name,           /* Name not used   */
                                   NULL);                /* Buffer not used */
                            close_chan(port);            /* Close the port  */
                            status = JM_NRM;             /* Set status      */
                            goto cleanup;                /* exit routine    */
                        }
                    }
                    else
                    {
                        *out_buffer = NAK;              /* Bad block        */
                        syst.s_sta = retry;             /* Char pointer     */
                        write_chan(1,out_buffer);       /* Send the NAK     */
                     }
                }
                close_chan(port);                        /* Aborted         */
                file_io(DELETE,                          /* Function        */
                        &handle,                         /* File handle     */
                        &file_name,                      /* Name            */
                        NULL);                           /* Buffer not used */
                status = JM_ABT;
		break;                                   /* Exit if() {}    */
            }
            else                                       /* Can't create file */
            {
                status = JM_CRE;
                break;                                   /* Exit while() {} */
            }
        break;                                           /* Exit case 'R'   */
        }
/****************************************************************************/
/*                          Send JMODEM file                                */
/****************************************************************************/
    case 'S':   /* Send JMODEM file */
        {
            if (!file_io(OPEN_READ, &handle, &file_name, NULL) )
            {
                buff = (JBUF *)out_buffer;            /* Assign type JBUF   */
                syst.s_byt = 0;                       /* Restore byte count */
                open_chan(port);                      /* Open COM port      */
                data_read = 0xFFFF;                   /* Initialize         */
                screen (SCR_STA,NULL,NULL);           /* Write status block */
                status = tx_sync();                   /* Synchronize        */
                if (!status)
                    screen (SCR_SYT,NULL,NULL);
                while  (  (!user_abort)               /* Ctrl - break       */
                       && (!status) )                 /* sent okay          */
                {
                    time(&start);                     /* Get starting time  */
                    data_read = file_io( READ       , /* Read a record      */
                                       &handle      , /* File pointer       */
                                       &file_buffer , /* Where to put       */
                                      syst.s_len );   /* Amount to read     */
                    if (!data_read)                   /* Past end of file   */
                        break;
                    syst.s_byt += (long) data_read;   /* Running count      */
		    screen (SCR_SYS,&syst,NULL);      /* Show status block  */
                    buff->blk_num = (unsigned char)
                                     ++syst.s_blk;    /* Block number       */
                    if (data_read != syst.s_len)      /* Byte request       */
                        buff->blk_typ = EOF_;         /* Into control-byte  */
                    else
                        buff->blk_typ = NORM;         /* Normal block       */
                    
                    cmp_size = encode (data_read,     /* Encode size        */
                                      file_buffer,    /* Source             */
                                      comp_buffer);   /* Destination        */

                    if ( cmp_size  < data_read  )     /* If compressed      */
                    {
			buff->len = (cmp_size+OVRHD); /* Length of block    */
                        buff->blk_typ |= COMP;        /* Show compressed    */
                        memcpy (&buff->blk_dat,       /* Start of data      */
                                   comp_buffer,       /* Copy from here     */
                                   cmp_size);         /* This much          */
                    }
                    else                              /* Not compressed     */
                    {
			buff->len = (data_read+OVRHD);/* Length of block    */
                        memcpy (&buff->blk_dat,       /* Copy to            */
                                   file_buffer,       /* Copy from          */
                                   data_read);        /* This amount        */
                    }
                    calc_crc(SET_CRC,                 /* Calculate CRC      */
                            buff->len ,               /* Length of block    */
                            out_buffer);              /* Where data is      */
                    status = send_blk(                /* Send the block     */
                             buff->len,               /* Block length       */
                             &syst,                   /* Read block ptr.    */
                             out_buffer);             /* Buffer pointer     */

                    time(&finish);                    /* Get end time       */
                    if (finish - start)               /* Guard div/zero     */
                        syst.s_cps = (short)          /* Calc Block CPS     */
                        (data_read / (finish - start) );
                    if ( buff->blk_typ == EOF_)       /* Last record        */
                        break;
                }
                close_chan(port);                     /* Close the port     */
                if (status)
                    syst.s_sta = abrt;                /* A text pointer     */
                else
                    syst.s_sta = done;                /* A text pointer     */

                file_io(CLOSE, &handle,
                        &file_name, NULL);            /* Close the file     */
                screen (SCR_SYS,&syst,NULL);          /* Show status block  */
            }
            else                                      /* File not found     */
            {
                status = JM_FNF;
            }
        break;  /* End of CASE 'S' */
        }
    }
    cleanup:
    free (in_buffer);                                  /* Free  buffers     */
    free (out_buffer);
    free (comp_buffer);
    free (file_buffer);
    /* Five-second timer to display error messages */
    if (status != JM_NRM)
    {
        time(&finish);
        start = 0;
        finish += 5;
        while ( finish > start )
            time(&start);
    }
    screen (SCR_END,NULL,NULL);                         /* Clear the screen */
    return status;                                      /* Normal exit      */
}
/****************************************************************************/
/*                          Send the JMODEM block                           */
/****************************************************************************/
unsigned short send_blk (blk_len, sys_ptr, buffer)
unsigned short blk_len;
SYS *sys_ptr;
unsigned char *buffer;
{
    unsigned char ack_buf;                  /* Buffer for ACK/NAK         */
    unsigned short tries = 10;              /* Attempts to send the block */
    while ((tries--) && (!user_abort))
    {
        write_chan(blk_len,buffer);          /* Send the JMODEM block      */
        flush();                             /* Clear back channel noise   */
        do
        {
            ack_buf = NULL;                  /* Clear the return buffer    */
            read_chan(1,&ack_buf);           /* Receive a response         */
        } while ( (ack_buf != ACK)           /* Stay in loop until we      */
               && (ack_buf != CAN)           /*  ... get something useful  */
               && (ack_buf != NAK)           /* This helps re-sync in noise*/
               && (ack_buf !=NULL)
               && (!user_abort) );

        if (ack_buf == CAN)                  /* Check for an abort         */
            break;                           /* User aborted               */
        if (ack_buf == ACK)                  /* If good block              */
        {
            if (tries == 9)                  /* If no retries              */
            {
		sys_ptr->s_len += 512;        /* Increase block-size       */
		if (sys_ptr->s_len > DAT_MAX) /* If too large              */
		    sys_ptr->s_len = DAT_MAX;
            }
            else
            {
	    sys_ptr->s_len = sys_ptr->s_len >> 1; /* Div by two             */
	    if (sys_ptr->s_len  < 0x40)       /* If less than minimum       */
		sys_ptr->s_len = 0x40;        /* Set to minimum             */
            }
        sys_ptr->s_sta = okay;                /* Show status is okay        */
        return JM_NRM;                        /* Show good                  */
        }
    sys_ptr->s_sta = retry;                   /* Show a retry               */
    screen (SCR_SYS, sys_ptr,NULL);           /* Write to screen            */
    }
	ack_buf = CAN;                        /* Send ^Xes                  */
        for (tries = 0; tries <6; tries++)    /* Six times                  */
            write_chan(1,&ack_buf);
        return JM_ABT;                        /* Abort local program        */

}
/****************************************************************************/
/*                        Receive the JMODEM block                          */
/****************************************************************************/
unsigned short recv_blk (blk_len, buffer)
unsigned short *blk_len;                  /* Pointer to the block-length   */
unsigned char *buffer;                    /* Pointer to the buffer         */
{
    JBUF *buff;                           /* Pointer type JBUF             */
    unsigned char nak_buf;                /* Buffer for ACK/NAK            */
    unsigned short tries = 10;            /* Attempts to receive the block */
    unsigned short ret_val;               /* Block length returned         */
    buff = (JBUF * )buffer;               /* Assign pointer type JBUF      */

    while ((tries--) && (!user_abort))
    {
        ret_val = read_chan(2,buffer);    /* Receive the block size        */
        if (ret_val == 2)                 /* If we received the length     */
        {
	    *blk_len = buff->len;         /* So caller knows size          */
            if (*blk_len > DAT_LEN)       /* If way out of line            */
                break;                    /* NAK it                        */
            ret_val = read_chan(          /* Get more data                 */
		      (*blk_len)-2 ,      /* Size to read                  */
		      &buff->blk_typ);    /* Where to put it               */
	    if (ret_val == (*blk_len)-2)  /* If we got what we requested   */
		return JM_NRM;
        }
    nak_buf = NAK;                          /* Get a NAK                   */
    write_chan(1,&nak_buf);                 /* Send to remote              */
    flush();                                /* Flush the buffer            */
    }
    nak_buf = CAN;                          /* Send ^Xes                   */
    for (tries = 0; tries <6; tries++)
    write_chan(1,&nak_buf);
    return JM_ABT;                          /* Abort local program        */
}
/****************************************************************************/
/*                         Synchronize during receive                       */
/****************************************************************************/
unsigned short rx_sync()
{
    short i;
    unsigned char ack_nak;              /* Single byte buffer for ACK/NAK  */
    flush();                            /* Clear the interrupt buffer      */
    while (!user_abort)
    {
        ack_nak = NULL;                 /* Clear the buffer                */
        read_chan(1,&ack_nak);          /* Receive ACK, NAK, or SYN        */
        if (ack_nak == CAN)             /* If a ^X                         */
            break;
        if ( ack_nak == ACK )           /* If a good response              */
            return JM_NRM;              /* Show handshake                  */
        if ( ack_nak == NAK )           /* If a good response              */
        {
            ack_nak = ACK;
            write_chan(1,&ack_nak);     /* Send a ACK response             */
	    return JM_NRM;
         }
         ack_nak = NAK;
         write_chan(1,&ack_nak);        /* Keep sending MAKs               */
    }
    ack_nak = CAN;
    for (i=0; i<8; i++)                 /* Send 8 ^Xes                     */
        write_chan(1,&ack_nak);         /* Send a response                 */
    return JM_ABT;
}
/****************************************************************************/
/*                         Synchronize during transmit                      */
/****************************************************************************/
unsigned short tx_sync()
{
    unsigned short ret_val;
    ret_val = rx_sync();                /* Call same routine for receive    */
    if (ret_val)                        /* If no success                    */
        return ret_val;                 /* Abort routines                   */
    flush();                            /* Else flush the input buffer      */
    timer = 5;                          /* 5 timer-ticks to wait            */
    while (timer)                       /* Wait for timer                   */
        ;
    return JM_NRM;                      /* Normal return                    */
}
/****************************************************************************/
/************************ E N D  O F   M O D U L E **************************/