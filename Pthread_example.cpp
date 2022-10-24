#include <iostream>
#include <pthread.h>
#include <thread>
#include <chrono>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

using namespace std;

pthread_t tid[5];
int counter;
pthread_mutex_t lock;

void* trythis(void* arg)
{
    //std::this_thread::sleep_for(chrono::seconds(5));
    pthread_mutex_lock(&lock);

    unsigned long i = 0;
    counter += 1;
    cout <<"\n Job " << counter << " has started \n";
    for(i = 0 ; i < (0xFFFFFFFF); i++)
        ;
    cout << "\n Job " << counter << " has finished \n";

    pthread_mutex_unlock(&lock);

    return NULL;
}

int main(void)
{
    int i = 0;
    int error;

    if (pthread_mutex_init(&lock, NULL) != 0) {
        printf("\n mutex init has failed\n");
        return 1;
    }

    while (i < 5)
    {
        error = pthread_create(&tid[i], NULL, &trythis, NULL);
        if (error != 0)
        {
            cout << "\n Thread can't be created : " << error;

        }
        i++;
    }
    
    for (int i = 0; i < 5; i++)
    {
        pthread_join(tid[i], NULL);
    }
    pthread_mutex_destroy(&lock); //cleans up resources, lock still effects other thread usage

    return 0;
}