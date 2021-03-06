
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "shared.h"
#include "page_operations.h"
#include "process_operations.h"
#include "FIFO.h"

/*
 * When memory is full FIFO algorithm assigns the spot of the oldest page to the 
 * new page that is waiting to be inserted into the free list.
 */

void runFIFO(process** prolist, page** pagelist) {

	int currentQuanta = 0;
	int hitCount = 0;
	int missCount = 0;
	int numOfProcessesDone = 0;
	process* process_head = *prolist;
	page* page_head = *pagelist;

	while (currentQuanta < 600) { // 600 quanta = 1 minute (max)

		for (int i = 0; i < NUMBER_PROCESS; i++) {

			if (process_head->completion_time > 0 && process_head->arrival_time <= currentQuanta) { // Only run that process if its completition if > 0

				printf("\nQUANTA: %d\n", currentQuanta);
				//printf("COMPLETION TIME: %d \n", process_head->completion_time);

				if (process_head->pagesowned[0].process_owner != NULL) { // Not first referencing made
					process_head->last_reference = getPageReference(process_head->page_size, process_head->last_reference); // The next page to reference
				}

				if (!isPageAlreadyInMemory(process_head, process_head->last_reference)) {

					// Page in not yet in memory. Add to free list
					printf("PAGE REFERENCED # %d\n", process_head->last_reference);
					printf("\n----INSERT - PAGE# %d OF PROCESS %c%c----\n", process_head->last_reference, process_head->name[0], process_head->name[1]);

					if(find4FreePages(*pagelist)) {
						// Found 4 free pages. Can insert into free list.

						if (!isMemoryFull(*pagelist)) {
							addPageToMemory(pagelist, process_head, currentQuanta, process_head->last_reference);
							if (page_head != NULL) page_head = page_head->next;
							print_pages(*pagelist);
						} else {	
							// Memory is full. Do swap with oldest page.
							swapWithOldestPageFIFO(pagelist, process_head, currentQuanta, process_head->last_reference);
							print_pages(*pagelist);
						}
						missCount++;
					} 
				} else {
					// Page is already in memory. a hit
					hitCount++;
				}
				process_head->completion_time -= 1;

				if (process_head->completion_time <= 0) {
					//printf("\nCOMPLETION TIME %d\n", process_head->completion_time);
					printf("\nPROCESS %c%c DONE. REMOVING PAGES\n", process_head->name[0], process_head->name[1]);
					// Process if finished, removing its free list from free memory
					removePageFromFreeList(pagelist, process_head->name[0], process_head->name[1]);
					numOfProcessesDone++;
				}
			}
			process_head = process_head->next;
			if (process_head == NULL) process_head = *prolist; // wrap around to head process again..
		}
		currentQuanta += 1; // Increment 1 quanta, which is
	}

	printStats(hitCount, missCount, numOfProcessesDone);
}


void swapWithOldestPageFIFO(page** pagelist, process* p1, int inMemoryTime, int pageNumber) {

	page* head = *pagelist;
	page* oldestPage = getOldestPage(*pagelist);
	page insert; // The page to insert into the page slot
	insert.status=1;
	insert.inMemoryTime = inMemoryTime;
	insert.process_owner = p1;
	insert.pageNumber = pageNumber;

	for (int i = 0; i < NUMBER_PAGES; i++) {
		if (head->process_owner->name[0] == oldestPage->process_owner->name[0] &&
			head->process_owner->name[1] == oldestPage->process_owner->name[1] &&
			head->inMemoryTime == oldestPage->inMemoryTime) {

			removePageFromAProcessArray(head->process_owner, oldestPage); // remove that page from that process's free list first
			(*head).status = 1;
			(*head).inMemoryTime = inMemoryTime;
			(*head).process_owner = p1;
			(*head).pageNumber = pageNumber;
			break;
		}
		head = head->next;
	}

	// Add to the process's free memory array list
	for(int x = 0; x < p1->page_size; x++) {
		if(p1->pagesowned[x].status != 1){
			p1->pagesowned[x] = insert;
			p1->num_page_in_freelist++;
			break;
		}
	}
}


