#pragma once

#include <cstddef>
#include <iostream>
#include <iterator>

// Suto stands for Otus
template<typename T>
class Sutolist {
    struct Node {
        T value {};
        Node* prev {};
        Node* next {};
    };

    Node* head {};
    Node* tail {};
    std::size_t len {};
public:
    class Iterator {
        friend class Sutolist;
        Node* ptr;
        Node* end_prev; // tail pointer for "end" iterator
    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = T*;
        using reference = T&;

        constexpr Iterator(Node* ptr, Node* tail = nullptr) : ptr {ptr}, end_prev {tail} {}

        reference operator*() const {
            return ptr->value;
        }
        pointer operator->() {
            return &(ptr->value);
        }

        Iterator& operator++() {
            if (ptr->next == nullptr)
                end_prev = ptr;
            ptr = ptr->next;

            return *this;
        }

        Iterator operator++(int) {
            Iterator tmp { *this };
            ++(*this);

            return tmp;
        }

        Iterator& operator--() {
            if (ptr == nullptr && end_prev != nullptr) {
                ptr = end_prev;
                end_prev = nullptr;

                return *this;
            }

            ptr = ptr->prev;

            return *this;
        }

        Iterator operator--(int) {
            Iterator tmp = *this;
            --(*this);

            return tmp;
        }

        Iterator operator+(difference_type n) const {
            Iterator iter = *this;
            std::advance(iter, n);

            return iter;
        }

        Iterator operator-(difference_type n) const {
            Iterator iter = *this;
            std::advance(iter, -n);

            return iter;
        }

        Iterator& operator+=(difference_type n) {
            std::advance(*this, n);

            return *this;
        }

        Iterator& operator-=(difference_type n) {
            std::advance(*this, -n);

            return *this;
        }

        friend bool operator==(const Iterator& a, const Iterator& b) {
            return a.ptr == b.ptr && a.end_prev == b.end_prev;
        };

        friend bool operator!=(const Iterator& a, const Iterator& b) {
            return a.ptr != b.ptr || a.end_prev != b.end_prev;
        };

        operator Node*() {
            return ptr;
        }
    };

    Sutolist() = default;

    Sutolist(std::initializer_list<T> list) {
        for (auto val: list)
            this->push_back(val);
    }

    Sutolist(const Sutolist<T>& other) {
        for (auto val : other)
            this->push_back(val);
    }

    Sutolist(Sutolist<T>&& other) noexcept :
        head {other.head}, tail {other.tail}, len {other.len} {
        other.head = nullptr;
        other.tail = nullptr;
        other.len = 0;
    }

    Sutolist& operator=(Sutolist&& other) noexcept {
        head = other.head;
        tail = other.tail;
        len = other.len;
        other.head = nullptr;
        other.tail = nullptr;
        other.len = 0;

        return *this;
    }

    ~Sutolist() {
        auto next = head;
        for (std::size_t i = 0; i < len; ++i) {
            auto current = next;
            next = next->next;
            delete current;
        }
    }

    const char* name() const {
        return "Sutolist";
    }

    std::size_t size() const {
        return len;
    }
    Iterator begin() const {
        return Iterator {head};
    }
    Iterator end() const {
        return Iterator {nullptr, tail};
    }

    const T& operator[](std::size_t i) const {
        return *iter_at(i);
    }

    T& operator[](std::size_t i) {
        return *iter_at(i);
    }

    Iterator push_back(T val) {
        return insert(end(), val);
    }

    Iterator insert(Iterator pos, T val) {
        auto prev_pos = nulliter;
        if (pos != begin())
            prev_pos = pos-1;

        auto node = new Node {val, prev_pos, pos};

        if (prev_pos != nulliter && prev_pos.ptr != nullptr)
            prev_pos.ptr->next = node;
        if (pos.ptr != nullptr)
            pos.ptr->prev = node;

        if (pos == begin())
            head = node;
        if (pos == end())
            tail = node;

        len++;

        return Iterator{node};
    }

    Iterator insert(std::size_t at, T val) {
        return insert(iter_at(at), val);
    }

    Iterator erase(Iterator pos) {
        if (pos == end())
            return pos;

        auto prev_pos = nulliter;
        if (pos != begin())
            prev_pos = pos-1;
        auto next_pos = pos+1;

        if (prev_pos != nulliter)
            prev_pos.ptr->next = next_pos.ptr;
        if (next_pos != nulliter && next_pos.ptr != nullptr)
            next_pos.ptr->prev = prev_pos.ptr;

        if (pos == begin())
            head = next_pos.ptr;
        if (next_pos == end())
            tail = prev_pos.ptr;

        delete pos.ptr;
        len--;

        return next_pos;
    }

    Iterator erase(std::size_t at) {
        return erase(iter_at(at));
    }

private:
    static constexpr Iterator nulliter { Iterator {nullptr} };

    Iterator iter_at(std::size_t index) const {
        if (index > len)
            throw std::out_of_range {"Sutolist::iter_at"};

        auto iter = nulliter;
        if (index <= len / 2) {
            iter = begin();
            iter += static_cast<typename Iterator::difference_type>(index);
        } else {
            iter = Iterator {tail};
            iter -= static_cast<typename Iterator::difference_type>(len - (++index));
        }

        return iter;
    }
};

template<typename T>
std::ostream& operator<<(std::ostream& out, const Sutolist<T>& list) {
    out << list.name() << " {";
    bool first = true;
    for (const auto& value : list) {
        if (!first)
            out << ", ";
        out << value;
        first = false;
    }
    out << "}";

    return out;
}
