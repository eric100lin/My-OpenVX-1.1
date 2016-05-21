/*
 * Copyright (c) 2012-2016 The Khronos Group Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and/or associated documentation files (the
 * "Materials"), to deal in the Materials without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Materials, and to
 * permit persons to whom the Materials are furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Materials.
 *
 * THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * MATERIALS OR THE USE OR OTHER DEALINGS IN THE MATERIALS.
 */

#include <vx_internal.h>

#define BILLION (1000000000)

vx_bool vxCreateSem(vx_sem_t *sem, vx_uint32 count)
{
#if defined(VX_PTHREAD_SEMAPHORE)
    int ret0, ret1;
    sem->count = (int)count;
    ret0 = pthread_cond_init(&sem->cond, 0);
    ret1 = pthread_mutex_init(&sem->mutex,0);
    VX_PRINT(VX_ZONE_OSAL, "sem_init(%p,%u)=>%d,%d errno=%d\n",sem,count,ret0,ret1,errno);
    if (ret0 == 0 && ret1 == 0)
#elif defined(__linux__) || defined(__ANDROID__) || defined(__QNX__) || defined(__CYGWIN__)
    int ret = sem_init(sem, 0, count);
    VX_PRINT(VX_ZONE_OSAL, "sem_init(%p,%u)=>%d errno=%d\n",sem,count,ret,errno);
    if (ret == 0)
#elif defined(_WIN32) || defined(UNDER_CE)
    *sem = CreateSemaphore(NULL, count, count, NULL);
    if (*sem)
#endif
        return vx_true_e;
    else
        return vx_false_e;
}

void vxDestroySem(vx_sem_t *sem)
{
#if defined(VX_PTHREAD_SEMAPHORE)
    int ret0 = -1, ret1 = -1;
    ret0 = pthread_cond_destroy(&sem->cond);
    ret1 = pthread_mutex_destroy(&sem->mutex);
    sem->count = -1;
    VX_PRINT(VX_ZONE_OSAL, "sem_destroy(%p)=>%d,%d errno=%d\n",sem,ret0,ret1,errno);
#elif defined(__linux__) || defined(__ANDROID__) || defined(__QNX__) || defined(__CYGWIN__)
    sem_destroy(sem);
#elif defined(_WIN32) || defined(UNDER_CE)
    CloseHandle(*sem);
#endif
}

vx_bool vxSemPost(vx_sem_t *sem)
{
#if defined(VX_PTHREAD_SEMAPHORE)
    int ret0 = -1, ret1 = -1, ret2 = -1;
    if (0 <= sem->count)
    {
        ret0 = pthread_mutex_lock(&sem->mutex);

        sem->count++;
        // DON'T ENABLE THIS CONDITION: if (sem->count == 1)
        // or enable with broadcast!
        {
            ret1 = pthread_cond_signal(&sem->cond);
        }

        ret2 = pthread_mutex_unlock(&sem->mutex);
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "sem_post(%p): LOCK ERROR\n",sem);
        return vx_false_e;
    }
    VX_PRINT(VX_ZONE_OSAL, "sem_post(%p)=%d,%d,%d v=%d errno=%d\n",sem,ret0,ret1,ret2,sem->count,errno);
    if (1 > 0)
#elif defined(__linux__) || defined(__ANDROID__) || defined(__QNX__) || defined(__CYGWIN__)
    int ret = sem_post(sem);
    VX_PRINT(VX_ZONE_OSAL, "sem_post(%p)=>%d errno=%d\n", sem, ret, errno);
    if (ret == 0)
#elif defined(_WIN32) || defined(UNDER_CE)
    if (ReleaseSemaphore(*sem, 1, NULL) == TRUE)
#endif
        return vx_true_e;
    else
        return vx_false_e;
}

