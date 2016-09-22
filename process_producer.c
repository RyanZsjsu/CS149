#include <stdio.h>
#include <stdlib.h>
#include "process_producer.h"

void PrintProcessList(struct process process_list[]){
	for(int x = 0; x < NUM_PROCESS; x++){
		printf("%c: Service Time: %.1f Priority: %d Arrival Time: %d\n", process_list[x].name, process_list[x].service_time, process_list[x].priority, process_list[x].arrival_time);
	}
}

void CreateProcesses(struct process** process_list){
	char alphabet[26] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	int arrival_time;
	float service_time;
	int priority;
	*process_list = calloc(NUM_PROCESS, sizeof(struct process));
	 //generate proesses
	for(int x = 0; x < NUM_PROCESS; x++){
		arrival_time = rand() % MAX_QUANTA;
		service_time = rand() % 99;
		priority = rand() % 5;
		if( service_time == 0 ) service_time += 1;
		service_time = service_time / 10.0;
		if( priority == 0 ) priority += 1;

		(*process_list)[x].arrival_time = arrival_time;
		(*process_list)[x].service_time = service_time;
		(*process_list)[x].priority = priority;
		(*process_list)[x].name = alphabet[x];
	}

}
