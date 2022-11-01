#include <iostream>
#include <fstream>
#include <pthread.h>
#include <thread>
#include <chrono>
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <random>
#include <time.h>
#include <algorithm>
// team = 'A' or 'B'
// team_flag = 'a' or 'b'

using namespace std;
int T1, T2, rows, columns;
char* field_map;
ofstream output_file;
pthread_mutex_t lock;
bool change_field_map_state; // updating screen view on change
bool game_finished = false;
bool verbose = true;

//returns the console color code corresponding to team char
string occupied_color(char c)
{
	if (c == 'a' || c == 'A')
	{
		return "\033[1;41m"; //red
	}
	else if (c == 'b' || c == 'B')
	{
		return "\033[1;44m"; //blue
	}
	else
	{
		return "\033[1;m"; //default
	}
}

// output the field_map in a graphical way
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

// clears field_map to be unoccupied
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

//outputs the contents of field_map to file map.bin
void output_binary_file()
{
	output_file.open("map.bin");

	for (int i = 0; i < rows; i++)
	{
		for (int j = 0; j < columns; j++)
		{
			output_file << field_map[i * columns + j];
		}
		output_file << "\n";
	}
	output_file.close();
}

//after location i,j is already 'hit', this method is called
void missile_vicinity(int i, int j, char team_flag)
{
	char team;
	if (team_flag == 'a')
		team = 'A';
	else
		team = 'B';

	int min_row = min(0, i - 1);
	int max_row = max(rows - 1, i + 1);
	int min_column = min(0, j - 1);
	int max_column = max(columns - 1, j + 1);
	
	int number_locations = (max_row - min_row) * (max_column - min_column);
	int positive_flag_count = 0;
	for (int u = min_row; u < max_row; u++)
	{
		for (int v = min_column; v < max_column; v++)
		{
			char current_location = field_map[u * columns + v];
			if (current_location == team || current_location == team_flag)
			{
				positive_flag_count++;
			}
		}
	}
	if ((positive_flag_count * 2) >= number_locations) // if there is a majority
	{
		// do not touch location i,j
		// do not touch team members 'A' or 'B'
		for (int u = min_row; u < max_row; u++)
		{
			for (int v = min_column; v < max_column; v++)
			{
				char current_location = field_map[u * columns + v];
				bool can_flag_location = (u != i || v != j) && (current_location != 'A' && current_location != 'B');
				if (can_flag_location)
				{
					field_map[u * columns + v] = team_flag;
				}
			}
		}
	}
}

//tries location i,j and returns true if there is a 'hit'
bool try_fire_missile(int i, int j, char team_flag)
{
	char location_oc = field_map[i * columns + j];
	if (location_oc == 'A' || location_oc == 'B')
	{
		return false;
	}
	if (location_oc == team_flag) // if already occupied by team 
	{
		field_map[i * columns + j] = 'u'; // becomes unoccupied
	}
	else
	{
		field_map[i * columns + j] = team_flag;
	}
	missile_vicinity(i, j, team_flag);
	return true;
}

//a struct for passing info into pthread active_team_member
struct Team_Member_Info {
	int i, j;
	char c;
};

// soldier thread for after soldier is deployed. soldier shoots missile and waits 1-3 seconds before firing again
void* active_team_member(void* arg)
{
	struct Team_Member_Info *tmi = (Team_Member_Info*)arg;
	char team = tmi->c;
	char team_flag;
	if (team == 'A')
	{
		team_flag = 'a';
	}
	else
	{
		team_flag = 'b';
	}
	while (!game_finished) // boolean value controlled by supervisor thread
	{
		int k = rand() % rows;
		int l = rand() % columns;
		pthread_mutex_lock(&lock);
		if (verbose && !game_finished)
		{
			cout << team << " at " << tmi->i << ", " << tmi->j << " firing at location " << k << ", " << l << endl;
		}
		// if the missle was success or if the map is already changed
		change_field_map_state = try_fire_missile(k, l, team_flag) || change_field_map_state; 
		pthread_mutex_unlock(&lock);
		int wait_time = rand() % 3 + 1; // random int 1 - 3
		this_thread::sleep_for(chrono::seconds(wait_time));

	}
	return NULL;
}



