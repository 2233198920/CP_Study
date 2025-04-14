#include <pthread.h>
#include <stdatomic.h>
#include <stdint.h>
#include <stdlib.h>
#include "thrd_pool.h"
#include "spinlock.h"

/**
 * author: mark 
 * QQ: 2548898954
 * shell: gcc thrd_pool.c -c -fPIC
 * shell: gcc -shared thrd_pool.o -o libthrd_pool.so -I./ -L./ -lpthread
 * usage: include thrd_pool.h & link libthrd_pool.so
 */

typedef struct spinlock spinlock_t;

typedef struct task_s {
    void *next;
    handler_pt func;
    void *arg;
} task_t;

/*
block = 1 (默认值): 工作线程尝试获取任务时，如果队列为空，会通过条件变量阻塞等待
block = 0: 工作线程尝试获取任务时，如果队列为空，立即返回 NULL 而不阻塞
*/
typedef struct task_queue_s {
    void *head;
    void **tail; 
    int block;   // 0: nonblock, 1: block
    spinlock_t lock;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} task_queue_t;

struct thrdpool_s {
    task_queue_t *task_queue;
    atomic_int quit;
    int thrd_count;
    pthread_t *threads;
};

// 对称
// 资源的创建   回滚式编程
// 业务逻辑     防御式编程

/**
 * 任务队列创建函数
 * 
 * 采用回滚式编程模式：如果在创建过程中的某步失败，则释放之前已分配的资源
 * 初始化流程：
 * 1. 分配内存
 * 2. 初始化互斥锁
 * 3. 初始化条件变量
 * 4. 初始化自旋锁
 * 5. 设置队列初始状态
 * 
 * @return 成功返回队列指针，失败返回NULL
 */
static task_queue_t *
__taskqueue_create() {
    int ret;
    // 为任务队列分配内存
    task_queue_t *queue = (task_queue_t *)malloc(sizeof(task_queue_t));
    if (queue) {
        // 初始化互斥锁，用于保护条件变量
        ret = pthread_mutex_init(&queue->mutex, NULL);
        if (ret == 0) {
            // 初始化条件变量，用于线程间同步（等待/通知机制）
            ret = pthread_cond_init(&queue->cond, NULL);
            if (ret == 0) {
                // 初始化自旋锁，用于保护队列数据结构
                spinlock_init(&queue->lock);
                // 初始化队列为空
                queue->head = NULL;
                // tail指向head指针的地址，这样可以统一处理空队列和非空队列的入队操作
                queue->tail = &queue->head;
                // 默认为阻塞模式
                queue->block = 1;
                return queue;
            }
            // 初始化条件变量失败，清理已初始化的互斥锁
            pthread_mutex_destroy(&queue->mutex);
        }
        // 初始化失败，释放已分配的内存
        free(queue);
    }
    return NULL;
}

/**
 * 将任务队列设置为非阻塞模式
 * 
 * 当线程池需要终止时，调用此函数通知所有等待的工作线程退出
 * 
 * @param queue 任务队列
 */
static void
__nonblock(task_queue_t *queue) {
    // 获取互斥锁以保护block变量的修改
    pthread_mutex_lock(&queue->mutex);
    // 设置为非阻塞模式
    queue->block = 0;
    // 释放互斥锁
    pthread_mutex_unlock(&queue->mutex);
    // 广播通知所有等待在条件变量上的线程，使它们可以检查block状态并退出
    pthread_cond_broadcast(&queue->cond);
}

/**
 * 向任务队列添加一个任务
 * 
 * 将任务添加到队列尾部，并通过条件变量通知等待的工作线程
 * 
 * @param queue 任务队列
 * @param task 待添加的任务，必须包含一个作为链表节点的指针
 */
static inline void 
__add_task(task_queue_t *queue, void *task) {
    // 不限定任务类型，只要该任务的结构起始内存是一个用于链接下一个节点的指针
    void **link = (void**)task;
    *link = NULL;

    spinlock_lock(&queue->lock);
    *queue->tail /* 等价于 queue->tail->next */ = link;
    queue->tail = link;
    spinlock_unlock(&queue->lock);
    pthread_cond_signal(&queue->cond);
}

/**
 * 从任务队列中弹出一个任务
 * 
 * 非阻塞地获取队列头部的任务。如果队列为空，则返回NULL
 * 
 * @param queue 任务队列
 * @return 成功返回任务指针，队列为空则返回NULL
 */
static inline void * 
__pop_task(task_queue_t *queue) {
    spinlock_lock(&queue->lock);
    if (queue->head == NULL) {
        spinlock_unlock(&queue->lock);
        return NULL;
    }

    task_t *task;
    task = queue->head;

    // queue->head = task->next; 常规写法，下面是避免出现next指针，更加通用
    void **link = (void**)task;
    queue->head = *link;

    if (queue->head == NULL) {
        queue->tail = &queue->head;
    }
    spinlock_unlock(&queue->lock);
    return task;
}

/**
 * 获取任务（阻塞模式）
 * 
 * 从任务队列获取一个任务。如果队列为空，则阻塞等待直到有新任务或队列被设置为非阻塞
 * 
 * @param queue 任务队列
 * @return 任务指针，如果队列为非阻塞且为空，则返回NULL
 */
