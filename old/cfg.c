/***********************************************
**
** cfg.c v0.01
**
** Generic config file handler
** (c) Gareth Watts 1998
**     gareth@omnipotent.net
**
** Alpha code as of 980601
*/

#include <stddef.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "cfg.h"



/**************************
*  PUBLIC METHODS
*/

void cfg_free(cfg *self) {
	struct cfg_node *ptr, *nextptr;
	int i;

	for(i=0;i<CFG_HASHSIZE;i++) {
		if(self->cfg_hash[i]) {
			for(ptr=self->cfg_hash[i];ptr;ptr=nextptr) {
				nextptr=ptr->next;
				free(ptr->optname);
				free(ptr->optvalue);
				free(ptr);
			}
		}
	}
	free(self->filename);
	free(self->lasterror);
	self->_queuefree(self);
	free(self);
}


#define CFG_BUZSIZE 4096
int cfg_openfile(cfg *self,char *fn,int mode) {
	char buffer[CFG_BUZSIZE];
	char *keyname,*keyvalue;
	int  comment;
	FILE *fh;

	self->filename=strdup(fn);
	fh=fopen(fn,"r");
	if (!fh) {
		if (mode==CFG_READONLY) {
			self->lasterror=strdup(strerror(errno));
			return(1);
		}
		return(0); /* read/write but file not found */
	}

	// READ AND PARSE FILE HERE
	while(feof(fh)==0) {
		fgets(buffer,CFG_BUZSIZE-1,fh);
		if (strlen(buffer)) {
			if (buffer[strlen(buffer)-1]=='\n') {
				buffer[strlen(buffer)-1]='\0';
			}
			if (buffer[0]=='#') {
				comment=1;
			} else {
				comment=0;
			}
		} else { comment=1;}
		if (!comment&&self->_splitline(self,buffer,&keyname,&keyvalue)) {
			return(1); /* memory error or somesuch */
		}
		if (!comment&&keyname&&keyvalue) {
			if (!self->_lookup(self,keyname)) {
				self->_insert(self,keyname,keyvalue,0);
			}
		}
			
	}
	return(0);
}	


char *cfg_getstring(cfg *self,char *keyname) {
	return(self->_lookup(self,keyname));
}


int cfg_getint(cfg *self,char *keyname) {
	int result=0;
	char *keydata;

	keydata=self->_lookup(self,keyname);
	if (keydata) 
		sscanf(keydata,"%d",&result);
	return(result);
}


/* This is virtually useless due to rounding errors */
float cfg_getfloat(cfg *self,char *keyname) {
	float result=0.0;
	char *keydata;

	keydata=self->_lookup(self,keyname);
	if (keydata) 
		sscanf(keydata,"%f",&result);
	return(result);
}


int cfg_setstring(cfg *self,char *keyname, char *keyvalue) {
	return(self->_set(self,keyname,keyvalue));
}


int cfg_setint(cfg *self,char *keyname, int keyvalue) {
	char buffer[200];

	sprintf(buffer,"%d",keyvalue);
	return(self->_set(self,keyname,buffer));
}

int cfg_setfloat(cfg *self,char *keyname, float keyvalue) {
	char buffer[200];

	sprintf(buffer,"%f",keyvalue);
	return(self->_set(self,keyname,buffer));
}

int cfg_flush(cfg *self) {
	if (self->filemode==CFG_READONLY) {
		return(0);
	} else {
		return(self->_flush(self));
	}
}


int cfg_autoflush(cfg *self,int aflush) {
	self->aflush=(aflush>0);
	return(0);
}



/**************************
*  PRIVATE/INTERNAL METHODS
*/

int _cfg_init(cfg *self) {
	memset(self->cfg_hash,0,sizeof(self->cfg_hash));
	return(0);
}


int _cfg_queuepush(cfg *self, struct cfg_node *node) {
	struct cfg_queuenode *ptr;

	ptr=(struct cfg_queuenode *)calloc(1,sizeof(struct cfg_queuenode));
	if (!ptr) return(1);
	ptr->ref=node;
	if (self->cfg_newsttail) {
		self->cfg_newsttail->next=ptr;
		self->cfg_newsttail=ptr;
	} else {
		self->cfg_newsthead=self->cfg_newsttail=ptr;
	}
	return(0);
}


