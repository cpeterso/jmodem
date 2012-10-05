/****************************************************************************/
/*   FILE JMODEM_E.C                                                        */
/*   Created 11-JAN-1990                 Richard B. Johnson                 */
/*                                       405 Broughton Drive                */
/*                                       Beverly, Massachusetts 01915       */
/*                                       BBS (508) 922-3166                 */
/*   open_chan();                                                           */
/*   close_chan();                                                          */
/*   read_chan();                                                           */
/*   write_chan();                                                          */
/*                      Communications I/O procedures                       */
/*   These procedures will have to be replaced for JMODEM to execute on     */
/*   a system other than a MS_DOS computer. They are VERY hardware-specific.*/
/*   These procedures are grouped so that they can be replaced as a unit.   */
/*   You must replace the following:                                        */
/*  (1) OPEN a communications channel.                                      */
/*  (2) CLOSE the opened channel.                                           */
/*  (3) READ [variable] bytes from the channel.                             */
/*  (4) WRITE [variable] bytes to the channel.                              */
/*                                                                          */
/*  When attempting to READ bytes, some sort of time-out must be provided   */
/*  within the routine to prevent a "wait-forever" syndrome.                */
/*                                                                          */
/*  VAX/VMS, QIO routines are ideal!                                        */
/*                                                                          */
/****************************************************************************/
#include <stdio.h>                      /* For FILE structure           */
#include <conio.h>                      /* For _inp() and _outp()       */
#include <dos.h>                        /* For _enable() and _disable() */
#include <memory.h>                     /* _memcpy();                   */
#include "jmodem.h"                     /* JMODEM defines               */
#include "uart.h"                       /* 8250 UART                    */
#pragma intrinsic (inp, outp, _disable, _enable, memcpy)
#pragma check_stack(off)
typedef struct  {
        unsigned short base;            /* Base port address            */
        unsigned short mask;            /* Interrupt controller mask    */
        unsigned short int_num;         /* Interrupt number             */
        } PORTS;

PORTS port_pars[] =     {
                        {0x3F8  ,       /* Base port address    COM1    */
                        0xEF    ,       /* IRQ4 11101111B       COM1    */
                        0x0C    ,       /* Interrupt number             */
                        }       ,
                        {
                        0x2F8   ,       /* Base port address    COM2    */
                        0xF7    ,       /* IRQ3 11110111B       COM2    */
                        0x0B    ,       /* Interrupt number             */
                        }       ,
                        {
                        0x3E8   ,       /* Base port address    COM3    */
                        0xEF    ,       /* IRQ4 11101111B       COM3    */
                        0x0C    ,       /* Interrupt number             */
                        }       ,
                        {
                        0x2E8   ,       /* Base port address    COM4    */
                        0xF7    ,       /* IRQ3 11110111B       COM4    */
                        0x0B    ,       /* Interrupt number             */
                        }
                        };
