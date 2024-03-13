#include "../queue.h"

#define LOOP_COUNT 3

int main() {
    int curr_val = 0;

    queue_init(5, "block");

    while (1) {
        scanf("%d", &curr_val);
        if (curr_val < 0) {
            break;
        }

        for (int i = 0; i < LOOP_COUNT; ++i)
            queue_push_back(curr_val + i);

        for (int i = 0; i < LOOP_COUNT; ++i) { 
            RequestInfo r_info = queue_pop_front();

            queue_dec_num_requests();

            printf("%d\n", r_info.connfd);
        }
    }

    return 0;
}