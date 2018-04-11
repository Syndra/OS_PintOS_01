#include <stdio.h>
#include <string.h>

#include "threads/init.h"
#include "threads/malloc.h"
#include "threads/synch.h"
#include "threads/thread.h"

#include "devices/timer.h"

#include "projects/crossroads/crossroads.h"
#include "projects/crossroads/mapdata.h"

struct semaphore print;
struct semaphore control[4];
int numberofcar;
char map_draw[7][7];
int car_location[7][7];

static void car_thread(void);
static void print_thread(void);

/* Main function. */
void
run_crossroads(char **argv)
{
	//variables
	char *input;
	char *token, *save_ptr;
	char **car_data;

	int i, j;

	//assert if input == null.
	ASSERT(argv != NULL);

	//semaphore initialize.
	sema_init(&print, 1);
	sema_init(&control[0], 1);
	sema_init(&control[1], 1);
	sema_init(&control[2], 1);
	sema_init(&control[3], 1);

	//initialize.
  input = argv[1];
	numberofcar = (strlen(input) + 1) / 4;
  car_data = (char **)malloc(sizeof(char *) * numberofcar);

	//save mapdata into temporary 2^array called map_draw.
	for(i = 0; i < 7; i++)
	{
		for(j = 0; j < 7; j++)
		{
			map_draw[i][j] = map_draw_default[i][j];
      car_location[i][j] =  0;
		}
	}
	//tokenize argv with token " : " .
	i = 0;
	for (token = strtok_r(input, ":", &save_ptr); token != NULL; token = strtok_r (NULL, ":", &save_ptr))
	{
    car_data[i] = (char *)malloc(sizeof(char) * 4);
		strlcpy(car_data[i], token, sizeof(token));
		i++;
	}
	//create thread for each input string.
	for(i = 0; i < numberofcar ; i++)
	{
		thread_create(car_data[i], PRI_DEFAULT, car_thread, NULL);
	}
	thread_create("print", PRI_DEFAULT, print_thread, NULL);
}

/* Print function. */
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

/* Update car location in car_location and map_draw.
	There is two condition for actual moving.
	1. There is no car on position of move position of status.
	2. Move position of status is not (-1, -1) that means end of route.
	If car can move, occupy next location and release previous location.
	If move position is end, equvalant function may called in same way.
	Mpve_position = try to occupy
	Currnent_position = already occupied
	If, move is successfully proceded, give 1 point to status And
	return status.
	Also signal for finishing its procedure, it makes status 40, Maximum of
	route status, and end of function calling this, to check whether it is
	finished or not. */
int
set_move(int source_num, int dest_num, int status, char car_name)
{
	struct position *move_position;
	struct position *current_position;
	move_position = &path[source_num][dest_num][status];

  if(move_position->row == -1)
  {
    current_position = &path[source_num][dest_num][status - 1];
    map_draw[current_position->row][current_position->col] = map_draw_default[current_position->row][current_position->col];
    car_location[current_position->row][current_position->col] = 0;
    return 40;
  }
  else if(is_occupied(move_position->row, move_position->col))
  {
    return status;
  }
  else
  {
    map_draw[move_position->row][move_position->col] = car_name;
    car_location[move_position->row][move_position->col] = 1;
    if(status != 0)
    {
      current_position = &path[source_num][dest_num][status - 1];
      map_draw[current_position->row][current_position->col] = map_draw_default[current_position->row][current_position->col];
      car_location[current_position->row][current_position->col] = 0;
    }
    status++;
    return status;
  }
}

/* Return true if car_location[row][col] is 1.
	1 means already ouccpied by another car.*/
bool
is_occupied(int row, int col)
{
  if(car_location[row][col] == 1)
    return true;
  else
    return false;
}

/* Print thread. */
static void
print_thread(void)
{
	while(numberofcar != 0)
	{
		printf("\f");
		sema_down(&print);
		print_mapdata();
		timer_mdelay(1000);
		sema_up(&print);
  	timer_sleep(1);
	}
	printf("\f");
	print_mapdata();
	printf("Test is done.");
}

/* car_info is implemented like string "aAB", or "bBC" from thread name.
	This function automatically divide this string into name, source,
	and destination. and if condition select the moving of car
	depends on each character source and destination.
  And semaphore 0, 1, 2, 3 means,
  0 |__| 1
  --|--|--
  2 |__| 3 each side of crossroad. */
static void
car_thread(void) {

	int process_status = 0;
	char * thread_name_;
	char car_name;
	char source;
	char destination;

	thread_name_ = (char *)malloc(sizeof(thread_name()));
	strlcpy(thread_name_, thread_name(), sizeof(thread_name()));

	car_name = thread_name_[0];
	source = thread_name_[1];
	destination = thread_name_[2];

  while(true){
		sema_down(&print);
		sema_up(&print);
		//exception handler
		if(source == destination)
		{
			numberofcar--;
			break;
		}
		//condition check
		else if(source == 'A' && destination == 'B')
		{
			if(process_status == 2)
			{
				sema_down(&control[2]);
			}
			if(process_status == 3)
			{
				sema_up(&control[2]);
			}
			process_status = set_move(0, 1, process_status, car_name);
  	}

		else if(source == 'A' && destination == 'C')
		{
			if(process_status == 2)
			{
				sema_down(&control[2]);
				sema_down(&control[3]);
			}
			if(process_status == 5)
			{
				sema_up(&control[3]);
				sema_up(&control[2]);
			}
			process_status = set_move(0, 2, process_status, car_name);
  	}

		else if(source == 'B' && destination == 'A')
		{
			if(process_status == 2)
			{
				sema_down(&control[0]);
				sema_down(&control[1]);
				sema_down(&control[3]);
			}
			if(process_status == 7)
			{
				sema_up(&control[3]);
				sema_up(&control[1]);
				sema_up(&control[0]);
			}
			process_status = set_move(1, 0, process_status, car_name);
  	}

		else if(source == 'B' && destination == 'C')
		{
			if(process_status == 2)
			{
				sema_down(&control[3]);
			}
			if(process_status == 3)
			{
				sema_up(&control[3]);
			}
			process_status = set_move(1, 2, process_status, car_name);
  	}

		else if(source == 'C' && destination == 'A')
		{
			if(process_status == 2)
			{
				sema_down(&control[0]);
				sema_down(&control[1]);
			}
			if(process_status == 5)
			{
				sema_up(&control[1]);
				sema_up(&control[0]);
			}
			process_status = set_move(2, 0, process_status, car_name);
  	}

		else if(source == 'C' && destination == 'B')
		{
      	if(process_status == 2)
		   	{
			  	sema_down(&control[0]);
			   	sema_down(&control[1]);
			   	sema_down(&control[2]);
		   	}
		  	if(process_status == 7)
		  	{
			   	sema_up(&control[2]);
			   	sema_up(&control[1]);
			   	sema_up(&control[0]);
		  	}
				process_status = set_move(2, 1, process_status, car_name);
  	}
		//end condition check
		if(process_status > 39)
		{
			numberofcar--;
			break;
		}
  	timer_sleep(1);
	} //end of while
}
