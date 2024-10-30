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
        // std::cout<<"enqueueing"<<q_tail->value<<std::endl;
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
            
            // std::cout<<"waiting"<<wakeup_dq.load()<<no_more_enqueues.load()<<std::endl;
            //   Wait until enqueuer wakes me up OR no more enqueues are coming
            //wait until enq is false OR no more is false
            while(!wakeup_dq.load() && !no_more_enqueues.load() );
            

             //busy wait if wakeup_dq is false and no_more_enq is false
            // std::cout<<"done waiting"<<std::endl;

         
            qLock.lock();
            if(q_head != nullptr){
                newHead = q_head->next;
            }
            else {
                newHead = nullptr;
            }
            if(newHead != nullptr){
                removeNode = q_head;
                newHead = q_head->next;
                wakeup_dq.store(true);
            }
        
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
        // std::cout<<"deq"<<*value<<std::endl;
        my_allocator_.freeNode(removeNode);
        qLock.unlock();
        return true;
    }

    void cleanup()
    {
        my_allocator_.cleanup();
    }
};
