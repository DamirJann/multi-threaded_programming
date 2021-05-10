#include <iostream>
#include <vector>
#include <map>
#include <pthread.h>
#include <dlfcn.h>


class Mutex_graph {
public:
    Mutex_graph(){
        graph_nodes.reserve(1000);
    }
private:
    enum Colors {
        WHITE,
        BLACK,
        GREY
    };

    struct Graph_node {
        Graph_node(const pthread_mutex_t* __mutex) : __mutex(__mutex), color(WHITE) {}

        const pthread_mutex_t* __mutex;
        Colors color;
        std::vector<Graph_node*> connected_nodes;

    public:
        bool is_connected(const Graph_node node){
            for (const auto neighbour: connected_nodes){
                if (neighbour->__mutex == node.__mutex){
                    return true;
                }
            }
            return false;
        }
    };

    std::vector<Graph_node> graph_nodes;

    bool dfs_find_cycle(Graph_node start_node) {
        for (Graph_node* node: start_node.connected_nodes) {
            if (node->color == WHITE) {
                node->color = GREY;
                if (dfs_find_cycle(*node)) return true;
            } else if (node->color == GREY) {
                return true;
            }
        }
        start_node.color = BLACK;
        return false;
    }
    Graph_node* find_by(const pthread_mutex_t *__mutex) {
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
            if (dfs_find_cycle(graph_node)){
                return true;
            }
        }
        return false;
    }

    void clear_nodes(){
        for (auto &graph_node : graph_nodes) {
            graph_node.color = WHITE;
        }
    }



    bool add_node(const pthread_mutex_t *__mutex) {
        graph_nodes.push_back(Graph_node(__mutex));
        return true;
    }

    void connect(const pthread_mutex_t* from, const pthread_mutex_t* to) {
        Graph_node* from_node = find_by(from);
        Graph_node* to_node = find_by(to);
        if (!from_node->is_connected(*to_node)) {
            from_node->connected_nodes.push_back(to_node);
        }
    }

};


extern "C" {
int (*orig_mutex_lock)(pthread_mutex_t *__mutex);
int (*orig_mutex_unlock)(pthread_mutex_t *__mutex);
int (*orig_mutex_init)(pthread_mutex_t *__mutex, const pthread_mutexattr_t * __mutexattr);

Mutex_graph graph;
pthread_mutex_t sanitizer_mut;
thread_local std::vector<pthread_mutex_t *> locked_mutex;

class Init {
public:
    Init() {
        orig_mutex_lock = (int (*)(pthread_mutex_t *)) dlsym(RTLD_NEXT, "pthread_mutex_lock");
        orig_mutex_unlock = (int (*)(pthread_mutex_t *)) dlsym(RTLD_NEXT, "pthread_mutex_unlock");
        orig_mutex_init = (int (*)(pthread_mutex_t*, const pthread_mutexattr_t*)) dlsym(RTLD_NEXT, "pthread_mutex_init");
        pthread_mutex_init(&sanitizer_mut, nullptr);

    }
};
Init init;

int pthread_mutex_lock(pthread_mutex_t *__mutex) {



    int pthread_id = pthread_self() % 10;

    // Start of critical zone
    (*orig_mutex_lock)(&sanitizer_mut);

    if (locked_mutex.empty()) {
        locked_mutex.push_back({__mutex});
    } else {
        pthread_mutex_t *pre_last_locked_mutex = locked_mutex.back();
        pthread_mutex_t *last_locked_mutex = __mutex;
        locked_mutex.push_back(__mutex);
        graph.connect(pre_last_locked_mutex, last_locked_mutex);
    }
    std::clog << "Thread " << pthread_id << " locked " << __mutex << std::endl
              << "Possible deadlock: " << (graph.is_cycle() ? "YES" : "NO") << std::endl;

    if (graph.is_cycle()) exit(-1);

    (*orig_mutex_unlock)(&sanitizer_mut);

    int result = (*orig_mutex_lock)(__mutex);
    // End of critical zone

    return result;
}
int pthread_mutex_unlock(pthread_mutex_t *__mutex) {
    (*orig_mutex_lock)(&sanitizer_mut);
    for (auto it = locked_mutex.begin(); it != locked_mutex.end(); it++) {
        if (*it == __mutex) {
            locked_mutex.erase(it);
            break;
        }

    }
    (*orig_mutex_unlock)(&sanitizer_mut);
    int result = (*orig_mutex_unlock)(__mutex);
    return result;
}
int pthread_mutex_init(pthread_mutex_t *__mutex, const pthread_mutexattr_t *__mutexattr) {
    (*orig_mutex_lock)(&sanitizer_mut);
    int result = (*orig_mutex_init)(__mutex, __mutexattr);
    graph.add_node(__mutex);
    (*orig_mutex_unlock)(&sanitizer_mut);
    return result;
}

}