#include "SimpleLRU.h"

namespace Afina {
    namespace Backend {

        void SimpleLRU::_cut(lru_node &node, bool save) {
            _cur_size -= node.key.size() + node.value.size();
            if (!node.prev) {
                _lru_head.swap(node.next);
                if (save)
                    node.next.release();
                else
                    node.next.reset();
                return;
            }

            if (!node.next) {
                _lru_tail = node.prev;
                if (save)
                    node.prev->next.release();
                else
                    node.prev->next.reset();
                return;
            }

            node.next->prev = node.prev;
            node.prev->next.swap(node.next);
            if (save)
                node.next.release();
            else
                node.next.reset();
            node.prev = nullptr;
        }


        void SimpleLRU::_pop_back() {
            _cur_size -= _lru_tail->key.size() + _lru_tail->value.size();

            _lru_index.erase(_lru_tail->key);
            _lru_tail = _lru_tail->prev;
            _lru_tail->next.reset();
        }

        void SimpleLRU::_push_front(lru_node *node) {

            while (_cur_size + (node->key.size() + node->value.size()) > _max_size)
                _pop_back();

            _cur_size += node->key.size() + node->value.size();

            if (_lru_head) {
                auto tmp = _lru_head.release();
                tmp->prev = node;
                node->prev = nullptr;
                node->next.reset(tmp);
            } else {
                _lru_tail = node;
                node->prev = nullptr;
                node->next = nullptr;
            }

            _lru_head.reset(node);

        }


// See MapBasedGlobalLockImpl.h
        bool SimpleLRU::Put(const std::string &key, const std::string &value) {
            if (_max_size < (key.size() + value.size()))
                return false;

            auto i = _lru_index.find(key);
            if (i != _lru_index.end()) {
                _cut(i->second.get());
                _push_front(&i->second.get());
                i->second.get().value = value;
            } else {
                lru_node *node = new lru_node({key, value});
                _push_front(node);
                _lru_index.insert(std::make_pair(std::ref(node->key), std::ref(*node)));
            }
            return true;
        }

// See MapBasedGlobalLockImpl.h
        bool SimpleLRU::PutIfAbsent(const std::string &key, const std::string &value) {
            if (_max_size < (key.size() + value.size()))
                return false;

            auto i = _lru_index.find(key);
            if (i == _lru_index.end()) {
                lru_node *node = new lru_node({key, value});
                _push_front(node);
                _lru_index.insert(std::make_pair(std::ref(node->key), std::ref(*node)));
                return true;
            }

            return false;
        }

// See MapBasedGlobalLockImpl.h
        bool SimpleLRU::Set(const std::string &key, const std::string &value) {
            auto i = _lru_index.find(key);
            if (i == _lru_index.end())
                return false;

            if (_max_size < (_cur_size + value.size() - i->second.get().value.size()))
                return false;

            _cut(i->second.get());
            _push_front(&i->second.get());
            i->second.get().value = value;
            return true;
        }

// See MapBasedGlobalLockImpl.h
        bool SimpleLRU::Delete(const std::string &key) {
            auto i = _lru_index.find(key);
            if (i == _lru_index.end())
                return false;

            _cut(i->second.get(), false);
            _lru_index.erase(i);
            return true;
        }

// See MapBasedGlobalLockImpl.h
        bool SimpleLRU::Get(const std::string &key, std::string &value) {
            auto i = _lru_index.find(key);
            if (i == _lru_index.end())
                return false;

            value = i->second.get().value;
            _cut(i->second.get());
            _push_front(&i->second.get());

            return true;
        }

    } // namespace Backend
} // namespace Afina