vx_bool vxSemWait(vx_sem_t *sem)
{
#if defined(VX_PTHREAD_SEMAPHORE)
    vx_bool res = vx_true_e;
    VX_PRINT(VX_ZONE_OSAL, "sem_wait(%p) ... v=%d errno=%d\n",sem,sem->count,errno);
    pthread_mutex_lock(&sem->mutex);
    while (0 == sem->count)
    {
        if (0 != pthread_cond_wait(&sem->cond, &sem->mutex))
        {
            VX_PRINT(VX_ZONE_ERROR, "sem_wait(%p): LOCK ERROR\n",sem);
            res = vx_false_e;
        }
    }
    if (res == vx_true_e)
    {
        sem->count--;
    }
    pthread_mutex_unlock(&sem->mutex);
    VX_PRINT(VX_ZONE_OSAL, "sem_wait(%p)=>%d v=%d errno=%d\n",sem,res == vx_true_e ? 0 : -1,sem->count,errno);
    if (res != vx_false_e)
#elif defined(__linux__) || defined(__ANDROID__) || defined(__QNX__) || defined(__CYGWIN__)
    int ret = sem_wait(sem);
    VX_PRINT(VX_ZONE_OSAL, "sem_wait(%p)=>%d errno=%d\n",sem,ret,errno);
    if (ret == 0)
#elif defined(_WIN32) || defined(UNDER_CE)
    if (WaitForSingleObject(*sem, INFINITE) == WAIT_OBJECT_0)
#endif
        return vx_true_e;
    else
        return vx_false_e;
}

vx_bool vxSemTryWait(vx_sem_t *sem)
{
#if defined(VX_PTHREAD_SEMAPHORE)
    vx_bool res = vx_true_e;
    VX_PRINT(VX_ZONE_OSAL, "sem_trywait(%p) ... v=%d errno=%d\n",sem,sem->count,errno);
    pthread_mutex_lock(&sem->mutex);
    if (0 == sem->count)
    {
        res = vx_false_e;
    }
    else
    {
        sem->count--;
    }
    pthread_mutex_unlock(&sem->mutex);
    VX_PRINT(VX_ZONE_OSAL, "sem_trywait(%p)=>%d v=%d errno=%d\n",sem,res != vx_false_e ? 0 : -1, sem->count,errno);
    if (res != vx_false_e)
#elif defined(__linux__) || defined(__ANDROID__) || defined(__QNX__) || defined(__CYGWIN__)
    int ret = sem_trywait(sem);
    VX_PRINT(VX_ZONE_OSAL, "sem_trywait(%p)=>%d errno=%d\n",sem,ret,errno);
    if (ret == 0)
#elif defined(_WIN32) || defined(UNDER_CE)
    if (WaitForSingleObject(*sem, 0) == WAIT_OBJECT_0)
#endif
        return vx_true_e;
    else
        return vx_false_e;
}

vx_bool vxDeinitEvent(vx_event_t *e)
{
#if defined(__linux__) || defined(__ANDROID__) || defined(__QNX__) || defined(__CYGWIN__) || defined(__APPLE__)
    int err = 0;
    do {
        err = pthread_cond_destroy(&e->cond);
        if (err == EBUSY) {
            pthread_mutex_lock(&e->mutex);
            e->set = vx_false_e;
            pthread_cond_broadcast(&e->cond);
            pthread_mutex_unlock(&e->mutex);
        }
    } while (err != 0);
    pthread_condattr_destroy(&e->attr);
    pthread_mutex_destroy(&e->mutex);
    return vx_true_e;
#elif defined(_WIN32) || defined(UNDER_CE)
    return CloseHandle(*e);
#endif
}

vx_bool vxInitEvent(vx_event_t *e, vx_bool autoreset)
{
#if defined(__linux__) || defined(__ANDROID__) || defined(__QNX__) || defined(__CYGWIN__) || defined(__APPLE__)
    int err = 0;
    err |= pthread_mutex_init(&e->mutex, NULL);
    err |= pthread_condattr_init(&e->attr);
    err |= pthread_cond_init(&e->cond, &e->attr);
    e->set = vx_false_e;
    e->autoreset = autoreset;
    if (err == 0)
#elif defined(_WIN32) || defined(UNDER_CE)
    BOOL manual = TRUE;
    if (autoreset)
        manual = FALSE;
    *e = CreateEvent(NULL, manual, FALSE, NULL);
    if (*e != NULL)
#endif
        return vx_true_e;
    else
        return vx_false_e;
}

