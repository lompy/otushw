#pragma once

#include <cstddef>
#include <functional>
#include <initializer_list>
#include <iostream>
#include <iterator>
#include <memory>
#include <new>
#include <unordered_map>

// suto stands for otus, loc -- for allocator
namespace sutoloc {

int version();

template <typename T, size_t block_size = 4096>
class allocator {
    static_assert(block_size >= sizeof(T), "block_size must fit T");
    static_assert((block_size & (block_size - 1)) == 0, "block_size must be power of two");

    struct size_align {
        size_t size{};
        std::align_val_t align{};

        template <class U>
        static size_align from_type() noexcept {
            return {sizeof(U), std::align_val_t{alignof(U)}};
        }

        bool operator==(const size_align& other) const noexcept {
            return size == other.size && align == other.align;
        };
    };

    struct size_align_hash {
        std::size_t operator()(const size_align& key) const noexcept {
            return std::hash<std::size_t>{}(key.size) + std::hash<std::align_val_t>{}(key.align);
        }
    };

    class block {
        void* mem{};
        std::unique_ptr<block> next;

        size_align key;
        size_t available{};
        size_t not_released{};

    public:
        explicit block(size_align key_v) noexcept
            : key{key_v}, available{block_size / key_v.size},
              not_released{block_size / key_v.size} {};

        ~block() { ::operator delete(mem, key.align); };

        block(const block& other) = delete;
        block& operator=(const block& other) = delete;
        block(block&& other) = default;
        block& operator=(block&& other) = default;

        block* get_next(bool build = false) {
            if (next)
                return next.get();

            if (!build)
                return nullptr;

            return (next = std::make_unique<block>(key)).get();
        }

        void* allocate(size_t num) {
            if (mem == nullptr)
                mem = ::operator new(block_size, key.align);

            if (is_full(num))
                throw std::bad_alloc{};

            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            void* ptr{static_cast<std::byte*>(mem) + (key.size * (max() - available))};
            available -= num;

            return ptr;
        }

        void deallocate(size_t num) noexcept {
            if (not_released > num)
                not_released -= num;
            else
                // reset when all released
                not_released = (available = max());
        };

        bool owns(void* ptr) const noexcept {
            if (mem == nullptr)
                return false;

            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            return ptr >= mem && ptr < static_cast<std::byte*>(mem) + block_size;
        };
        [[nodiscard]] size_t max() const noexcept { return block_size / key.size; };
        [[nodiscard]] bool is_full(size_t num) const noexcept { return available < num; };
        [[nodiscard]] bool is_released() const noexcept { return not_released == 0; };
    };

    std::shared_ptr<std::unordered_map<size_align, std::unique_ptr<block>, size_align_hash>> blocks;

    block* head() {
        auto key{size_align::template from_type<T>()};
        if (!blocks->operator[](key))
            blocks->operator[](key) = std::make_unique<block>(key);

        return blocks->operator[](key).get();
    };

public:
    using value_type = T;

    allocator()
        : blocks{std::make_shared<
              std::unordered_map<size_align, std::unique_ptr<block>, size_align_hash>>()} {};

    template <class U>
    explicit allocator(const allocator<U, block_size>& other) noexcept : blocks{other.blocks} {}

    template <class U>
    struct rebind {
        using other = allocator<U, block_size>;
    };

    T* allocate(size_t num) {
        if (sizeof(T) * num > block_size)
            return static_cast<T*>(::operator new(sizeof(T) * num, std::align_val_t{alignof(T)}));

        block* blk{head()};
        while (blk->is_full(num))
            blk = blk->get_next(true);

        return static_cast<T*>(blk->allocate(num));
    };

    void deallocate(T* ptr, size_t num) {
        if (sizeof(T) * num > block_size) {
            ::operator delete(static_cast<void*>(ptr), std::align_val_t{alignof(T)});

            return;
        }

        block* blk{head()};
        while (blk != nullptr && !blk->owns(ptr))
            blk = blk->get_next();

        if (blk == nullptr)
            throw std::bad_alloc{};

        blk->deallocate(num);
    };
};

// this my list implementation copied from
// https://github.com/lompy/otushw/blob/main/cpp-basic/06-07/src/sutolist.hpp
// added allocator template parameter, and fixed linter warnings
template <typename T, typename Allocator = std::allocator<T>>
class list {
    struct node {
        T value{};
        node* prev{};
        node* next{};

        // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
        explicit node(const T& val, node* prv = nullptr, node* nxt = nullptr)
            : value(val), prev(prv), next(nxt) {}
    };

    node* head{};
    node* tail{};
    std::size_t len{};

    using node_alloc = typename std::allocator_traits<Allocator>::template rebind_alloc<node>;
    node_alloc alloc{};

public:
    class iterator {
        friend class list;
        node* ptr;
        node* end_prev; // tail pointer for "end" iterator
    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = T*;
        using reference = T&;

        // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
        explicit constexpr iterator(node* ptr, node* tail = nullptr) : ptr{ptr}, end_prev{tail} {}

        reference operator*() const { return ptr->value; }
        pointer operator->() { return &(ptr->value); }

        iterator& operator++() {
            if (ptr->next == nullptr)
                end_prev = ptr;
            ptr = ptr->next;

            return *this;
        }

