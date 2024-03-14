#include "../queue.h"
#include "../segel.h"
#define MAX_NUM_OF_REQUESTS_IN_QUEUE 10
#define NUM_THREADS 5


void* thread_routine(void* nothing)
{
    while (1) {
        RequestInfo r_info = queue_pop_front();
        
        //printf("%d\n", r_info.connfd); // "handle" the request.

        queue_dec_num_requests();
    }
}



int main(int argc, char** argv) {
    

    queue_init(MAX_NUM_OF_REQUESTS_IN_QUEUE, "block");

    pthread_t bitches[NUM_THREADS];
    for (int i = 0; i < NUM_THREADS; ++i)
        pthread_create(&bitches[i], NULL, thread_routine, NULL);


    int val;

    // PRESS CTRL+D TO TERMINATE
    while (scanf("%d", &val) != EOF) { 
        queue_push_back(val);
    }
       
    return 0;
}