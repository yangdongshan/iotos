#include <typedef.h>
#include <mm.h>
#include "ringbuf.h"


static inline bool ringbuf_is_empty(ringbuf_t *rb)
{
    return (rb->head == rb->tail);
}

static inline bool ringbuf_is_full(ringbuf_t *rb)
{
    return ((rb->head - rb->tail) & (rb->size - 1)) == (rb->size - 1);
}

static inline uint32_t ringbuf_free_items(ringbuf_t *rb)
{
    return ((rb->head - rb->tail) & (rb->size - 1));
}

ringbuf_t* ringbuf_init(uint32_t size)
{
    ringbuf_t *rb = (ringbuf_t*)malloc(sizeof(ringbuf_t));
    if (rb == NULL)
        return NULL;

    char *buf = (char*)malloc(size);
    if (buf == NULL)
        return NULL;

    rb->buf = buf;
    rb->tail = 0;
    rb->head = 0;
    rb->size = size;

    return rb;
}

int ringbuf_destory(ringbuf_t *rb)
{
    int ret = 0;

    if (rb) {
        if (rb->buf)
            free(rb->buf);

        free(rb);
        ret = 1;
    }

    return ret;
}

void ringbuf_queue(ringbuf_t *rb, char data)
{
    // if the ringbuf is full, overwrite it
    if(ringbuf_is_full(rb)) {
        rb->tail = ((rb->tail + 1) & (rb->size - 1));
    }

    rb->buf[rb->head] = data;
    rb->head = ((rb->head + 1) & (rb->size - 1));
}

void ringbuf_queue_arr(ringbuf_t *rb, const char *data, uint32_t size)
{
    uint32_t i;
    for(i = 0; i < size; i++) {
        ringbuf_queue(rb, data[i]);
    }
}

int ringbuf_dequeue(ringbuf_t *rb, char *data)
{
    if(ringbuf_is_empty(rb)) {
        return 0;
    }

    *data = rb->buf[rb->tail];
    rb->tail = ((rb->tail + 1) & (rb->size - 1));

    return 1;
}

int ringbuf_dequeue_arr(ringbuf_t *rb, char *data, uint32_t len)
{
    if(ringbuf_is_empty(rb)) {
        return 0;
    }

    char *data_ptr = data;
    uint32_t cnt = 0;
    while((cnt < len) && ringbuf_dequeue(rb, data_ptr)) {
        cnt++;
        data_ptr++;
    }

    return cnt;
}

int ringbuf_peek(ringbuf_t *rb, char *data, uint32_t index)
{
    if(index >= ringbuf_free_items(rb)) {
        return 0;
    }

    uint32_t data_index = ((rb->tail + index) & (rb->size - 1));
    *data = rb->buf[data_index];

    return 1;
}

