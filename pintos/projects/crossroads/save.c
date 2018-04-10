#include <stdio.h>
#include <string.h>

#include "threads/init.h"
#include "threads/malloc.h"
#include "threads/synch.h"
#include "threads/thread.h"

#include "devices/timer.h"

#include "projects/crossroads/crossroads.h"
#include "projects/crossroads/mapdata.h"

struct semaphore control[6];
struct semaphore s;
struct semaphore print;
int numberofcar;
int print_turn;
char map_draw[7][7];
int *process_status;

void
run_crossroads(char **argv)
{
	//variables
	char *input;
	char *token, *save_ptr;
	char **car_data;

	int i;

	//assert if input == null.
	ASSERT(argv != NULL);

	//semaphore initialize.
	sema_init(&s, 1);
	sema_init(&print, 1);
	sema_init(%control[0], 1);
	sema_init(%control[1], 1);
	sema_init(%control[2], 1);
	sema_init(%control[3], 1);
	sema_init(%control[4], 1);
	sema_init(%control[5], 1);

	//initialize.
	input = (char *)malloc(sizeof(argv[0]));
	strlcpy(input, argv[0], sizeof(argv[0]));
	numberofcar = (strlen(input) - 1) / 4;
	print_turn = numberofcar;
	process_status = (int *)malloc(sizeof(int)*numberofcar);

	for(i = 0; i < numberofcar; i++)
	{
		car_data[i] = (char *)malloc(4);
	}
	for(i = 0; i < 7; i++)
	{
		for(j = 0; j < 7; j++)
		{
			map_draw[i][j] = map_draw_default[i][j];
		}
	}

	//tokenize argv with token " : " .
	i = 0;
	for (token = strtok_r (s, ":", &save_ptr); token != NULL; token = strtok_r (NULL, " ", &save_ptr))
	{
		strlcpy(car_data[i], token, sizeof(token));
		i++;
	}

	//make number of car threads
	for(i = 0; i < numberofcar; i++)
	{
		thread_create(car_data[i], PRI_DEFAULT, move_car, NULL);
	}

}

void
print_mapdata(void)
{
	int i, j;
	for(i = 0; i < 7; i++)
	{
			for(j = 0; j < 7; j++)
			{
				printf("%c", map_draw[i][j]);
			}
			printf("\n");
	}
}
void
set_move(int source_num, int dest_num, int status, char car_name) {
	struct position move_position;
	struct position prev_position;
	move_position = path[source_num][dest_num][status];
	if(move_position != NULL)
	{
		map_draw[move_position.row][move_position.col] = car_name;

		if(status != 0)
		{
			prev_position = path[source_num][dest_num][status - 1];
			map_draw[prev_position.row][prev_position.col] = map_draw_default[prev_position.row][prev_position.col];
		}
	}
}

/* car_info is implemented like string "aAB", or "bBC" from thread name.
	This function automatically divide this string into name, source,
	and destination. and if condition select the moving of car
	depends on each character source and destination. */
void
move_car(void) {

	int process_status = 0;
	int loop = 10;
	char * thread_name;
	char car_name;
	char source;
	char destination;

	thread_name = (char *)malloc(sizeof(thread_name()));
	strlcpy(thread_name, thread_name(), sizeof(thread_name()));

	car_name = thread_name[0];
	source = thread_name[1];
	destination = thread_name[2];
	while(loop != 0){
	sema_down(&s);

	if(source == 'A' && destination == 'B')
	{
		if(process_status == 2)
		{
			sema_down(&control[1]);
			sema_down(&control[5]);
		}
		if(process_status == 3)
		{
			sema_up(&control[5]);
			sema_up(&control[1]);
		}
		set_move(1, 2, process_status);
	}

	else if(source == 'A' && destination == 'C')
	{
		if(process_status == 2)
		{
			sema_down(&control[0]);
			sema_down(&control[2]);
			sema_down(&control[3]);
			sema_down(&control[5]);
		}
		/* critical section. */
		if(process_status == 5)
		{
			sema_up(&control[5]);
			sema_up(&control[3]);
			sema_up(&control[2]);
			sema_up(&control[0]);
		}
		set_move(1, 3, process_status);
	}

	else if(source == 'B' && destination == 'A')
	{
		if(process_status == 2)
		{
			sema_down(&control[1]);
			sema_down(&control[3]);
			sema_down(&control[4]);
			sema_down(&control[5]);
		}
		/* critical section. */
		if(process_status == 7)
		{
			sema_up(&control[5]);
			sema_up(&control[4]);
			sema_up(&control[3]);
			sema_up(&control[1]);
		}
		set_move(2, 1, process_status);
	}

	else if(source == 'B' && destination == 'C')
	{
		if(process_status == 2)
		{
			sema_down(&control[1]);
			sema_down(&control[2]);
		}
		/* critical section. */
		if(process_status == 3)
		{
			sema_up(&control[2]);
			sema_up(&control[1]);
		}
		set_move(2, 3, process_status);
	}

	else if(source == 'C' && destination == 'A')
	{
		if(process_status == 2)
		{
			sema_down(&control[2]);
			sema_down(&control[5]);
		}
		/* critical section. */
		if(process_status == 5)
		{
			sema_up(&control[5]);
			sema_up(&control[2]);
		}
		set_move(3, 1, process_status);
	}

	else if(source == 'C' && destination == 'B')
	{
		if(process_status == 2)
		{
			sema_down(&control[0]);
			sema_down(&control[1]);
			sema_down(&control[2]);
			sema_down(&control[4]);
		}
		/* critical section. */
		if(process_status == 7)
		{
			sema_up(&control[4]);
			sema_up(&control[2]);
			sema_up(&control[1]);
			sema_up(&control[0]);
		}
		set_move(3, 2, process_status);
	}

	else
	{
		//wrong input like AA or BB or NULL string
	}
	sema_up(&s);

	sema_down(&print);

	print_turn--;

	if(print_turn == 0){
		print_mapdata();
		print_turn = numberofcar;
	}

	sema_up(&print);
	timer_sleep(100);
	loop--;
}
}
