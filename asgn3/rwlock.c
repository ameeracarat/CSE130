#include "rwlock.h"
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

typedef struct rwlock {

    PRIORITY p;

    pthread_mutex_t mutex;
    pthread_cond_t reader_cond;
    pthread_cond_t writer_cond;

    int active_readers;
    int active_writers;
    int waiting_readers;
    int waiting_writers;
    int nway;
    int count;

} rwlock_t;

rwlock_t *rwlock_new(PRIORITY p, uint32_t n) {

    rwlock_t *rw = (rwlock_t *) malloc(sizeof(rwlock_t));

    pthread_mutex_init(&(rw->mutex), NULL);
    pthread_cond_init(&(rw->reader_cond), NULL);
    pthread_cond_init(&(rw->writer_cond), NULL);

    rw->active_readers = 0;
    rw->active_writers = 0;
    rw->waiting_readers = 0;
    rw->waiting_writers = 0;
    rw->p = p;
    rw->nway = n;

    //rw->flag = 3;
    rw->count = n;

    return rw;
}

void rwlock_delete(rwlock_t **rw) {
    if (rw == NULL || *rw == NULL)
        return;

    pthread_mutex_destroy(&((*rw)->mutex));
    pthread_cond_destroy(&((*rw)->reader_cond));
    pthread_cond_destroy(&((*rw)->writer_cond));

    free(*rw);
    *rw = NULL;
}

void reader_lock(rwlock_t *rw) {
    if (rw == NULL) {
        return;
    }
    pthread_mutex_lock(&rw->mutex);
    rw->waiting_readers++;
    //fprintf(stderr, "waiting_readers: %d\n", rw->waiting_readers);

    if (rw->p == READERS) {
        while (rw->active_writers > 0) {
            pthread_cond_wait(&rw->reader_cond, &rw->mutex);
        }

    }

    else if (rw->p == WRITERS) {

        while (rw->active_writers > 0 || rw->waiting_writers > 0) {

            pthread_cond_wait(&rw->reader_cond, &rw->mutex);
        }
    } else if (rw->p == N_WAY) {
        while ((rw->count <= 0 && rw->waiting_writers > 0) || rw->active_writers > 0) {
            pthread_cond_wait(&rw->reader_cond, &rw->mutex);
        }
    }

    rw->waiting_readers--;
    rw->active_readers++;
    rw->count--;

    // fprintf(stderr, "waiting_readers: %d\n", rw->waiting_readers);
    // fprintf(stderr, "active_readers: %d\n", rw->active_readers);

    pthread_mutex_unlock(&rw->mutex);
}

void reader_unlock(rwlock_t *rw) {

    if (rw == NULL) {
        return;
    }

    pthread_mutex_lock(&rw->mutex);
    rw->active_readers--;
    //  fprintf(stderr, "active_readers: %d\n", rw->active_readers);

    if (rw->waiting_readers > 0) {
        pthread_cond_broadcast(&rw->reader_cond);
    }
    if (rw->waiting_writers > 0) {
        pthread_cond_broadcast(&rw->writer_cond);
    }

    pthread_mutex_unlock(&rw->mutex);
}

void writer_lock(rwlock_t *rw) {
    if (rw == NULL)
        return;

    pthread_mutex_lock(&rw->mutex);
    rw->waiting_writers++;
    //  fprintf(stderr, "waiting_writers: %d\n", rw->waiting_writers);

    if (rw->p == READERS) {
        while (rw->active_writers > 0 || (rw->active_readers > 0) || rw->waiting_readers > 0) {
            pthread_cond_wait(&rw->writer_cond, &rw->mutex);
        }
    } else if (rw->p == WRITERS) {

        while (rw->active_writers > 0 || (rw->active_readers > 0)) {

            pthread_cond_wait(&rw->writer_cond, &rw->mutex);
        }
    } else if (rw->p == N_WAY) {
        while (rw->active_writers > 0 || rw->active_readers > 0
               || (rw->waiting_readers > 0 && rw->count > 0)) {
            pthread_cond_wait(&rw->writer_cond, &rw->mutex);
        }
    }
    rw->count = rw->nway;

    rw->waiting_writers--;
    rw->active_writers++;
    // fprintf(stderr, "waiting_writers: %d\n", rw->waiting_writers);
    // fprintf(stderr, "active_writers: %d\n", rw->active_writers);

    pthread_mutex_unlock(&rw->mutex);
}

void writer_unlock(rwlock_t *rw) {
    if (rw == NULL) {
        return;
    }

    pthread_mutex_lock(&rw->mutex);

    rw->active_writers--;
    // fprintf(stderr, "active_writers: %d\n", rw->active_writers);
    // fprintf(stderr, "waiting_writers: %d\n", rw->waiting_writers);

    if (rw->waiting_readers > 0) {
        pthread_cond_broadcast(&rw->reader_cond);
    }
    if (rw->waiting_writers > 0) {
        pthread_cond_broadcast(&rw->writer_cond);
    }

    pthread_mutex_unlock(&rw->mutex);
}
