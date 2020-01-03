#include <sys/types.h>
#include <unistd.h>

#include <utils/log.h>
#include <utils/common.h>
#include <thread/thread.h>

#define LOG_TAG "test_thread"

static void thread1_cleanup(void) {
    LOGI("loop1 cleanup\n");
}

static void thread2_cleanup(void) {
    LOGI("loop2 cleanup\n");
}

static void thread1_loop(struct pthread_wrapper* pthread, void* param) {
    pthread_testcancel();

    LOGI("loop1, pid=%d, tid=%d, posix_id=%ld, param=%ld\n", getpid(),
            pthread->get_pid(pthread), pthread_self(), (long)param);

    pthread_exit(NULL);
}

static void thread2_loop(struct pthread_wrapper* pthread, void* param) {
    pthread_testcancel();

    LOGI("loop2, pid=%d, tid=%d, posix_id=%ld, param=%ld\n", getpid(),
            pthread->get_pid(pthread), pthread_self(), (long)param);

    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    struct thread* thread1 = _new(struct thread, thread);
    struct thread* thread2 = _new(struct thread, thread);

    thread1->set_thread_count(thread1, 10);
    thread1->runnable.run = thread1_loop;
    thread1->runnable.cleanup = thread1_cleanup;
    thread1->start(thread1, NULL);

    thread2->set_thread_count(thread2, 5);
    thread2->runnable.run = thread2_loop;
    thread1->runnable.cleanup = thread2_cleanup;
    thread2->start(thread2, NULL);

    thread1->wait(thread1);
    thread2->wait(thread2);

    LOGI("start therad1 again.\n");
    thread1->start(thread1, NULL);
    thread1->wait(thread1);

    _delete(thread1);
    _delete(thread2);

    LOGI("test done\n");

    return 0;
}