#if defined(__linux__) || defined(__ANDROID__) || defined(__QNX__) || defined(__CYGWIN__) || defined(__APPLE__)
vx_bool vxWaitEventInternal(vx_event_t *e, vx_uint32 ms)
{
    int retcode = 0;
    vx_bool ret = vx_false_e;
    if (ms < VX_INT_FOREVER)
    {
        struct timespec time_spec;
        struct timeval now;
        gettimeofday(&now, NULL);
        time_spec.tv_sec = now.tv_sec + (ms / 1000);
        time_spec.tv_nsec = (now.tv_usec * 1000) + ((ms%1000) * 1000000);
        if (time_spec.tv_nsec > BILLION) {
            time_spec.tv_sec += 1;
            time_spec.tv_nsec -= BILLION;
        }
        retcode = pthread_cond_timedwait(&e->cond, &e->mutex, &time_spec);
    }
    else
        retcode = pthread_cond_wait(&e->cond, &e->mutex);
    if (retcode == ETIMEDOUT && e->set == vx_false_e)
        ret = vx_false_e;
    else if (retcode == ETIMEDOUT && e->set == vx_true_e)
        ret = vx_true_e;
    else if (retcode == 0 && e->set == vx_false_e)
        ret = vx_false_e;
    else if (retcode == 0 && e->set == vx_true_e)
        ret = vx_true_e;
    return ret;
}
#endif

vx_bool vxWaitEvent(vx_event_t *e, vx_uint32 timeout)
{
    vx_bool ret = vx_false_e;
#if defined(__linux__) || defined(__ANDROID__) || defined(__QNX__) || defined(__CYGWIN__) || defined(__APPLE__)
    pthread_mutex_lock(&e->mutex);
    if (e->autoreset == vx_false_e) {
        if (e->set == vx_false_e)
            ret = vxWaitEventInternal(e, timeout);
        else
            ret = vx_true_e;
    } else {
        ret = vxWaitEventInternal(e, timeout);
        if (ret == vx_true_e && e->set == vx_true_e)
            e->set = vx_false_e;
        else if (ret == vx_true_e && e->set == vx_false_e)
            ret = vx_false_e;
    }
    pthread_mutex_unlock(&e->mutex);
    return ret;
#elif defined(_WIN32) || defined(UNDER_CE)
    DWORD status = WaitForSingleObject(*e, timeout);
    if (status == WAIT_OBJECT_0)
        ret = vx_true_e;
#endif
    return ret;
}

vx_bool vxSetEvent(vx_event_t *e)
{
#if defined(__linux__) || defined(__ANDROID__) || defined(__QNX__) || defined(__CYGWIN__) || defined(__APPLE__)
    int err = 0;
    pthread_mutex_lock(&e->mutex);
    e->set = vx_true_e;
    err = pthread_cond_broadcast(&e->cond);
    pthread_mutex_unlock(&e->mutex);
    if (err == 0)
        return vx_true_e;
    else
        return vx_false_e;
#elif defined(_WIN32) || defined(UNDER_CE)
    return (vx_bool)SetEvent(*e);
#endif
}

vx_bool vxResetEvent(vx_event_t *e)
{
#if defined(__linux__) || defined(__ANDROID__) || defined(__QNX__) || defined(__CYGWIN__) || defined(__APPLE__)
    pthread_mutex_lock(&e->mutex);
    e->set = vx_false_e;
    pthread_mutex_unlock(&e->mutex);
    return vx_true_e;
#elif defined(_WIN32) || defined(UNDER_CE)
    return (vx_bool)ResetEvent(*e);
#endif
}

vx_bool vxJoinThread(vx_thread_t thread, vx_value_t *value)
{
    vx_bool joined = vx_false_e;
    if (thread)
    {
#if defined(__linux__) || defined(__ANDROID__) || defined(__QNX__) || defined(__CYGWIN__) || defined(__APPLE__)
        if (pthread_join(thread, (void **)value) == 0)
            joined = vx_true_e;
#elif defined(_WIN32) || defined(UNDER_CE)
        DWORD status = WaitForSingleObject(thread, INFINITE);
        if (status == WAIT_OBJECT_0) {
            GetExitCodeThread(thread, (LPDWORD)&value);
            joined = vx_true_e;
        }
#endif
    }
    return joined;
}

