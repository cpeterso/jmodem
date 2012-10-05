/****************************************************************************/
/*   FILE JMODEM_B.C                                                        */
/*   Created 11-JAN-1990            Richard B. Johnson                      */
/*                                  405 Brougton Drive                      */
/*                                  Beverly, Massachusetts 01915            */
/*                                  BBS (508) 922-3166                      */
/*                                                                          */
/*   allocate_memory();  (All memory allocation except for screen)          */
/*   get port();         (Parse, get ASCII port)                            */
/*   get_inp();          (Parse, get filename)                              */
/*   get_fun();          (Parse, get function S,R )                         */
/*   get_prt();          (Convert ASCII port to numeric offset)             */
/*                                                                          */
/****************************************************************************/
#include <stdio.h>                              /* Used for _puts();        */
#include <stdlib.h>                             /* Used for _malloc();      */
#include <malloc.h>                             /* Used for _malloc();      */
#include <string.h>                             /* Used for _strcpy(), etc  */
#include "jmodem.h"                             /* JMODEM primatives        */
/****************************************************************************/
/*                          Allocate memory                                 */
/****************************************************************************/
unsigned char *allocate_memory(buf_len)
unsigned short buf_len;
{
    unsigned char *memory;
    memory = (unsigned char *) malloc ( buf_len );
    if (memory == NULL)
        puts("\nMemory allocation failed!");
    return memory;
}
/****************************************************************************/
/*                     Get port offset number ( 0 - 3)                      */
/****************************************************************************/
unsigned short get_port (prt_str)
char prt_str;
{
    return ( ((unsigned short) prt_str) - '1');  /* Subtract ASCII bias + 1 */

}
/****************************************************************************/
/*                          Get filename                                    */
/****************************************************************************/
char *get_inp (argc, argv)
unsigned short argc;
char *argv[];
{
    short i=0;
    static char name[64];               /* Max DOS filename/path size    */
    if (argc >= 3)                      /* Check command-line parameters */
    {
        strcpy (name, argv[2]);         /* Copy the file name            */
        while (name[i])                 /* Until the NULL character      */
        {
	    if ( ( name[i] <= 'z')      /* Check upper limit             */
	      && ( name[i] >= 'a') )    /* Check lower limit             */
                  name[i] &= 95;        /* Map to upper case             */
        i++;
        }
        return name;                    /* Return a pointer to the name  */
    }
    return NULL;
}
/****************************************************************************/
/*                          Get function  (S or R)                          */
/****************************************************************************/

char *get_fun (argc,argv)
unsigned short argc;
char *argv[];
{
    static char funct[3];
    if (argc >= 3)                              /* Command-line parameters */
    {
        strcpy (funct, argv[1]);
        funct[1] = NULL;                        /* Blank out port number  */
        funct[0] = funct[0] & 95;               /* Map to upper case      */
        if (funct[0] == 'S' || funct[0] == 'R') /* Check valid parameters */
            return &funct[0];
    }
    return NULL;
}
/****************************************************************************/
/*                      Get port ASCII number (1 - 4)                       */
/****************************************************************************/
char *get_prt (argc,argv)
unsigned short argc;
char *argv[];
{
    static char prt[3];
    if (argc >= 3)                              /* Command-line parameters  */
    {
        strcpy (prt, argv[1]);                  /* Get port number          */
        if (prt[1] > '0' && prt[1] < '5')       /* Check for valid ports    */
            return &prt[1];                     /* Pointer to the ASCII num */
    }
    return NULL;
}
/****************************************************************************/
/************************ E N D  O F   M O D U L E **************************/