#if !defined(STRINGPIECE_H) 
#define STRINGPIECE_H


#include <string.h>
#include <string>

// 不保存任何数据, 只是一个指针和长度的记录类
using namespace std;
class StringPiece {
public:
    StringPiece() : m_data(nullptr), m_len(0) {}
    StringPiece(const char* d, int l) : m_data(d), m_len(l) {}
    StringPiece(const std::string& str) : m_data(str.data()), m_len(str.size()) {}
    StringPiece(const char* ptr) : m_data(ptr), m_len(strlen(ptr)) {}

    const char* data() const { return m_data; }
    int length() const { return m_len; }

    string as_string() const {
        return string(data(), length());
    }

private:
    const char* m_data;
    int m_len;
};



#endif // STRINGPIECE_H)    
