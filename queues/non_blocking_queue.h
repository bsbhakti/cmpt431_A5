#include "../common/allocator.h"

#define LFENCE asm volatile("lfence" : : : "memory")
#define SFENCE asm volatile("sfence" : : : "memory")

template<class P>
struct pointer_t {
    P* ptr;

    P* address(){
        // Get the address by getting the 48 least significant bits of ptr
        uintptr_t mask = 0x0000FFFFFFFFFFFF;
        uintptr_t ptrBits = reinterpret_cast<uintptr_t>(ptr) & mask; //reinterpret_cast allows casting between pointer types
        return reinterpret_cast<P*>(ptrBits);

    }
    uint count(){
        // Get the count from the 16 most significant bits of ptr
        uintptr_t ptrBits =reinterpret_cast<uintptr_t>(ptr);
        uint16_t count = static_cast<uint16_t>(ptrBits >> 48);
        return (count);
    }
};

template <class T>
class Node
{
public:
    T value;
    pointer_t<Node<T>> next;
};

template <class T,class P>
void make_address(P *node, uint count, pointer_t<Node<T>> *newPtr){
    uintptr_t castedPtr = reinterpret_cast<uintptr_t> (node);
    if(count == 0){
        count =1;
        std::cout<<"Incrementing count"<<count <<std::endl;
    }
    uintptr_t newCount = static_cast<uintptr_t>(count) << 48;
    uintptr_t combined = newCount | castedPtr;

    newPtr->ptr = reinterpret_cast<P*> (combined);
    // Node<T> * ptr = next.address();
    // uint count = next.count() +1;
    // // uintptr_t mask = 0xFFFFFFFFFFFF0000;
    // uintptr_t countBitsShifted = static_cast<uintptr_t>(count) << 48; //shift count by 48 places
    // uintptr_t res = countBitsShifted | reinterpret_cast<uintptr_t>(ptr); 
    // return pointer_t<Node<T>>{reinterpret_cast<Node<T>*>(res)};
}

template <class T>
class NonBlockingQueue
{
    pointer_t<Node<T>> q_head;
    pointer_t<Node<T>> q_tail;
    CustomAllocator my_allocator_;
public:
    
    NonBlockingQueue() : my_allocator_()
    {
        std::cout << "Using NonBlockingQueue\n";
    }

    void initQueue(long t_my_allocator_size){
        std::cout << "Using Allocator\n";
        my_allocator_.initialize(t_my_allocator_size, sizeof(Node<T>));
        // Initialize the queue head or tail here
        Node<T>* node = (Node<T>*)my_allocator_.newNode();
        node->next.ptr = NULL;
        // node->next
        q_head.ptr = q_tail.ptr = node;
        // my_allocator_.freeNode(newNode);
    }

    void enqueue(T value)
    {
        // Use LFENCE and SFENCE as mentioned in pseudocode
        Node <T> *node = (Node<T>* )my_allocator_.newNode();
        node->value = value;
        node->next.ptr = NULL;
        SFENCE;
        pointer_t<Node<T>> tail;
        while(true) {
            tail = q_tail;
            LFENCE;
            pointer_t<Node<T>> next = tail.address()->next;
            LFENCE;
            if (tail.ptr == q_tail.ptr){
                if (next.address() == NULL) {
                    // if(&tail.address()->next.compare_exchange_weak(next, next.count()+1>));
                    pointer_t<Node<T>> newPtr;
                    make_address(node,next.count() +1, &newPtr );
                    if(CAS(&(tail.address()->next), next, newPtr)) {
                        SFENCE;
                        std::cout<<"Logically enqueued"<<value<<std::endl;
                        std::cout<<"this is new made"<<newPtr.address()<<" "<<newPtr.count()<<std::endl;
                        std::cout<<"this is what we made new node with"<<node<<"  "<<next.count()+1<<std::endl;
                        break;
                    }
                }
                else {
                    pointer_t<Node<T>> newPtr;
                    make_address(next.address(),tail.count() + 1, &newPtr );
                    CAS(&q_tail, tail, newPtr);	// ELABEL
                    // std::cout<<"Trailing tail"<<q_tail.ptr->value <<std::endl;

                    // continue;
                }
            }
        }
        SFENCE;
        pointer_t<Node<T>> newPtr;
        make_address(node,tail.count() + 1, &newPtr );
        std::cout<<"Im here2"<<std::endl;
        CAS(&q_tail, tail, newPtr);
        SFENCE;
        std::cout<<"Im here3"<<std::endl;

        std::cout<<"Hopefuky tail is "<< q_tail.address() <<"look" <<std::endl;
        std::cout<<"Hopefuky tail is "<< q_tail.address()->value <<"look" <<std::endl;


        // std::cout<<"here write code1"<<std::endl;
    }

    bool dequeue(T *value)
    {
        // Use LFENCE and SFENCE as mentioned in pseudocode
        pointer_t<Node<T>> head;
        std::cout<<"inside dequeue"<<std::endl;
        while(true){
            // std::cout<<"inside dequeue 0.1"<<std::endl;

            head = q_head;
            // std::cout<<"inside dequeue 0.2"<<std::endl;

            LFENCE;
            // std::cout<<"inside dequeue 0.3"<<std::endl;

            pointer_t<Node<T>> tail = q_tail;
            // std::cout<<"inside dequeue 0.4"<<std::endl;

            LFENCE;
            // std::cout<<"inside dequeue 0.5"<<std::endl;

            pointer_t<Node<T>> next = head.address()->next;
            // std::cout<<"inside dequeue 0.6"<<std::endl;

            LFENCE;
            // std::cout<<"inside dequeue 1"<<std::endl;

            if (head.ptr == q_head.ptr) {
                std::cout<<"inside dequeue 1a"<<std::endl;

                if(head.address() == tail.address()) {
                    if(next.address() == NULL)
                            return false;
                    std::cout<<"inside dequeue 2"<<std::endl;
                    
                    pointer_t<Node<T>> newPtr;
                    make_address(next.address(),tail.count() + 1, &newPtr );
                    CAS(&q_tail, tail, newPtr);	//DLABEL
                }
                else {
                    std::cout<<"inside dequeue 3a"<<std::endl;

                    // *value = next.address()->value;
                    pointer_t<Node<T>> newPtr;
                    make_address(next.address(),head.count() + 1, &newPtr );
                    std::cout<<"inside dequeue 3b dequeing"<<q_head.ptr->value<<std::endl;

                    if(CAS(&q_head, head, newPtr))
                        // std::cout<<"inside dequeue 3c new head:"<<q_head.ptr->value<<std::endl;
                        break;
                }
            }
        }
        std::cout<<"inside dequeue 4"<<std::endl;
        
        my_allocator_.freeNode(head.address());
        std::cout<<"inside dequeue 5"<<std::endl;

        return true;

        // bool ret_value = false;
        // return ret_value;
    }

    void cleanup()
    {
        my_allocator_.cleanup();
    }

};

