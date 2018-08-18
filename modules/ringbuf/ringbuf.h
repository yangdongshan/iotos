#ifndef RINGBUF_H
#define RINGBUF_H

#ifdef __cplusplus
extern "C" {
#endif

#include <typedef.h>

typedef struct ringbuf {
  char *buf;
  uint32_t size;
  uint32_t tail;
  uint32_t head;
} ringbuf_t;

typedef struct ringbuf ringbuf_t;

ringbuf_t* ringbuf_init(uint32_t size);

int ringbuf_destory(ringbuf_t *rb);

void ringbuf_queue(ringbuf_t *buffer, char data);

void ringbuf_queue_arr(ringbuf_t *buffer, const char *data, uint32_t size);

int ringbuf_dequeue(ringbuf_t *buffer, char *data);

int ringbuf_dequeue_arr(ringbuf_t *buffer, char *data, uint32_t len);

int ringbuf_peek(ringbuf_t *buffer, char *data, uint32_t index);


#ifdef __cplusplus
}
#endif

#endif /* RINGBUF_H */
