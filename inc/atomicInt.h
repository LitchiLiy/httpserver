#if !defined(AUTOMATICINT_H)    
#define AUTOMATICINT_H



#include <atomic>
#include <stdint.h>

typedef int64_t seq_t;




template <typename T>
class AtomicInt
{
public:
    AtomicInt() : m_val(0) {}

    T get() { return m_val.load(std::memory_order_seq_cst); }

    // 将delta加到原来的值上, 返回原来的值
    T getAndAdd(T delta) { return m_val.fetch_add(delta, std::memory_order_seq_cst); }

    // 将x加到原来的值上, 返回加后的值
    T addAndGet(T x)
    {
        return getAndAdd(x) + x;
    }

    /*
        @brief 获得一个加一之后的序列号, 唯一的
    */
    T incrementAndGet()
    {
        return addAndGet(1);
    }

private:
    std::atomic<T> m_val; // T类型的原始值
};

typedef AtomicInt<int64_t> AtomicInt64;
typedef AtomicInt<int32_t> AtomicInt32;

#endif // AUTOMATICINT_H)   
