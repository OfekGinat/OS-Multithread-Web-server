#include "queue.h"

/* >>>>>>>>>> !!! IMPORTANT: DON'T FORGET TO ADD A RULE TO THE Makefile FOR THIS SOURCE CODE !!! */

void queue_init(int max_requests, char* schedalg)
{
    MAX_REQUESTS_NUM = max_requests;
    N = MAX_REQUESTS_NUM + 1;
    p_read = 0;
    p_write = 0;
    queue_size = 0;
    num_requests_in_handling = 0;
    queue = (RequestInfo*)malloc(N * sizeof(RequestInfo)); // ASSUME SUCCESS?

    // >>>>>>>>>> !!! MAYBE NEED TO MOVE THIS INSIDE "if (strcmp(schedalg, "block") == 0)" !!!
    // >>>>>>>>>> (IF NOT ALL THREE, MAYBE ONE OR TWO OF THE initS, OR
    // >>>>>>>>>> MAYBE SHOULD APPEAR IN SEVERAL PLACES...)
    cond_init(&queue_insert_allowed_c);
    cond_init(&queue_remove_allowed_c);
    mutex_init(&queue_lock_m);

    if (strcmp(schedalg, "block") == 0) {
        p_push_back = _block_push_back;
        p_pop_front = _block_pop_front;
    } else if (strcmp(schedalg, "dt") == 0) {
        p_push_back = _drop_tail_push_back;
        p_pop_front = _drop_tail_pop_front;
    } else if (strcmp(schedalg, "dh") == 0) {
        p_push_back = _drop_head_push_back;
        p_pop_front = _drop_head_pop_front;
    } /*else if (strcmp(schedalg, "bf") == 0) {
        p_push_back = _block_flush_push_back;
        p_pop_front = _block_flush_pop_front;
    } else { // schedalg == "random"
        p_push_back = _drop_random_push_back;
        p_pop_front = _drop_random_pop_front;
    }*/
}


void _enqueue(RequestInfo request_info)
{
    queue[p_write] = request_info;
    p_write = (p_write + 1) % N;
    queue_size++;
}


// >>>>>>>>>> OPTION: CHANGE TO void queue_push_back(RequestInfo request_info) 
// >>>>>>>>>> AND LET THE SERVER CREATE THE NEW ELEMNT TO BE ADDED
void queue_push_back(int connfd)
{
    /* >>>>>>>>>> PROBABLY NEED TO ADD THINGS LIKE: 
        int arrival_time;
        if ( (arrival_time = gettimeofday()) == -1) {
            report error somehow;
            exit or something;
        }
    */
    RequestInfo request_info = { connfd /* , more info like arrival_time, ... */ };
    p_push_back(request_info);
}


void _block_push_back(RequestInfo request_info)
{
    mutex_lock(&queue_lock_m);
    while ((queue_size + num_requests_in_handling) == MAX_REQUESTS_NUM) {
        cond_wait(&queue_insert_allowed_c, &queue_lock_m);
    } // >>>>>>>>>> DO assert( (queue_size + num_requests_in_handling) < MAX_REQUESTS_NUM ) AFTER while?

   _enqueue(request_info);

    cond_signal(&queue_remove_allowed_c);
    mutex_unlock(&queue_lock_m);
}