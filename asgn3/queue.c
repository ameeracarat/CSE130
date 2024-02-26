#include "queue.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <semaphore.h>
#include <stdbool.h>

#include <stdint.h>
#include <pthread.h>
#include <unistd.h>

struct queue {
    void **elements;
    int front;
    int rear;
    int size;
    int capacity;
    sem_t sem_mutex;
    sem_t sem_full;
    sem_t sem_empty;
};

queue_t *queue_new(int size) {
    queue_t *q = (queue_t *) malloc(sizeof(queue_t));
    if (q == NULL)
        return NULL;

    q->elements = (void **) malloc(sizeof(void *) * size);
    if (q->elements == NULL) {
        free(q);
        return NULL;
    }

    q->front = 0;
    q->rear = 0;
    q->size = 0;
    q->capacity = size;
    int rc = 0;

    rc = sem_init(&(q->sem_mutex), 0, 1);
    assert(!rc);

    rc = sem_init(&(q->sem_full), 0, size);
    assert(!rc);

    rc = sem_init(&(q->sem_empty), 0, 0);
    assert(!rc);

    return q;
}

void queue_delete(queue_t **q) {
    if (q == NULL || *q == NULL)
        return;

    free((*q)->elements);
    free(*q);
    *q = NULL;
}

bool queue_push(queue_t *q, void *elem) {

    if (q == NULL)
        return false;

    sem_wait(&(q->sem_full));

    sem_wait(&(q->sem_mutex));

    q->elements[q->rear] = elem;
    q->rear = (q->rear + 1) % q->capacity;

    sem_post(&(q->sem_mutex));

    sem_post(&(q->sem_empty));

    return true;
}

bool queue_pop(queue_t *q, void **elem) {
    sem_wait(&(q->sem_empty));
    sem_wait(&(q->sem_mutex));

    *elem = q->elements[q->front];
    q->front = (q->front + 1) % q->capacity;

    sem_post(&(q->sem_mutex));
    sem_post(&(q->sem_full));

    return true;
}
