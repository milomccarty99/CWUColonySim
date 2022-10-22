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
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void* my_thread(void* arg) 
{
    pthread_mutex_lock(&lock);
    cout<<"hello from thread" <<endl;
    while(counter < 10)
    {
        this_thread::sleep_for(chrono::seconds(1));

        counter++;
        q.push("queue push");
    }
    pthread_mutex_unlock(&lock);
    return 0;
}

int main(int argc, char** argv)
{
    cout << "hello world" << endl;
    for(int i = 0; i< 10; i++)
    {
         // given allocated thread ID, no attributes, the function running our thread, and NULL parameter
        pthread_create(&tid[i],NULL,&my_thread,(void*)NULL);
    }
    for(int i = 0; i < 10; i++)
    {
        pthread_join(tid[i], NULL);
    }
    //q.push("");
    this_thread::sleep_for(chrono::seconds(10));

    while(!q.empty())
    {
        cout<<q.front() << endl;
        q.pop();
    }
    return 0;
}



class soldier
{

};