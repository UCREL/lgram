//                            Package   : omniEvents
// getopt.cc                  Created   : 1/4/98
//                            Author    : Paul Nader (pwn)
//
//    Copyright (C) 1998 Paul Nader.
//
//    This file is part of the omniEvents application.
//
//    omniEvents is free software; you can redistribute it and/or
//    modify it under the terms of the GNU Lesser General Public
//    License as published by the Free Software Foundation; either
//    version 2.1 of the License, or (at your option) any later version.
//
//    omniEvents is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//    Lesser General Public License for more details.
//
//    You should have received a copy of the GNU Lesser General Public
//    License along with this library; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// Description:
//
//    getopt implementation for WIN32 platforms.
//

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include "win32.h"

static char* letP =NULL;    // Speichert den Ort des Zeichens der naechsten Option
static char SW ='-';       // DOS-Schalter, entweder '-' oder '/'

int optind = 1;
char *optarg;
int opterr = 1;

int getopt(int argc, char *argv[], const char *optionS)
{
   unsigned char ch;
   char *optP;

   if(argc>optind)
   {
      if(letP==NULL)
      {
        // Initialize letP
         if( (letP=argv[optind])==NULL || *(letP++)!=SW )
            goto gopEOF;
         if(*letP == SW)
         {
            // "--" is end of options.
            optind++;
            goto gopEOF;
         }
      }

      if((ch=*(letP++))== '\0')
      {
         // "-" is end of options.
         optind++;
         goto gopEOF;
      }
      if(':'==ch || (optP=(char*)strchr(optionS,ch)) == NULL)
      {
         goto gopError;
      }
      // 'ch' is a valid option
      // 'optP' points to the optoin char in optionS
      if(':'==*(++optP))
      {
         // Option needs a parameter.
         optind++;
         if('\0'==*letP)
         {
            // parameter is in next argument
            if(argc <= optind)
               goto gopError;
            letP = argv[optind++];
         }
         optarg = letP;
         letP = NULL;
      }
      else
      {
         // Option needs no parameter.
         if('\0'==*letP)
         {
            // Move on to next argument.
            optind++;
            letP = NULL;
         }
         optarg = NULL;
      }
      return ch;
   }
gopEOF:
   optarg=letP=NULL;
   return EOF;
    
gopError:
   optarg = NULL;
   errno  = EINVAL;
   if(opterr)
      perror ("error in command line");
   return ('?');
}
