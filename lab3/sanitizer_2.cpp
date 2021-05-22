#include <unordered_map>
#include <set>
#include <list>
#include <dlfcn.h>
#include <unistd.h>
#include <iostream>
#include <algorithm>

extern "C" {
    static int (*orig_pthread_mutex_lock)(pthread_mutex_t *__mutex);
    static int (*orig_pthread_mutex_unlock)(pthread_mutex_t *__mutex);
    static int (*orig_pthread_mutex_init)(pthread_mutex_t *__mutex, const pthread_mutexattr_t *__mutexattr);
}

pthread_mutex_t __sanitizer_mutex;

void init() {
    orig_pthread_mutex_init = (int (*)(pthread_mutex_t*, const pthread_mutexattr_t*))dlsym(RTLD_NEXT, "pthread_mutex_init");
    orig_pthread_mutex_lock = (int (*)(pthread_mutex_t*))dlsym(RTLD_NEXT, "pthread_mutex_lock");
    orig_pthread_mutex_unlock = (int (*)(pthread_mutex_t*))dlsym(RTLD_NEXT, "pthread_mutex_unlock");

    orig_pthread_mutex_init(&__sanitizer_mutex, nullptr);
}

class StartUp {
public:
    StartUp() {
        init();
    }
};

StartUp __startup;

class MutexGraph {
public:
    void add_mutex(pthread_mutex_t* mutex) {
        graph[mutex] = std::set<pthread_mutex_t*>();
    }

    bool mutex_lock(pthread_t thread_id, pthread_mutex_t* mutex) {
        if (mutexes_locked.count(thread_id) == 0) {
            mutexes_locked[thread_id] = std::list<pthread_mutex_t*>();
        }
        if (mutexes_locked[thread_id].size()) {
            graph[*(mutexes_locked[thread_id].rbegin())].insert(mutex);
        }
        mutexes_locked[thread_id].push_back(mutex);

        std::set<pthread_mutex_t*> visited;
        visited.insert(mutex);
        bool result = false;
        for (auto& i : graph[mutex]) {
            if (graph.count(i) == 0) {
                continue;
            }
            result |= dfs_inner(i, visited);
        }
        return result;
    }

    void mutex_unlock(pthread_t thread_id, pthread_mutex_t* mutex) {
        auto& mutex_list = mutexes_locked[thread_id];
        auto it = std::find(mutex_list.begin(), mutex_list.end(), mutex);
//        if (it != mutex_list.begin()) {
//            auto& graph_edges = graph[*std::prev(it)];
//            graph_edges.erase(std::find(graph_edges.begin(), graph_edges.end(), mutex));
//            if (std::next(it) != mutex_list.end()) {
//                graph_edges.push_back(*std::next(it));
//            }
//        }
        mutex_list.erase(it);
    }

    void print_tree() {
        std::cout << "Threads:\n";
        for (auto& i : mutexes_locked) {
            std::cout << i.first << ':';
            for (auto& j : i.second) {
                std::cout << j << ' ';
            }
            std::cout << '\n';
        }
        std::cout << "Tree:\n";
        for (auto& i : graph) {
            std::cout << i.first << ':';
            for (auto& j : i.second) {
                std::cout << j << ' ';
            }
            std::cout << '\n';
        }
    }
private:
    bool dfs_inner(pthread_mutex_t* vertex, std::set<pthread_mutex_t*>& visited) {
        if (visited.count(vertex) > 0) {
            return true;
        } else {
            visited.insert(vertex);
            bool result = false;
            for (auto& i : graph[vertex]) {
                if (graph.count(i) == 0) {
                    continue;
                }
                result |= dfs_inner(i, visited);
            }
            if (!result) {
                visited.erase(vertex);
            }
            return result;
        }
    }

    std::unordered_map<pthread_mutex_t*, std::set<pthread_mutex_t*>> graph;
    std::unordered_map<pthread_t , std::list<pthread_mutex_t*>> mutexes_locked;
};


MutexGraph __graph;

int pthread_mutex_init(pthread_mutex_t* __mutex, const pthread_mutexattr_t* __mutexattr) {
    orig_pthread_mutex_lock(&__sanitizer_mutex);
    __graph.add_mutex(__mutex);
    orig_pthread_mutex_unlock(&__sanitizer_mutex);
    return orig_pthread_mutex_init(__mutex, __mutexattr);
}

int pthread_mutex_lock(pthread_mutex_t* __mutex) {
    orig_pthread_mutex_lock(&__sanitizer_mutex);
    pthread_t thread_id = pthread_self();
    bool result = __graph.mutex_lock(thread_id, __mutex);
    if (result) {
        printf("Warning!!! There is a potential deadlock in your code!\n");
        exit(1);
    }
    orig_pthread_mutex_unlock(&__sanitizer_mutex);
    return orig_pthread_mutex_lock(__mutex);
}

int pthread_mutex_unlock(pthread_mutex_t* __mutex) {
    orig_pthread_mutex_lock(&__sanitizer_mutex);
    pthread_t thread_id = pthread_self();
    __graph.mutex_unlock(thread_id, __mutex);
    orig_pthread_mutex_unlock(&__sanitizer_mutex);
    return orig_pthread_mutex_unlock(__mutex);
}