struct cfg_queuenode *_cfg_queuenext(cfg *self, struct cfg_queuenode *last) {
	if (last) {
		return(last->next);
	} else {
		return(self->cfg_newsthead);
	}
}


int _cfg_queuefree(cfg *self) {
	struct cfg_queuenode *ptr,*next;

	for(ptr=self->cfg_newsthead;ptr;ptr=next) {
		next=ptr->next;
		free(ptr);
	}
	self->cfg_newsthead=self->cfg_newsttail=NULL;
	return(0);
}


int _cfg_getkey(cfg *self,char *str) {
        unsigned int total; 
        int i=1; 
        int j=0; 

        total=(*str)-0x20; 
        str++; 
        while(*str) {
                total+=i*((*str)-0x20); 
                str++; 
                i+=(33+j++)*j; 
        }
        return(total % CFG_HASHSIZE);  
}


int _cfg_insert(cfg *self,char *keyname, char *keyval,int dirty) {
	int key,match=0;
	struct cfg_node *ptr,*nextptr;;

	key=self->_getkey(self,keyname);
	if (!self->cfg_hash[key]) {
		self->cfg_hash[key]=(struct cfg_node *)calloc(1,sizeof(struct cfg_node));
		if (!(self->cfg_hash[key]->optname=strdup(keyname))) {
			return(1);
		}
		if (!(self->cfg_hash[key]->optvalue=strdup(keyval))) {
			return(1);
		}
		if (dirty) self->_queuepush(self,self->cfg_hash[key]);
	} else {
		for(nextptr=self->cfg_hash[key];(nextptr)&&(!match);nextptr=ptr->next) {
			ptr=nextptr;
			match=(strcasecmp(ptr->optname,keyname)==0);
		}

		if (match) { /* node already exists / overwrite */
			free(ptr->optvalue);
			ptr->dirty=dirty;
		} else { /* new node */
			ptr->next=(struct cfg_node *)calloc(1,sizeof(struct cfg_node));
			ptr=ptr->next;
			if (!(ptr->optname=strdup(keyname))) {
				return(1);
			}
			if (dirty) self->_queuepush(self,ptr);
		}
		if (!(ptr->optvalue=strdup(keyval))) {
			return(1);
		}
	}
	return(0);
}


struct cfg_node *_cfg_getnode(cfg *self, char *keyname) {
	int key;
	struct cfg_node *ptr,*next;
	int match=0;

	key=self->_getkey(self,keyname);
	if (!self->cfg_hash[key])
		return(NULL);
	for(next=self->cfg_hash[key];next&&!match;next=ptr->next) {
		ptr=next;
		match=(strcasecmp(ptr->optname,keyname)==0);
	}
	if (match)
		return(ptr);
	return(NULL);
}


char *_cfg_lookup(cfg *self,char *keyname) {
	struct cfg_node *ptr;

	ptr=self->_getnode(self,keyname);
	if (ptr)
		return(ptr->optvalue);
	else
		return(NULL);
}


int _cfg_cleardirty(cfg *self,struct cfg_node *ptr) {
	ptr->dirty=0;
	return(0);
}


int _cfg_set(cfg *self,char *keyname,char *keyvalue) {
	int key;

	key=self->_getkey(self,keyname);
	if (self->_insert(self,keyname,keyvalue,1)!=0) {
		return(1); /* error */
	}
	if (self->aflush) {
		return(self->flush(self));
	}
	return(0);
}

