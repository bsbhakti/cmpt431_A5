#include "../common/allocator.h"

template <class T>
class Node
{
    public:
    T value;
    Node<T>* next;
};

template <class T>
class OneLockQueue
{
    Node<T>* q_head;
    Node<T>* q_tail;
    CustomAllocator my_allocator_;
    std::mutex qLock;

public:
    OneLockQueue() : my_allocator_()
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

        // Node<T> *prev = nullptr;
        // for(long i = 0; i < t_my_allocator_size; i++){
        //     Node<T>* newNode = (Node<T>*)my_allocator_.newNode(); //creating the node
        //     newNode->next = nullptr;

        //     if(i == 0){
        //         q_head = q_tail = newNode;
        //     }
        //     else {
        //         prev->next == newNode;
        //         q_tail = newNode;
        //     }
        //     prev = newNode;
        // }
       
        // my_allocator_.freeNode(newNode); // why are we freeing it??
    }

    void enqueue(T value)
    {
        Node<T> * newNode = (Node<T>* )my_allocator_.newNode();
        newNode->value = value;
        newNode->next = NULL;
        // Append to q_tail and update the queue
        qLock.lock();
        q_tail->next = newNode;
        q_tail = newNode;
        qLock.unlock();
    }

    bool dequeue(T *value)
    {   
        bool ret = false;
        qLock.lock();
        Node<T>* removeNode = q_head;
        Node<T>* newHead = q_head->next;
        if(newHead != NULL){
            // Queue is not empty
            T p_value = newHead->value; //why?
            // Update q_head
            q_head = newHead;
            ret = true;
        }
        qLock.unlock();
        my_allocator_.freeNode(removeNode);
        return ret;
    }

    void cleanup()
    {
        my_allocator_.cleanup();
    }
};