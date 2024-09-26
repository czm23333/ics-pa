#pragma once
#include <common.h>
#include <utility>
#include <bits/stl_iterator_base_types.h>

#include "debug.h"

template<typename T>
class mlist {
    struct node {
        node *last = nullptr;
        node *next = nullptr;
        T data;

        explicit node(const T &data) noexcept : data(data) {
        }

        explicit node(T &&data) noexcept : data(std::move(data)) {
        }

        template<typename... Args>
        explicit node(Args... args) noexcept : data(std::forward<Args>(args)...) {
        }
    };

    class iterator {
        friend class mlist;
        node *prev = nullptr;
        node *current = nullptr;

        iterator(node *prev, node *current) noexcept : prev(prev), current(current) {
        }

    public:
        using difference_type = std::ptrdiff_t;
        using value_type = T;
        using pointer = T*;
        using reference = T&;
        using iterator_category = std::bidirectional_iterator_tag;

        T &operator*() const noexcept {
            if (current == nullptr)
                panic("out of bound");
            return current->data;
        }

        iterator &operator++() noexcept {
            if (current == nullptr)
                panic("out of bound");
            prev = current;
            current = current->next;
            return *this;
        }

        iterator operator++(int) noexcept {
            if (current == nullptr)
                panic("out of bound");
            iterator tmp = *this;
            prev = current;
            current = current->next;
            return tmp;
        }

        iterator &operator--() noexcept {
            if (prev == nullptr)
                panic("out of bound");
            if (current != nullptr) prev = current->last;
            current = prev;
            prev = prev->last;
            return *this;
        }

        iterator operator--(int) noexcept {
            if (prev == nullptr)
                panic("out of bound");
            if (current != nullptr) prev = current->last;
            iterator tmp = *this;
            current = prev;
            prev = prev->last;
            return tmp;
        }

        bool operator==(const iterator &other) const noexcept {
            return current == other.current;
        }

        bool operator!=(const iterator &other) const noexcept {
            return current != other.current;
        }
    };

    static void destroy_nodes(node *head) {
        if (head == nullptr) return;
        destroy_nodes(head->next);
        delete head;
    }

    static std::pair<node *, node *> clone_nodes(node *last, const node *head) {
        if (head == nullptr) return std::make_pair(nullptr, nullptr);
        node *new_head = new node(head->data);
        new_head->last = last;
        auto [a, b] = clone_nodes(new_head, head->next);
        new_head->next = a;
        return std::make_pair(new_head, b == nullptr ? new_head : b);
    }

    node *head = nullptr;
    node *tail = nullptr;
    size_t size_v = 0;

    void insert_node(node *prev, node *cur, node *next) {
        cur->last = prev;
        cur->next = next;
        if (prev != nullptr) prev->next = cur;
        if (next != nullptr) next->last = cur;
        if (next == head) head = cur;
        if (prev == tail) tail = cur;
        ++size_v;
    }

    void remove_node(node *cur) {
        if (cur == nullptr) return;
        node *prev = cur->last;
        node *next = cur->next;
        if (prev != nullptr) prev->next = next;
        if (next != nullptr) next->last = prev;
        if (cur == head) head = next;
        if (cur == tail) tail = prev;
        delete cur;
        --size_v;
    }

public:
    using iterator = iterator;

    mlist() noexcept = default;

    mlist(const mlist &other) {
        auto [a, b] = clone_nodes(nullptr, other.head);;
        head = a;
        tail = b;
        size_v = other.size_v;
    }

    mlist(mlist &&other) noexcept {
        head = other.head;
        tail = other.tail;
        size_v = other.size_v;
        other.head = nullptr;
        other.tail = nullptr;
        other.size_v = 0;
    }

    ~mlist() noexcept {
        destroy_nodes(head);
    }

    iterator begin() noexcept {
        return iterator(nullptr, head);
    }

    iterator end() noexcept {
        return iterator(tail, nullptr);
    }

    iterator insert(iterator pos, const T &data) noexcept {
        node *new_node = new node(data);
        insert_node(pos.prev, new_node, pos.current);
        return iterator(pos.prev, new_node);
    }

    iterator insert(iterator pos, T &&data) noexcept {
        node *new_node = new node(std::move(data));
        insert_node(pos.prev, new_node, pos.current);
        return iterator(pos.prev, new_node);
    }

    template<typename... Args>
    iterator emplace(iterator pos, Args... args) noexcept {
        node *new_node = new node(std::forward<Args>(args)...);
        insert_node(pos.prev, new_node, pos.current);
        return iterator(pos.prev, new_node);
    }

    void erase(iterator pos) noexcept {
        remove_node(pos.current);
    }

    iterator push_back(const T &data) noexcept {
        return insert(end(), data);
    }

    iterator push_back(T &&data) noexcept {
        return insert(end(), std::move(data));
    }

    template<typename... Args>
    iterator emplace_back(Args... args) noexcept {
        return emplace(end(), std::forward<Args>(args)...);
    }

    iterator push_front(const T &data) noexcept {
        return insert(begin(), data);
    }

    iterator push_front(T &&data) noexcept {
        return insert(begin(), std::move(data));
    }

    template<typename... Args>
    iterator emplace_front(Args... args) noexcept {
        return emplace(begin(), std::forward<Args>(args)...);
    }

    void pop_back() noexcept {
        erase(--end());
    }

    void pop_front() noexcept {
        erase(begin());
    }

    [[nodiscard]] size_t size() const noexcept {
        return size_v;
    }
};