void vxSleepThread(vx_uint32 milliseconds)
{
#if defined(__linux__) || defined(__ANDROID__) || defined(__QNX__) || defined(__CYGWIN__)
    struct timespec rtsp;
    rtsp.tv_sec = 0;
    rtsp.tv_nsec = milliseconds * 100000;
    nanosleep(&rtsp, NULL);
#elif defined(_WIN32) || defined(UNDER_CE)
    Sleep(milliseconds);
#endif
}

vx_thread_t vxCreateThread(vx_thread_f func, void *arg)
{
    vx_thread_t thread = 0;
#if defined(__linux__) || defined(__ANDROID__) || defined(__QNX__) || defined(__CYGWIN__) || defined(__APPLE__)
    pthread_create(&thread, NULL, (pthread_f)func, arg);
#elif defined(_WIN32) || defined(UNDER_CE)
    thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)func, arg, CREATE_SUSPENDED, NULL);
    if (thread)
        ResumeThread(thread);
#endif
    return thread;
}


void vxDestroyThreadpool(vx_threadpool_t **ppool)
{
    vx_threadpool_t *pool = (ppool ? *ppool : NULL);
    if (pool)
    {
        uint32_t i;
        for (i = 0u; i < pool->numWorkers; i++)
        {
            vx_value_t ret;
            vxPopQueue(pool->workers[i].queue);
            vxJoinThread(pool->workers[i].handle, &ret);
            vxStopCapture(&pool->workers[i].perf);
            pool->workers[i].handle = 0;
            vxDestroyQueue(&pool->workers[i].queue);
            pool->workers[i].queue = (vx_queue_t *)NULL;
        }
        free(pool->workers);
        pool->workers = (vx_threadpool_worker_t *)NULL;
        vxDestroySem(&pool->sem);
        vxDeinitEvent(&pool->completed);
        free(pool);
        *ppool = NULL;
    }
}

static vx_value_t vxWorkerThreadpool(void *arg)
{
    vx_threadpool_worker_t *pool_worker = (vx_threadpool_worker_t *)arg;
    vx_bool ret = vx_false_e;

    /* capture the launch latency */
    vxStopCapture(&pool_worker->perf);

    VX_PRINT(VX_ZONE_OSAL, "Threadpool worker %p active, waiting on queue!\n", arg);

    /*! \bug assign this thread to the next available core */
    //thread_nextaffinity();

    vxInitPerf(&pool_worker->perf); // reset
    vxStartCapture(&pool_worker->perf);

    while (vxReadQueue(pool_worker->queue, &pool_worker->data) == vx_true_e)
    {
        vx_threadpool_f function = pool_worker->function;
        VX_PRINT(VX_ZONE_OSAL, "Worker received workitem!\n");
        pool_worker->active = vx_true_e;
        vxStopCapture(&pool_worker->perf);
        ret = function(pool_worker); /* <=== WORK IS DONE HERE */
        vxSemWait(&pool_worker->pool->sem);
        pool_worker->pool->numCurrentItems--;
        if (pool_worker->pool->numCurrentItems <= 0)
        {
            vxSetEvent(&pool_worker->pool->completed);
        }
        vxSemPost(&pool_worker->pool->sem);
        vxStartCapture(&pool_worker->perf);
        pool_worker->active = vx_false_e;

        // if (ret == vx_false_e)
        //     break;
    }
    VX_PRINT(VX_ZONE_OSAL, "Worker exiting!\n");
    return (vx_value_t)ret;
}

