#include <iostream>
#include <vector>
#include <map>
#include <pthread.h>
#include <dlfcn.h>
#include <algorithm>
#include <set>


class Mutex_graph {
public:
    Mutex_graph() {
        graph_nodes.reserve(1000000);
    }

private:
    enum Colors {
        WHITE,
        BLACK,
        GREY
    };

    struct Graph_node {
        Graph_node(const pthread_mutex_t *__mutex) : __mutex(__mutex), color(WHITE) {}

        const pthread_mutex_t *__mutex;
        Colors color;
        std::set<Graph_node *> connected_nodes;

    };

    std::vector<Graph_node> graph_nodes;

    bool dfs_find_cycle(Graph_node *start_node) {
        for (Graph_node *node: start_node->connected_nodes) {
            if (node->color == WHITE) {
                node->color = GREY;
                if (dfs_find_cycle(node)) return true;
            } else if (node->color == GREY) {
                return true;
            }
        }
        start_node->color = BLACK;
        return false;
    }

    Graph_node *find_by(const pthread_mutex_t *__mutex) {
        for (auto &node: graph_nodes) {
            if (node.__mutex == __mutex) {
                return &node;
            }
        }
        return nullptr;
    }

public:
    bool is_cycle() {

        for (auto &graph_node : graph_nodes) {
            clear_nodes();
            if (dfs_find_cycle(&graph_node)) {
                return true;
            }
        }
        return false;
    }

    void clear_nodes() {
        for (auto &graph_node : graph_nodes) {
            graph_node.color = WHITE;
        }
    }


    bool add_node(const pthread_mutex_t *__mutex) {
        graph_nodes.push_back(__mutex);
        return true;
    }

    void connect(const pthread_mutex_t *from, const pthread_mutex_t *to) {
        Graph_node *from_node = find_by(from);
        Graph_node *to_node = find_by(to);
        from_node->connected_nodes.insert(to_node);
    }

};


extern "C" {
int (*orig_mutex_lock)(pthread_mutex_t *__mutex) = (int (*)(pthread_mutex_t *)) dlsym(RTLD_NEXT, "pthread_mutex_lock");
int
(*orig_mutex_unlock)(pthread_mutex_t *__mutex) = (int (*)(pthread_mutex_t *)) dlsym(RTLD_NEXT, "pthread_mutex_unlock");
int (*orig_mutex_init)(pthread_mutex_t *__mutex, const pthread_mutexattr_t *__mutexattr) = (int (*)(pthread_mutex_t *,
                                                                                                    const pthread_mutexattr_t *)) dlsym(
        RTLD_NEXT,
        "pthread_mutex_init");

Mutex_graph graph;
pthread_mutex_t sanitizer_mut;
thread_local std::vector<pthread_mutex_t *> locked_mutex;

class Init {
public:
    Init() {
        (*orig_mutex_init)(&sanitizer_mut, nullptr);
    }
};
Init init;

int pthread_mutex_lock(pthread_mutex_t *__mutex) {


    if (!locked_mutex.empty()) {
        pthread_mutex_t *pre_last_locked_mutex = locked_mutex.back();
        pthread_mutex_t *last_locked_mutex = __mutex;

        (*orig_mutex_lock)(&sanitizer_mut);
        graph.connect(pre_last_locked_mutex, last_locked_mutex);
        if (graph.is_cycle()) exit(EXIT_FAILURE);
        (*orig_mutex_unlock)(&sanitizer_mut);
    }

    locked_mutex.push_back(__mutex);
    return (*orig_mutex_lock)(__mutex);
}
int pthread_mutex_unlock(pthread_mutex_t *__mutex) {
    (*orig_mutex_lock)(&sanitizer_mut);
    locked_mutex.erase(std::remove(locked_mutex.begin(), locked_mutex.end(), __mutex), locked_mutex.end());
    (*orig_mutex_unlock)(&sanitizer_mut);

    return (*orig_mutex_unlock)(__mutex);
}
int pthread_mutex_init(pthread_mutex_t *__mutex, const pthread_mutexattr_t *__mutexattr) {
    (*orig_mutex_lock)(&sanitizer_mut);
    graph.add_node(__mutex);
    (*orig_mutex_unlock)(&sanitizer_mut);

    return (*orig_mutex_init)(__mutex, __mutexattr);
}

}