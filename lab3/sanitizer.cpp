#include <iostream>
#include <vector>
#include <map>
#include <pthread.h>
#include <dlfcn.h>
#include <algorithm>
#include <set>
#include <memory>

class Mutex_graph {
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
        std::set<std::unique_ptr<Graph_node>> connected_nodes;
    };

    std::vector<std::unique_ptr<Graph_node>> graph_nodes;

    bool dfs_find_cycle(std::unique_ptr<Graph_node>& start_node) {
        for (auto &node: start_node->connected_nodes) {
            if (node->color == WHITE) {
                node->color = GREY;
                if (dfs_find_cycle(const_cast<std::unique_ptr<Graph_node> &>(node))) return true;
            } else if (node->color == GREY) {
                return true;
            }
        }
        start_node->color = BLACK;
        return false;
    }

    std::unique_ptr<Graph_node> find_by(const pthread_mutex_t *__mutex) {
        for (auto &graph_node : graph_nodes) {
            if (graph_node->__mutex == __mutex) {
                return std::move(graph_node);
            }
        }
        return nullptr;
    }

public:
    bool is_cycle() {
        clear_nodes();
        for (auto &graph_node : graph_nodes) {
            if (dfs_find_cycle(const_cast<std::unique_ptr<Graph_node> &>(graph_node))) {
                return true;
            }
        }
        return false;
    }

    void clear_nodes() {
        for (auto &graph_node : graph_nodes) {
            graph_node->color = WHITE;
        }
    }


    bool add_node(const pthread_mutex_t *__mutex) {
        if (find_by(__mutex) == nullptr) {
            std::unique_ptr<Graph_node> node = std::make_unique<Graph_node>(Graph_node(__mutex));
            graph_nodes.push_back(std::move(node));
        }
        return true;
    }

    void connect(const pthread_mutex_t *from, const pthread_mutex_t *to) {
        std::unique_ptr<Graph_node> from_node = find_by(from);
        std::unique_ptr<Graph_node> to_node = find_by(to);
        from_node->connected_nodes.insert(std::move(to_node));
    }

};


extern "C" {
int (*orig_mutex_lock)(pthread_mutex_t *__mutex) = (int (*)(pthread_mutex_t *)) dlsym(RTLD_NEXT, "pthread_mutex_lock");
int (*orig_mutex_unlock)(pthread_mutex_t *__mutex) = (int (*)(pthread_mutex_t *)) dlsym(RTLD_NEXT, "pthread_mutex_unlock");
Mutex_graph graph;
pthread_mutex_t sanitizer_mut = PTHREAD_MUTEX_INITIALIZER;
thread_local std::vector<pthread_mutex_t *> locked_mutex;

int pthread_mutex_lock(pthread_mutex_t *__mutex) {
    (*orig_mutex_lock)(&sanitizer_mut);
    graph.add_node(__mutex);
    if (!locked_mutex.empty()) {
        pthread_mutex_t *pre_last_locked_mutex = locked_mutex.back();
        pthread_mutex_t *last_locked_mutex = __mutex;
        graph.connect(pre_last_locked_mutex, last_locked_mutex);
    }
    if (graph.is_cycle()) exit(EXIT_FAILURE);
    locked_mutex.push_back(__mutex);
    (*orig_mutex_unlock)(&sanitizer_mut);

    return (*orig_mutex_lock)(__mutex);
}

int pthread_mutex_unlock(pthread_mutex_t *__mutex) {
    (*orig_mutex_lock)(&sanitizer_mut);
    locked_mutex.erase(std::remove(locked_mutex.begin(), locked_mutex.end(), __mutex), locked_mutex.end());
    (*orig_mutex_unlock)(&sanitizer_mut);

    return (*orig_mutex_unlock)(__mutex);
}
}