vx_threadpool_t *vxCreateThreadpool(vx_uint32 numThreads,
                                    vx_uint32 numWorkItems,
                                    vx_size sizeWorkItem,
                                    vx_threadpool_f worker,
                                    void *arg)
{
    vx_threadpool_t *pool = VX_CALLOC(vx_threadpool_t);
    void *tmp_arg = arg;
    if (pool)
    {
        uint32_t i;
        vxCreateSem(&pool->sem, 1u);
        pool->numWorkers = numThreads;
        pool->numWorkItems = numWorkItems;
        pool->sizeWorkItem = (uint32_t)sizeWorkItem;
        vxInitEvent(&pool->completed, vx_false_e);
        pool->workers = (vx_threadpool_worker_t *)calloc(pool->numWorkers, sizeof(vx_threadpool_worker_t));
        if (pool->workers)
        {
            VX_PRINT(VX_ZONE_OSAL, "Created %u threadpool workers\n", pool->numWorkers);
            for (i = 0u; i < pool->numWorkers; i++)
            {
                vx_threadpool_worker_t *pool_worker = &pool->workers[i];
                pool_worker->queue = vxCreateQueue();
                pool_worker->index = i;
                pool_worker->arg = tmp_arg;
                pool_worker->function = worker;
                pool_worker->pool = pool; /* back reference to top level info */
                vxInitPerf(&pool_worker->perf);
                vxStartCapture(&pool_worker->perf); /* capture the launch latency */
                pool_worker->handle = vxCreateThread(&vxWorkerThreadpool, pool_worker);
            }
        }
    }
    return pool;
}

vx_bool vxIssueThreadpool(vx_threadpool_t *pool, vx_value_set_t workitems[], uint32_t numWorkItems)
{
    uint32_t i;
    vx_bool wrote = vx_false_e;

    vxSemWait(&pool->sem);
    vxResetEvent(&pool->completed); /* we're going to have items to work on, so clear the event */
    for (i = 0u; i < numWorkItems; i++)
    {
        uint32_t index = 0xFFFFFFFFu;
        uint32_t count = 0u; /* cycle-detector */
        /* issue the work to the next workers_get, but don't wait if it's full */
        do {
            count++;
            index = pool->nextWorkerIndex;
            pool->nextWorkerIndex = (pool->nextWorkerIndex + 1u) % pool->numWorkers;
            pool->numCurrentItems++;
            wrote = vxWriteQueue(pool->workers[index].queue, &workitems[i]);
            if (wrote == vx_false_e)
            {
                pool->numCurrentItems--;
            }
        } while ((wrote == vx_false_e) && (count < pool->numWorkers));
        /* there's too much work to do, there's an overflow condition. some of the work may have been issued, others not. */
        if (wrote == vx_false_e)
        {
            break;
        }
    }

    /* if none of the writes worked for whatever reason, and we didn't have any previous pending work (how?) then set the completed event */
    if (pool->numCurrentItems == 0)
    {
        vxSetEvent(&pool->completed);
    }
    vxSemPost(&pool->sem);
    return wrote;
}

vx_bool vxCompleteThreadpool(vx_threadpool_t *pool, vx_bool blocking)
{
    vx_bool ret = vx_false_e;
    if (blocking)
    {
        ret = vxWaitEvent(&pool->completed, VX_INT_FOREVER);
    }
    else
    {
        if (pool->numCurrentItems == 0)
        {
            ret = vx_true_e;
        }
    }
    return ret;
}


vx_uint64 vxCaptureTime()
{
    vx_uint64 cap = 0;
#if defined(__linux__) || defined(__ANDROID__) || defined(__QNX__) || defined(__CYGWIN__)
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    cap = (vx_uint64)((vx_uint64)t.tv_nsec + (vx_uint64)t.tv_sec*BILLION);
#elif defined(__APPLE__)
    cap = mach_absolute_time();
#elif defined(_WIN32) || defined(UNDER_CE)
    LARGE_INTEGER t;
    QueryPerformanceCounter(&t);
    cap = (vx_uint64)t.QuadPart;
#endif
    return cap;
}

vx_uint64 vxGetClockRate()
{
    vx_uint64 freq = 0;
#if defined(__linux__) || defined(__ANDROID__) || defined(__QNX__) || defined(__CYGWIN__)
    struct timespec t;
    clock_getres(CLOCK_MONOTONIC, &t);
    freq = (vx_uint64)(BILLION/t.tv_nsec);
#elif defined(__APPLE__)
    freq = BILLION;
#elif defined(_WIN32) || defined(UNDER_CE)
    LARGE_INTEGER f;
    QueryPerformanceFrequency(&f);
    freq = (vx_uint64)f.QuadPart;
#endif
    return freq;
}

