#ifndef _QUEUE_H_
#define _QUEUE_H_

#include "segel.h"


// >>>>>>>>>> MIGHT NEED TO ADD RequestInfo (p_pop_front*) (void) 
// >>>>>>>>>> AND SET IT ACCORDING TO schedalg SIMILARLY TO p_push_back
// >>>>>>>>>> (MIGHT NEED DIFFERENT KINDS OF _<schedalg>_pop_front()...)


/** 
*    RequestInfo -
*        An element of the queue.
*        Holds client's request information.
**/
typedef struct {
    int connfd;
    // >>>>>>>>>> NOTE: WE'LL HAVE TO ADD FIELDS FOR INFO 
} RequestInfo;



/* ~~~~~~~~~~~~~~~~~~ Queue interface functions ~~~~~~~~~~~~~~~~~~ */

/**
*   Initialize the queue with desired maximum number
*   of simultanious requests and overload handling policy.
*   Call only after checking that all command line 
*   arguments are valid!
**/
void queue_init(int max_requests, char* schedalg);

/**
*   Insert a new request at the back of the queue.
**/
void queue_push_back(int connfd);

/**
*   Remove the oldest request in the queue and
*   mark it as being handled (increase 
*   num_requests_in_handling).
**/
RequestInfo queue_pop_front(); // ---> IMPLEMENT

/**
*    When done handling a request, a worker thread
*    should call this function to decrease the number
*    of requests currently being handled
*    (num_requests_in_handling).
**/
void queue_dec_num_requests(); // ---> IMPLEMENT



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

#define cond_init(cond_addr) do {                                   \
    int _error_code = pthread_cond_init((cond_addr), NULL);         \
    if (_error_code != 0) {                                         \
        posix_error(_error_code, "pthread_cond_init error");        \
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



/* ~~~~~~~~~~~~~~~~~~ Queue variables ~~~~~~~~~~~~~~~~~~ */

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
*        ring buffer index for next write.
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

// >>>>>>>>>> IMPORTANT: MAKE SURE p_pop_front IS NEEDED AND SHOULD ACT SIMILARLY TO p_push_back 
/**
*     p_pop_front - 
*        Is used in queue_pop_front().
*        Will be set according to overload handling policy
*        to one of { _block_pop_front(), _drop_tail_pop_front(),
*                    _drop_head_pop_front(), ... }
**/
RequestInfo (p_pop_front*) (void);



/* ~~~~~~~~~~~~~~~~~~ Queue helper functions ~~~~~~~~~~~~~~~~~~ */

/* ~~~~~~~~~~~~~ Insertion: ~~~~~~~~~~~~~ */

/** 
*    Insert an element at the back of the queue.
**/ 
// >>>>>>>>>> NOTE: HELPER FUNCTION, PROBABLY SHOULDN'T USE ANY LOCKS (ONLY INSERT AN ELEMENT) 
void _enqueue(RequestInfo request_info);

void _block_push_back(RequestInfo request_info); // ---> IMPLEMENT

void _drop_tail_push_back(RequestInfo request_info); // ---> IMPLEMENT

void _drop_head_push_back(RequestInfo request_info); // ---> IMPLEMENT

// >>>>>>>>>> TODO: ADD FOR BONUS SCHEDALGS  
// void _block_flush_push_back(RequestInfo request_info);

// void _drop_random_push_back(RequestInfo request_info);

/* ~~~~~~~~~~~~~ Removal: ~~~~~~~~~~~~~ */

/** 
*    Remove the element at the front of the queue.
**/
// >>>>>>>>>> NOTE: HELPER FUNCTION, PROBABLY SHOULDN'T USE ANY LOCKS (ONLY REMOVE AN ELEMENT) 
RequestInfo _dequeue(); // ---> IMPLEMENT

RequestInfo _block_pop_front(); // ---> IMPLEMENT

RequestInfo _drop_tail_pop_front(); // ---> IMPLEMENT

RequestInfo _drop_head_pop_front(); // ---> IMPLEMENT

// >>>>>>>>>> TODO: ADD FOR BONUS SCHEDALGS 
// RequestInfo _block_flush_pop_front();

// RequestInfo _drop_random_pop_front();


#endif