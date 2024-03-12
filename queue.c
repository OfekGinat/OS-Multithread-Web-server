#include "queue.h"



/* >!>!>!>!>!>!>!>!>!> !!! IMPORTANT: DON'T FORGET TO ADD A RULE TO THE Makefile FOR THIS SOURCE CODE !!! <!<!<!<!<!<!<!<!<!< */



/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                             Queue variables
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

// >>>>>>>>>> SHOULD ALL/ANY OF THESE VARS BE static?

/** 
*    queue -
*        The requests queue, implemented as a ring buffer.
**/
RequestInfo* queue;

/** 
*    queue_insert_allowed_c -
*        Condition variable for inserting into the queue.
*        Must be initialized! 
**/
cond_t queue_insert_allowed_c; 

/** 
*    queue_remove_allowed_c -
*        Condition variable for removing from the queue.
*        Must be initialized! 
**/
cond_t queue_remove_allowed_c; 

/** 
*    queue_lock_m -
*        Lock for accessing the queue.
*        Must be initialized! 
**/
mutex_t queue_lock_m;

/** 
*    p_read -
*        Ring buffer index for next read.
**/
int p_read;

/** 
*    p_write -
*        Ring buffer index for next write.
**/
int p_write;

/** 
*    MAX_REQUESTS_NUM -
*        Maximum number of requests that the system
*        can handle at any given moment.
**/
int MAX_REQUESTS_NUM; // is constant

/** 
*    N -
*        Ring buffer's total size (equals MAX_REQUESTS_NUM + 1).
**/
int N; // is constant

/** 
*   queue_size -
*        Number of requests waiting to be handled.
**/
int queue_size;

/** 
*    num_requests_in_handling -
*       Number of requests currently being handled
*       by thr worker threads.
**/
int num_requests_in_handling;

/**
*     p_push_back - 
*        Is used in queue_push_back().
*        Will be set according to overload handling policy
*        to one of { _block_push_back(), _drop_tail_push_back(),
*                    _drop_head_push_back(), ... }
**/
void (p_push_back*) (RequestInfo);

/**
*     p_pop_front - 
*        Is used in queue_pop_front().
*        Will be set according to overload handling policy
*        to one of { _block_pop_front(), _drop_tail_pop_front(),
*                    _drop_head_pop_front(), ... }
**/
RequestInfo (p_pop_front*) (void); // >>>>>>>>>> IMPORTANT: MAKE SURE p_pop_front IS NEEDED AND SHOULD ACT SIMILARLY TO p_push_back 



/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                            Queue helper functions 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/********************* Insertion: *********************/
/** 
*    Insert an element at the back of the queue.
**/ 
// >>>>>>>>>> NOTE: HELPER FUNCTION, PROBABLY SHOULDN'T USE ANY LOCKS (ONLY INSERT AN ELEMENT) 
void _enqueue(RequestInfo request_info);

void _block_push_back(RequestInfo request_info); 

void _drop_tail_push_back(RequestInfo request_info); // ---> IMPLEMENT

void _drop_head_push_back(RequestInfo request_info); // ---> IMPLEMENT

// void _block_flush_push_back(RequestInfo request_info); // ---> TODO: ADD FOR BONUS SCHEDALGS

// void _drop_random_push_back(RequestInfo request_info); // ---> TODO: ADD FOR BONUS SCHEDALGS

/********************* Removal: *********************/
/** 
*    Remove the element at the front of the queue.
**/
// >>>>>>>>>> NOTE: HELPER FUNCTION, PROBABLY SHOULDN'T USE ANY LOCKS (ONLY REMOVE AN ELEMENT) 
RequestInfo _dequeue(); 

RequestInfo _block_pop_front(); 

RequestInfo _drop_tail_pop_front(); // ---> IMPLEMENT

RequestInfo _drop_head_pop_front(); // ---> IMPLEMENT

// RequestInfo _block_flush_pop_front(); // ---> TODO: ADD FOR BONUS SCHEDALGS

// RequestInfo _drop_random_pop_front(); // ---> TODO: ADD FOR BONUS SCHEDALGS



/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                                Utilities 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

// >>>>>>>>>> IMPORTANT: MAYBE ADD MACROS TO CLEAN THINGS UP -
/* >>>>>>>>>> NOTE: MY REASONING FOR USING MACROS AND NOT FUNCTIONS IS TO PREVENT 
   >>>>>>>>>> WEIRD LOCK RELATED BUGS, BUT I'M NOT SURE THIS IS ACTUALLY BETTER, 
   >>>>>>>>>> LET'S FIND OUT AND SEE */

