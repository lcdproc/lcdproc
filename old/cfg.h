/***********************************************
**
** cfg.h v0.01
**
** Generic config file handler
** (c) Gareth Watts 1998
*/


#define CFG_READONLY 1
#define CFG_READWRITE 2


struct cfg_node {
	char *optname;
	char *optvalue;
	int dirty;
	struct cfg_node *next;
};


struct cfg_queuenode {
	struct cfg_node *ref;
	struct cfg_queuenode *next;
};


#define CFG_HASHSIZE 1297
typedef struct cfg {

	// PUBLIC METHODS
	void (*free)(struct cfg *self);
	int (*openfile)(struct cfg *self,char *fn,int mode);
	int (*autoflush)(struct cfg *self,int flushmode);
	char *(*getstring)(struct cfg *self,char *key);
	int (*getint)(struct cfg *self,char *key);
	float (*getfloat)(struct cfg *self,char *key);
	int (*setstring)(struct cfg *self,char *key, char *value);
	int (*setint)(struct cfg *self,char *key, int value);
	int (*setfloat)(struct cfg *self,char *key, float value);
	int (*flush)(struct cfg *self);

	// PUBLIC DATA
	char *lasterror;


	// PRIVATE METHODS
	int (*_init)(struct cfg *self);
	int (*_getkey)(struct cfg *self,char *str);
	int (*_insert)(struct cfg *self,char *keyname,char *keyval,int dirty);
	struct cfg_node *(*_getnode)(struct cfg *self,char *keyname);
	char *(*_lookup)(struct cfg *self,char *keyname);
	int (*_cleardirty)(struct cfg *self,struct cfg_node *ptr);
	int (*_splitline)(struct cfg *self,char *src, char **f, char **s);
	int (*_set)(struct cfg *self, char *keyname, char *keyvalue);
	int (*_queuepush)(struct cfg *self, struct cfg_node *node);
	struct cfg_queuenode *(*_queuenext)(struct cfg *self, struct cfg_queuenode *last);
	int (*_queuefree)(struct cfg *self);
	int (*_flush)(struct cfg *self);

	// PRIVATE DATA
	struct cfg_node *cfg_hash[CFG_HASHSIZE];
	struct cfg_queuenode *cfg_newsthead;
	struct cfg_queuenode *cfg_newsttail;
	char *filename;
	int filemode;
	int aflush;
} cfg;

cfg *cfg_new();