page* getOldestPage(page* pagelist) {

	page* head = pagelist;

	page* oldestPage = malloc(sizeof(page));
	oldestPage = head; //set first page as oldest by default

	for (int i = 0; i < NUMBER_PAGES; i++) {
		if (head->inMemoryTime < oldestPage->inMemoryTime) {
			oldestPage = head;
			break;
		}
		head = head->next;
	}
	return oldestPage;
}


bool isMemoryFull(page* llist) {
	int pagesFound = 0;
	page* head = llist;
	bool isMemFull = false;

	for (int x = 0; x < NUMBER_PAGES; x++) {
	    if(head->status == 1) {
	        pagesFound++; // Status 1 means page is taken so increment pagesFound
	    }
	    head = head->next;
	}
	
	if (pagesFound >= NUMBER_PAGES){ 
		isMemFull = true;
	} 
	return isMemFull;
}


bool isPageAlreadyInMemory(process* p1, int pageNumber) {

	bool isInMemory = false;

	for(int x = 0; x < p1->page_size; x++){ // Loop through that process's free array list
		if(p1->pagesowned[x].pageNumber == pageNumber) {
			isInMemory = true; 
			break;
		}
	}
	return isInMemory;
}


void removePageFromAProcessArray(process* p1, page* oldestPage) {

	for(int x = 0; x < p1->page_size; x++) {
		if(oldestPage->pageNumber == p1->pagesowned[x].pageNumber) {

			printf("\n****EVICTING - PAGE# %d OF PROCESS %c%c****\n\n", p1->pagesowned[x].pageNumber, p1->name[0], p1->name[1]);
			p1->pagesowned[x].status = 0; // Set the process's free page list of that page to free
			p1->pagesowned[x].pageNumber = -1; // Set back to free default value
			p1->num_page_in_freelist--;
			break;
		}
	} 
}


void removePageFromFreeList(page** pagelist, char nameOne, char nameTwo) {

	page* head = *pagelist;
	bool isRemovalExists = false;

	for (int x = 0; x < NUMBER_PAGES; x++) {
		if (head->process_owner == NULL) break;
		// Setting the page to be available for other pages to take its spot
		if (head->process_owner->name[0] == nameOne && head->process_owner->name[1] == nameTwo) {
			head->pageNumber = 0;
			head->status = 0;
			isRemovalExists = true; // For process name removal
		} 
		head = head->next;			
	}

	head = *pagelist;
	if (isRemovalExists) {
		// Change process name to '.' for all pages referenced to it
		for (int x = 0; x < NUMBER_PAGES; x++) {
			if (head->process_owner == NULL) break;
			if (head->process_owner->name[0] == nameOne && head->process_owner->name[1] == nameTwo) head->process_owner->name[0] = '.';
			head = head->next;			
		}
	}
	printf("AFTER REMOVAL\n");
	print_pages(*pagelist);
}

bool find4FreePages(page* llist) {
	return true;
}

void print_pages(page* llist) {
	
	page* head = llist;
	for(int x = 0; x < NUMBER_PAGES; x++){
		if(head == NULL) break;

		if (head->process_owner == NULL || head->process_owner->name[0] == '.') {
			printf("[(#: .),"); // hole
		} else {						
			printf("[(#: %d),", head->pageNumber);
		}
		if (head->status == 1) {
			printf("(P: %c%c)] ", head->process_owner->name[0], head->process_owner->name[1]);
		} else {
			printf("(P:.)] "); // hole
		}
		head = head->next;
	}
	printf("\n\n");
}


void printStats(int hitCount, int missCount, int numOfProcessesDone) {
	printf("\n\n****************************\n");
	printf("         HIT: %d         \n", hitCount);
	printf("         MISS: %d        \n", missCount);
	printf("      HIT RATIO: %.2f     \n", (float) hitCount / (hitCount + missCount));
	printf("      MISS RATIO: %.2f     \n", ((float) missCount / (hitCount + missCount)));
	printf("****************************\n\n");
	printf("NUMBER OF PROCESSES SUCCESSFULLY SWAPPED: %d\n\n", numOfProcessesDone);

}