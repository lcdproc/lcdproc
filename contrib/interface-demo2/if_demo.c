/* if_demo.c

 interfacedemo for "pyramid" LCD device.
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <netdb.h>
#include <errno.h>
#include <wait.h>
#include <net/if.h>
#include <sys/socket.h>

#include "lib-myconnect.h"
#include "lib-mylcd.h"

#include "interface.h"
#include "sockets.h"
#include "net-support.h"

#define DEBUG 0
#define SUCC_DUR 10 /* Duration in sec. how long a success-message is displayed */

/* External programs to be called. */
#define INTERFACE_SCRIPT "./scripts/MyIPSet"
#define REBOOT_SCRIPT "./scripts/MyReboot"
#define HALT_SCRIPT "./scripts/MyHalt"

/* Modes for action */
#define BLANK 0
#define UPDATE 1
#define IFSELECTED 2
#define IPCHANGED 3
#define NMCHANGED 4
#define GWCHANGED 5
#define MENULEFT 6
#define SELECT 7

struct iftype
{
    char name[32];
    char ip[16];
    char netmask[16];
    char gateway[16];
};

struct
{
    unsigned int sIF;
    unsigned int IFnum;
    struct iftype IFdata[32];
} status;

int skfd;

void print_help()
{
    printf("Usage:\n");
    printf("  [-p #] specify Portnumber (defaults to 13666).\n");
    printf("  -d ... specify comma separated list of devices to manage.\n");
    printf("\n");
    printf("Example: if_demo -d eth0,wlan0 -p 12345\n\n");
}


void init_screen(int SD)
{
    char *buffer;

    buffer=basic_send(SD, "hello");
    buffer=basic_send(SD, "client_set -name {Managment}");
    buffer=basic_send(SD, "screen_add main");
    buffer=basic_send(SD, "screen_set main -name {main}");
    buffer=basic_send(SD, "screen_set main -heartbeat off");
    buffer=basic_send(SD, "screen_set main -priority foreground");
    /* buffer=basic_send(SD, "screen_set main -duration 80"); */
    /* buffer=basic_send(SD, "screen_set main -cursor on"); */
}

void alert_screen(int SD, char *line1, char *line2, int duration)
{
    char *buffer;
    char mesg[255];

    /* buffer=basic_send(SD, "widget_del main mainscroller"); */
    buffer=basic_send(SD, "screen_add info");
    buffer=basic_send(SD, "screen_set info -priority alert");
            sprintf(mesg, "screen_set info -timeout %d", duration);
    buffer=basic_send(SD, mesg);
    buffer=basic_send(SD, "widget_add info infostring1 string");
    buffer=basic_send(SD, "widget_add info infostring2 string");
            sprintf(mesg, "widget_set info infostring1 1 1 {%s}", line1);
    buffer=basic_send(SD, mesg);
            sprintf(mesg, "widget_set info infostring2 1 2 {%s}", line2);
    buffer=basic_send(SD, mesg);
}


void interfaces(int SD, char *cs_iflist)
{
    char *buffer;
    char *retval;
    char *next=cs_iflist;
    char mesg[255];
    unsigned int orig_len=strlen(cs_iflist);
    unsigned int i=0;

    /* Now we fill the interfaces to the status variable. */
    while (1)
    {
        retval=strtok(next, ",");
        next=NULL;
        if (retval==NULL)
            break; /* or return? */
        strcpy(status.IFdata[i].name, retval);
        /* If interface is up, then get it's numbers and use them, else set to default-Values */
        /* Default values: */
        sprintf(status.IFdata[i].ip, "000.000.000.000");
        sprintf(status.IFdata[i].netmask, "255.255.255.255");
        sprintf(status.IFdata[i].gateway, "000.000.000.000");
        if (DEBUG)
            printf("IF %s: IP=%s, NM=%s, GW=%s\n", status.IFdata[i].name, status.IFdata[i].ip, status.IFdata[i].netmask, status.IFdata[i].gateway);
        sprintf(mesg, "menu_add_item SIF %d action {sel. %s} -next CHSET", i, status.IFdata[i].name);
        buffer=basic_send(SD, mesg);
        i++;
    }
    status.IFnum=i+1;
    /* now prepare cs_iflist so that we can generate a menu entry. */
    for (i=0; i<orig_len; i++)
        if (cs_iflist[i]=='\0') cs_iflist[i]='\t';
    /* sprintf(mesg, "menu_add_item SIF selection ring {IF:} -strings {%s}", cs_iflist); */ /* For use of a selection ring */
    /* buffer=basic_send(SD, mesg); */
    status.sIF=0;
}

