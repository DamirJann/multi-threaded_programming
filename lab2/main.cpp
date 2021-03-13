#include <iostream>
#include <queue>
#include <pthread.h>
#include <fstream>

using namespace std;
const int N = 1;
bool isTasksSupplied = false;
int ind = 0;
pthread_cond_t cond;
queue<int> taskQueue;
pthread_mutex_t mutex;
int waitingThreadsCount = 0;


void *producer_routine(void *arg) {

//    clog << "producer " << "started\n";
    // open stream to read data
    ifstream ifstream("input.txt");
    // result sum, which is calculated by this thread
    int sum = 0;
    int number;

    // add tasks while there's data
    while (!isTasksSupplied) {
        // lock mutex
        // start of CRITICAL ZONE
        pthread_mutex_lock(&mutex);

        // if we can read, supply new task
        if (ifstream >> number) {
            sum += number;
            taskQueue.push(number);
            pthread_cond_signal(&cond);
//            clog << "producer added task " << number <<  endl;
        // otherwise go out
        } else {
            isTasksSupplied = true;
        }

        // unlock mutex
        // end of CRITICAL ZONE
        pthread_mutex_unlock(&mutex);
    }

//    clog << "produced delivered tasks" << endl << waitingThreadsCount << taskQueue.size();

    // after all tasks have been delivered, producer wakes up threads
    while (true) {
        pthread_mutex_lock(&mutex);

        // check if there are waiting Threads and wake up them
        if (!taskQueue.empty() || waitingThreadsCount != 0) {
            pthread_cond_signal(&cond);
        }

        // unlock and go out if no waiting Threads
        if (taskQueue.empty() && waitingThreadsCount == 0) {
            pthread_mutex_unlock(&mutex);
            break;
        }

        pthread_mutex_unlock(&mutex);
    }

//    cout << "Real sum is " << sum << endl;
    clog << "producer " << "ended\n";
}


void *consumer_routine(void *arg) {
    long long int sum = 0;
    int id = ind++;

//    clog << "consumer " << id << " started\n";

    while (true) {
        // lock mutex
        // start of CRITICAL ZONE
//        cout << id << " mut\n";
        pthread_mutex_lock(&mutex);
//        cout << id << " onmut\n";
//        cout << "here " << id;
        // if task
        // wait until producer wake up thread and
//        cout << taskQueue.size() << isTasksSupplied << waitingThreadsCount << endl;
        if (taskQueue.empty() && !isTasksSupplied) {
            waitingThreadsCount++;
//            cout << id << "in" << waitingThreadsCount << endl;
            pthread_cond_wait(&cond, &mutex);
            waitingThreadsCount--;
//            cout << id << "out" << endl << waitingThreadsCount;
        }

//        cout << taskQueue.size() << isTasksSupplied << waitingThreadsCount << endl;
        // if there's a task
        // take it
        if (!taskQueue.empty()) {
            int number = taskQueue.front();
            sum += number;
            taskQueue.pop();
//            clog << "consumer " << id << " took " << number << " " << taskQueue.size() << endl;
            // otherwise check weather consumers have done all tasks
        }

        if (taskQueue.empty() && isTasksSupplied) {
            // unlock mutex
            // end of CRITICAL ZONE
            pthread_mutex_unlock(&mutex);
            break;
        }
        // unlock mutex
        // end of CRITICAL ZONE
        pthread_mutex_unlock(&mutex);
    }

    // pass sum through parameter
    *(int *) arg = sum;

//    clog << "consumer " << id << " ended\n";
    return 0;

}

void *consumer_interruptor_routine(void *arg) {

}

int run_threads() {
    pthread_t producer_thread;
    pthread_t consumer_thread[N];
    pthread_create(&producer_thread, nullptr, producer_routine, nullptr);
    long long int *localSum = new long long int[N];


    for (int i = 0; i < N; i++) {
        localSum[i] = 0;
        pthread_create(&consumer_thread[i], nullptr, consumer_routine, (void *) &localSum[i]);
    }
    pthread_join(producer_thread, nullptr);

    long long int sum = 0;
    for (int i = 0; i < N; i++) {
        pthread_join(consumer_thread[i], nullptr);
        sum += localSum[i];
    }

    return sum;
}


int get_tid() {
    return 0;
}


int main() {
    ofstream output("input.txt");
    for (int i = 0; i < 100; i++) {
        output << i << ' ';
    }
    output.close();
    pthread_cond_init(&cond, nullptr);
    int anw = run_threads();
    cout << "Sum, which was calculated by threads is " << anw << endl;
    return 0;
}