void vxStartCapture(vx_perf_t *perf)
{
    perf->beg = vxCaptureTime();
}

vx_float32 vxTimeToMS(vx_uint64 c) {
#define NS_PER_MSEC  (1000000.0f)
    return (vx_float32)c/NS_PER_MSEC;
}

void vxStopCapture(vx_perf_t *perf)
{
#if defined(__APPLE__)
    static mach_timebase_info_data_t    sTimebaseInfo;
#endif
    perf->end = vxCaptureTime();
    perf->tmp = perf->end - perf->beg;
#if defined(__APPLE__)
    if (sTimebaseInfo.denom == 0) {
        (void) mach_timebase_info(&sTimebaseInfo);
    }
    perf->tmp = perf->tmp * sTimebaseInfo.numer / sTimebaseInfo.denom;
#endif
    perf->sum += perf->tmp;
    perf->num++;
    perf->avg = perf->sum / perf->num;
    perf->min = (perf->min < perf->tmp ? perf->min : perf->tmp);
    perf->max = (perf->max > perf->tmp ? perf->max : perf->tmp);
}

void vxPrintPerf(vx_perf_t *perf) {
    VX_PRINT(VX_ZONE_PERF, "beg:"VX_FMT_TIME"ms end:"VX_FMT_TIME"ms tmp:"VX_FMT_TIME"ms sum:"VX_FMT_TIME"ms num:%d avg:"VX_FMT_TIME"ms\n",
             vxTimeToMS(perf->beg),
             vxTimeToMS(perf->end),
             vxTimeToMS(perf->tmp),
             vxTimeToMS(perf->sum),
             (int)perf->num,
             vxTimeToMS(perf->avg));
}

void vxInitPerf(vx_perf_t *perf)
{
    memset(perf, 0, sizeof(vx_perf_t));
    perf->min = UINT64_MAX;
	perf->max = 0;
}

void vxPrintQueue(vx_queue_t *q)
{
    vx_uint32 i;
    VX_PRINT(VX_ZONE_OSAL, "Queue: %p, lock=%p s,e=[%d,%d] popped=%s\n",q, &q->lock, q->start_index, q->end_index, (q->popped?"yes":"no"));
    for (i = 0; i < VX_INT_MAX_QUEUE_DEPTH; i++)
    {
        if (q->data[i])
        {
            VX_PRINT(VX_ZONE_OSAL, "[%u] = {" VX_FMT_VALUE ", " VX_FMT_VALUE ", " VX_FMT_VALUE "}\n", i, q->data[i]->v1, q->data[i]->v2, q->data[i]->v3);
        }
    }
}

void vxInitQueue(vx_queue_t *q)
{
    if (q)
    {
        memset(q->data, 0, sizeof(q->data));
        q->start_index = 0;
        q->end_index = -1;
        vxCreateSem(&q->lock, 1);
        vxInitEvent(&q->readEvent, vx_false_e);
        vxInitEvent(&q->writeEvent, vx_false_e);
        vxSetEvent(&q->writeEvent);
    }
}

void vxDestroyQueue(vx_queue_t **pq)
{
    vx_queue_t *q = (pq ? *pq : NULL);
    if (q)
    {
        vxDeinitQueue(q);
        free(q);
        *pq = NULL;
    }
}

vx_queue_t *vxCreateQueue(vx_uint32 numItems, vx_size itemSize) {
    vx_queue_t *q = VX_CALLOC(vx_queue_t);
    if (q) vxInitQueue(q);
    return q;
}

