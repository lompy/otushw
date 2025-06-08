#pragma once

#include <cstddef>
#include <iostream>
#include <iterator>

// Suto stands for Otus, vec for vector
template<typename T>
class Sutovec {
    T* store {};
    std::size_t cap {};
    std::size_t len {};

public:
    class Iterator {
        T* ptr;
    public:
        using iterator_category = std::random_access_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = T;
        using pointer = T*;
        using reference = T&;

        constexpr Iterator(pointer p) : ptr(p) {}

        reference operator*() const {
            return *ptr;
        }
        pointer operator->() {
            return ptr;
        }

        Iterator& operator++() {
            ptr++;

            return *this;
        }

        Iterator operator++(int) {
            Iterator tmp { *this };
            ++(*this);

            return tmp;
        }

        Iterator& operator--() {
            ptr--;

            return *this;
        }

        Iterator operator--(int) {
            Iterator tmp = *this;
            --(*this);

            return tmp;
        }

        difference_type operator-(const Iterator& other) const {
            return ptr - other.ptr;
        }

        Iterator operator+(difference_type n) const {
            return Iterator(ptr + n);
        }

        Iterator operator-(difference_type n) const {
            return Iterator(ptr - n);
        }

        Iterator& operator+=(difference_type n) {
            ptr += n;

            return *this;
        }

        Iterator& operator-=(difference_type n) {
            ptr -= n;
            return *this;
        }

        friend bool operator==(const Iterator& a, const Iterator& b) {
            return a.ptr == b.ptr;
        };

        friend bool operator!=(const Iterator& a, const Iterator& b) {
            return a.ptr != b.ptr;
        };
    };

    Sutovec() = default;

    Sutovec(std::size_t length, T val = T{}) {
        init(length);
        len = length;
        for (std::size_t i = 0; i < length; ++i)
            store[i] = val;
    }

    Sutovec(std::initializer_list<T> list) {
        init(list.size());
        for (const auto& it: list)
            push_back(it);
    }

    Sutovec(const Sutovec<T>& from) {
        init(from.size());
        len = from.size();
        std::copy(from.begin(), from.end(), begin());
    }

    Sutovec(Sutovec<T>&& other) noexcept : store {other.store}, cap {other.cap}, len {other.len} {
        other.store = nullptr;
        other.cap = 0;
        other.len = 0;
    }

    Sutovec& operator=(Sutovec&& other) noexcept {
        store = other.store;
        cap = other.cap;
        len = other.len;
        other.store = nullptr;
        other.cap = 0;
        other.len = 0;

        return *this;
    }

    ~Sutovec() {
        delete[] store;
    }

    const char* name() const {
        return "Sutovec";
    }

    std::size_t size() const {
        return len;
    }
    std::size_t capacity() const {
        return cap;
    }

    Iterator begin() const {
        return Iterator {store};
    }
    Iterator end() const {
        return std::next(begin(), static_cast<typename Iterator::difference_type>(len));
    }

    const T& operator[](std::size_t i) const {
        return *std::next(begin(), i);
    }

    T& operator[](std::size_t i) {
        return *std::next(begin(), static_cast<typename Iterator::difference_type>(i));
    }

    T& back() {
        return store[len - 1];
    }

    const T& back() const {
        return store[len - 1];
    }

    Iterator push_back(T val) {
        return insert(end(), val);
    }

    Iterator insert(Iterator pos, T val) {
        auto new_pos = grow_if_full(pos);
        if (new_pos == nulliter)
            new_pos = end();
        std::copy_backward(new_pos, end(), end()+1);

        *new_pos = val;
        len++;

        return new_pos;
    }

    Iterator insert(std::size_t at, T val) {
        return insert(std::next(begin(), static_cast<typename Iterator::difference_type>(at)), val);
    }

    Iterator erase(Iterator pos) {
        if (pos == end())
            return pos;

        std::copy(pos+1, end(), pos);
        len--;

        return pos;
    }

    Iterator erase(typename Iterator::difference_type at) {
        if (at < 0)
            return erase(std::next(end(), at));

        return erase(std::next(begin(), at));
    }

private:
    static constexpr Iterator nulliter { Iterator {nullptr} };

    Iterator grow_if_full(Iterator gap_pos = nulliter) {
        if (cap > len)
            return gap_pos;

        auto new_cap = cap + (cap == 0 ? 1 : cap);
        auto new_store = new T[new_cap];

        auto new_pos = nulliter;
        if (gap_pos != nulliter) {
            new_pos = std::copy(begin(), gap_pos, new_store);
            std::copy(gap_pos, end(), new_pos);
        } else {
            std::copy(begin(), end(), new_store);
        }

        delete[] store;

        store = new_store;
        cap = new_cap;

        return new_pos;
    }

    void init(std::size_t capacity) {
        if (store)
            return;

        store = new T[capacity];
        cap = capacity;
    }
};

template<typename T>
std::ostream& operator<<(std::ostream& out, const Sutovec<T>& sutovec) {
    out << sutovec.name() << " {";
    bool first = true;
    for (const auto& value : sutovec) {
        if (!first)
            out << ", ";
        out << value;
        first = false;
    }
    out << "}";

    return out;
}
