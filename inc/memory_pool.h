#if !defined(__MEMORY_POOL_H__)
#define __MEMORY_POOL_H__

#include <climits>
#include <cstddef>

// T类型的内存池.
template<typename T, size_t BlockSize = 4096>
class MemoryPool {
public:
    // 定义一系列T类型的指针.
    typedef T value_type;
    typedef T* pointer;
    typedef T& reference;
    typedef const T* const_pointer;
    typedef const T& const_reference;
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
    typedef std::false_type propagate_on_container_move_assignment;
    typedef std::true_type propagate_on_container_move_assignment;
    typedef std::true_type propagate_on_container_copy_assignment;

    template<typename U> struct rebind {
        typedef MemoryPool<U> other;
    }

    MemoryPool();
    // MemoryPool(const MemoryPool& memoryPool);

    MemoryPool(MemoryPool&& memoryPool);  // 右值引用
    MemoryPool& operator=(MemoryPool&&);

    template<typename U>
    MemoryPool(const MemoryPool<U>& memoryPool);

    ~MemoryPool();
    // 禁用复制构造
    MemoryPool& operator=(const MemoryPool&) = delete;

    // 函数api
    // 返回T类型的指针地址。
    pointer address(reference x) const;
    const_pointer address(const_reference x) const;

    pointer allocate(size_type n = 1, const_pointer hint = 0);
    void deallocate(pointer p, size_type n = 1);

    size_type max_size() const;

    template<class U, class... Args> void construct(U* p, Args&&... args);
    template<class U> void destroy(U* p);

    template<class... Args> pointer newElement(Args&&... args);
    void deleteElement(pointer p);

private:
    union Slot_ {
        value_type element;
        Slot_* next;
    };

    typedef char* data_pointer_;
    typedef Slot_ slot_type_;
    typedef Slot_* slot_pointer_;

    slot_pointer_ currentBlock_;
    slot_pointer_ currentSlot_;
    slot_pointer_ lastSlot_;
    slot_pointer_ freeSlots_;

    size_type padPointer(data_pointer_ p, size_type align) const;
    void allocateBlock();

    static_assert(BlockSize >= 2 * sizeof(slot_type_), "BlockSize too small.");

};



#endif // __MEMORY_POOL_H__)