#define mutex_init(lock_addr) do {                                  \
    int _error_code = pthread_mutex_init((lock_addr), NULL);        \
    if (_error_code != 0) {                                         \
        posix_error(_error_code, "pthread_mutex_init error");       \
    }                                                               \
} while (0)                                                         \


#define mutex_lock(lock_addr) do {                                  \
    int _error_code = pthread_mutex_lock((lock_addr));              \
    if (_error_code != 0) {                                         \
        posix_error(_error_code, "pthread_mutex_lock error");       \
    }                                                               \
} while (0)                                                         \


#define mutex_unlock(lock_addr) do {                                \
    int _error_code = pthread_mutex_unlock((lock_addr));            \
    if (_error_code != 0) {                                         \
        posix_error(_error_code, "pthread_mutex_unlock error");     \
    }                                                               \
} while (0)                                                         \


#define cond_init(cond_addr) do {                                   \
    int _error_code = pthread_cond_init((cond_addr), NULL);         \
    if (_error_code != 0) {                                         \
        posix_error(_error_code, "pthread_cond_init error");        \
    }                                                               \
} while (0)                                                         \


#define cond_wait(cond_addr, lock_addr) do {                        \
    int _error_code = pthread_cond_wait((cond_addr), (lock_addr));  \
    if (_error_code != 0) {                                         \
        posix_error(_error_code, "pthread_cond_wait error");        \
    }                                                               \
} while (0)                                                         \


#define cond_signal(cond_addr) do {                                 \
    int _error_code = pthread_cond_signal((cond_addr));             \
    if (_error_code != 0) {                                         \
        posix_error(_error_code, "pthread_cond_signal error");      \
    }                                                               \
} while (0)                                                         \


// >>>>>>>>>> OR:
/*
    #define _pthread_do(pthread_function_name,  pthread_function_call) do {     \
        int _error_code = (pthread_function_call);                              \
        if (_error_code != 0) {                                                 \
            posix_error(_error_code, "pthread_function_name");                  \
        }                                                                       \
    } while (0)                                                                 \
*/ 
// >>>>>>>>>> FOR EXAMPLE - _pthread_do(pthread_cond_wait, pthread_cond_wait(&queue_insert_allowed_c, &queue_lock_m));
// >>>>>>>>>> VERY UGLY...



/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                        Queue function implementations 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

void queue_init(int max_requests, char* schedalg)
{
    MAX_REQUESTS_NUM = max_requests;
    N = MAX_REQUESTS_NUM + 1;
    p_read = 0;
    p_write = 0;
    queue_size = 0;
    num_requests_in_handling = 0;
    errno = 0; // needed for unix_error().
    queue = (RequestInfo*)malloc(N * sizeof(RequestInfo)); 
    if (queue == NULL) {
        unix_error("malloc error");
    }

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


void _enqueue(RequestInfo request_info)
{
    queue[p_write] = request_info;
    p_write = (p_write + 1) % N;
    queue_size++;
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


/*
 ---> ****************************************************************************
 ---> TODO: ADD THE REST OF _<overload handling policy>_push_back() FUNCTIONS HERE
 ---> ****************************************************************************
*/


RequestInfo queue_pop_front()
{
    return p_pop_front();
}


RequestInfo _dequeue()
{
    RequestInfo request_info = queue[p_read];
    p_read = (p_read + 1) % N;
    queue_size--;
    return request_info;
}


RequestInfo _block_pop_front()
{
    mutex_lock(&queue_lock_m);
    while (queue_size == 0) {
        cond_wait(&queue_remove_allowed_c, &queue_lock_m);
    }

    RequestInfo request_info = _dequeue();
    
    mutex_unlock(&queue_lock_m);
    return request_info;
}


/*
 ---> ****************************************************************************
 ---> TODO: ADD THE REST OF _<overload handling policy>_pop_front() FUNCTIONS HERE
 ---> ****************************************************************************
*/


/*
     >!>!>!>!>!>!>!>!>!> VERY IMPORTANT: <!<!<!<!<!<!<!<!<!< 
        MAYBE WE NEED A FEW VERSIONS OF THIS,
        WITH SOMETHING LIKE A void (p_dec_num_requests*) (void)
        AND MATCHING _<overload handling policy>_dec_num_requests()
        ( IF SO, PROBABLY NEED TO CHANGE THIS FUNCTION TO -
            void queue_dec_num_requests()
            {
                p_dec_num_requests();
            }
        OR SOMETHING LIKE THIS ).
*/
void queue_dec_num_requests()
{
    mutex_lock(&queue_lock_m);

    num_requests_in_handling--;
    cond_signal(&queue_insert_allowed_c); // >>>>>>>>>> MIGHT BE SPECIFIC FOR schedalg == "block"

    mutex_unlock(&queue_lock_m);
}