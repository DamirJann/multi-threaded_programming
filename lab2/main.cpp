#include <iostream>
#include <queue>
#include <pthread.h>
#include <fstream>

using namespace std;

int N;
bool isTasksSupplied = false;
bool isTasksDone = false;
int waitingThreadsCount = 0;
int sleep_time;
static int lastThreadId = 0;
bool isDebugMode;
static int doneTaskCount = 0;

thread_local int id;
pthread_cond_t consumer_cond, producer_cond;
pthread_mutex_t mutex;
pthread_mutex_t idMutex;
pthread_mutex_t doneTaskCountMutex;


void initialize_id();
void *producer_routine(void *arg);
void *consumer_routine(void *arg);
void *consumer_interruptor_routine(void *arg);
int run_threads();
int get_tid();
void fill_file();
struct ThreadStruct;


void initialize_id(){
    pthread_mutex_lock(&idMutex);
    id = ++lastThreadId;
    pthread_mutex_unlock(&idMutex);
}

struct ThreadStruct{
    long long int sum = 0;
    queue<int>* taskQueue;
};

void *producer_routine(void *arg) {

    //getting_id
    initialize_id();
    queue<int>* taskQueue = ((queue<int>*) arg);

    // open stream to read data
    ifstream cin("input.txt");
    // result sum, which is calculated by this thread
    int sum = 0;
    int number;
    // add tasks while there's data
    while (!isTasksSupplied) {
        // lock mutex
        // start of CRITICAL ZONE

        pthread_mutex_lock(&mutex);
        if (!taskQueue->empty()) {
            pthread_cond_wait(&mutex, &producer_cond);
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

    // getting_id
    initialize_id();

    ThreadStruct* threadStruct = (ThreadStruct*) arg;

    while (true) {
        bool isDidTask = false;
        pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, nullptr);
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
            doneTaskCount++;
            isDidTask = true;
            isTasksDone = isTasksSupplied && threadStruct->taskQueue->empty();

            if (isDebugMode) {
                cout << "(" << get_tid() << ", " << threadStruct->sum << ")" << endl;
            }
            // otherwise check weather consumers have done all tasks
        }

        if (threadStruct->taskQueue->empty() && isTasksSupplied) {
            // unlock mutex
            // end of CRITICAL ZONE
            pthread_mutex_unlock(&mutex);
            break;
        }
        // unlock mutex
        // end of CRITICAL ZONE
        pthread_mutex_unlock(&mutex);

        if (isDidTask) _sleep(rand()%sleep_time);
    }

    return nullptr;
}



void *consumer_interruptor_routine(void *arg) {

    //getting_id
    initialize_id();

    pthread_t* p_thread =  (pthread_t *)arg;

    while (true){
        // start of CRITICAL ZONE
        pthread_mutex_lock(&mutex);
        // try to stop random thread
        pthread_cancel(p_thread[rand()%N]);
        // break if all tasks were did
        if (isTasksDone){
            pthread_mutex_unlock(&mutex);
            break;
        }
        // end of CRITICAL ZONE
        pthread_mutex_unlock(&mutex);
    }

    return nullptr;
}

int run_threads() {
    queue<int> taskQueue;
    pthread_t producer_thread;
    pthread_t interrupted_thread;
    pthread_t consumer_thread[N];
    ThreadStruct threadStruct[N];
    long long int sum = 0;

    // start producer
    pthread_create(&producer_thread, nullptr, producer_routine, (void *) &taskQueue);
    // start consumers
    for (int i = 0; i < N; i++) {
        threadStruct[i].sum = 0;
        threadStruct[i].taskQueue = &taskQueue;
        pthread_create(&consumer_thread[i], nullptr, consumer_routine, (void *) &(threadStruct[i]));
    }
    // start interrupter
    pthread_create(&interrupted_thread, nullptr, consumer_interruptor_routine, (void *) consumer_thread);

    // end consumer
    for (int i = 0; i < N; i++) {
        pthread_join(consumer_thread[i], nullptr);
        sum += threadStruct[i].sum;
        cout << threadStruct[i].sum << endl;
    }

    // end producer
    pthread_join(producer_thread, nullptr);

    // end interrupter
    pthread_join(interrupted_thread, nullptr);


    return sum;
}


int get_tid() {
    return id;
}

void fill_file(int number_count){
    ofstream output("input.txt");
    for (int i = 0; i < number_count; i++) {
        output << i << ' ';
    }
    output.close();
}

int main(int argc, char** argv) {
    switch (argc) {
        case 4:{
            isDebugMode = true;
            N = stoi(string(argv[2]));
            sleep_time = stoi(string(argv[3]));
            break;
        }
        case 3:{
            isDebugMode = false;
            N = stoi(string(argv[1]));
            sleep_time = stoi(string(argv[2]));
            break;
        }
        default:{
            N = 1000;
            sleep_time = 1000;
            break;
        }
    }

    fill_file(10000);
    pthread_cond_init(&consumer_cond, nullptr);
    pthread_cond_init(&producer_cond, nullptr);

    int anw = run_threads();
    clog << "Sum, which was calculated by threads is " << anw << endl;

    return 0;
}

