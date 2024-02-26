#include "rwlock.h"
#include <stdlib.h>
#include <pthread.h>

typedef struct rwlock {

    PRIORITY p;

    pthread_mutex_t mutex;
    pthread_cond_t reader_cond;
    pthread_cond_t writer_cond;

    int readers;
    int writers;
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

    int rc = 0;

    rc = pthread_mutex_init(&rw->mutex, NULL);
    rc = pthread_mutex_init(&rw->reader_cond, NULL);
    rc = pthread_mutex_init(&rw->writer_cond, NULL);

    rw->readers = 0;
    rw->writers = 0;
    rw->waiting_readers = 0;
    rw->waiting_writers = 0;
    rw->priority = p;
    rw->nway = n;

    return rw;

    // if (rw == NULL)
    //     return NULL;

    // rw->priority = p;
    // rw->n = n;

    // if (p == N_WAY && n == 0) {
    //     free(rw);
    //     return NULL;
    // }

    // if (pthread_rwlock_init(&(rw->lock), NULL) != 0) {
    //     free(rw);
    //     return NULL;
    // }

    // return rw;
}

void rwlock_delete(rwlock_t **rw) {
    if (rw == NULL || *rw == NULL)
        return;

    pthread_mutex_destroy(&((*rw)->mutex));
    pthread_mutex_destroy(&((*rw)->reader_cond));
    pthread_mutex_destroy(&((*rw)->writer_cond));

    free(*rw);
    *rw = NULL;
}

void reader_lock(rwlock_t *rw) {
    if (rw == NULL) {
        return;
    }
    pthread_mutex_lock(&rw->mutex);
    rw->waiting_readers++;

    while (rw->writers > 0 || (rw->priority == WRITERS && rw->waiting_writers > 0)) {
        pthread_cond_wait(&waiting_writers);
    }
    rw->readers++;

    rw->waiting_readers--;
    pthread_mutex_unlock(&rw->mutex);
}

void reader_unlock(rwlock_t *rw) {
    if (rw == NULL)
        return;

    pthread_mutex_lock(&rw->mutex);
    rw->waiting_readers++;

    while (rw->writers > 0 || (rw->priority WRITERS && rw->waiting_writers > 0)) {
        pthread_cond_wait(&waiting_writers);
    }

    rw->waiting_readers--;
    pthread_mutex_unlock(&rw->mutex);
}

void writer_lock(rwlock_t *rw) {
    if (rw == NULL)
        return;

    if (rw->priority == WRITERS) {
        pthread_rwlock_wrlock(&(rw->lock));
    } else if (rw->priority == N_WAY) {
        // Implement logic for N_WAY priority
    }
}

void writer_unlock(rwlock_t *rw) {
    if (rw == NULL)
        return;

    pthread_rwlock_unlock(&(rw->lock));
}
