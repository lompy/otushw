#pragma once

#include <future>
#include <optional>
#include <thread>

// RingIncIter is a helper for a more concise logic with iter++.
template<typename Container>
class RingIncIter {
    Container& container;
    typename Container::iterator iter;
public:
    RingIncIter(Container& c) : container(c), iter(c.begin()) {};
    RingIncIter& operator++() {
        if (++iter == container.end())
            iter = container.begin();

        return *this;
    };

    typename Container::iterator operator++(int) {
        auto tmp = iter;
        ++(*this);

        return tmp;
    };

    typename Container::reference operator*() {
        return *iter;
    };
    typename Container::pointer operator->() {
        return &(*iter);
    };
};

// FirstValueWaiter should be used in a single thread.
// It uses async to create limited amount of async futures at a time.
// Best result in parallelization is reached when all
// enqueued functions do approx equal amount of work.
// There is no timeout. If any enqueued function blocks forever
// the waiter may also block forever.
// It is not a thread pool, we have an overhead of constructing
// and destructing the threads, but I think code much simpler without any mutexes.
template<typename T>
class FirstValueWaiter {
    using Result = std::optional<T>;
    using Future = std::future<Result>;

    std::vector<Future> futures { std::thread::hardware_concurrency()/2 };
    RingIncIter<std::vector<Future>> iter { futures };
    Result result { std::nullopt };
public:
    FirstValueWaiter() = default;

    FirstValueWaiter(FirstValueWaiter&) = delete;
    FirstValueWaiter(const FirstValueWaiter&) = delete;
    FirstValueWaiter& operator=(FirstValueWaiter&&) = delete;
    FirstValueWaiter& operator=(const FirstValueWaiter&) = delete;

    // Blocks until random future is finished if no empty slots.
    // Returns immediately and don't create any futures if result is already found.
    void enqueue(std::function<Result()> func) {
        if (result.has_value() || (iter->valid() && (result = iter->get()).has_value()))
            return;

        *iter++ = std::async(std::launch::async, std::move(func));
    };

    Result wait() {
        if (result.has_value())
            return result;

        while (iter->valid())
            if ((result = iter++->get()).has_value())
                return result;

        return std::nullopt;
    };
};
