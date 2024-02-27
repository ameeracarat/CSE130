#include "rwlock.h"
#include <stdlib.h>
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

} rwlock_t;

rwlock_t *rwlock_new(PRIORITY p, uint32_t n) {

    //only use if statement for writer/reader unlock

    //if reader --> return writers
    //if writers --> return reader

    //n way looks for num of waiting readers/writers and (n way readers if its greater than passed number)

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

    while (rw->active_writers > 0 || (rw->p == WRITERS && rw->waiting_writers > 0)) {
        pthread_cond_wait(&rw->reader_cond, &rw->mutex);
    }

    rw->active_readers++;
    rw->waiting_readers--;
 
    pthread_mutex_unlock(&rw->mutex);
}

void reader_unlock(rwlock_t *rw) {

    if (rw == NULL) {
        return;
    }

    pthread_mutex_lock(&rw->mutex);
    rw->active_readers--;

    if (rw->active_readers == 0 && (rw->p == WRITERS && rw->waiting_writers > 0)) {
        pthread_cond_wait(&rw->writer_cond, &rw->mutex);
    }

    rw->waiting_readers--;
    pthread_mutex_unlock(&rw->mutex);
}

void writer_lock(rwlock_t *rw) {
    if (rw == NULL)
        return;
}

void writer_unlock(rwlock_t *rw) {
    if (rw == NULL)
        return;
}
