#include <timer.h>
#include <iostream>
#include <vector>


void print() {
    std::cout << "hello" << std::endl;
}

int main() {
    // 创建10个Timer
    std::vector<Timer> timers;
    for (int i = 0; i < 10; ++i) {
        timers.push_back(Timer(print, Timestamp(), 1.0));
    }
    return 0;
}
