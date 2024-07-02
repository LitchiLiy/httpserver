
#ifndef MEMORY_POOL_H
#define MEMORY_POOL_H

#include <climits>
#include <cstddef>
#include <type_traits>
#include <stdint.h>
#include <utility>  


template <typename T, size_t BlockSize = 4096>
class MemoryPool
{
public:
    /* Member types */
    typedef T               value_type;
    typedef T* pointer;
    typedef T& reference;
    typedef const T* const_pointer;
    typedef const T& const_reference;
    typedef size_t          size_type;
    typedef ptrdiff_t       difference_type;
    typedef std::false_type propagate_on_container_copy_assignment;
    typedef std::true_type  propagate_on_container_move_assignment;
    typedef std::true_type  propagate_on_container_swap;

    template <typename U> struct rebind {
        typedef MemoryPool<U> other;
    };

    /* Member functions */
    MemoryPool() noexcept;
    MemoryPool(const MemoryPool& memoryPool) noexcept;
    MemoryPool(MemoryPool&& memoryPool) noexcept;
    template <class U> MemoryPool(const MemoryPool<U>& memoryPool) noexcept;

    ~MemoryPool() noexcept;

    MemoryPool& operator=(const MemoryPool& memoryPool) = delete;
    MemoryPool& operator=(MemoryPool&& memoryPool) noexcept;

    pointer address(reference x) const noexcept;
    const_pointer address(const_reference x) const noexcept;

    // Can only allocate one object at a time. n and hint are ignored
    pointer allocate(size_type n = 1, const_pointer hint = 0);
    void deallocate(pointer p, size_type n = 1);

    size_type max_size() const noexcept;

    template <class U, class... Args> void construct(U* p, Args&&... args);
    template <class U> void destroy(U* p);

    template <class... Args> pointer newElement(Args&&... args);
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

    size_type padPointer(data_pointer_ p, size_type align) const noexcept;
    void allocateBlock();

    static_assert(BlockSize >= 2 * sizeof(slot_type_), "BlockSize too small.");
};


template <typename T, size_t BlockSize>
inline typename MemoryPool<T, BlockSize>::pointer
MemoryPool<T, BlockSize>::allocate(size_type n, const_pointer hint)
{
    if (freeSlots_ != nullptr) {
        pointer result = reinterpret_cast<pointer>(freeSlots_);
        freeSlots_ = freeSlots_->next;
        return result;
    }
    else {
        if (currentSlot_ >= lastSlot_)
            allocateBlock();
        return reinterpret_cast<pointer>(currentSlot_++);
    }
}



template <typename T, size_t BlockSize>
inline void
MemoryPool<T, BlockSize>::deallocate(pointer p, size_type n)
{
    if (p != nullptr) {
        reinterpret_cast<slot_pointer_>(p)->next = freeSlots_;
        freeSlots_ = reinterpret_cast<slot_pointer_>(p);
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
    new (p) U(std::forward<Args>(args)...); // 完美转发
}



template <typename T, size_t BlockSize>
template <class U>
inline void
MemoryPool<T, BlockSize>::destroy(U* p)
{
    p->~U();
}



template <typename T, size_t BlockSize>
template <class... Args>
inline typename MemoryPool<T, BlockSize>::pointer
MemoryPool<T, BlockSize>::newElement(Args&&... args)
{
    pointer result = allocate();
    construct<value_type>(result, std::forward<Args>(args)...);
    return result;
}



template <typename T, size_t BlockSize>
inline void
MemoryPool<T, BlockSize>::deleteElement(pointer p)
{
    if (p != nullptr) {
        p->~value_type();
        deallocate(p);
    }
}

#endif // MEMORY_POOL_H
