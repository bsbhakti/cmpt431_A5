#include "../common/allocator.h"
#include <mutex>
#include <limits> 

template <class T>
class Node
{
    public:
    T value;
    Node<T>* next;
};

template <class T>
class TwoLockQueue
{
    Node<T>* q_head;
    Node<T>* q_tail;
    CustomAllocator my_allocator_;
    std::mutex qLockEnq;
    std::mutex qLockDeq;


public:
    TwoLockQueue() : my_allocator_()
    {
        std::cout << "Using OneLockQueue\n";
    }

    void initQueue(long t_my_allocator_size){
        std::cout << "Using Allocator\n";
        my_allocator_.initialize(t_my_allocator_size, sizeof(Node<T>)); //initalizing the allocator
        // Initialize the queue head or tail here
        Node<T>* newNode = (Node<T>*)my_allocator_.newNode(); //creating the node
        newNode->next = nullptr;
        q_head = q_tail = newNode;

        // my_allocator_.freeNode(newNode); // why are we freeing it??
    }

    void enqueue(T value)
    {     
        Node<T> * newNode = (Node<T>* )my_allocator_.newNode();
        newNode->value = value;
        newNode->next = nullptr;
        qLockEnq.lock();
        q_tail->next = newNode;
        q_tail = newNode;
        qLockEnq.unlock();
    }

    bool dequeue(T *value)
    {   
        // return true;
        bool ret = false;
        qLockDeq.lock();
        Node<T>* removeNode = q_head;
        Node<T>* newHead = q_head->next;

        if(newHead == NULL){
            qLockDeq.unlock();
            return false;
        }
        
        q_head = newHead;
        *value = newHead->value;
        my_allocator_.freeNode(removeNode);
        qLockDeq.unlock();
        return true;
    }

    void cleanup()
    {
        my_allocator_.cleanup();
    }
};