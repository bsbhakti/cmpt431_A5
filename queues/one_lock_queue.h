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
        // Node<T>* newNode = (Node<T>*)my_allocator_.newNode(); //creating the node
        // newNode->next = nullptr;
        // newNode->value = std::numeric_limits<double>::infinity();
        q_head = q_tail = nullptr;

        // my_allocator_.freeNode(newNode); // why are we freeing it??
    }

    void enqueue(T value)
    {   qLock.lock();
        if(q_tail == nullptr){
            Node<T> * newNode = (Node<T>* )my_allocator_.newNode();
            newNode->value = value;
            newNode->next = nullptr;
            q_tail = newNode;
            q_head = newNode;
            // std::cout <<"inserting"<<q_tail->value<<std::endl;
        }
        else {
            Node<T> * newNode = (Node<T>* )my_allocator_.newNode();
            newNode->value = value;
            newNode->next = nullptr;
            // std::cout <<"inserting"<<value<<std::endl;
            // Append to q_tail and update the queue
            q_tail->next = newNode;
            q_tail = newNode;
            // std::cout <<"e head: "<<q_head->value<<"tail: "<<q_tail->value<<std::endl;
        }
        qLock.unlock();

    }

    bool dequeue(T *value)
    {   
        // return true;
        bool ret = false;
        qLock.lock();
        Node<T>* removeNode = q_head;
        // std::cout <<"d head: "<<q_head->value<<"tail: "<<q_tail->value<<std::endl;

        if(removeNode == nullptr){
            // std::cout<<"head is null returning false"<<std::endl;
            qLock.unlock();
            return false;
        }
        else {
            ret = true;
            Node<T>* newHead = q_head->next;
            if(newHead != NULL){
                // Queue is not empty
                T p_value = newHead->value; //why?
                // Update q_head
                // std::cout<<"head and next is not null returning true new head: "<<p_value<<std::endl;
                q_head = newHead;
            }
            else {
                // std::cout<<"head is not null, next is null, returning true"<<std::endl;
                q_head = NULL;
                q_tail = NULL;
            }
        }
        // std::cout<<"head is not null returning true dequeing "<<removeNode->value<<std::endl;
        *value = removeNode->value;
        my_allocator_.freeNode(removeNode);
        qLock.unlock();
        // std::cout<<"DONE deq"<<std::endl;

        return ret;
    }

    void cleanup()
    {
        my_allocator_.cleanup();
    }
};