// tries to deploy a team member returns true if successful, false otherwise
bool try_deploy_team_member(int i, int j, char team, pthread_t tid)
{
	char location = field_map[i * columns + j];
	if (location != 'u')
	{
		return false;
	}
	field_map[i * columns + j] = team;
	struct Team_Member_Info* tmi;
	tmi = (Team_Member_Info*)malloc(sizeof(struct Team_Member_Info));
	(*tmi).i = i;
	(*tmi).j = j;
	(*tmi).c = team;
	pthread_create(&tid, NULL, active_team_member, (void*)tmi);
	return true;
}

// true if a win condition has been reached, false otherwise
bool check_game_finished() 
{
	char winning_char;
	for (int i = 0; i < rows; i++)
	{
		for (int j = 0; j < columns; j++)
		{
			
			char currentChar = field_map[i * columns + j];
			if (currentChar == 'u')
				return false;
		}
	}
	return true;
}

char calculate_winning_team()
{
	int aCount = 0;
	int bCount = 0;
	for (int i = 0; i < rows; i++)
	{
		for (int j = 0; j < columns; j++)
		{
			char currentChar = field_map[i * columns + j];
			if (currentChar == 'A' || currentChar == 'a')
			{
				aCount++;
			}
			else if (currentChar == 'B' || currentChar == 'b')
			{
				bCount++;
			}
		}
	}

	if (aCount > bCount)
	{
		return 'A';
	}
	else if (bCount > aCount)
	{
		return 'B';
	}
	else
	{
		return 'T';
	}
}

void display_winning_team()
{
	char winningTeam = calculate_winning_team();
	if (winningTeam == 'T')
	{
		cout << "It is a tie." << endl;
	}
	else
	{
		cout << "Team " << winningTeam << " has won." << endl;
	}
}
// supervisor thread mainly checking if the game has completed or not
void* supervisor(void* arg)
{
	while (!game_finished)
	{
		if (change_field_map_state)
		{
			pthread_mutex_lock(&lock);
			game_finished = check_game_finished();
			change_field_map_state = false;
			system("clear");
			display_field_map();
			output_binary_file();
			pthread_mutex_unlock(&lock);
		}
	}
	return NULL;
}

// deploy team A and B with valid locations
void deploy_teams(pthread_t tid[])
{
	//deploy team a
	int t = 0;
	while (t < T1)
	{
		int i = rand() % rows;
		int j = rand() % columns;
		if (try_deploy_team_member(i, j, 'A', tid[t])) // set this to random value
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
		if (try_deploy_team_member(i, j, 'B', tid[t + T1])) // set this to random value
		{
			t++;
		}
	}
}

// main
int main(int argc, char** argv)
{
	srand(time(NULL)); // initialize seed for rand()
	if (argc != 5)
	{
		cout << "improper arguments given" << endl;
		return -1;
	}
	try // try catch for user input
	{
		T1 = stoi(argv[1]);
		T2 = stoi(argv[2]);
		rows = stoi(argv[3]);
		columns = stoi(argv[4]);
	}
	catch (exception &err)
	{
		cout << "something is wrong with the given arguments" << endl;
		return -1;
	}
	cout << T1 << "  " << T2 << "  " << rows  << "  " <<  columns << endl; // display arguments for sanity

	if ((T1 + T2) > (rows * columns))
	{
		cout << "soldier error: more than the maximum allowed" << endl; 
		return -1;
	}
	if (T1 <= 0 || T2 <= 0)
	{
		cout << "not enough troops provided" << endl;
	}
	field_map = (char*)malloc(sizeof(char) * rows * columns);
	pthread_mutex_init(&lock, NULL);
	reset_field_map();
	int numTeamMemberThreads = T1 + T2;
	pthread_t tid[numTeamMemberThreads];
	deploy_teams(tid);
	//create and join supervisor thread
	pthread_t superTid;
	pthread_create(&superTid, NULL, supervisor, NULL); 
	pthread_join(superTid, NULL);
	display_winning_team();
	//end of main
	pthread_mutex_destroy(&lock);
	return 0;
}

