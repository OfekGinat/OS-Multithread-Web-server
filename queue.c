#include "segel.h"
#include "queue.h"



/* >!>!>!>!>!>!>!>!>!> !!! IMPORTANT: DON'T FORGET TO ADD A RULE TO THE Makefile FOR THIS SOURCE CODE !!! <!<!<!<!<!<!<!<!<!< */



/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                             Queue variables
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

// >!>!>!>!>!>!>!>!>!> IS IT OK FOR LOCKS AND COND VARS TO BE static?

/** 
*    queue -
*        The requests queue, implemented as a ring buffer.
**/
static RequestInfo* queue; 

/** 
*    queue_insert_allowed_c -
*        Condition variable for inserting into the queue.
*        Must be initialized! 
**/
static pthread_cond_t queue_insert_allowed_c; 

/** 
*    queue_remove_allowed_c -
*        Condition variable for removing from the queue.
*        Must be initialized! 
**/
static pthread_cond_t queue_remove_allowed_c; 

/** 
*    queue_lock_m -
*        Lock for accessing the queue.
*        Must be initialized! 
**/
static pthread_mutex_t queue_lock_m;

/** 
*    p_read -
*        Ring buffer index for next read.
**/
static int p_read;

/** 
*    p_write -
*        Ring buffer index for next write.
**/
static int p_write;

/** 
*    MAX_REQUESTS_NUM -
*        Maximum number of requests that the system
*        can handle at any given moment.
**/
static int MAX_REQUESTS_NUM; // is constant

/** 
*    N -
*        Ring buffer's total size (equals MAX_REQUESTS_NUM + 1).
**/
static int N; // is constant

/** 
*   queue_size -
*        Number of requests waiting to be handled.
**/
static int queue_size;

/** 
*    num_requests_in_handling -
*       Number of requests currently being handled
*       by thr worker threads.
**/
static int num_requests_in_handling;

/**
*     p_push_back - 
*        Is used in queue_push_back().
*        Will be set according to overload handling policy
*        to one of { _block_push_back(), _drop_tail_push_back(),
*                    _drop_head_push_back(), ... }
**/
static void (*p_push_back)(RequestInfo);

/**
*     p_pop_front - 
*        Is used in queue_pop_front().
*        Will be set according to overload handling policy
*        to one of { _block_pop_front(), _drop_tail_pop_front(),
*                    _drop_head_pop_front(), ... }
**/
static RequestInfo (*p_pop_front)(void); 

/**
*     p_dec_num_requests - 
*        Is used in queue_dec_num_requests().
*        Will be set according to overload handling policy
*        to one of { _block_dec_num_requests(), _drop_tail_dec_num_requests(),
*                    _drop_head_dec_num_requests(), ... }
**/
static void (*p_dec_num_requests)(void);



/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                            Queue helper functions 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/***************************** Insertion: *****************************/
/** 
*    Insert an element at the back of the queue.
**/  
void _enqueue(RequestInfo request_info);

void _block_push_back(RequestInfo request_info); 

void _drop_tail_push_back(RequestInfo request_info); 

void _drop_head_push_back(RequestInfo request_info);

// void _block_flush_push_back(RequestInfo request_info); // ---> TODO: ADD FOR BONUS SCHEDALGS

// void _drop_random_push_back(RequestInfo request_info); // ---> TODO: ADD FOR BONUS SCHEDALGS


/***************************** Removal: *****************************/
/** 
*    Remove the element at the front of the queue.
**/
RequestInfo _dequeue(); 

RequestInfo _pop_front(); 

// RequestInfo _block_flush_pop_front(); // ---> TODO: ADD FOR BONUS SCHEDALGS

// RequestInfo _drop_random_pop_front(); // ---> TODO: ADD FOR BONUS SCHEDALGS


/****************** Monitoring number of requests: ******************/
void _block_dec_num_requests();

void _drop_all_dec_num_requests();

// void _block_flush_dec_num_requests(); // ---> TODO: ADD FOR BONUS SCHEDALGS

// void _drop_random_dec_num_requests(); // ---> TODO: ADD FOR BONUS SCHEDALGS