int _cfg_splitline(cfg *self,char *src, char **f, char **s) {
	char *brkpoint;
	int offset,slen,fpos;
	static char first[4096],second[4096];

	*f=*s=NULL;
	if ((brkpoint=strchr(src,'='))==NULL) {
		return(0);
	}
	offset=brkpoint-src;
	slen=strlen(src);
	if (offset==slen-1) {
		return(0);
	}
	if (offset>4094) {offset=4094;}
	
	memset(first,0,4096);
	memset(second,0,4096);
	strncpy(first,src,offset);
	strncpy(second,brkpoint+1,4095);
	
	fpos=strlen(first)-1;
	while((first[fpos]==' ')&&fpos) {
		first[fpos]='\0';
		fpos--;
	}
	*f=first;
	for(*s=second;(**s)&&(**s==' ');(*s)++);
	return(0);
}



#define CFGF_MAXLINES 50000
#define CFGF_LINELEN 4096
int _cfg_realflush(cfg *self) {
	char *outbuf[CFGF_MAXLINES];
	char buffer[CFGF_LINELEN];
	int linenum=0;
	int comment,i;
	char *keyname,*keyvalue;
	struct cfg_queuenode *qptr;
	struct cfg_node *ptr;
	FILE *fh;
	char *rd;

	fh=fopen(self->filename,"r");
	if (fh) {
		while((feof(fh)==0)&&(linenum<CFGF_MAXLINES)) {
			rd=fgets(buffer,CFGF_LINELEN-1,fh);
			if (rd&&strlen(buffer)) {
				if (buffer[strlen(buffer)-1]=='\n') {
					buffer[strlen(buffer)-1]='\0';
				}
				if (buffer[0]=='#') {
					comment=1;
				} else {
					comment=0;
				}
				if (comment) {
					outbuf[linenum]=strdup(buffer);
				} else {
					if (self->_splitline(self,buffer,&keyname,&keyvalue)) {
						fclose(fh);
						return(1); /* memory error or somesuch */
					}
					if ((keyname)&&(ptr=self->_getnode(self,keyname))&&(ptr->dirty)) {
						sprintf(buffer,"%s = %s",ptr->optname,ptr->optvalue);
						outbuf[linenum]=strdup(buffer);
						self->_cleardirty(self,ptr);
					} else {
						outbuf[linenum]=strdup(buffer);
					}
				}
				linenum++;
			}
		}
		fclose(fh);
	}
	for(qptr=self->_queuenext(self,NULL);qptr;qptr=self->_queuenext(self,qptr)) {
		sprintf(buffer,"%s=%s",qptr->ref->optname,qptr->ref->optvalue);
		outbuf[linenum]=strdup(buffer);
		linenum++;
	}
	fh=fopen(self->filename,"w");
	if (!fh) {
		self->lasterror=strdup(strerror(errno));
		return(1);
	}
	for(i=0;i<linenum;i++) {
		fprintf(fh,"%s\n",outbuf[i]);
		free(outbuf[i]);
	}
	fclose(fh);
	self->_queuefree(self);
	return(0);
}






/*****************
*  THE CONSTRUCTOR
*/

cfg *cfg_new() {
	cfg *ptr;

	ptr=(cfg *)calloc(1,sizeof(cfg));
	if (!ptr) return(NULL);

	ptr->getstring=cfg_getstring;
	ptr->getint=cfg_getint;
	ptr->getfloat=cfg_getfloat;
	ptr->openfile=cfg_openfile;
	ptr->setstring=cfg_setstring;
	ptr->setint=cfg_setint;
	ptr->setfloat=cfg_setfloat;
	ptr->flush=cfg_flush;
	ptr->autoflush=cfg_autoflush;
	ptr->free=cfg_free;

	ptr->_getkey=_cfg_getkey;
	ptr->_insert=_cfg_insert;
	ptr->_lookup=_cfg_lookup;
	ptr->_cleardirty=_cfg_cleardirty;
	ptr->_set=_cfg_set;
	ptr->_splitline=_cfg_splitline;
	ptr->_queuepush=_cfg_queuepush;
	ptr->_queuenext=_cfg_queuenext;	
	ptr->_queuefree=_cfg_queuefree;
	ptr->_getnode=_cfg_getnode;
	ptr->_flush=_cfg_realflush;

	if (_cfg_init(ptr)) return(NULL);
	return(ptr);
}

