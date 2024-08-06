/*
    测试LFUCache这个类有没有用, 主要是调用get和set两个函数对其进行输入就行.
*/


#include <lfuCache.h>
#include <string>
#include <iostream>


using namespace std;

int main() {

    LFUCache* cache = &getCache();
    // 产生一个随机数, 这个随机数是整数, 范围在26到1之间

    string keyC = "abcdefghijklmnopqrstuvwxyz";
    for (int i = 0; i < 1000; i++) {
        int randNum = rand() % 26;
        char kkey = keyC[randNum];

        string key = string(1, kkey);
        string ret{};
        string val = string(1, kkey - 'a' + 'A');
        if (cache->get(key, ret)) {
            // printf("cache hit \t key = %c val = %c\n", key.c_str(), val.c_str());
            cout << "cache hit " << key << " " << ret << endl;

        }
        else {
            cache->set(key, val);
            printf("cache miss \t key = %s val = %s\n", key.c_str(), val.c_str());
        }
    }

    return 0;
}
