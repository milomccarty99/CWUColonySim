#include <iostream>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

using namespace std;

pthread_t tid[2];
int counter;

void* trythis(void* arg)
{
    cout << counter << endl;
    unsigned long i = 0;
    counter += 1;
    cout <<"\n Job " << counter << " has started \n";
    for(i = 0 ; i < (0xFFFFFFFF); i++)
        ;
    cout << "\n Job " << counter << " has finished \n";

    return NULL;
}

int main(void)
{
    int i = 0;
    int error;
    while (i < 2)
    {
        error = pthread_create(&tid[i], NULL, &trythis, NULL);
        if (error != 0)
        {
            cout << "\n Thread can't be created : " << error;

        }
        i++;
    }

    pthread_join(tid[0], NULL);
    pthread_join(tid[1], NULL);

    return 0;
}