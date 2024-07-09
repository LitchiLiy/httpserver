#if !defined(LFU_CACHE_H)
#define LFU_CACHE_H
#include <string>
#include <mutex>
#include <unordered_map>


#define LFU_CAPACITY 10 

using std::string;

template<typename T>
class Node {
public:
    void setPre(Node* p) { pre_ = p; };
    void setNext(Node* p) { next_ = p; };
    Node* getPre() { return pre_; };
    Node* getNext() { return next_; };
    T& getValue() { return value; };

private:
    Node* pre_;
    Node* next_;
    T value;
};

// 文件名到文件的映射
struct Key
{
    string key_, value_;
};

typedef Node<Key>* key_node;

class KeyList {

public:
    void init(int freq);
    void destory();
    int getFreq() { return freq_; }
    void add(key_node& node);
    void del(key_node& node);
    bool isEmpty() { return Dummyhead_ == tail_; };
    key_node getLast() { return tail_; };

private:
    int freq_;
    key_node Dummyhead_; // 头节点始终在头部不会动.
    key_node tail_;
};

typedef Node<KeyList>* freq_node;


class LFUCache {
private:
    freq_node Dummyhead_; // 大链表头节点, 里面每个节点都是小链表的头节点
    size_t capacity_;
    std::mutex mtx_;

    std::unordered_map<string, key_node> kmap_;
    std::unordered_map<string, freq_node> fmap_;

    void addFreq(key_node& nowk, freq_node& nowf);
    void del(freq_node& node);
    void init();

public:
    LFUCache() = default;
    LFUCache(int capicity);
    ~LFUCache();

    bool get(string& key, string& value);  // 通过key返回value并进行LFU操作
    void set(string& key, string& value);   // 更新LFU缓存
    size_t getCapacity() const { return capacity_; }
};








void KeyList::init(int freq) {
    freq_ = freq;
    Dummyhead_ = new Node<Key>();
    tail_ = Dummyhead_;
    Dummyhead_->setNext(nullptr);
}
void KeyList::destory() {
    while (Dummyhead_ != nullptr) {
        key_node pre = Dummyhead_;
        Dummyhead_ = Dummyhead_->getNext();
        delete pre;
    }
}
void KeyList::add(key_node& node) {    // 把新添加的node塞到dummy之后.
    if (Dummyhead_->getNext() == nullptr) {
        tail_ = node;
    }
    else {
        Dummyhead_->getNext()->setPre(node);
    }
    node->setNext(Dummyhead_->getNext());
    node->setPre(Dummyhead_);
    Dummyhead_->setNext(node);
}
void KeyList::del(key_node& node) {
    node->getPre()->setNext(node->getNext());

    if (node->getNext() == nullptr) {
        tail_ = node->getPre();
    }
    else {
        node->getNext()->setPre(node->getPre());
    }
}

LFUCache::LFUCache(int capicity) :
    capacity_(capicity) {
    init();
}

LFUCache::~LFUCache() {
    while (Dummyhead_) {
        freq_node pre = Dummyhead_;
        Dummyhead_ = Dummyhead_->getNext();
        pre->getValue().destory();
        delete pre;
    }
}

void LFUCache::init() {
    Dummyhead_ = new Node<KeyList>();
    Dummyhead_->getValue().init(0);
    Dummyhead_->setNext(nullptr);
}

// 更新节点, 如果不存在则创建一个频率的节点, 然后把该节点放在头位置
void LFUCache::addFreq(key_node& nowk, freq_node& nowf) {
    freq_node nxt;
    if (nowf->getNext() == nullptr || nowf->getNext()->getValue().getFreq() != nowf->getValue().getFreq() + 1) {
        nxt = new Node<KeyList>();
        nxt->getValue().init(nowf->getValue().getFreq() + 1);
        if (nowf->getNext() != nullptr) {
            nowf->getNext()->setPre(nxt);
        }

        nxt->setNext(nowf->getNext());
        nowf->setNext(nxt);
        nxt->setPre(nowf);
    }
    else {
        nxt = nowf->getNext();
    }

    fmap_[nowk->getValue().key_] = nxt;

    if (nowf != Dummyhead_) {
        nowf->getValue().del(nowk);
    }
    nxt->getValue().add(nowk);

    // assert(!nxt->getValue().isEmpty());
    if (nowf != Dummyhead_ && nowf->getValue().isEmpty())
        del(nowf);
}

/**
 * @brief 输入一个key， 从val中返回value += 原本的频度值, 这里的val不是键值对的val和set不同, 这里的val返回的是频度
 *
 * @param key 键值对的key
 * @param val 返回该key在内部保存的频度
 * @return true
 * @return false
 */
bool LFUCache::get(string& key, string& val) {
    if (!capacity_) return false;
    std::lock_guard<std::mutex> lock(mtx_);
    if (fmap_.find(key) != fmap_.end()) {
        // 缓存命中
        key_node nowk = kmap_[key];
        freq_node nowf = fmap_[key];
        val += nowk->getValue().value_;
        addFreq(nowk, nowf);
        return true;
    }
    // 未命中
    return false;
}


/**
 * @brief 输入key, 如果用get没在列表中找到这个key, 就调用这个函数用来在map中新建一个key-value, key是键, val是值
 *
 * @param key 键值对的key
 * @param val 键值对的value
 */
void LFUCache::set(string& key, string& val) {
    if (!capacity_) return;
    // printf("kmapsize = %d capacity = %d\n", kmap_.size(), capacity_);
    std::lock_guard<std::mutex> lock(mtx_);
    // 缓存满了
    // 从频度最⼩的⼩链表中的节点中删除最后⼀个节点（⼩链表中的删除符合LRU）
    if (kmap_.size() == capacity_) {
        // printf("need to delete a Node\n");
        freq_node head = Dummyhead_->getNext();
        key_node last = head->getValue().getLast();
        head->getValue().del(last);
        kmap_.erase(last->getValue().key_);
        fmap_.erase(last->getValue().key_);
        // delete last;
        delete last;
        // 如果频度最⼩的链表已经没有节点，就删除
        if (head->getValue().isEmpty()) {
            del(head);
        }
    }
    key_node nowk = new Node<Key>();


    nowk->getValue().key_ = key;
    nowk->getValue().value_ = val;
    addFreq(nowk, Dummyhead_);
    kmap_[key] = nowk;
    fmap_[key] = Dummyhead_->getNext();
}
void LFUCache::del(freq_node& node) {
    node->getPre()->setNext(node->getNext());
    if (node->getNext() != nullptr) {
        node->getNext()->setPre(node->getPre());
    }
    node->getValue().destory();
    delete node;


}
LFUCache& getCache() {
    static LFUCache cache(LFU_CAPACITY);
    return cache;
}

#endif // LFU_CACHE_H)