
#ifndef STACK_ALLOC_H
#define STACK_ALLOC_H

#include <memory>

/*
一个用链表构建成的堆栈类, 用来保存分配的空间对象来保存T类型的元素
*/



template <typename T>
struct StackNode_
{
    T data;
    StackNode_* prev;
};

// T是想要保存的类型, 后面的Alloc是内存分配器
template <class T, class Alloc = std::allocator<T> >
class StackAlloc
{
public:
    typedef StackNode_<T> Node;
    typedef typename Alloc::template rebind<Node>::other allocator;

    /** Default constructor */
    StackAlloc() { head_ = 0; }
    /** Default destructor */
    ~StackAlloc() { clear(); }

    /** Returns true if the stack is empty */
    bool empty() { return (head_ == 0); }

    // 清楚堆栈中的Alloc对象所有的内存
    void clear() {
        Node* curr = head_;
        while (curr != 0)
        {
            Node* tmp = curr->prev;
            allocator_.destroy(curr);
            allocator_.deallocate(curr, 1);
            curr = tmp;
        }
        head_ = 0;
    }

    // 堆栈中压入一个T类型的元素, 用Node包装起来压入栈中.
    void push(T element) {
        Node* newNode = allocator_.allocate(1);
        allocator_.construct(newNode, Node());
        newNode->data = element;
        newNode->prev = head_;
        head_ = newNode;
    }

    // 从栈顶中推出一个T类型的元素的分配对象
    T pop() {

        T result = head_->data;
        Node* tmp = head_->prev;
        allocator_.destroy(head_);
        allocator_.deallocate(head_, 1);
        head_ = tmp;
        return result;

    }

    /** Return the topmost element */
    T top() { return (head_->data); }

private:
    allocator allocator_;
    Node* head_;
};

#endif // STACK_ALLOC_H