#include "../queue.h"

#define MAX_NUM_OF_REQUESTS 3
#define NUM_THREADS 10


void* thread_routine(void* nothing)
{
    while (1) {
        RequestInfo r_info = queue_pop_front();

        queue_dec_num_requests();

        printf("%d\n", r_info.connfd);
    }
}



int main() {
    int curr_val = 0;

    queue_init(MAX_NUM_OF_REQUESTS, "block");

    pthread_t bitches[NUM_THREADS];
    for (int i = 0; i < NUM_THREADS; ++i)
        pthread_create(&bitches[i], NULL, thread_routine, NULL);


    while (1) {
        scanf("%d", &curr_val);
        if (curr_val < 0) {
            break;
        }

        queue_push_back(curr_val);

    }

    return 0;
}