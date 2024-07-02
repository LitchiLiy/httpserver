#include <memory_pool.h>

template <typename T, size_t BlockSize>
MemoryPool<T, BlockSize>::MemoryPool() {
    currentBlock_ = nullptr;
    currentSlot_ = nullptr;
    lastSlot_ = nullptr;
    freeSlots_ = nullptr;
}


// 移动构造初始化, 对一个即将销毁的对象取代他们的资源
template <typename T, size_t BlockSize>
MemoryPool<T, BlockSize>::MemoryPool(MemoryPool&& memoryPool)
{
    currentBlock_ = memoryPool.currentBlock_;
    memoryPool.currentBlock_ = nullptr;
    currentSlot_ = memoryPool.currentSlot_;
    lastSlot_ = memoryPool.lastSlot_;
    freeSlots_ = memoryPool.freeSlots;
}



template <typename T, size_t BlockSize>
template<class U>
MemoryPool<T, BlockSize>::MemoryPool(const MemoryPool<U>& memoryPool)
noexcept :
    MemoryPool()
{}


// 右值， 移动赋值运算符， 取代资源
template <typename T, size_t BlockSize>
MemoryPool<T, BlockSize>&
MemoryPool<T, BlockSize>::operator=(MemoryPool&& memoryPool)
noexcept
{
    if (this != &memoryPool)
    {
        std::swap(currentBlock_, memoryPool.currentBlock_);
        currentSlot_ = memoryPool.currentSlot_;
        lastSlot_ = memoryPool.lastSlot_;
        freeSlots_ = memoryPool.freeSlots;
    }
    return *this;
}



template <typename T, size_t BlockSize>
MemoryPool<T, BlockSize>::~MemoryPool()
noexcept
{
    slot_pointer_ curr = currentBlock_;
    while (curr != nullptr) {
        slot_pointer_ prev = curr->next;
        operator delete(reinterpret_cast<void*>(curr));
        curr = prev;
    }
}



template <typename T, size_t BlockSize>
inline typename MemoryPool<T, BlockSize>::pointer
MemoryPool<T, BlockSize>::address(reference x)
const noexcept
{
    return &x;
}
template <typename T, size_t BlockSize>
inline typename MemoryPool<T, BlockSize>::const_pointer
MemoryPool<T, BlockSize>::address(const_reference x)
const noexcept
{
    return &x;
}


// 分配一个新地址给私有变量. 构建一个新block. 对齐地址.
// 负数也能取模, a % b只要保证b是正数, 那么结果就如你所想那样结果都是正数, b是负数则未定义. 
template <typename T, size_t BlockSize>
void
MemoryPool<T, BlockSize>::allocateBlock()
{
    // operator new是一个全局运算， 分配原始内存， 通常带一个数字。
    data_pointer_ newBlock = reinterpret_cast<data_pointer_>
        (operator new(BlockSize));
    reinterpret_cast<slot_pointer_>(newBlock)->next = currentBlock_;  // 当前block的地址
    currentBlock_ = reinterpret_cast<slot_pointer_>(newBlock);  // 更新地址. currBlock获得新地址. 新地址的next指向老地址
    // Pad block body to staisfy the alignment requirements for elements
    data_pointer_ body = newBlock + sizeof(slot_pointer_);  // 指向element成员内存地址
    size_type bodyPadding = padPointer(body, alignof(slot_type_)); // 获得对齐后的内存地址, alignof表示最优对其值.
    currentSlot_ = reinterpret_cast<slot_pointer_>(body + bodyPadding);  // 指向真正位置的element地址
    lastSlot_ = reinterpret_cast<slot_pointer_>   // 最后一个slot地址.
        (newBlock + BlockSize - sizeof(slot_type_) + 1);
}



template <typename T, size_t BlockSize>
inline typename MemoryPool<T, BlockSize>::pointer
MemoryPool<T, BlockSize>::allocate(size_type n, const_pointer hint)
{
    if (freeSlots_ != nullptr) {
        pointer result = reinterpret_cast<pointer>(freeSlots_);
        freeSlots_ = freeSlots_->next;  // freeslot有, 则从free中给一个, 
        return result;
    }
    else {
        if (currentSlot_ >= lastSlot_)
            allocateBlock();  // 分配新slotBlock
        return reinterpret_cast<pointer>(currentSlot_++); // 否则给一个新slot. currentSlot是union类型的指针, ++ 直接移动到下一个Slot地址中去了.
    }
}


// 删除一个slot上面的内容就是: 将slot的next指向frede, 然后free成为自己, 但是没有删除内存中原有的数据.
template <typename T, size_t BlockSize>
inline void
MemoryPool<T, BlockSize>::deallocate(pointer p, size_type n)
{
    if (p != nullptr) {
        reinterpret_cast<slot_pointer_>(p)->next = freeSlots_;  // next->frede
        freeSlots_ = reinterpret_cast<slot_pointer_>(p); // 本体也是free(虽然上面可能有数据)
    }
}



template <typename T, size_t BlockSize>
inline typename MemoryPool<T, BlockSize>::size_type
MemoryPool<T, BlockSize>::max_size()
const noexcept
{
    size_type maxBlocks = -1 / BlockSize;
    return (BlockSize - sizeof(data_pointer_)) / sizeof(slot_type_) * maxBlocks;
}



template <typename T, size_t BlockSize>
template <class U, class... Args>
inline void
MemoryPool<T, BlockSize>::construct(U* p, Args&&... args)
{
    new (p) U(std::forward<Args>(args)...);
}


// destory实现: 调用U类型的析构函数. 
template <typename T, size_t BlockSize>
template <class U>
inline void
MemoryPool<T, BlockSize>::destroy(U* p)
{
    p->~U();
}


// 从内存中分配一个slot, 然后给这个slot赋值一个new的T类型的对象., 返回这个slot的地址给你
template <typename T, size_t BlockSize>
template <class... Args>
inline typename MemoryPool<T, BlockSize>::pointer
MemoryPool<T, BlockSize>::newElement(Args&&... args)
{
    pointer result = allocate();
    construct<value_type>(result, std::forward<Args>(args)...);
    return result;
}


// 析构这个对象之后, 然后把内存放入freeslto中.
template <typename T, size_t BlockSize>
inline void
MemoryPool<T, BlockSize>::deleteElement(pointer p)
{
    if (p != nullptr) {
        p->~value_type();
        deallocate(p);
    }
}


template <typename T, size_t BlockSize>
inline typename MemoryPool<T, BlockSize>::size_type
MemoryPool<T, BlockSize>::padPointer(data_pointer_ p, size_type align)
const noexcept
{
    uintptr_t result = reinterpret_cast<uintptr_t>(p);
    return ((align - result) % align);
}