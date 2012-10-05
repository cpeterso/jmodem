/****************************************************************************/
/*   FILE JMODEM_C.C                                                        */
/*   Created 11-JAN-1990                   Richard B. Johnson               */
/*                                         405 Broughton Drive              */
/*                                         Beverly, Massachusetts 01915     */
/*                                         BBS (508) 922-3166               */
/*                                                                          */
/*   File I/O                                                               */
/*   file_io();                                                             */
/*   All of the file I/O is called from this one block to ease portability  */
/*                                                                          */
/****************************************************************************/
#include <stdio.h>                      /* Used for NULL                   */
#include <dos.h>                        /* Used for file-size              */
#include <io.h>                         /* Ysed for Unix / DOS type files  */
#include <fcntl.h>                      /* Used for O_BINARY, etc          */
#include <string.h>                     /* Used for _strchr(), etc         */
#include "jmodem.h"                     /* Used for JMODEM primatives      */

/*   Note:                                                                  */
/*   I originally used fopen();, fclose();, fread();, and fwrite(); the     */
/*   default C stream-I/O files. However, a bug in Microsoft's implementa-  */
/*   tion results in corruption of files larger than about 512k. The bug    */
/*   inserts a null into the  files every approximately 512k.               */
/*   Therefore, I changed to the UNIX-style disk I/O.                       */
/*                                                                          */
/****************************************************************************/
/* Merged from \include\sys\stat.h because improperly organized header-file */
/* REQUIRES that two other files be included first! Thanks, Microsoft...    */
#define S_IFMT      0170000         /* file type mask */
#define S_IFDIR     0040000         /* directory */
#define S_IFCHR     0020000         /* character special */
#define S_IFREG     0100000         /* regular */
#define S_IREAD     0000400         /* read permission, owner */
#define S_IWRITE    0000200         /* write permission, owner */
#define S_IEXEC     0000100         /* execute/search permission, owner */
/****************************************************************************/
struct find_t o_file;                           /* Get file size          */
unsigned short file_io(command, handle, buffer, len)
unsigned short command;                         /* Read/write/open, etc.  */
short *handle;                                  /* File handle            */
unsigned char **buffer;                         /* Working buffer pointer */
unsigned short len;                             /* Bytes to read/write    */
{
    char temp[65];                              /* To rename a file       */
    char *dot;                                  /* Find the "."           */
    unsigned short error;                       /* Possible error-code    */
    switch (command)                            /* GOTO in disguise       */
    {
/**************************************************************************/
/*      Open the file for read. If the open fails, return non-zero.       */
/**************************************************************************/
        case OPEN_READ:
        {
            screen (SCR_FIL,NULL,*buffer);
            if ( (*handle = open(*buffer,O_RDONLY|O_BINARY)) != -1 )
            {
                _dos_findfirst(*buffer,_A_NORMAL, &o_file);
                syst.s_byt = o_file.size;       /* Show file size        */
                screen (SCR_FOK,&syst,NULL);    /* Show success          */
                return JM_NRM;
            }
        screen (SCR_FNF,NULL,NULL);             /* Show failure          */
        return JM_FNF;
        }
/**************************************************************************/
/*       Create a new file. If the file already exists, rename it.        */
/**************************************************************************/
       case CREATE:
        {
            screen (SCR_FIL,NULL,*buffer);
            if ( (*handle = open(*buffer,O_RDONLY|O_BINARY)) != -1 )
            {                                    /* If file already exists */
                close (*handle);                 /* Close the open file    */
                strcpy (temp,*buffer);           /* Copy the file name     */
                dot = strchr (temp, '.');        /* Find "." in string     */
                strcpy (dot, ".OLD");            /* FILENAME.OLD           */
                error = rename (*buffer,temp);   /* Rename the file        */
                if (error)                       /* Can't rename the file  */
                {
                    screen (SCR_FCR,NULL,temp);  /* Tell user can't rename */
                    return JM_REN;               /* Quit                   */
                }
                else
                {
                   screen (SCR_FRN,NULL,temp);   /* Tell user we renamed   */
                }
            }
/* Note:                                                                    */
/*     Microsoft's own rules of MS-DOS state that a file being CREATED is   */
/*     open for WRITE (naturally, who would wish to READ from an empty      */
/*     file?).                                                              */
/*     Unix also provides this same logic.                                  */
/*     In the following code, if I did not OR in O_RDWR, I would create a   */
/*     file I can't write to!. If I did not also include the [optional]     */
/*     parameters, S_IWRITE OR S_IREAD, I create a file I can't delete!!!   */
/*     These are implementation bugs.                                       */
/*                                                                          */
            if  ( (*handle = open(               /* Open (create) new file  */
                               *buffer,          /* File name               */
                                O_CREAT          /* Create new file         */
                               |O_BINARY         /* Binary mode             */
                               |O_RDWR ,         /* Read/write (bug??)      */
                                S_IWRITE         /* Definite bug            */
                               |S_IREAD)) != -1) /* More of same bug        */
            {
                screen (SCR_FOK,NULL,temp);      /* Show success            */
                return JM_NRM;
            }
            else
            {
                screen (SCR_FCR,NULL,temp);      /* Show failure            */
                return JM_CRE;
            }
        }
/**************************************************************************/
/*      Write 'len' bytes to the file. Return the actual bytes written    */
/**************************************************************************/
        case WRITE:
        {
        return ( write ( *handle, *buffer, len) );
        }
/**************************************************************************/
/*      Read 'len' bytes from the file. Return the actual bytes read.     */
/**************************************************************************/
        case READ:
        {
        return ( read ( *handle, *buffer, len) );
        }
/**************************************************************************/
/*      Close the file. Return any error-code (ignored).                  */
/**************************************************************************/
        case CLOSE:
        {
        return (close (*handle));
        }
/**************************************************************************/
/*      Delete the file named in 'buffer'. Return any error code.         */
/**************************************************************************/
        case DELETE:
        {
        return (remove (*buffer));
        }
    }
}
/**************************************************************************/
/********************** E N D  O F  M O D U L E  **************************/