extern unsigned char *int_buffer;       /* pointer to interrupt buffer  */
extern unsigned short user_abort;       /* User abort flag              */
unsigned char *write_ptr;               /* Interrupt buffer             */
unsigned char *read_ptr;                /* Interrupt buffer             */
unsigned short port;                    /* Port number                  */
unsigned short old_mask;                /* Old interrupt control mask   */
unsigned short old_ier;                 /* Old interrupt enable regis   */
unsigned short old_mcr;                 /* Old modem control register   */
unsigned short old_stat;                /* Modem status for flow-contr  */
unsigned short timer;                   /* Global timer                 */
unsigned short hardware_port;           /* Physical port                */
void interrupt far fatal_abort(void);   /* Abort vector                 */
void interrupt far com_int(void);       /* Interrupt service routine    */
void interrupt far tim_int(void);       /* Timer interrupt              */
void (interrupt far *old_tim)();        /* Pointer to old timer intr.   */
void (interrupt far *old_com)();        /* Pointer to old commu intr.   */
void (interrupt far *old_brk)();        /* Pointer to old break key.    */
/****************************************************************************/
/*                 Open the communications channel                          */
/*                                                                          */
/*    Under MS-DOS this involves saving the com-port vector, interrupt      */
/*    controller mask, and the user-tick timer vector.                      */
/*    New vectors and masks and patched for the communications interrupt    */
/*    service routine and the local timer. These vectors will be restored   */
/*    within the CLOSE channel routine.                                     */
/*                                                                          */
/****************************************************************************/
unsigned short open_chan (user_port)
unsigned short user_port;               /* Port offset ( 0 - 3 )           */
{
    short i;
    flush();                            /* Initialize the interrupt buffer */
    hardware_port =
       port_pars[user_port].base;       /* Set hardware port               */
    outp(hardware_port+MCR, 0x0F);      /* Turn everything on.             */
    old_ier = inp(hardware_port +IER);  /* Get interrupt enable regis      */
    old_mcr = inp(hardware_port +MCR);  /* Get old modem control register  */
    old_brk = _dos_getvect(0x1B);       /* Get old break key vector        */
   _dos_setvect(0x1B,fatal_abort);      /* Set fatal abort vector (1)      */
   _dos_setvect(0x23,fatal_abort);      /* Set fatal abort vector (2)      */
    old_mask = inp(0x21);               /* Save old interrupt mask         */
    old_tim = _dos_getvect(0x1C);       /* Get old DOS timer-tick vector   */
    old_com = _dos_getvect(
       port_pars[user_port].int_num);   /* Get old communications vector   */
    _dos_setvect(0x1C,tim_int);         /* Set new timer interrupt         */
    _dos_setvect(
       port_pars[user_port].int_num,    /* Set new communications vector   */
       com_int);
    outp(0x21,old_mask &
       port_pars[user_port].mask);      /* Set interrupt enable mask       */
    outp(hardware_port+IER, IER_ERBFI); /* Enable received data available  */
    for (i=0; i<8; i++)                 /* Edge-triggering, read the ports */
    {
        outp(0x20,0x20);                /* Reset the hardware controller   */
        inp(hardware_port + i);         /* Port to clear                   */
    }
    outp(0x20,0x20);                    /* Reset the hardware controller   */
    flush();                            /* Clear interrupt buffer again    */
    old_stat = inp(hardware_port+MSR)
               &0xF0;                   /* Get current modem status        */
    return JM_NRM;
}
/****************************************************************************/
/*                 Close the communications channel                         */
/*                                                                          */
/*    Under MS-DOS this involves restoring the interrupt vectors and        */
/*    controller mask that was saved during the OPEN routine.               */
/*                                                                          */
/****************************************************************************/
unsigned short close_chan (user_port)
unsigned short user_port;
{
    outp(hardware_port+IER,old_ier);    /* Set old interrupt enable        */
    outp(0x21,old_mask);                /* Restore old interrupt mask      */
    _dos_setvect(
       port_pars[user_port].int_num,    /* Set old communications vector   */
       old_com);
    _dos_setvect(0x1C,old_tim);         /* Set old timer interrupt         */
    _dos_setvect(0x1B,old_brk);         /* Set old break interrupt         */
    return JM_NRM;
}
/****************************************************************************/
/*              Read from the communications channel                        */
/*                                                                          */
/*    This involves transferring data from the interrupt buffer and         */
/*    maintaining the interrupt buffer pointers. A timeout is established.  */
/*                                                                          */
/****************************************************************************/
unsigned short read_chan (bytes, buffer)
unsigned short bytes;                   /* Bytes requested                 */
unsigned char *buffer;                  /* Pointer to the user's buffer    */
{
    unsigned short count;               /* Byte count                      */
    unsigned short avail;               /* Bytes available                 */
    timer = TIMOUT;                     /* Set initial timeout value       */
    count = bytes;                      /* Set byte-count                  */

    while (count && timer)              /* If byte request or no timeout   */
    {
        avail = write_ptr - read_ptr;   /* Bytes available                 */
        if (avail)                      /* If bytes available              */
        {
            if (avail > count)          /* If more bytes than we need      */
                avail = count;          /* Take only what we need          */
            memcpy (buffer   ,          /* User's buffer                   */
                    read_ptr ,          /* Interrupt buffer pointer        */
                    avail)   ;          /* Copy to user's buffer           */
            count -= avail;             /* Update count                    */
            read_ptr +=avail;           /* Update read pointer             */
            buffer   +=avail;           /* Update write pointer            */
            timer = TIMOUT;             /* Set new timer value             */
        }
        _disable();                     /* Clear interrupts                */
        if (read_ptr == write_ptr)      /* If no bytes available           */
        {
            read_ptr = int_buffer;      /* Initialize the interrupt buffer */
            write_ptr = int_buffer;     /* Initialize the interrupt buffer */
        }
        _enable();                      /* Enable interrupts               */
    }
    return(bytes - count);              /* Actual characters received      */
}
/****************************************************************************/
/*                      Flush the interrupt buffer                          */
/****************************************************************************/
void flush()
{
    _disable();
    read_ptr = int_buffer;              /* Initialize the interrupt buffer */
    write_ptr = int_buffer;             /* Initialize the interrupt buffer */
    _enable();
}
/****************************************************************************/
/*                      Communications transmit routine                     */
/*    Write 'bytes' bytes from buffer to the UART. Don't return until done  */
/*    unless the carrier failed or the hardware broke.                      */
/****************************************************************************/
unsigned short write_chan (bytes, buffer)
unsigned short bytes;                         /* Bytes to send             */
unsigned char *buffer;                        /* Pointer to the buffer     */
{
    unsigned short status;

    timer = TIMOUT;
    while ((bytes && timer) && !user_abort )  /* Bytes, no abort, no timout */
    {
        while ( (status = (inp(hardware_port+MSR) & 0xF0) )!= old_stat)
        {                                  /* Flow control loop            */
            if (!(status & MSR_RLSD))      /* If the modem carrier failed  */
            {
                user_abort = 0x0FFFF;      /* Set the abort flag           */
                return JM_ABT;
            }
        }
        status = inp(hardware_port+LSR);   /* Get line-status              */
        if (status & LSR_THRE)             /* If TX holding register empty */
        {
            outp(hardware_port,*buffer++); /* Send the byte                */
            bytes--;                       /* Bump the byte-count          */
            timer = TIMOUT;                /* Set new timer-value          */
        }
    }
    return JM_NRM;
}
/****************************************************************************/
/*                Communications adapter hardware interrupt                 */
/*    This is very simple because we interrupt on receive only. Since we    */
/*    must wait until the entire block has been received and checked be-    */
/*    for doing anything else, the transmitter is polled.                   */
/*                                                                          */
/****************************************************************************/
void interrupt far com_int()
{
    *write_ptr = (unsigned char)
        inp(hardware_port);                    /* Put byte in buffer        */
    outp(0x20,0x20);                           /* Reset hardware controller */
    if (write_ptr < int_buffer + DAT_LEN )     /* Check buffer for overflow */
        write_ptr++;                           /* Bump pointer if room      */
}
/****************************************************************************/
/*                            Timer interrupt                               */
/*    A WORD (timer) gets decremented every timer-tick if it is not already */
/*    zero. This is used to set time-out values in the communication pro-   */
/*    cedures so that a "wait-forever" can't occur.                         */
/*                                                                          */
/****************************************************************************/
void interrupt far tim_int()
{
    if (timer)                          /* If NZ                           */
        timer--;                        /* Bump the timer                  */
    outp(0x20,0x20);                    /* Reset the hardware controller   */
    _enable();                          /* Allow network interrupts        */
    _chain_intr(old_tim);               /* Go to old timer-tick routine    */
}
/****************************************************************************/
/*                          A B O R T   trap                                */
/*    Control-C and control-break vectors are set to point here so that     */
/*    a user-break harmlessly sets a flag so that interrupt vectors may     */
/*    properly restored upon program exit.                                  */
/*                                                                          */
/****************************************************************************/
void interrupt far fatal_abort()
{
    user_abort = 0xFFFF;                              /* Set abort flag     */
}
/****************************************************************************/
/******************** E N D   O F   M O D U L E *****************************/