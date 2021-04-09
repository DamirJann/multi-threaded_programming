#include <iostream>
#include <queue>
#include <pthread.h>
#include <fstream>
#include <ctime>
#include <memory>
#include <atomic>

using namespace std;

int N;
bool isTasksSupplied(false);
int waitingThreadsCount(0);
int max_sleep_time;
bool isDebugMode;

pthread_cond_t consumer_cond, producer_cond;
pthread_mutex_t mutex;

// atomic function
int get_tid() {
    static atomic_int lastThreadId(0);
    thread_local unique_ptr<int> id = make_unique<int>(-1);

    if (*id == -1){
        // ++lastThreadId increments and return value atomically because lastThreadId is atomic
        // no need mutex
        *id = ++lastThreadId;
    }

    return *id;
}


struct ConsumerStruct{
    int sum = 0;
    queue<int>* taskQueue{};
};

void *producer_routine(void *arg) {

    queue<int>* taskQueue = ((queue<int>*) arg);

    // result sum, which is calculated by this thread
    int sum = 0;
    int number;
    // add tasks while there's data
    while (!isTasksSupplied) {
        // lock mutex
        // start of CRITICAL ZONE

        pthread_mutex_lock(&mutex);
        if (!taskQueue->empty()) {
            pthread_cond_wait(&producer_cond, &mutex);
        }
        // if we can read, supply new task
        if (cin >> number) {
            sum += number;
            taskQueue->push(number);
            pthread_cond_signal(&consumer_cond);
            // otherwise go out
        } else {
            isTasksSupplied = true;
        }
        // unlock mutex
        // end of CRITICAL ZONE
        pthread_mutex_unlock(&mutex);
    }

    // after all tasks have been delivered, producer wakes up threads
    while (true) {
        pthread_mutex_lock(&mutex);

        // check if there are waiting Threads and wake up them
        if (!taskQueue->empty() || waitingThreadsCount != 0) {
            pthread_cond_signal(&consumer_cond);
        }

        // unlock and go out if no waiting Threads
        if (taskQueue->empty() && waitingThreadsCount == 0) {
            pthread_mutex_unlock(&mutex);
            break;
        }

        pthread_mutex_unlock(&mutex);
    }
    return nullptr;
}


void *consumer_routine(void *arg) {

    ConsumerStruct* threadStruct = (ConsumerStruct*) arg;
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, nullptr);

    while (true) {
        bool isDidTask = false;
        // lock mutex
        // start of CRITICAL ZONE
        pthread_mutex_lock(&mutex);
        // if task
        // wait until producer wake up thread and
        if (threadStruct->taskQueue->empty() && !isTasksSupplied) {
            pthread_cond_signal(&producer_cond);
            waitingThreadsCount++;
            pthread_cond_wait(&consumer_cond, &mutex);
            waitingThreadsCount--;
        }

        // if there's a task
        // take it
        if (!threadStruct->taskQueue->empty()) {
            int number = threadStruct->taskQueue->front();
            threadStruct->sum += number;
            threadStruct->taskQueue->pop();
            isDidTask = true;
            if (isDebugMode) {
                cout << "(" << get_tid() << ", " << threadStruct->sum << ")" << endl ;
            }
            // otherwise check weather consumers have done all tasks
        }

        // if all tasks are done
        if (isTasksSupplied && threadStruct->taskQueue->empty()) {
            // unlock mutex
            // end of CRITICAL ZONE
            pthread_mutex_unlock(&mutex);
            break;
        }
        // unlock mutex
        // end of CRITICAL ZONE
        pthread_mutex_unlock(&mutex);

        // make thread sleep
        if (isDidTask && max_sleep_time != 0) {
            int sleepTime = rand() % max_sleep_time + 1;
            // 1 milisecond = 10^6 nanosecond
            struct timespec   ts = {0, sleepTime * 1'000'000};
            struct timespec tr{};
            nanosleep(&ts, &tr);
        }
    }
    return nullptr;
}



void *consumer_interruptor_routine(void *arg) {

    vector<pthread_t>* p_thread = (vector<pthread_t> *) arg;

    while (!isTasksSupplied){
        // try to stop random thread
        pthread_cancel(p_thread->at(rand()%N));
    }

    return nullptr;
}

int run_threads() {
    queue<int> taskQueue;
    pthread_t producer_thread;
    pthread_t interrupted_thread;
    vector<ConsumerStruct> threadStruct(N);
    vector<pthread_t> consumer_thread(N);
    int sum = 0;
    pthread_mutex_init(&mutex, nullptr);
    pthread_cond_init(&producer_cond, nullptr);
    pthread_cond_init(&consumer_cond, nullptr);

    // start producer
    pthread_create(&producer_thread, nullptr, producer_routine, (void *) &taskQueue);
    // start consumers
    for (int i = 0; i < N; i++) {
        threadStruct[i] = ConsumerStruct{0, &taskQueue};
        pthread_create(&consumer_thread[i], nullptr, consumer_routine, (void *) &(threadStruct[i]));
    }
    // start interrupter
    pthread_create(&interrupted_thread, nullptr, consumer_interruptor_routine, (void *) &consumer_thread);
    //end interrupter
    pthread_join(interrupted_thread, nullptr);
    // end consumer
    for (int i = 0; i < N; i++) {
        pthread_join(consumer_thread[i], nullptr);
        sum += threadStruct[i].sum;
    }
    // end producer
    pthread_join(producer_thread, nullptr);

    return sum;
}

int main(int argc, char** argv) {

    switch (argc) {
        case 4: {
            isDebugMode = true;
            N = stoi(string(argv[2]));
            max_sleep_time = stoi(string(argv[3]));
            break;
        }
        case 3: {
            isDebugMode = false;
            N = stoi(string(argv[1]));
            max_sleep_time = stoi(string(argv[2]));
            break;
        }
        default: {
            return 1;
        }
    }

    cout << run_threads() << endl;
    return 0;
}
