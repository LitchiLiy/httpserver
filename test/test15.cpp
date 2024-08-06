#include <memoryPool.h>
#include <iostream>

#include <memory>

/*
如果遇到以下问题
undefined reference to `MemoryPool<unsigned long, 4096ul>::allocate(unsigned long, unsigned long const*)'
说明这个模板需要显式实例化, 在cpp文件加一条
template class MemoryPool<unsigned long long, 4096ul>;在末尾就好了

输出: deadbedd
*/



int main() {
    MemoryPool<size_t, (ulong)4096> pool;
    size_t* x = pool.allocate(1);

    *x = 0xDEADBEDD;
    std::cout << std::hex << *x << std::endl;
    pool.deallocate(x);
    return 0;
}
