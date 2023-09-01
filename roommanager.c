#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>

#include "roomman.h"

/**
 * @brief Print usage information
 */
static void rtfm(char* argv[])
{	int count = 0;
	while (argv[count]!= NULL)
	{
		count++;
	}
	
	char building[10];
	char room_name[10];
	uint16_t capacity[10];

	printf("Usage: %s {<Option>} <param1> {<param2>}\n", argv[0]);
	printf("Function: Room management\n");
	printf("Options:\n");
	printf("     -l {<building>}                   - show current list of rooms\n");
	printf("     -n <building> <room> [<capacity>] - add new room\n");
	printf("     -u <building> <room> <capacity>   - update the capacity of the room\n");
	printf("     -t <building> <room>              - toggle room reservation\n");
	printf("     -d <building> <room>              - delete a room\n");
	
	if (strcmp(argv[1],"-l") == 0)
	{	
		uint16_t* capacity_print;

		int32_t initial = -1;
		if (argv[2] == NULL)
		{
			int32_t room_id = roomman_directory(&initial,NULL,NULL);
			if(room_id < 0)
			{ 
				fprintf(stderr,"Error:%d \n",room_id);
			}
			else{
				printf("Building\t Name\t Capacity\t Reservation\n");
			while (room_id >= 0)
			{
				int8_t room_entry = roomman_readentry(room_id,building,room_name,capacity);
				if( room_entry < 0){
					printf("Error: Room not found.\n");
					break;
				}
				
				capacity_print = &capacity;
				if (room_entry ==1)
				{	
					printf("%s\t\t %s\t %d\t \t TRUE\t\n",building,room_name,*capacity_print);
				}
				else
				{	
					printf("%s\t\t %s\t %d\t FALSE\t\n",building,room_name,*capacity_print);

				}
				
				
				room_id =roomman_directory(&initial,NULL,NULL);
				if (room_id < 0)
				{	break;
					
				}
				

			}
			

			}
			
		}
		else{
			
			int32_t initial = -1;
			char* buiding_name =argv[2];
			int32_t room_id = roomman_directory(&initial,buiding_name,NULL);
			if(room_id < 0)
			{ 
				fprintf(stderr,"Error:%d \n",room_id);
			}
			printf("Building\t Name\t Capacity\t Reservation\n");
			while (room_id >= 0)
			{
				int8_t room_entry = roomman_readentry(room_id,building,room_name,capacity);
				if( room_entry < 0){
					printf("Error: Room not found.\n");
					break;
				}
				capacity_print = &capacity;

				if (room_entry ==1)
				{
					printf("%s\t\t %s\t %d\t TRUE\t\n",building,room_name,*capacity_print);
				}
				else
				{
					printf("%s\t\t %s\t %d\t FALSE\t\n",building,room_name,*capacity_print);

				}
				
				
				room_id =roomman_directory(&initial,NULL,NULL);
				if (room_id < 0)
				{
					fprintf(stderr,"Error:%d \n",room_id);
				}
			}
			}

		}
		
	if (strcmp(argv[1],"-n") == 0)
	{
		if (argv[2] != NULL && argv[3] != NULL)
		{
			char *building_name = argv[2];
			char *room_name = argv[3];
			uint16_t capacity;
			char*end;
			if (argv[4] == NULL)
			{
				int32_t room_id = roomman_create_room(building_name,room_name,10);
				if (room_id >= 0)
				{
					printf("Done!");
				}
				else
				{
					printf("In function roomman_create_room(): Error: the room already exists\n");
				}
				
			}
			else
			{	
				capacity = strtoul(argv[4],&end,3);
				int32_t room_id = roomman_create_room(building_name,room_name,capacity);
				if (room_id >= 0)
				{
					printf("Done!");
				}
				else
				{
					
					printf("In function roomman_create_room(): Error: the room already exists\n");
				}
			}
			
		}
		
	}
	
	if (strcmp(argv[1],"-u") == 0)
	{	
		if (argv[2] != NULL && argv[3] != NULL && atoi(argv[3])!= 0 && argv[4] != NULL && atoi(argv[4])!= 0 && argv[5]==NULL)
		{
			char *building_name = argv[2];
			char *room_name = argv[3];
			uint16_t capacity;
			char*end;
			bool update_result;
			capacity = strtoul(argv[4],&end,10);
			printf("%d",capacity);
			int32_t room_id = roomman_lookup(building_name,room_name);
			if (room_id >= 0)
			{
				update_result = roomman_update_capacity(room_id,capacity);
				if (update_result == true)
				{
					printf("Updated room capacity for room %s in building %s to %d",room_name,building_name,capacity);
				}
				else
				{
					printf("Update Capacity failed!");
				}
				
			}
			else
			{
				printf("In function roomman_lookup(): Error: Room not found");
			}
			
			}
		
		
		if (count != 4)
		{	
			printf("Too many args");
		}
		}
	
	if (strcmp(argv[1],"-t") == 0){
			if (argv[2] != NULL && argv[3] != NULL)
			{ 
				char *building_name = argv[2];
				char *room_name = argv[3];
				int32_t room_id = roomman_lookup(building_name,room_name);
				if (room_id >= 0)
				{
					
					int8_t reservation =  roomman_readentry(room_id,building,room_name,capacity);
					if (reservation == 1)
					{	bool clear_re = roomman_clear_reservation(room_id);
						if (clear_re == true)
						{
							printf("Cleared reservation for room %s in building %s",room_name,building_name);
						}
						else 
						{
							switch (room_id)
							{
							case -3:
								printf("In function roomman_lookup(): Error: the specified name is too long");
								break;
							default:
								printf("In function roomman_lookup(): Error: Room not found");
								break;
							}
						}
						
					
					}
					else if (reservation == 0)
					{
						bool book_room = roomman_reserve_room(room_id);
						if (book_room == true)
						{
							printf("Reserved room %s in building %s", room_name, building_name);
						}
						else
						{
							printf("Can not reserved room");
						}
					}
					else
					{
						printf("Can not reserve room");

					}
				
				}
				else
				{
					printf("In function roomman_lookup(): Error: Room not found");
				}
				
			}
	}	
	
	if (strcmp(argv[1],"-d") == 0){
			if (argv[2] != NULL && argv[3] != NULL&& atoi(argv[3])!= 0)
			{	char *building_name = argv[2];
				char *room_name = argv[3];
				int32_t room_id = roomman_lookup(building_name,room_name);
				if (room_id >= 0)
				{
					int8_t check = roomman_delete_room(room_id);
					if (check == 0)
					{
						printf("Room %s in building %s deleted",room_name,building_name);
					}
					else{
						printf("Can not delete room %s in building %s deleted",room_name,building_name);
					}
					
				}
				else
				{
					switch (room_id)
						{
						case -3:
							printf("In function roomman_lookup(): Error: the specified name is too long");
							break;
						default:
							printf("In function roomman_lookup(): Error: Room not found");
							break;
						}
				}
				
				
				
			}


	}

	}	
		

/**
 * @brief Main program
 */
int main(int argc, char* argv[])
 {	
	//roomman_init(true);
    rtfm(argv);

	return 0;
}

