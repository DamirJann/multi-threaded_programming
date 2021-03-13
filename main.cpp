using namespace std;

class DiningPhilosophers {
public:
    DiningPhilosophers() {
    }



    void wantsToEat(int philosopher,
                    function<void()> pickLeftFork,
                    function<void()> pickRightFork,
                    function<void()> eat,
                    function<void()> putLeftFork,
                    function<void()> putRightFork) {



        // calculate left and right fork indexes
        int leftFork = (philosopher + 1) % 5;
        int rightFork = philosopher;

        // in order to avoid deadlock the first philosopher
        // firstly takes r-fork then l-fork
        if (philosopher == 0 or philosopher == 3){
            // if fork is taken wait
            // otherwise lock fork and go to the next line
            fork[rightFork].lock();
            pickRightFork();
            fork[leftFork].lock();
            pickLeftFork();
        }
            // common strategy - taking l-fork before r-fork
        else{
            fork[leftFork].lock();
            pickLeftFork();
            fork[rightFork].lock();
            pickRightFork();
        }


        // if you are here you took both forks and you're ready to eat
        eat();
        // firstly say about putting fork
        // then unlock resource
        // inverse order of action lead to error
        putLeftFork();
        fork[leftFork].unlock();
        putRightFork();
        fork[rightFork].unlock();
    }

private:
    // every fork is a resource, which can be either lock or unlock.
    array<mutex, 5> fork;
};