void old_autointerfaces(int SD)
{
    struct if_nameindex *if_list;
    char buffer[1000]="";
    int i;

    if_list=if_nameindex();
    for (i=0; if_list[i].if_name!=NULL; i++)
    {
        if (strcmp(if_list[i].if_name, "lo")==0)
            continue;
        if (strlen(buffer)==0)
            sprintf(buffer, "%s", if_list[i].if_name);
        else
            sprintf(buffer, "%s,%s", buffer, if_list[i].if_name);
    }
    interfaces(SD, buffer);
}

int do_if_action(int SD, struct interface *ife, void *cookie)
{
    int *opt_a = (int *) cookie;
    int res;
    struct aftype *ap;
    static unsigned int i=0;
    char mesg[255];
    char *buffer;

    res = do_if_fetch(ife);
    if (strcmp(ife->name, "lo")==0)
        return 0;
    if (res >= 0) {
        if ((ife->flags & IFF_UP) || *opt_a)
        {
            ap = get_afntype(ife->addr.sa_family);
            strcpy(status.IFdata[i].name, ife->name);
            if (ife->has_ip)
            {
                sprintf(status.IFdata[i].ip, "%s", ap->sprint(&ife->addr, 1));
                sprintf(status.IFdata[i].netmask, "%s", ap->sprint(&ife->netmask, 1));
                sprintf(status.IFdata[i].gateway, "%s", ap->sprint(&ife->addr, 1));
            } else
            {
                sprintf(status.IFdata[i].ip, "000.000.000.000");
                sprintf(status.IFdata[i].netmask, "255.255.255.255");
                sprintf(status.IFdata[i].gateway, "000.000.000.000");
            }
            sprintf(mesg, "menu_add_item SIF %d action {sel. %s} -next CHSET", i, status.IFdata[i].name);
            buffer=basic_send(SD, mesg);
        }
    }
    i++;
    return res;
}

void autointerfaces(int SD)
{
    int res;
    int opt_a=0;

    /* Create a channel to the NET kernel. */
    if ((skfd = sockets_open(0)) < 0) {
	perror("socket");
	exit(1);
    }

    res=for_all_interfaces(SD, do_if_action, &opt_a);
    close(skfd);
}

void set_main_info(int SD, char *ifname, char *ip, char *netmask, char *gateway)
{
    char *buffer;
    char mesg[255];

    sprintf(mesg, "widget_set main mainscroller 1 1 16 2 v 10 {%*s%*s%*s%*s%*s%*s%*s%*s}",
            16, " Watching IF:   ",
            16, ifname,
            16, " IP-Address:    ",
            16, ip,
            16, " Netmask:       ",
            16, netmask,
            16, " Gateway:       ",
            16, gateway);
    buffer=basic_send(SD, mesg);
}


void CHSET_menus_add(int SD)
{
    char *buffer;
    char mesg[255];

    sprintf(mesg, "menu_add_item CHSET CHIP ip {%s IP} -v6 false -value {%s}",
            status.IFdata[status.sIF].name, status.IFdata[status.sIF].ip);
    buffer=basic_send(SD, mesg);
    sprintf(mesg, "menu_add_item CHSET CHNM ip {%s netmask} -v6 false -value {%s}",
            status.IFdata[status.sIF].name, status.IFdata[status.sIF].netmask);
    buffer=basic_send(SD, mesg);
    sprintf(mesg, "menu_add_item CHSET CHGW ip {%s gateway} -v6 false -value {%s}",
            status.IFdata[status.sIF].name, status.IFdata[status.sIF].gateway);
    buffer=basic_send(SD, mesg);
    buffer=basic_send(SD, "menu_set_item CHSET CHIP -next CHNM");
    buffer=basic_send(SD, "menu_set_item CHSET CHNM -next CHGW");
    set_main_info(SD, status.IFdata[status.sIF].name, status.IFdata[status.sIF].ip, status.IFdata[status.sIF].netmask, status.IFdata[status.sIF].gateway);
}

void CHSET_menus(int SD)
{
    char *buffer;
    char mesg[255];

    sprintf(mesg, "menu_set_item CHSET CHIP -text {%s IP} -v6 false -value {%s}",
            status.IFdata[status.sIF].name, status.IFdata[status.sIF].ip);
    buffer=basic_send(SD, mesg);
    sprintf(mesg, "menu_set_item CHSET CHNM -text {%s netmask} -v6 false -value {%s}",
            status.IFdata[status.sIF].name, status.IFdata[status.sIF].netmask);
    buffer=basic_send(SD, mesg);
    sprintf(mesg, "menu_set_item CHSET CHGW -text {%s gateway} -v6 false -value {%s}",
            status.IFdata[status.sIF].name, status.IFdata[status.sIF].gateway);
    buffer=basic_send(SD, mesg);
    set_main_info(SD, status.IFdata[status.sIF].name, status.IFdata[status.sIF].ip, status.IFdata[status.sIF].netmask, status.IFdata[status.sIF].gateway);
}