vx_bool vxWriteQueue(vx_queue_t *q, vx_value_set_t *data)
{
    vx_bool wrote = vx_false_e;
    if (q)
    {
        // wait for the queue to be writeable
        VX_PRINT(VX_ZONE_OSAL, "About to wait on queue %p\n", q);
        while (vxWaitEvent(&q->writeEvent, VX_INT_FOREVER) == vx_true_e)
        {
            VX_PRINT(VX_ZONE_OSAL, "Signalled!\n");
            vxSemWait(&q->lock);
            if (q->popped == vx_false_e)
            {
                // cause other writers to block
                vxResetEvent(&q->writeEvent);
                // add the value to the data array if space is available
                if (q->start_index != q->end_index)
                {
                    if (q->end_index == -1) // empty
                        q->end_index = q->start_index;
                    q->data[q->end_index] = data;
                    q->end_index = (q->end_index + 1)%VX_INT_MAX_QUEUE_DEPTH;
                    wrote = vx_true_e;
                    // cause other writers to unblock.
                    if (q->start_index != q->end_index)
                        vxSetEvent(&q->writeEvent);
                    vxPrintQueue(q);
                }
                else
                {
                    // two writers may have raced to enter...
                    // the second may have a full queue here...
                }

                if (q->end_index != -1)
                    vxSetEvent(&q->readEvent);
            }
            vxSemPost(&q->lock);
            if (q->popped == vx_true_e || wrote == vx_true_e)
                break;
        }
    }
    return wrote;
}

vx_bool vxReadQueue(vx_queue_t *q, vx_value_set_t **data)
{
    vx_bool red = vx_false_e;
    if (q)
    {
        VX_PRINT(VX_ZONE_OSAL, "About to wait on queue %p\n", q);
        while (vxWaitEvent(&q->readEvent, VX_INT_FOREVER) == vx_true_e)
        {
            VX_PRINT(VX_ZONE_OSAL, "Signalled!\n");
            vxSemWait(&q->lock);
            if (q->popped == vx_false_e)
            {
                if (q->end_index != -1) // not empty
                {
                    *data = q->data[q->start_index];
                    q->data[q->start_index] = NULL;
                    q->start_index = (q->start_index + 1)%VX_INT_MAX_QUEUE_DEPTH;
                    red = vx_true_e;
                    if (q->start_index == q->end_index) // wrapped to empty
                    {
                        vxResetEvent(&q->readEvent);
                        q->end_index = -1;
                    }
                    vxPrintQueue(q);
                }

                vxSetEvent(&q->writeEvent);
            }
            vxSemPost(&q->lock);
            if (q->popped == vx_true_e || red == vx_true_e)
                break;
        }
        VX_PRINT(VX_ZONE_OSAL, "Leaving with %d\n", red);
    }
    return red;
}

void vxPopQueue(vx_queue_t *q)
{
    if (q)
    {
        vxSemWait(&q->lock);
        q->popped = vx_true_e;
        vxSetEvent(&q->readEvent);
        vxSetEvent(&q->writeEvent);
        vxSemPost(&q->lock);
    }
}

void vxDeinitQueue(vx_queue_t *q)
{
    if (q)
    {
        q->start_index = 0;
        q->end_index = -1;
        vxDestroySem(&q->lock);
        vxDeinitEvent(&q->readEvent);
        vxDeinitEvent(&q->writeEvent);
    }
}

vx_module_handle_t vxLoadModule(vx_char * name)
{
    vx_module_handle_t mod;

#if defined(__linux__) || defined(__ANDROID__) || defined(__APPLE__) || defined(__QNX__) || defined(__CYGWIN__)
    mod = dlopen(name, RTLD_NOW|RTLD_LOCAL);
    if (mod == 0)
    {
        VX_PRINT(VX_ZONE_ERROR, "%s\n", dlerror());
    }
#elif defined(_WIN32)
    mod = LoadLibrary(name);
#endif
    return mod;
}

void vxUnloadModule(vx_module_handle_t mod)
{
#if defined(__linux__) || defined(__ANDROID__) || defined(__APPLE__) || defined(__QNX__) || defined(__CYGWIN__)
    dlclose(mod);
#elif defined(_WIN32)
    FreeLibrary(mod);
#endif
}

vx_symbol_t vxGetSymbol(vx_module_handle_t mod, vx_char *name)
{
#if defined(__linux__) || defined(__ANDROID__) || defined(__APPLE__) || defined(__QNX__) || defined(__CYGWIN__)
    return (vx_symbol_t)dlsym(mod, name);
#elif defined(_WIN32)
    return (vx_symbol_t)GetProcAddress(mod, name);
#endif
}

/******************************************************************************/
// EXTERNAL API (NO COMMENTS HERE, SEE HEADER FILES)
/******************************************************************************/