/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
            Wrappers for mutex and condition variable functions
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

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

    /* 
       >>>>>>>>> MAKE SURE TO PUT HERE initS WE NEED FOR ALL CASES
    */
    cond_init(&queue_remove_allowed_c);
    mutex_init(&queue_lock_m);

    if (strcmp(schedalg, "block") == 0) {
        p_push_back = _block_push_back;
        p_pop_front = _pop_front;
        p_dec_num_requests = _block_dec_num_requests;
        cond_init(&queue_insert_allowed_c);
    } else if (strcmp(schedalg, "dt") == 0) {
        p_push_back = _drop_tail_push_back;
        p_pop_front = _pop_front;
        p_dec_num_requests = _drop_all_dec_num_requests;
    } else if (strcmp(schedalg, "dh") == 0) {
        p_push_back = _drop_head_push_back;
        p_pop_front = _pop_front;
        p_dec_num_requests = _drop_all_dec_num_requests;
    } /*else if (strcmp(schedalg, "bf") == 0) {
        p_push_back = _block_flush_push_back;
        p_pop_front = _block_flush_pop_front;
        p_dec_num_requests = _block_flush_dec_num_requests;
    } else if (strcmp(schedalg, "random") == 0) { 
        p_push_back = _drop_random_push_back;
        p_pop_front = _drop_random_pop_front;
        p_dec_num_requests = _drop_random_dec_num_requests;
    }*/ else {
        fprintf(stderr, "Invalid overload handling");
	    exit(1);
    }
}


void queue_push_back(RequestInfo request_info)
{
    p_push_back(request_info);
}


RequestInfo queue_pop_front()
{
    return p_pop_front();
}


void queue_dec_num_requests()
{
    p_dec_num_requests();
}


void _enqueue(RequestInfo request_info)
{
    queue[p_write] = request_info;
    p_write = (p_write + 1) % N;
    queue_size++;
}


RequestInfo _dequeue()
{
    RequestInfo request_info = queue[p_read];
    p_read = (p_read + 1) % N;
    queue_size--;
    return request_info;
}


RequestInfo _pop_front()
{
    mutex_lock(&queue_lock_m);
    while (queue_size == 0) {
        cond_wait(&queue_remove_allowed_c, &queue_lock_m);
    }
    RequestInfo request_info = _dequeue();
    num_requests_in_handling++; 

    mutex_unlock(&queue_lock_m);
    return request_info;
}


void _drop_all_dec_num_requests()
{
    mutex_lock(&queue_lock_m);
    num_requests_in_handling--;
    mutex_unlock(&queue_lock_m);
}


/*~~~~~~~~~~~~~~~~~~~~ "block" functions start: ~~~~~~~~~~~~~~~~~~~~*/
void _block_push_back(RequestInfo request_info)
{
    mutex_lock(&queue_lock_m);
    while ((queue_size + num_requests_in_handling) == MAX_REQUESTS_NUM) {
        cond_wait(&queue_insert_allowed_c, &queue_lock_m);
    }

    _enqueue(request_info);

    cond_signal(&queue_remove_allowed_c);
    mutex_unlock(&queue_lock_m);
}


void _block_dec_num_requests()
{
    mutex_lock(&queue_lock_m);

    num_requests_in_handling--;
    cond_signal(&queue_insert_allowed_c); 

    mutex_unlock(&queue_lock_m);
}
/*~~~~~~~~~~~~~~~~~~~~~ "block" functions end. ~~~~~~~~~~~~~~~~~~~~~*/


/*~~~~~~~~~~~~~~~~~~~~ "drop_tail" functions start: ~~~~~~~~~~~~~~~~~~~~*/
void _drop_tail_push_back(RequestInfo request_info)
{
    mutex_lock(&queue_lock_m);
    if ((queue_size + num_requests_in_handling) == MAX_REQUESTS_NUM) {
        Close(request_info.connfd);
    }
    else {
        _enqueue(request_info);
        cond_signal(&queue_remove_allowed_c);
    }
    mutex_unlock(&queue_lock_m);
}
/*~~~~~~~~~~~~~~~~~~~~~ "drop_tail" functions end. ~~~~~~~~~~~~~~~~~~~~~*/


/*~~~~~~~~~~~~~~~~~~~~ "drop_head" functions start: ~~~~~~~~~~~~~~~~~~~~*/
void _drop_head_push_back(RequestInfo request_info)
{
    mutex_lock(&queue_lock_m);
    if ((queue_size + num_requests_in_handling) == MAX_REQUESTS_NUM) {
        if (queue_size == 0) {
            Close(request_info.connfd);
            mutex_unlock(&queue_lock_m);
            return;
        }
        RequestInfo temp_request = _dequeue();
        Close(temp_request.connfd);
    }
    _enqueue(request_info);
    cond_signal(&queue_remove_allowed_c);
    mutex_unlock(&queue_lock_m);
}
/*~~~~~~~~~~~~~~~~~~~~~ "drop_head" functions end. ~~~~~~~~~~~~~~~~~~~~~*/
