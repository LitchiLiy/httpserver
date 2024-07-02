/*
    测试内存池的使用

不知道为什么, 官方的代码Alloc和memory_pool不能弄在一起, 测不动, 搞不了

不知道为什么我把inline的成员函数全丢到.h文件里就能跑了
*/

#include <iostream>
#include <cassert>
#include <time.h>
#include <vector>

#include <memoryPool.h>
#include <stackAlloc.h>



#define ELEMS 1000000
#define REPS 50

int main()
{
    clock_t start;



    // 首先压力测试stackAlloc这个东西.
    StackAlloc<int, std::allocator<int> > stackDefault;  // 构建一个stack, 记录alloc的int
    start = clock();
    for (int j = 0; j < REPS; j++)
    {
        assert(stackDefault.empty());
        for (int i = 0; i < ELEMS / 4; i++) {
            // Unroll to time the actual code and not the loop
            stackDefault.push(i);
            stackDefault.push(i);
            stackDefault.push(i);
            stackDefault.push(i);
        }
        for (int i = 0; i < ELEMS / 4; i++) {
            // Unroll to time the actual code and not the loop
            stackDefault.pop();
            stackDefault.pop();
            stackDefault.pop();
            stackDefault.pop();
        }
    }
    std::cout << "Default Allocator Time: ";
    std::cout << (((double)clock() - start) / CLOCKS_PER_SEC) << "\n\n";

    // // stackAlloc测试成功之后, 看看MemoryPool这个东西能不能成.
    StackAlloc<int, MemoryPool<int> > stackPool;
    start = clock();
    for (int j = 0; j < REPS; j++)
    {
        assert(stackPool.empty());
        for (int i = 0; i < ELEMS / 4; i++) {
            // Unroll to time the actual code and not the loop
            stackPool.push(i);
            stackPool.push(i);
            stackPool.push(i);
            stackPool.push(i);
        }
        for (int i = 0; i < ELEMS / 4; i++) {
            // Unroll to time the actual code and not the loop
            stackPool.pop();
            stackPool.pop();
            stackPool.pop();
            stackPool.pop();
        }
    }
    std::cout << "MemoryPool Allocator Time: ";
    std::cout << (((double)clock() - start) / CLOCKS_PER_SEC) << "\n\n";


    std::cout << "Here is a secret: the best way of implementing a stack"
        " is a dynamic array.\n";

    // 测试一下VEC的速度
    std::vector<int> stackVector;
    start = clock();
    for (int j = 0; j < REPS; j++)
    {
        assert(stackVector.empty());
        for (int i = 0; i < ELEMS / 4; i++) {
            // Unroll to time the actual code and not the loop
            stackVector.push_back(i);
            stackVector.push_back(i);
            stackVector.push_back(i);
            stackVector.push_back(i);
        }
        for (int i = 0; i < ELEMS / 4; i++) {
            // Unroll to time the actual code and not the loop
            stackVector.pop_back();
            stackVector.pop_back();
            stackVector.pop_back();
            stackVector.pop_back();
        }
    }
    std::cout << "Vector Time: ";
    std::cout << (((double)clock() - start) / CLOCKS_PER_SEC) << "\n\n";

    std::cout << "The vector implementation will probably be faster.\n\n";
    std::cout << "MemoryPool still has a lot of uses though. Any type of tree"
        " and when you have multiple linked lists are some examples (they"
        " can all share the same memory pool).\n";

    return 0;
}