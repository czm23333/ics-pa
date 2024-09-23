#pragma once
#include "debug.h"
#include <utility>
#include <new>

template <typename T>
class mvector {
    static T* alloc_storage(size_t count) {
        return static_cast<T*>(aligned_alloc(alignof(T), count * sizeof(T)));
    }

    static void free_storage(T* ptr, size_t count) {
        for (size_t i = 0; i < count; i++) ptr[i].~T();
        free(ptr);
    }

    size_t size_v;
    size_t capacity_v;
    T* data_v;

    void grow_if_necessary() {
        if (size_v < capacity_v) return;
        capacity_v *= 2;
        T* new_data_v = alloc_storage(capacity_v);
        for (size_t i = 0; i < size_v; i++) new (new_data_v + i) T (std::move(data_v[i]));
        free_storage(data_v, size_v);
        data_v = new_data_v;
    }
public:
    mvector() noexcept : size_v(0), capacity_v(4), data_v(alloc_storage(capacity_v)) {}

    mvector(const mvector& other) noexcept : size_v(other.size_v), capacity_v(other.capacity_v), data_v(alloc_storage(other.capacity_v)) {
        for (size_t i = 0; i < size_v; i++) new (data_v + i) T (other.data_v[i]);
    }

    mvector(mvector&& other) noexcept : size_v(other.size_v), capacity_v(other.capacity_v), data_v(other.data_v) {
        other.size_v = 0;
        other.capacity_v = 0;
        other.data_v = nullptr;
    }

    ~mvector() {
        if (data_v != nullptr)
            free_storage(data_v, size_v);
    }

    T* data() noexcept {
        return data_v;
    }

    const T* data() const noexcept {
        return data_v;
    }

    size_t size() const noexcept {
        return size_v;
    }

    size_t capacity() const noexcept {
        return capacity_v;
    }

    T& operator[](size_t index) noexcept {
        if (index >= size_v) panic("out of bound, index %u >= size %u", index, size_v);
        return data_v[index];
    }

    const T& operator[](size_t index) const noexcept {
        if (index >= size_v) panic("out of bound, index %u >= size %u", index, size_v);
        return data_v[index];
    }

    T& push_back(const T& element) noexcept {
        grow_if_necessary();
        return *new (data_v + size_v++) T (element);
    }

    T& push_back(T&& element) noexcept {
        grow_if_necessary();
        return *new (data_v + size_v++) T (std::move(element));
    }

    template <typename... Args>
    T& emplace_back(Args... args) noexcept {
        grow_if_necessary();
        return *new (data_v + size_v++) T (args...);
    }

    void pop_back() noexcept {
        if (size_v == 0) panic("pop from empty vector");
        data_v[--size_v]->~T();
    }

    void clear() noexcept {
        for (size_t i = 0; i < size_v; i++) data_v[i]->~T();
        size_v = 0;
    }

    T& front() noexcept {
        if (size_v == 0) panic("out of bound: front > empty");
        return data_v[0];
    }

    const T& front() const noexcept {
        if (size_v == 0) panic("out of bound: front > empty");
        return data_v[0];
    }

    T& back() noexcept {
        if (size_v == 0) panic("out of bound: back > empty");
        return data_v[size_v - 1];
    }

    const T& back() const noexcept {
        if (size_v == 0) panic("out of bound: back > empty");
        return data_v[size_v - 1];
    }

    void shrink_to_fit() {
        if (size_v == capacity_v) return;
        T* new_data_v = alloc_storage(size_v);
        for (size_t i = 0; i < size_v; i++) new (new_data_v + i) T (std::move(data_v[i]));
        free_storage(data_v, size_v);
        data_v = new_data_v;
    }

    T* begin() noexcept {
        return data_v;
    }

    T* end() noexcept {
        return data_v + size_v;
    }

    const T* begin() const noexcept {
        return data_v;
    }

    const T* end() const noexcept {
        return data_v + size_v;
    }

    const T* cbegin() const noexcept {
        return data_v;
    }

    const T* cend() const noexcept {
        return data_v + size_v;
    }
};
