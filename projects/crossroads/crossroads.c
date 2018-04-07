#include <stdio.h>
#include <string.h>

#include "threads/init.h"
#include "threads/malloc.h"
#include "threads/synch.h"
#include "threads/thread.h"

#include "devices/timer.h"

#include "projects/crossroads/crossroads.h"
#include "projects/crossroads/mapdata.h"

struct semaphore s;
struct semaphore control[4];
int numberofcar;
int print_turn;
char map_draw[7][7];
int car_location[7][7];

static void car_thread(void);

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
  sema_init(&s, 1);
	sema_init(&control[0], 1);
	sema_init(&control[1], 1);
	sema_init(&control[2], 1);
	sema_init(&control[3], 1);

	//initialize.
	//input = (char *)malloc(sizeof(strlen(argv[1]));
	//strlcpy(input, argv[1], sizeof(strlen(argv[1]));
  input = argv[1];

  printf("input string : %s ", input);

	numberofcar = (strlen(input) + 1) / 4;

  printf("lenght : %d number of car : %d\n", strlen(input), numberofcar);

	print_turn = numberofcar;
  car_data = (char **)malloc(sizeof(4));

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
  print_mapdata();

	 for(i = 0; i < numberofcar; i++)
	 {
	 	thread_create(car_data[i], PRI_DEFAULT, car_thread, NULL);
	 }

}

/* Print map in kernel. */
void
print_mapdata(void)
{
	int i, j;
	for(i = 0; i < 7; i++)
	{
			for(j = 0; j < 7; j++)
			{
				printf("%c", map_draw[i][j]);
        //printf("%d", car_location[i][j]);
			}
			printf("\n");
	}
  // for(i = 0;i<17;i++)
  // {
  // printf("\n");
  // }
    //timer_mdelay(300);
}

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

bool
is_occupied(int row, int col)
{
  if(car_location[row][col] == 1)
    return true;
  else
    return false;
}

/* car_info is implemented like string "aAB", or "bBC" from thread name.
	This function automatically divide this string into name, source,
	and destination. and if condition select the moving of car
	depends on each character source and destination.
  And semaphore 0, 1, 2, 3 means,
  0 |__| 1
  --|--|--
  2 |__| 3*/
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

  printf("thread init %c:%c:%c\n", car_name, source, destination);

  while(true){
	if(source == 'A' && destination == 'B')
	{
		if(process_status == 2)
		{
			sema_down(&control[2]);
      printf("now in critical section! A-B\n");
		}
		if(process_status == 3)
		{
			sema_up(&control[2]);
      printf("out of critical section! A-B\n");
		}
		process_status = set_move(0, 1, process_status, car_name);
    printf("thread : [%s] car name : %c status : %d\n", thread_name(), car_name, process_status);
  }

	else if(source == 'A' && destination == 'C')
	{
		if(process_status == 2)
		{
			sema_down(&control[2]);
			sema_down(&control[3]);
      printf("now in critical section! A-C\n");
		}
		if(process_status == 5)
		{
			sema_up(&control[3]);
			sema_up(&control[2]);
      printf("out of critical section! A-C\n");
		}
		process_status = set_move(0, 2, process_status, car_name);
    printf("thread : [%s] car name : %c status : %d\n", thread_name(), car_name, process_status);
  }

	else if(source == 'B' && destination == 'A')
	{
		if(process_status == 2)
		{
			sema_down(&control[0]);
			sema_down(&control[1]);
			sema_down(&control[3]);
      printf("now in critical section! B-A\n");
		}
		if(process_status == 7)
		{
			sema_up(&control[3]);
			sema_up(&control[1]);
			sema_up(&control[0]);
      printf("out of critical section! B-A\n");
		}
		process_status = set_move(1, 0, process_status, car_name);
    printf("thread : [%s] car name : %c status : %d\n", thread_name(), car_name, process_status);
  }

	else if(source == 'B' && destination == 'C')
	{
		if(process_status == 2)
		{
			sema_down(&control[3]);
      printf("now in critical section! B-C\n");
		}
		if(process_status == 3)
		{
			sema_up(&control[3]);
      printf("out of critical section! B-C\n");
		}
		process_status = set_move(1, 2, process_status, car_name);
    printf("thread : [%s] car name : %c status : %d\n", thread_name(), car_name, process_status);
  }

	else if(source == 'C' && destination == 'A')
	{
		if(process_status == 2)
		{
			sema_down(&control[0]);
			sema_down(&control[1]);
      printf("now in critical section! C-A\n");
		}
		if(process_status == 5)
		{
			sema_up(&control[1]);
			sema_up(&control[0]);
      printf("out of critical section! C-A\n");
		}
		process_status = set_move(2, 0, process_status, car_name);
    printf("thread : [%s] car name : %c status : %d\n", thread_name(), car_name, process_status);
  }

	else if(source == 'C' && destination == 'B')
	{
      if(process_status == 2)
		   {
			   sema_down(&control[0]);
			   sema_down(&control[1]);
			   sema_down(&control[2]);
         printf("now in critical section! C-B\n");
		   }
		  if(process_status == 7)
		  {
			   sema_up(&control[2]);
			   sema_up(&control[1]);
			   sema_up(&control[0]);
         printf("out of critical section! C-B\n");
		  }
		process_status = set_move(2, 1, process_status, car_name);
    printf("thread : [%s] car name : %c status : %d\n", thread_name(), car_name, process_status);
  }

  print_turn--;
	if(print_turn == 0)
  {
		print_mapdata();
		print_turn = numberofcar;
	}

  if(process_status > 39)
  {
    printf("thread [%s] is finished!\n", thread_name());
    numberofcar--;
    break;
  }
  timer_sleep(1);
}
}
