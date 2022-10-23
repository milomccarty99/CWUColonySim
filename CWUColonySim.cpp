#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <queue>
#include <string>
#include <mutex>
#include <pthread.h>
#include <thread>
#include <chrono>


using namespace std;
pthread_t tid[10];
queue<string> q;
int counter = 0;

int main(int argc, char** argv)
{
	cout << "hello from main" << endl;
}


class soldier
{

};