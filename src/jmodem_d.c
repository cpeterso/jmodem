/****************************************************************************/
/*   FILE JMODEM_D.C                                                        */
/*   Created 11-JAN-1990                  Richard B. Johnson                */
/*                                        405 Broughton Drive               */
/*                                        Beverly, Massachusetts 01915      */
/*                                        BBS (508) 922-3166                */
/*                                                                          */
/*   disp();        (Displays a "USAGE" message)                            */
/*   encode();      (Data compression routine  )                            */
/*   decode();      (Data expansion routine    )                            */
/*   calc_crc();    (CRC checking and setting  )                            */
/*                                                                          */
/****************************************************************************/
#include <stdio.h>                              /* For _printf()            */
#include <stdlib.h>                             /* For _rotl()              */
#include "jmodem.h"                             /* JMODEM primatives        */
#pragma intrinsic (_rotl)
/****************************************************************************/
/*                          Print the 'Usage' prompt.                       */
/****************************************************************************/
void disp()
{
    unsigned short i;
    printf("\nUsage:");
    for (i=1; i < 5; i++)
        printf("\nJMODEM S%d FILENAME.TYP < Send a file using COM%d  >",i,i);
    for (i=1; i < 5; i++)
        printf("\nJMODEM R%d FILENAME.TYP <Receive a file using COM%d>",i,i);
    return;
}
/****************************************************************************/
/*                   Encode (compress) the input string.                    */
/*   The routine looks for groups of identical characters and replaces them */
/*   with the character  0xBB, a word denoting the number of characters to  */
/*   duplicate, followed by the character to duplicate.                     */
/*                                                                          */
/****************************************************************************/

unsigned short encode(len, in_buffer, out_buffer)
unsigned short len;                             /* Length of input string   */
unsigned char *in_buffer;                       /* Pointer to input buffer  */
unsigned char *out_buffer;                      /* Pointer to output buffer */
{
    unsigned short how_many=0;                  /* Character count          */
    unsigned short count=0;                     /* Output byte count        */
    unsigned char dupl;                         /* Character to replace     */

    while (len)                                 /* While bytes in buffer    */
    {
        if ( (*in_buffer == 0xBB)               /* If the sentinel byte     */
          || (*in_buffer == *(in_buffer+1)) )   /* If two adjacent the same */
        {
            *out_buffer++ = 0xBB;               /* Insert , bump pointer    */
            dupl = *in_buffer;                  /* Save duplicate character */
            how_many = 0;                       /* Duplicate char count     */
            while ( (*in_buffer++ == dupl)      /* Count duplicates         */
			      && (len) )        /* While bytes still left.  */
            {
                how_many++;                     /* Identical characters     */
                len--;
            }

            *out_buffer++ =
               (unsigned char) how_many;       /* Character count, low byte */
            *out_buffer++ =
              (unsigned char) (how_many >> 8); /* Character count high byte */
            *out_buffer++ = dupl;              /* The duplicate character   */
            count += 4;                        /* Adjust byte count         */
            in_buffer--;                       /* Non-duplicate character   */
        }
        else
        {
	    *out_buffer++ = *in_buffer++;      /* Copy byte                 */
	    count++;                           /* Character count           */
            len--;
        }
	if ( count > DAT_MAX )                 /* Check buffer limit        */
            return JM_MAX;
    }
    return count;                              /* New length                */
}
/****************************************************************************/
/*                     Decode (expand) the encoded string.                  */
/*    Routine checks for a sentinel byte, 0xBB, and if found, takes the     */
/*    following word as the number of identical bytes to add. The actual    */
/*    byte to add is located following the length-word.                     */
/*                                                                          */
/****************************************************************************/
unsigned short decode(len, in_buffer, out_buffer)
unsigned short len;                             /* Length to input string   */
unsigned char *in_buffer;                       /* Pointer to input buffer  */
unsigned char *out_buffer;                      /* Pointer to output buffer */
{
    unsigned short i=0;
    unsigned short j=0;
    unsigned char c;
    unsigned char *limit;                       /* Length of input buffer   */

    limit = in_buffer + len;                    /* Set end pointer once     */
    while (in_buffer < limit)
    {
        if (*in_buffer == 0xBB )                /* If the sentinel byte     */
        {
             in_buffer ++;                      /* Next character           */
             i = (unsigned short) *in_buffer++; /* Get low byte, incr       */
             i = i | (unsigned short)
                 *in_buffer++ << 8;             /* Get high byte, incr      */
             c = *in_buffer++;                  /* Get byte, incr           */
             for ( ; i >0; i--)                 /* Don't alter i at start   */
             {
                 *out_buffer++ = c;             /* Expand byte              */
                 j++;                           /* Character count          */
             }
        }
        else                                    /* Else, just copy          */
        {
            *out_buffer++ = *in_buffer++;
            j++;                                /* Character count          */
        }
    }
    return j;                                   /* New string length        */
}
/****************************************************************************/
/*                  Calculate the simple JMODEM CRC                         */
/*    Routine obtains a pointer to the buffer of characters to be checked.  */
/*    The first passed parameter is the length of the buffer. The CRC is    */
/*    returned.                                                             */
/*                                                                          */
/****************************************************************************/
unsigned short calc_crc(command, length, buffer)
unsigned short command;                     /* Set or Check CRC             */
unsigned short length;                      /* Buffer length                */
unsigned char *buffer;                      /* Pointer to the buffer        */
{
    unsigned short crc=0;                   /* Start at zero                */
    unsigned short i;                       /* Byte count                   */

    length -=2;                             /* Don't CRC the CRC            */
    do
    {
    crc += (unsigned short) *buffer++;      /* Sum first                    */
    i = length & 0x07;                      /* 7 bits max to rotate         */
    crc  = _rotl(crc,i);                    /* Rotate i bits left           */
    } while (--length);

    switch (command)
    {
        case GET_CRC:
        {
            i = (unsigned short) *buffer++;
            i = i |  (unsigned short) (*buffer << 8 );
            return (crc - i);               /* Return AX = 0 if CRC okay    */
        }
        case SET_CRC:
        {
            *buffer++ = (unsigned char) crc;
            *buffer   = (unsigned char) (crc >> 8);
            return crc;                     /* Return CRC though ignored    */
        }
        default:
            return JM_MAX;                  /* Bad data, buffer overflow    */
    }
}
/****************************************************************************/
/************************ E N D  O F   M O D U L E **************************/