void action(int SD, char *input)
{
    char *next=input;
    char *retstr;
    int len=0;
    int mode=0;
    int myid;

    if(DEBUG)
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
        switch (mode)
        {
        case BLANK:
            {
                if (strcmp(retstr, "update")==0) mode=UPDATE;
                if (strcmp(retstr, "leave")==0) mode=MENULEFT;
                if (strcmp(retstr, "select")==0) mode=SELECT;
                break;
            }
        case UPDATE:
            {
                /* if (strcmp(retstr, "sIF")==0) mode=IFSELECTED; */
                if (strcmp(retstr, "CHIP")==0) mode=IPCHANGED;
                if (strcmp(retstr, "CHNM")==0) mode=NMCHANGED;
                if (strcmp(retstr, "CHGW")==0) mode=GWCHANGED;
                break;
            }
        case IFSELECTED:
            {
                status.sIF=atoi(retstr);
                CHSET_menus(SD);
                mode=BLANK;
                break;
            }
        case IPCHANGED:
            {
                strcpy(status.IFdata[status.sIF].ip, retstr);
                mode=BLANK;
                break;
            }
        case NMCHANGED:
            {
                strcpy(status.IFdata[status.sIF].netmask, retstr);
                mode=BLANK;
                break;
            }
        case GWCHANGED:
            {
                strcpy(status.IFdata[status.sIF].gateway, retstr);
                mode=BLANK;
                break;
            }
        case MENULEFT:
            {
                CHSET_menus(SD);
                mode=BLANK;
                break;
            }
        case SELECT:
            /* Now it gets a bit difficult: If we want to call the
             external script, we have to do the fork twice of
             wait for the child process (which we don't want to).
             The approach ist repeated for every call to external
             programm. */
            {
                if (strcmp(retstr, "APPLY")==0)
                {
                    char mesg[17];
                    myid=fork();
                    switch (myid)
                    {
                        int mynewid;
                    case 0:
                        mynewid=fork();
                        switch (mynewid)
                        {
                        case 0: /* Child of the child */
                            close(SD);
                            execl(INTERFACE_SCRIPT,
                                  INTERFACE_SCRIPT,
                                  status.IFdata[status.sIF].name,
                                  status.IFdata[status.sIF].ip,
                                  status.IFdata[status.sIF].netmask,
                                  status.IFdata[status.sIF].gateway,
                                  NULL);
                            break;
                        case -1:
                            sprintf(mesg, "Error %d", errno);
                            alert_screen(SD, mesg, "during apply.", SUCC_DUR);
                            exit(0);
                            break;
                        default: /* At this point, the child exits and the grandchild's parent is set to "1". */
                            exit(0);
                            break;
                        }
                        break;
                    case -1:
                        sprintf(mesg, "Error %d", errno);
                        alert_screen(SD, mesg, "during apply.", SUCC_DUR);
                        break;
                    default:
                        sprintf(mesg, "for IF %s", status.IFdata[status.sIF].name);
                        alert_screen(SD, "Settings applied", mesg, 160);
                        waitpid(myid, NULL, 0); /* the child's exitstate is read, but we don't need it. */
                        break;
                    }
                } else if (strcmp(retstr, "HALT")==0)
                {
                    char mesg[17];
                    myid=fork();
                    switch (myid)
                    {
                        int mynewid;
                    case 0:
                        mynewid=fork();
                        switch(mynewid)
                        {
                        case 0:
                            close(SD);
                            execl(HALT_SCRIPT,
                                  HALT_SCRIPT,
                                  NULL);
                            break;
                        case -1:
                            sprintf(mesg, "Error %d", errno);
                            alert_screen(SD, mesg, "during halt.", SUCC_DUR);
                            exit(0);
                            break;
                        default:
                            exit(0);
                            break;
                        }
                        break;
                    case -1:
                        sprintf(mesg, "Error %d", errno);
                        alert_screen(SD, mesg, "during halt.", SUCC_DUR);
                        break;
                    default:
                        alert_screen(SD, "Going down for", "halt now.", 160);
                        waitpid(myid, NULL, 0);
                        break;
                    }
                } else if (strcmp(retstr, "REBOOT")==0)
                {
                    char mesg[17];
                    myid=fork();
                    switch (myid)
                    {
                        int mynewid;
                    case 0:
                        mynewid=fork();
                        switch (mynewid)
                        {
                        case 0:
                            close(SD);
                            execl(REBOOT_SCRIPT,
                                  REBOOT_SCRIPT,
                                  NULL);
                            break;
                        case -1:
                            sprintf(mesg, "Error %d", errno);
                            alert_screen(SD, mesg, "during reboot.", SUCC_DUR);
                            exit(0);
                            break;
                        default:
                            exit(0);
                            break;
                        }
                        break;
                    case -1:
                        sprintf(mesg, "Error %d", errno);
                        alert_screen(SD, mesg, "during reboot.", SUCC_DUR);
                        break;
                    default:
                        alert_screen(SD, "Going down for", "reboot now.", 160);
                        waitpid(myid, NULL, 0);
                        break;
                    }
                } else /* Assume the Selection is a number and refers to the Index of an interface */
                {
                    status.sIF=atoi(retstr);
                    CHSET_menus(SD);
                }
                mode=BLANK;
                break;
            }
        }
    }
    return;
}


