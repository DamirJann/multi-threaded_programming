#include <iostream>
#include <vector>
using namespace std;
const int N = 100;
enum Colors {
    WHITE,
    BLACK,
    GREY
};



class Node{
public:
    int id;
    vector<Node*> neighbors;
    Colors color = WHITE;

    explicit Node(int id) {
        this->id = id;
    }

public:
    void link(Node* node){
        neighbors.push_back(node);
    }
    void remove(Node * node){
        for (int i = 0; i < neighbors.size(); i++){
            if (neighbors.at(i) == node) {
                neighbors.erase(neighbors.begin() + i);
            }
        }
    }

};


bool isCycle(Node* startNode){
    for (Node* node: startNode->neighbors){
        if (node->color == WHITE){
                node->color = GREY;
                if (isCycle(node)) return true;
        }
        else if (node->color == GREY){
            return true;
        }
    }

    startNode->color = BLACK;
    return false;
}

int main() {

    Node n1(1), n2(2), n3(3), n4(4), n5(5);
    n1.link(&n2);
    n2.link(&n3);
    n3.link(&n5);

    cout << "Is there cycle: " << (isCycle(&n1) ? "YES" : "NO") << endl;
    return 0;
}

