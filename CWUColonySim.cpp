#include <iostream>
#include <pthread.h>
#include <thread>
#include <chrono>
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <random>
#include <time.h>
// team = 'A' or 'B'
// team_flag = 'a' or 'b'

using namespace std;
pthread_t tid[10];
int counter = 0;
char* field_map;
int T1, T2, rows, columns;
pthread_mutex_t lock;
bool change_field_map_state; // updating screen view on change
bool game_finished = false;


string occupied_color(char c)
{
	if (c == 'a' || c == 'A')
	{
		return "\033[1;41m";
	}
	else if (c == 'b' || c == 'B')
	{
		return "\033[1;44m";
	}
	else
	{
		return "\033[1;m";
	}
}
void display_field_map()
{
	for (int i = 0; i < rows; i++)
	{
		for (int j = 0; j < columns; j++)
		{
			char location_char = field_map[i * columns + j];
			cout << occupied_color(location_char) <<  location_char << "\033[0m";
		}
		cout << endl;
	}
}
void reset_field_map()
{
	for (int i = 0; i < rows; i++)
	{
		for (int j = 0; j < columns; j++)
		{
			field_map[i * columns + j] = 'u';
		}
	}
}
bool try_fire_missile(int i, int j, char team_flag)
{
	char location_oc = field_map[i * columns + j];
	if (location_oc == 'A' || location_oc == 'B')
	{
		return false;
	}
	if (location_oc == team_flag)
	{
		field_map[i * columns + j] = 'u'; // u for unoccupied
		// test change surrounding territory
	}
	else
	{
		field_map[i * columns + j] = team_flag;
		// test change surrounding territory
	}
	return true;
}
void* active_team_member(void* arg)
{
	char team = *((char*) &arg);
	char team_flag;
	if (team == 'A')
	{
		team_flag = 'a';
	}
	else
	{
		team_flag = 'b';
	}
	while (true) // boolean value controlled by supervisor thread
	{
		int k = rand() % rows;
		int j = rand() % columns;
		pthread_mutex_lock(&lock);
		change_field_map_state = try_fire_missile(k, j, team_flag) || change_field_map_state; // if the missle was success or if the map is already changed
		pthread_mutex_unlock(&lock);
		int wait_time = rand() % 3 + 1; // random int 1 - 3
		this_thread::sleep_for(chrono::seconds(wait_time));

	}
}
bool try_deploy_team_member(int i, int j, char team)
{
	char location = field_map[i * columns + j];
	if (location != 'u')
	{
		return false;
	}
	field_map[i * columns + j] = team;
	pthread_t tid;
	pthread_create(&tid, NULL, active_team_member, (void*)(long)team);
	return true;
}
bool check_game_finished() // naive implementation
{
	for (int i = 0; i < rows; i++)
	{
		for (int j = 0; j < columns; j++)
		{
			if (field_map[i * columns + j] == 'u')
			{
				return false;
			}
		}
	}
	return true;
}
void* supervisor(void* arg)
{
	while (!game_finished)
	{
		pthread_mutex_lock(&lock);
		if (change_field_map_state)
		{
			game_finished = check_game_finished();
			change_field_map_state = false;
			//update binary file TODO
			system("clear");
			display_field_map();
		}
		pthread_mutex_unlock(&lock);
	}
	return NULL;
}

int main(int argc, char** argv)
{
	//to do:
	// continue simulation from file
	srand(time(NULL)); // initialize seed for rand()
	if (argc != 5)
	{
		cout << "improper arguments given" << endl;
		return -1;
	}
	try
	{
		T1 = stoi(argv[1]);
		T2 = stoi(argv[2]);
		rows = stoi(argv[3]);
		columns = stoi(argv[4]);

	}
	catch (exception &err)
	{
		cout << "something went wrong with the given arguments" << endl;
		return -1;
	}
	cout << T1 << "  " << T2 << "  " << rows  << "  " <<  columns << endl; // display arguments for sanity

	if ((T1 + T2) > (rows * columns))
	{
		cout << "soldeir error: more than the maximum allowed" << endl; 
		return -1;
	}
	if (T1 <= 0 || T2 <= 0)
	{
		cout << "not enough troops provided" << endl;
	}
	field_map = (char*)malloc(sizeof(char) * rows * columns);
	pthread_mutex_init(&lock, NULL);
	reset_field_map();
	
	//display_field_map();
	//deploy team a
	int t = 0;
	while (t<T1)
	{
		int i = rand() % rows;
		int j = rand() % columns;
		if (try_deploy_team_member(i, j, 'A')) // set this to random value
		{
			t++;
		}
	}
	//deploy team b
	t = 0;
	while (t < T2)
	{
		int i = rand() % rows;
		int j = rand() % columns;
		if (try_deploy_team_member(i, j, 'B')) // set this to random value
		{
			t++;
		}
	}


	display_field_map();
	
	pthread_t tid;
	pthread_create(&tid, NULL, supervisor, NULL); // might add boolean flag for what supervisor should look for
	pthread_join(tid, NULL);


	// destroy after threads using lock are created

	pthread_mutex_destroy(&lock);
	return 0;
}