int main(int argc, char *argv[])
{
    int SD;
    int conn_port=13666;
    int i;
    int j;
    char *buffer;
    int starttime;
    int nowtime;
    int devpos=-1;
    int manualif=0;

    for(i = 1, j = 0; i < argc; i++)
    {
        if(argv[i][0] == '-')
        {
            switch(argv[i][1])
            {
            /* Port number specified */
            case 'p':
                {
                    if(argc < i + 1)
                            print_help();
                    conn_port = atoi(argv[++i]);
                    if(conn_port < 1 && conn_port > 0xffff)
                            fprintf(stderr, "Warning:  Port %d outside of standard range\n", conn_port);
                    break;
                }
            /* List of devices */
            case 'd':
                {
                    devpos=i+1;
                    manualif=1;
                    break;
                }
            /* otherwise...  Get help!*/
            default:
                {
                    print_help();
                    exit(0);
                    break;
                }
            }
        }
    }

    if (manualif && devpos<1)
    {
        printf("No interfaces specified, exiting (use -h for usage information).\n");
        close(SD);
        exit(1);
    }

    SD=get_connection("localhost", conn_port);
    if (SD<0)
        exit(0);

    init_screen(SD);

    buffer=basic_send(SD, "menu_add_item {} SIF menu {Current IF}");
    buffer=basic_send(SD, "menu_add_item {} CHSET menu {IF setting}");
    if (manualif)
        interfaces(SD, argv[devpos]);
    else
        autointerfaces(SD);

    /* buffer=basic_send(SD, "menu_set_item {} SIF -next CHSET"); */ /* Doesn't work like that */

    buffer=basic_send(SD, "menu_set_main {}");
    /* buffer=basic_send(SD, "widget_add main maintitle title");
    buffer=basic_send(SD, "widget_set main maintitle {IF-Status}"); */
    buffer=basic_send(SD, "widget_add main mainscroller scroller");
    buffer=basic_send(SD, "widget_set main mainscroller 1 1 16 2 v 4 {Please stand by. }");

    CHSET_menus_add(SD); /* Remember to set the status.sIF to a valid value before call to CHSET_menus*() (e.g. done by interfaces()). */
    buffer=basic_send(SD, "menu_add_item CHSET APPLY action {Apply settings} -next _quit_");
    buffer=basic_send(SD, "menu_set_item CHSET CHGW -next APPLY");

    buffer=basic_send(SD, "menu_add_item {} ASKREBOOT menu {Confirm reboot?} -is_hidden true");
    buffer=basic_send(SD, "menu_add_item ASKREBOOT REBOOT action {      [Yes][No]} -next _quit_");

    buffer=basic_send(SD, "menu_add_item {} ASKHALT menu {Really halt?} -is_hidden true");
    buffer=basic_send(SD, "menu_add_item ASKHALT HALT action {      [Yes][No]} -next _quit_");

    buffer=basic_send(SD, "menu_add_item {} SHUTDOWN menu {Shutdown}");
    buffer=basic_send(SD, "menu_add_item SHUTDOWN DOHALT action {Halt system} -next ASKHALT");
    buffer=basic_send(SD, "menu_add_item SHUTDOWN DOREBOOT action {Reboot system} -next ASKREBOOT");

    starttime=time(NULL);
    while (1)
    {
        nowtime=time(NULL);
        /* sprintf(cmd, "widget_set main c0 1 2 \"%ld\"\n", nowtime-starttime); */
        /* if nothing else to do, send noop, so we know for sure, that basic_send gets something to read */
        /* buffer=basic_send(SD, "noop\n"); */
        buffer=basic_read(SD);
        action(SD, buffer);
        usleep(250000);
    } /*  while ((nowtime-starttime)<500); */

    close(SD);

    return 0;
}