        iterator operator++(int) {
            iterator tmp{*this};
            ++(*this);

            return tmp;
        }

        iterator& operator--() {
            if (ptr == nullptr && end_prev != nullptr) {
                ptr = end_prev;
                end_prev = nullptr;

                return *this;
            }

            ptr = ptr->prev;

            return *this;
        }

        iterator operator--(int) {
            iterator tmp = *this;
            --(*this);

            return tmp;
        }

        iterator operator+(difference_type n) const {
            iterator iter = *this;
            std::advance(iter, n);

            return iter;
        }

        iterator operator-(difference_type n) const {
            iterator iter = *this;
            std::advance(iter, -n);

            return iter;
        }

        iterator& operator+=(difference_type n) {
            std::advance(*this, n);

            return *this;
        }

        iterator& operator-=(difference_type n) {
            std::advance(*this, -n);

            return *this;
        }

        friend bool operator==(const iterator& a, const iterator& b) {
            return a.ptr == b.ptr && a.end_prev == b.end_prev;
        };

        friend bool operator!=(const iterator& a, const iterator& b) {
            return a.ptr != b.ptr || a.end_prev != b.end_prev;
        };

        explicit operator node*() { return ptr; }
    };

    list() = default;

    list(std::initializer_list<T> list) {
        for (auto val : list)
            this->push_back(val);
    }

    list(const list& other) {
        for (auto val : other)
            this->push_back(val);
    }

    list(list&& other) noexcept : head{other.head}, tail{other.tail}, len{other.len} {
        other.head = nullptr;
        other.tail = nullptr;
        other.len = 0;
    }

    list& operator=(const list& other) {
        if (other == this)
            return *this;
        for (auto val : other)
            this->push_back(val);
    }

    list& operator=(list&& other) noexcept {
        head = other.head;
        tail = other.tail;
        len = other.len;
        other.head = nullptr;
        other.tail = nullptr;
        other.len = 0;

        return *this;
    }

    ~list() {
        auto next = head;
        for (std::size_t i = 0; i < len; ++i) {
            auto current = next;
            next = next->next;
            std::allocator_traits<node_alloc>::destroy(alloc, current);
            std::allocator_traits<node_alloc>::deallocate(alloc, current, 1);
        }
    }

    [[nodiscard]] const char* name() const { return "list"; }
    [[nodiscard]] std::size_t size() const { return len; }
    [[nodiscard]] iterator begin() const { return iterator{head}; }
    [[nodiscard]] iterator end() const { return iterator{nullptr, tail}; }
    [[nodiscard]] bool empty() const { return head == nullptr; }

    const T& operator[](std::size_t i) const { return *iter_at(i); }

    T& operator[](std::size_t i) { return *iter_at(i); }

    iterator push_back(T val) { return insert(end(), val); }

    iterator insert(iterator pos, T val) {
        auto prev_pos = nulliter;
        if (pos != begin())
            prev_pos = pos - 1;

        auto new_node{std::allocator_traits<node_alloc>::allocate(alloc, 1)};
        node* prev_ptr = nullptr;
        if (pos != begin())
            prev_ptr = prev_pos.ptr;
        std::allocator_traits<node_alloc>::construct(alloc, new_node, val, prev_ptr, pos.ptr);

        if (prev_pos != nulliter && prev_pos.ptr != nullptr)
            prev_pos.ptr->next = new_node;
        if (pos.ptr != nullptr)
            pos.ptr->prev = new_node;

        if (pos == begin())
            head = new_node;
        if (pos == end())
            tail = new_node;

        len++;

        return iterator{new_node};
    }

    iterator insert(std::size_t at, T val) { return insert(iter_at(at), val); }

    iterator erase(iterator pos) {
        if (pos == end())
            return pos;

        auto prev_pos = nulliter;
        if (pos != begin())
            prev_pos = pos - 1;
        auto next_pos = pos + 1;

        if (prev_pos != nulliter)
            prev_pos.ptr->next = next_pos.ptr;
        if (next_pos != nulliter && next_pos.ptr != nullptr)
            next_pos.ptr->prev = prev_pos.ptr;

        if (pos == begin())
            head = next_pos.ptr;
        if (next_pos == end())
            tail = prev_pos.ptr;

        std::allocator_traits<node_alloc>::destroy(alloc, pos.ptr);
        std::allocator_traits<node_alloc>::deallocate(alloc, pos.ptr, 1);
        len--;

        return next_pos;
    }

    iterator erase(std::size_t at) { return erase(iter_at(at)); }

private:
    static constexpr iterator nulliter{iterator{nullptr}};

    iterator iter_at(std::size_t index) const {
        if (index > len)
            throw std::out_of_range{"list::iter_at"};

        auto iter = nulliter;
        if (index <= len / 2) {
            iter = begin();
            iter += static_cast<typename iterator::difference_type>(index);
        } else {
            iter = iterator{tail};
            iter -= static_cast<typename iterator::difference_type>(len - (++index));
        }

        return iter;
    }
};

template <typename T>
std::ostream& operator<<(std::ostream& out, const list<T>& list) {
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

}; // namespace sutoloc
