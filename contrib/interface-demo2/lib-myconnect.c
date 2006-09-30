/* lib-myconnect.c:

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

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>

/* TCP connection to given host/port
 */
int get_connection(char *peername, int peerport)
{
    struct sockaddr_in peeraddr;
    int SocketDescriptor;
    struct hostent *HostEntry;

    HostEntry=gethostbyname(peername);
    if (HostEntry==NULL)
    {
        switch (h_errno)
        {
        case HOST_NOT_FOUND:
            printf("Host not found.\n");
            return -1;
            break;
        case NO_ADDRESS:
            printf("No IP found for valid hostname.\n");
            return -1;
            break;
        case NO_RECOVERY:
            printf("Name service error.\n");
            return -1;
            break;
        case TRY_AGAIN:
            printf("Temporary error in name resolution.\n");
            return -1;
            break;
        default:
            break;
        }
    }
    /* peeraddr must contain information to reach the server */
    memset(&peeraddr, 0, sizeof(peeraddr));
    peeraddr.sin_family = AF_INET;
    peeraddr.sin_port = htons(peerport);
    peeraddr.sin_addr = *(struct in_addr *) HostEntry->h_addr;

    SocketDescriptor=socket(PF_INET, SOCK_STREAM, 0);
    if (SocketDescriptor==-1)
    {
        printf("Error creating socket\n");
        return -1;
    }

    if (connect(SocketDescriptor, (struct sockaddr *) &peeraddr, sizeof(peeraddr))==-1)
    {
        perror("Error connecting");
        return -1;
    }
    return SocketDescriptor;
}


