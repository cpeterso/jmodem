/****************************************************************************/
/*    FILE UART.H                                                           */
/*    Created 11-JAN-1990                   Richard B. Johnson              */
/*                                          450 Broughton Drive             */
/*                                          Beverly, Massachusetts 01915    */
/*                                          BBS (508) 922-3166              */
/*        Parameters for the 8250 UART and hardware ports.                  */
/****************************************************************************/
#define BASE            0x00            /* Base port                    */
#define TX_BUF          0x00            /* Transmitter buffer           */
#define RX_BUF          0x00            /* Receiver buffer              */
#define IER             0x01            /* Interrupt enable register    */
#define IIR             0x02            /* Interrupt ident register     */
#define LCR             0x03            /* Line control register        */
#define MCR             0x04            /* Modem control register       */
#define LSR             0x05            /* Line status register         */
#define MSR             0x06            /* Modem status register        */

#define MSR_DCTS        0x01            /* Delta clear to send          */
#define MSR_DDSR        0x02            /* Delta data set ready         */
#define MSR_TERI        0x04            /* Trailing edge ring indicator */
#define MSR_DSLSD       0x08            /* Delta receive line signal    */
#define MSR_CTS         0x10            /* Clear to send                */
#define MSR_DSR         0x20            /* Data set ready               */
#define MSR_RI          0x40            /* Ring indicator               */
#define MSR_RLSD        0x80            /* Received line signal detect  */

#define LSR_DR          0x01            /* Data ready                   */
#define LSR_OR          0x02            /* Overrun error                */
#define LSR_PE          0x04            /* Parity error                 */
#define LSR_FE          0x08            /* Framing error                */
#define LSR_BI          0x10            /* Break interrupt              */
#define LSR_THRE        0x20            /* Trans holding register empty */
#define LSR_TSRE        0x40            /* Trans shift register empty   */
#define LSR_NOP         0x80            /* Not used                     */

#define IER_ERBFI       0x01            /* Enable Recev data available  */
#define IER_ETBEI       0x02            /* Enable Trans holding empty   */
#define IER_ELSI        0x04            /* Enable Recev line status     */
#define IER_EDSSI       0x08            /* Enable Modem status          */

#define IIR_MOD         0x00            /* Modem status                 */
#define IIR_IP          0x01            /* Interrupt pending            */
#define IIR_THE         0x02            /* Tx holding register empty    */
#define IIR_RDA         0x04            /* Received data available      */
#define IIR_RLS         0x06            /* Receiver line status         */
/****************************************************************************/
/*********************** E N D  O F   M O D U L E ***************************/
