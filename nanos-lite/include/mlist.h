#pragma once
#include <common.h>
#include <utility>

#include "debug.h"

template<typename T>
class mlist {
    struct node {
        node* last = nullptr;
        node* next = nullptr;
        T data;

        explicit node(const T& data) noexcept : data(data) {}
        explicit node(T&& data) noexcept : data(std::move(data)) {}
        template<typename... Args>
        explicit node(Args args...) noexcept : data(std::forward<Args>(args)...) {}
    };

    class iterator {
        node* current = nullptr;
        public:
        T& operator*() const noexcept {
            if (current == nullptr) panic("out of bound");
            return current->data;
        }
    };

    static void destroy_nodes(node* head) {
        if (head == nullptr) return;
        destroy_nodes(head->next);
        delete head;
    }

    static node* clone_nodes(node* last, const node* head) {
        if (head == nullptr) return nullptr;
        node* new_head = new node(head->data);
        new_head->last = last;
        new_head->next = clone_nodes(new_head, head->next);
        return new_head;
    }

    node* head = nullptr;
    size_t size = 0;
public:
    mlist() noexcept = default;

    mlist(const mlist& other) {
        head = clone_nodes(nullptr, other.head);
        size = other.size;
    }

    mlist(mlist&& other) noexcept {
        head = other.head;
        size = other.size;
        other.head = nullptr;
        other.size = 0;
    }

    ~mlist() noexcept {
        destroy_nodes(head);
    }


};
