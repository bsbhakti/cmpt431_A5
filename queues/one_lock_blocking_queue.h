#include "../common/allocator.h"
#include <mutex>
#include <condition_variable>


template <class T>
class Node
{
    public:
    T value;
    Node<T>* next;
};

extern std::atomic<bool> no_more_enqueues;

template <class T>
class OneLockBlockingQueue
{
    CustomAllocator my_allocator_;
    Node<T>* q_head;
    Node<T>* q_tail;
    std::mutex qLock;
    std::atomic<bool> wakeup_dq;   // signal for enqueuers to wake up waiting deq
    uint total_elements;
  

public:
    OneLockBlockingQueue() : my_allocator_()
    {
        std::cout << "Using OneLockBlockingQueue\n";
    }

    void initQueue(long t_my_allocator_size){
        std::cout << "Using Allocator\n";
        my_allocator_.initialize(t_my_allocator_size, sizeof(Node<T>)); //initalizing the allocator
        // Initialize the queue head or tail here
        Node<T>* newNode = (Node<T>*)my_allocator_.newNode(); //creating the node
        newNode->next = nullptr;
        q_head = q_tail = newNode;
        wakeup_dq.store(false);
        total_elements = 0;
    }

    void enqueue(T value)
    {
	    // add your enqueue code here
        Node<T> * newNode = (Node<T>* )my_allocator_.newNode();
        newNode->value = value;
        newNode->next = nullptr;
        qLock.lock();
        q_tail->next = newNode;
        q_tail = newNode;
        if(total_elements == 0){
            wakeup_dq.store(true);
        }
        total_elements ++;
        qLock.unlock();
    }

    bool dequeue(T *value)
    {
        bool ret = false;
        qLock.lock();
        Node<T>* removeNode = q_head;
        Node<T>* newHead = q_head->next;


        while(newHead == NULL){
            //q is empty -> wait 
            qLock.unlock();
            bool expected = true;
            bool desired = false;

            while(!wakeup_dq.compare_exchange_weak(expected, desired) && !no_more_enqueues.load()){
                expected = true;
            } //busy wait if wakeup_dq is false and no_more_enq is false
         
            qLock.lock();

            // q is empty and no more exq is true
            if (newHead == nullptr && no_more_enqueues.load()) {
                wakeup_dq.store(false);
                qLock.unlock();
                return false;
            }
        }
        q_head = newHead;
        *value = newHead->value;
        total_elements --;
        my_allocator_.freeNode(removeNode);
        qLock.unlock();
        return true;
    }

    void cleanup()
    {
        my_allocator_.cleanup();
    }
};
