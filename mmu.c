#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "list.h"
#include "util.h"

void TOUPPER(char * arr)
{
  
    for(int i=0;i<strlen(arr);i++)
    {
        arr[i] = toupper(arr[i]);
    }
}
void get_input(char *args[], int input[][2], int *n, int *size, int *policy) 
{
  	FILE *input_file = fopen(args[1], "r");
	  if (!input_file) 
    {
		    fprintf(stderr, "Error: Invalid filepath\n");
		    fflush(stdout);
		    exit(0);
	  }
    parse_file(input_file, input, n, size);  
    fclose(input_file);  
    TOUPPER(args[2]);  
    if((strcmp(args[2],"-F") == 0) || (strcmp(args[2],"-FIFO") == 0))
        *policy = 1;
    else if((strcmp(args[2],"-B") == 0) || (strcmp(args[2],"-BESTFIT") == 0))
        *policy = 2;
    else if((strcmp(args[2],"-W") == 0) || (strcmp(args[2],"-WORSTFIT") == 0))
        *policy = 3;
    else 
    {
       printf("usage: ./mmu <input file> -{F | B | W }  \n(F=FIFO | B=BESTFIT | W-WORSTFIT)\n");
       exit(1);
    }        
}
void allocate_memory(list_t * freelist, list_t * alloclist, int pid, int blocksize, int policy) 
{
    node_t *block_node = NULL; 
    node_t *current = freelist->head;
    node_t *previous = NULL; 
    while(current)
    {
      if (current->blk->end - current->blk->start >= blocksize)
      {
        block_node = current; 
        break;
      } 
      previous = current;
      current = current->next;
    }
    if (!block_node)
    {
      printf("Error: Memory Allocation %d blocks\n", blocksize);
      return;
    }
    else 
    { 
      if (!previous)
      {
        freelist->head = freelist->head->next;   
      } else
      {
        previous->next = block_node->next;
      }
    }  
    int blk_end = block_node->blk->end; 
    block_node->blk->pid = pid;
    block_node->blk->end = block_node->blk->start + blocksize - 1;
    list_add_ascending_by_address(alloclist, block_node->blk);
    if (block_node->blk->end != blk_end)
    {
      block_t *fragment = (block_t*)malloc(sizeof(block_t));
      fragment->pid = 0;
      fragment->start = block_node->blk->end + 1;
      fragment->end = blk_end;
      if (policy == 1)
      {
        list_add_to_back(freelist, fragment);
      }
      else if (policy == 2)
      {
        list_add_ascending_by_blocksize(freelist, fragment);
      }
      else if (policy == 3)
      {
        list_add_descending_by_blocksize(freelist, fragment);
      }
    }
}
void deallocate_memory(list_t * alloclist, list_t * freelist, int pid, int policy) 
{ 
    node_t *current = alloclist->head;
    node_t *previous = NULL;
    block_t *deallocate_blk = NULL;
    while (current && current->blk->pid != pid) 
    {
      previous = current;
      current = current->next;
    }
    if (!current)
    {
      printf("Error: Can't locate Memory Used by PID: %d\n", pid);
      return;
    }
    if (!previous)
    {
      alloclist->head = alloclist->head->next;
      deallocate_blk = current->blk;
    }
    else
    {
      previous->next = current->next;
      deallocate_blk = current->blk;
    }
    deallocate_blk->pid = 0;
    if (policy == 1)
    {
      list_add_to_back(freelist, deallocate_blk);
    }
    else if (policy == 2)
    {
      list_add_ascending_by_blocksize(freelist, deallocate_blk);
    }
    else if (policy == 3)
    {
      list_add_descending_by_blocksize(freelist, deallocate_blk);
    }
}
list_t* coalese_memory(list_t * list)
{
  list_t *temp_list = list_alloc();
  block_t *blk;
  
  while((blk = list_remove_from_front(list)) != NULL) 
  {  
        list_add_ascending_by_address(temp_list, blk);
  }
  list_coalese_nodes(temp_list);
  return temp_list;
}
void print_list(list_t * list, char * message)
{
    node_t *current = list->head;
    block_t *blk;
    int i = 0;  
    printf("%s:\n", message);
    while(current != NULL)
    {
        blk = current->blk;
        printf("Block %d:\t START: %d\t END: %d", i, blk->start, blk->end);      
        if(blk->pid != 0)
            printf("\t PID: %d\n", blk->pid);
        else  
            printf("\n");      
        current = current->next;
        i += 1;
    }
}
int main(int argc, char *argv[]) 
{
   int PARTITION_SIZE, input_data[200][2], N = 0, Memory_Mgt_Policy;
  
   list_t *FREE_LIST = list_alloc();   
   list_t *ALLOC_LIST = list_alloc();  
   int i;
  
   if(argc != 3) 
   {
       printf("usage: ./mmu <input file> -{F | B | W }  \n(F=FIFO | B=BESTFIT | W-WORSTFIT)\n");
       exit(1);
   }  
   get_input(argv, input_data, &N, &PARTITION_SIZE, &Memory_Mgt_Policy);
   block_t * partition = malloc(sizeof(block_t));  
   partition->start = 0;
   partition->end = PARTITION_SIZE + partition->start - 1;                                   
   list_add_to_front(FREE_LIST, partition);                                            
   for(i = 0; i < N; i++)
   {
       printf("************************\n");
       if(input_data[i][0] != -99999 && input_data[i][0] > 0) 
       {
             printf("ALLOCATE: %d FROM PID: %d\n", input_data[i][1], input_data[i][0]);
             allocate_memory(FREE_LIST, ALLOC_LIST, input_data[i][0], input_data[i][1], Memory_Mgt_Policy);
       }
       else if (input_data[i][0] != -99999 && input_data[i][0] < 0) 
       {
             printf("DEALLOCATE MEM: PID %d\n", abs(input_data[i][0]));
             deallocate_memory(ALLOC_LIST, FREE_LIST, abs(input_data[i][0]), Memory_Mgt_Policy);
       }
       else 
       {
             printf("COALESCE/COMPACT\n");
             FREE_LIST = coalese_memory(FREE_LIST);
       }        
       printf("************************\n");
       print_list(FREE_LIST, "Free Memory");
       print_list(ALLOC_LIST,"\nAllocated Memory");
       printf("\n\n");
   }  
   list_free(FREE_LIST);
   list_free(ALLOC_LIST);  
   return 0;
}