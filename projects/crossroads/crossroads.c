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
	sema_init(&print, 1);
	sema_init(&control[0], 1);
	sema_init(&control[1], 1);
	sema_init(&control[2], 1);
	sema_init(&control[3], 1);

	//initialize.
	input = (char *)malloc(sizeof(argv[1])*4);
	strlcpy(input, argv[1], sizeof(argv[1])*4);

  printf("input string : %s ", input);

	numberofcar = (strlen(input) + 1) / 4;

  printf("lenght : %d number of car : %d\n", strlen(input), numberofcar);

	print_turn = numberofcar;
	process_status = (int *)malloc(sizeof(int)*numberofcar);
  car_data = (char **)malloc(sizeof(4));

	for(i = 0; i < 7; i++)
	{
		for(j = 0; j < 7; j++)
		{
			map_draw[i][j] = map_draw_default[i][j];
		}
	}
  printf("/////////////////////////////\n");
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
			}
			printf("\n");
	}
}

bool
set_move(int source_num, int dest_num, int status, char car_name)
{
	struct position move_position;
	struct position prev_position;
	move_position = path[source_num][dest_num][status];
	if(move_position.row != -1)
	{
		map_draw[move_position.col][move_position.row] = car_name;

		if(status != 0)
		{
			prev_position = path[source_num][dest_num][status - 1];
			map_draw[prev_position.col][prev_position.row] = map_draw_default[prev_position.col][prev_position.row];
		}
    return false;
	}
  else
  {
    return true;
  }
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
  int loop = 10;
	char * thread_name_;
	char car_name;
	char source;
	char destination;
  bool isfinished  = false;

	thread_name_ = (char *)malloc(sizeof(thread_name()));
	strlcpy(thread_name_, thread_name(), sizeof(thread_name()));

	car_name = thread_name_[0];
	source = thread_name_[1];
	destination = thread_name_[2];

  while(loop > 0){
	sema_down(&s);

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
		//isfinished = set_move(1, 2, process_status, car_name);
    process_status++;
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
		//isfinished = set_move(1, 3, process_status, car_name);
    process_status++;
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
		//isfinished = set_move(2, 1, process_status, car_name);
    process_status++;
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
		//isfinished = set_move(2, 3, process_status, car_name);
    process_status++;
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
		//isfinished = set_move(3, 1, process_status, car_name);
    process_status++;
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
		//isfinished = set_move(3, 2, process_status, car_name);
    process_status++;
    printf("thread : [%s] car name : %c status : %d\n", thread_name(), car_name, process_status);
  }

	sema_down(&print);

	print_turn--;

	if(print_turn == 0){
		print_mapdata();
		print_turn = numberofcar;
	}

	sema_up(&print);

  sema_up(&s);
  // if(isfinished)
  // {
  //   printf("thread [%s] is finished!\n", thread_name());
  //   break;
  // }
  loop--;
  timer_mdelay(40);
}
}
