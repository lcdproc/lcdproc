#ifndef CLIENT_DATA_H
#define CLIENT_DATA_H

#include "../shared/LL.h"

#define CLIENT_NAME_SIZE 256

typedef struct client_data {
   int ack;
   char *name;
   // and other stuff...  doesn't matter yet
   LL *screenlist;
   // list of accepted keys... (?)
   LL *menulist;

} client_data;

// sets up an existing (empty) client_data struct
int client_data_init (client_data * d);
// destroys members of a client's data
int client_data_destroy (client_data * d);

#endif
