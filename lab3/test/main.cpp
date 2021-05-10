#include <iostream>
#include <mutex>
#include <vector>

void *test1_thread1(void *arg) {
    std::vector<pthread_mutex_t> *mut_v = (std::vector<pthread_mutex_t> *) arg;

    pthread_mutex_lock(&((*mut_v)[0]));
    pthread_mutex_lock(&((*mut_v)[1]));

    pthread_mutex_unlock(&((*mut_v)[1]));
    pthread_mutex_unlock(&((*mut_v)[0]));
}

void *test1_thread2(void *arg) {

    std::vector<pthread_mutex_t> *mut_v = (std::vector<pthread_mutex_t> *) arg;

    pthread_mutex_lock(&((*mut_v)[1]));
    pthread_mutex_lock(&((*mut_v)[0]));

    pthread_mutex_unlock(&((*mut_v)[0]));
    pthread_mutex_unlock(&((*mut_v)[1]));
}

void *test2_thread1(void *arg) {
    std::vector<pthread_mutex_t> *mut_v = (std::vector<pthread_mutex_t> *) arg;

    pthread_mutex_lock(&((*mut_v)[0]));
    pthread_mutex_lock(&((*mut_v)[1]));

    pthread_mutex_unlock(&((*mut_v)[1]));
    pthread_mutex_unlock(&((*mut_v)[0]));
}

void *test2_thread2(void *arg) {

    std::vector<pthread_mutex_t> *mut_v = (std::vector<pthread_mutex_t> *) arg;

    pthread_mutex_lock(&((*mut_v)[1]));
    pthread_mutex_unlock(&((*mut_v)[1]));
    pthread_mutex_lock(&((*mut_v)[0]));

    pthread_mutex_unlock(&((*mut_v)[0]));
}

/*
   possible DeadLock
   m0 -> m1
   ^      |
   |<-----v
*/
void deadlock_test1() {
    std::vector<pthread_mutex_t> mut_v(2);
    pthread_mutex_init(&mut_v[0], nullptr);
    pthread_mutex_init(&mut_v[1], nullptr);
    std::vector<pthread_t> threads(3);

    pthread_create(&threads[0], nullptr, test1_thread1, (void *) &mut_v);
    pthread_create(&threads[1], nullptr, test1_thread2, (void *) &mut_v);

    pthread_join(threads[0], nullptr);
    pthread_join(threads[1], nullptr);

};


void deadlock_test2() {
    std::vector<pthread_mutex_t> mut_v(2);
    pthread_mutex_init(&mut_v[0], nullptr);
    pthread_mutex_init(&mut_v[1], nullptr);
    std::vector<pthread_t> threads(2);

    pthread_create(&threads[0], nullptr, test2_thread1, (void *) &mut_v);
    pthread_create(&threads[1], nullptr, test2_thread2, (void *) &mut_v);

    pthread_join(threads[0], nullptr);
    pthread_join(threads[1], nullptr);
}

void deadlock_test3() {
    std::vector<pthread_mutex_t> mut_v(2);
    pthread_mutex_init(&mut_v[0], nullptr);
    pthread_mutex_init(&mut_v[1], nullptr);
    std::vector<pthread_t> threads(2);

    pthread_create(&threads[0], nullptr, test2_thread1, (void *) &mut_v);
    pthread_create(&threads[1], nullptr, test2_thread2, (void *) &mut_v);

    pthread_join(threads[0], nullptr);
    pthread_join(threads[1], nullptr);
}

int main() {
    // potential deadlock
//    deadlock_test1();
    // non-deadlock
    deadlock_test2();
    // the same as previous one
    deadlock_test3();

    return 0;
}