static inline void * 
__get_task(task_queue_t *queue) {
    task_t *task;
    // 虚假唤醒
    while ((task = __pop_task(queue)) == NULL) {
        pthread_mutex_lock(&queue->mutex);
        if (queue->block == 0) {
            pthread_mutex_unlock(&queue->mutex);
            return NULL;
        }
        // 1. 先 unlock(&mtx)
        // 2. 在 cond 休眠
        // --- __add_task 时唤醒
        // 3. 在 cond 唤醒
        // 4. 加上 lock(&mtx);
        pthread_cond_wait(&queue->cond, &queue->mutex);
        pthread_mutex_unlock(&queue->mutex);
    }
    return task;
}

/**
 * 销毁任务队列
 * 
 * 释放队列中的所有任务及相关资源
 * 
 * @param queue 需要销毁的任务队列
 */
static void
__taskqueue_destroy(task_queue_t *queue) {
    task_t *task;
    while ((task = __pop_task(queue))) {
        free(task);
    }
    spinlock_destroy(&queue->lock);
    pthread_cond_destroy(&queue->cond);
    pthread_mutex_destroy(&queue->mutex);
    free(queue);
}

/**
 * 工作线程函数
 * 
 * 线程池中每个工作线程的执行函数，循环获取任务并执行
 * 
 * @param arg 线程池指针
 * @return NULL
 */
static void *
__thrdpool_worker(void *arg) {
    thrdpool_t *pool = (thrdpool_t*) arg;
    task_t *task;
    void *ctx;

    while (atomic_load(&pool->quit) == 0) {
        task = (task_t*)__get_task(pool->task_queue);
        if (!task) break;
        handler_pt func = task->func;
        ctx = task->arg;
        free(task);
        func(ctx);
    }
    
    return NULL;
}

/**
 * 终止线程池中的所有线程
 * 
 * 设置退出标志并等待所有线程结束
 * 
 * @param pool 线程池指针
 */
static void 
__threads_terminate(thrdpool_t * pool) {
    atomic_store(&pool->quit, 1);
    __nonblock(pool->task_queue);
    int i;
    for (i=0; i<pool->thrd_count; i++) {
        pthread_join(pool->threads[i], NULL);
    }
}

/**
 * 创建线程池中的工作线程
 * 
 * 创建指定数量的工作线程，如果创建过程中失败，则回滚并释放资源
 * 
 * @param pool 线程池指针
 * @param thrd_count 要创建的线程数量
 * @return 成功返回0，失败返回-1
 */
static int 
__threads_create(thrdpool_t *pool, size_t thrd_count) {
    pthread_attr_t attr;   // 线程属性
	int ret;

    ret = pthread_attr_init(&attr);

    if (ret == 0) {
        pool->threads = (pthread_t *)malloc(sizeof(pthread_t) * thrd_count);
        if (pool->threads) {
            int i = 0;
            for (; i < thrd_count; i++) {
                if (pthread_create(&pool->threads[i], &attr, __thrdpool_worker, pool) != 0) {
                    break;
                }
            }
            pool->thrd_count = i;
            pthread_attr_destroy(&attr);
            if (i == thrd_count)
                return 0;
            __threads_terminate(pool);
            free(pool->threads);
        }
        ret = -1;
    }
    return ret; 
}

/**
 * 终止线程池
 * 
 * 设置退出标志并唤醒所有阻塞的工作线程，但不等待线程结束
 * 
 * @param pool 要终止的线程池
 */
void
thrdpool_terminate(thrdpool_t * pool) {
    atomic_store(&pool->quit, 1);
    __nonblock(pool->task_queue);
}

/**
 * 创建线程池
 * 
 * 创建线程池并启动指定数量的工作线程
 * 
 * @param thrd_count 工作线程数量
 * @return 成功返回线程池指针，失败返回NULL
 */
thrdpool_t *
thrdpool_create(int thrd_count) {
    thrdpool_t *pool;

    pool = (thrdpool_t*)malloc(sizeof(*pool));
    if (pool) {
        task_queue_t *queue = __taskqueue_create();
        if (queue) {
            pool->task_queue = queue;
            atomic_init(&pool->quit, 0);
            if (__threads_create(pool, thrd_count) == 0)
                return pool;
            __taskqueue_destroy(queue);
        }
        free(pool);
    }
    return NULL;
}

/**
 * 向线程池提交任务
 * 
 * 创建一个任务并添加到线程池的任务队列，由工作线程异步执行
 * 
 * @param pool 线程池
 * @param func 任务处理函数
 * @param arg 传递给任务处理函数的参数
 * @return 成功返回0，失败返回-1
 */
int
thrdpool_post(thrdpool_t *pool, handler_pt func, void *arg) {
    if (atomic_load(&pool->quit) == 1) 
        return -1;
    task_t *task = (task_t*) malloc(sizeof(task_t));
    if (!task) return -1;
    task->func = func;
    task->arg = arg;
    __add_task(pool->task_queue, task);
    return 0;
}

/**
 * 等待线程池中所有任务完成并销毁线程池
 * 
 * 阻塞调用线程直到所有工作线程结束，然后清理所有资源
 * 
 * @param pool 要销毁的线程池
 */
void
thrdpool_waitdone(thrdpool_t *pool) {
    int i;
    for (i=0; i<pool->thrd_count; i++) {
        pthread_join(pool->threads[i], NULL);
    }
    __taskqueue_destroy(pool->task_queue);
    free(pool->threads);
    free(pool);
}
