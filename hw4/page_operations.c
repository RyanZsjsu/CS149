#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include "shared.h"



void generate_pageList(page** list)
{
	page* b_list;
	page* head;
	b_list = malloc(sizeof(page));
	b_list->next = malloc(sizeof(page));
	b_list->status = 0;
	head = b_list->next;

	for(int x = 1; x < NUMBER_PAGES; x++){ //create/initialize page list with 100 avaliable pages.
		
		head->next = malloc(sizeof(page));
		head = head->next;
	}
	head->next = NULL;
	//free(head); needed?
	*list = b_list;
	return;
}

bool find_4FreePages(page* llist)
{
	int pagesFound = 0;
	while(pagesFound < 4)
	{
	for(int x = 1; x < NUMBER_PAGES; x++) //iterate through linked list of pages to find 4 available pages.
	{
          if(head->status = 0)
          {
          	pagesFound++;
          	head->status = 1;

          }
          head = head->next;
	}
}
		if(pagesFound == 4){
			return true;
		}
		
else{
 return false;
}
}
