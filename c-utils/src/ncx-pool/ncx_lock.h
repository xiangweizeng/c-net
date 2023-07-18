#ifndef _NCX_LOCK_H_
#define _NCX_LOCK_H_

#include <pthread.h>

typedef struct {
    pthread_mutex_t mutex;
} ncx_shmtx_t;

static ncx_shmtx_t ncx_shmtx_create()
{
    ncx_shmtx_t shmtx;
    pthread_mutex_init(&shmtx.mutex, NULL);
    return shmtx;
}

static void ncx_shmtx_release(ncx_shmtx_t *shmtx)
{
    pthread_mutex_destroy(&(shmtx->mutex));
}

#ifdef K210
#define ncx_shmtx_lock(x)   {  /*void*/ }
#define ncx_shmtx_unlock(x) {  /*void*/ }
#else
#define ncx_shmtx_lock(x)   {  pthread_mutex_lock(&x.mutex); }
#define ncx_shmtx_unlock(x) {   pthread_mutex_unlock(&x.mutex); }
#endif

#endif
