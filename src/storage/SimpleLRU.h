#ifndef AFINA_STORAGE_SIMPLE_LRU_H
#define AFINA_STORAGE_SIMPLE_LRU_H

#include <map>
#include <memory>
#include <mutex>
#include <string>

#include <afina/Storage.h>

namespace Afina {
namespace Backend {

/**
 * # Map based implementation
 * That is NOT thread safe implementaiton!!
 */
class SimpleLRU : public Afina::Storage {
public:
    SimpleLRU(size_t max_size = 1024) : _max_size(max_size) {}
    SimpleLRU(const SimpleLRU&) = delete;
    SimpleLRU& operator=(const SimpleLRU&) = delete;

    ~SimpleLRU() {
        _lru_index.clear();
        if (_lru_head) {
            while (_lru_head->next) {
                std::unique_ptr<lru_node> tmp;
                tmp.swap(_lru_head->next);
                _lru_head.swap(tmp);
                tmp.reset();
            }
            _lru_head.reset();
        }
    }

    // Implements Afina::Storage interface
    bool Put(const std::string &key, const std::string &value) override;

    // Implements Afina::Storage interface
    bool PutIfAbsent(const std::string &key, const std::string &value) override;

    // Implements Afina::Storage interface
    bool Set(const std::string &key, const std::string &value) override;

    // Implements Afina::Storage interface
    bool Delete(const std::string &key) override;

    // Implements Afina::Storage interface
    bool Get(const std::string &key, std::string &value) override;

private:


    // LRU cache node
    using lru_node = struct lru_node {
        std::string key;
        std::string value;
        lru_node* prev;
        std::unique_ptr<lru_node> next;

    };

    void _cut(lru_node& node);
    //void _pop_back();
    void _pop_back();
    //void _push_front(lru_node* node);
    void _push_front(lru_node *node);
    // Maximum number of bytes could be stored in this cache.
    // i.e all (keys+values) must be less the _max_size
    std::size_t _max_size;
    std::size_t _cur_size = 0;
    // Main storage of lru_nodes, elements in this list ordered descending by "freshness": in the head
    // element that wasn't used for longest time.
    //
    // List owns all nodes
    std::unique_ptr<lru_node> _lru_head = nullptr;
    lru_node* _lru_tail = nullptr;

    // Index of nodes from list above, allows fast random access to elements by lru_node#key
    std::map<std::reference_wrapper<const std::string>, std::reference_wrapper<lru_node>, std::less<std::string>> _lru_index;
};

} // namespace Backend
} // namespace Afina

#endif // AFINA_STORAGE_SIMPLE_LRU_H
