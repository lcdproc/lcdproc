#include <stdlib.h>
#include <stdio.h>
#include "LL.h"



/* Tested functions:
     LL_New();
     LL_Rewind();
     LL_AddNode();
     LL_Next();
     LL_Get();
     LL_Push();
     LL_Unshift();
     LL_InsertNode();
     LL_Roll();
     LL_UnRoll();
     LL_SwapNodes();
     LL_Sort();
     LL_Find();
 */

typedef struct my_struct 
{
  int num;
  char str[16];
} my_struct;


int list_list(LL *list)
{
  my_struct *foo;
  

  if(!list) return -1;
  
  printf("Listing the list contents...\n");
  printf("    Items: %i\n", LL_Length(list));
  
  
  LL_Rewind(list);
  
  do{
    foo = (my_struct *)LL_Get(list);
    if(!foo)
    {
      printf("Can't read list data\n");
      return -1;
    }
    printf("\tItem: %i\n", foo->num);
    
  } while(LL_Next(list) == 0);
  
  
  return 0;
}

int compare(void *one, void *two)
{
  my_struct *a, *b;

  if(!one  ||  !two) return 0;

  
  a = (my_struct *)one;
  b = (my_struct *)two;

  return (a->num - b->num);
  
}



int main()
{
  LL *list = NULL;
  my_struct *foo;
  my_struct bar;
  int i,j;
  

  list = LL_new();
  if(!list)
  {
    printf("Cannot create list.\n");
    return -1;
  }

  for(i=0; i<10; i++)
  {
    foo = malloc(sizeof(my_struct));
    if(!foo)
    {
      printf("Can't allocate new struct\n");
      return -1;
    }

    foo->num = i;
    
    LL_InsertNode(list, (void *)foo);
    printf("Added node: %i\n", foo->num);
    
  }

  if(list_list(list) != 0) return -1;
  
  printf("Sorting the list...\n");

  if(LL_Sort(list, compare) < 0)
  {
    printf("Error sorting.\n");
    return -1;
  }

  if(list_list(list) != 0) return -1;

  bar.num = 5;

  printf("Searching for item \"5\"\n");
  LL_Rewind(list);
  
  foo = LL_Find(list, compare, &bar);
  if(foo)
  {
    printf("    Found...  %i\n", foo->num);
  }
  

  printf("Deallocating list...\n");
  LL_Rewind(list);
  do{
    foo = LL_Get(list);
    free(foo);
    
  } while(LL_Next(list) == 0);

  if(LL_Destroy(list) == 0)  return 0;
  

  printf("Attempting to access list...\n");

  printf("List = %x\n", list);
  
  LL_Rewind(list);
  
  do{
    foo = LL_Get(list);
    if(foo) printf("    Item: %i\n", foo->num);
    
  } while(LL_Next(list) == 0);
  
  
  
  return 0;
  
}
