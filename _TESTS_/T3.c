#include "../queue.h"

// MIGHT BE A FAULTY TEST...


#define MAX_NUM_OF_REQUESTS 3
#define NUM_THREADS 10


pthread_mutex_t global_lock_m;

pthread_cond_t read_val_allowed_c;

pthread_cond_t write_val_allowed_c;

int val_is_set; // state variable for access_val_allowed_c.

int global_val;



void* thread_routine(void* nothing) // main version
{
    while (1) {
        pthread_mutex_lock(&global_lock_m);
        while (val_is_set == 0) {
            pthread_cond_wait(&read_val_allowed_c, &global_lock_m);
        }
        queue_push_back(global_val);
        val_is_set = 0;
        pthread_cond_signal(&write_val_allowed_c);
        pthread_mutex_unlock(&global_lock_m);

        RequestInfo r_info = queue_pop_front();

        queue_dec_num_requests();

        printf("%d\n", r_info.connfd);
    }
}


/*
void* thread_routine(void* nothing)
{
    while (1) {
        pthread_mutex_lock(&global_lock_m);
        while (val_is_set == 0) {
            pthread_cond_wait(&read_val_allowed_c, &global_lock_m);
        }
        printf("%d\n", global_val);
        val_is_set = 0;
        pthread_cond_signal(&write_val_allowed_c);
        pthread_mutex_unlock(&global_lock_m);
    }
}
*/


/*
void* thread_routine(void* nothing)
{
    while (1) {
        pthread_mutex_lock(&global_lock_m);
        while (val_is_set == 0) {
            pthread_cond_wait(&read_val_allowed_c, &global_lock_m);
        }

        queue_push_back(global_val);
        RequestInfo r_info = queue_pop_front();
        queue_dec_num_requests();
        printf("%d\n", r_info.connfd);

        val_is_set = 0;
        pthread_cond_signal(&write_val_allowed_c);
        pthread_mutex_unlock(&global_lock_m);
    }
}
*/


int main(int argc, char** argv) {
    val_is_set = 0; // very important!
    int curr_val;

    pthread_mutex_init(&global_lock_m, NULL); // assume success.
    pthread_cond_init(&read_val_allowed_c, NULL); // assume success.
    pthread_cond_init(&write_val_allowed_c, NULL); // assume success.

    queue_init(MAX_NUM_OF_REQUESTS, "block");

    pthread_t bitches[NUM_THREADS];
    for (int i = 0; i < NUM_THREADS; ++i)
        pthread_create(&bitches[i], NULL, thread_routine, NULL);

    for (int i = 1; i < argc; ++i) { 
        curr_val = atoi(argv[i]);
        if (curr_val < 0) {
            break;
        }
        
        pthread_mutex_lock(&global_lock_m);
        while (val_is_set == 1) {
            pthread_cond_wait(&write_val_allowed_c, &global_lock_m);
        }
        global_val = curr_val;
        val_is_set = 1;
        pthread_cond_signal(&read_val_allowed_c);
        pthread_mutex_unlock(&global_lock_m);
    }

    //for (int i = 0; i < NUM_THREADS; ++i)
    //    pthread_join(bitches[i], NULL);

    sleep(0.7);

    return 0;
}