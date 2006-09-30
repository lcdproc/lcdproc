/* lib-mylcd.h

 used by interfacedemo for "pyramid" LCD device.
 */

/*
 Copyright (C) 2005 Silvan Marco Fin <silvan@kernelconcepts.de>

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301

 */

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "lib-mylcd.h"

#define DEBUG 0

/* Request information from LCDd */

int get_integer(int SD, char *request)
{
    static char output[MAXREAD];

    write(SD, request, strlen(request));
    read(SD, output, MAXREAD-1);
    return atoi(output);
}

/* Used to parse the reply from the hello-msg.
 */
void parse_hello(char *input)
{
    char *next=input;
    char *retval;

    while (1)
    {
        retval=strtok(next, " ");
        next=NULL;
        if (retval==NULL)
            break;
        if (strcmp(retval, "wid")==0)
            screen.width=atoi(strtok(next, " "));
        if (strcmp(retval, "hgt")==0)
            screen.height=atoi(strtok(next, " "));
    }
}

int parse_reply(char *input)
{
    char *next=input;
    char *retstr;
    int retint=UNKNOWNKEY;
    int inputlen=strlen(input);
    int len=0;

    printf("Eingabe: %s\n", input);
    while (1)
    {
        retstr=strtok(next, " \n");
        next=NULL;
        if (retstr==NULL)
        {
            input[0]=0; /* wofür war das? */
            break;
        }
        len+=strlen(retstr)+1;
        /* key detected? */
        if (strcmp(retstr, "key")==0)
        {
            retstr=strtok(next, " \n");
            len+=strlen(retstr)+1;
            if (strcmp(retstr, "Escape")==0)
                retint=ESCAPEKEY;
            if (strcmp(retstr, "Down")==0)
                retint=DOWNKEY;
            if (strcmp(retstr, "Right")==0)
                retint=RIGHTKEY;
            if (strcmp(retstr, "Enter")==0)
                retint=ENTERKEY;

            input=strncpy(input, &input[len], inputlen-len+1);

            return retint;
        }
    }
    return NOKEY;
}

char *basic_send(int SD, char *request)
{
    static char answer[MAXREAD];
    if (DEBUG)
        printf("Sending command: %s\n", request);
    memset(answer, 0, MAXREAD);
    write(SD, request, strlen(request));
    write(SD, "\n", strlen("\n"));
    read(SD, answer, MAXREAD);
    if (DEBUG)
        printf("Answer to command to LCDproc: %s\n", answer);

    return answer;
}

char *basic_read(int SD)
{
    static char answer[MAXREAD];
    memset(answer, 0, MAXREAD);
    if (DEBUG)
        printf("Going for read!\n");
    read(SD, answer, MAXREAD);
    if (DEBUG)
        printf("Read returned: %s\n", answer);

    return answer;
}


