#ifndef SQUEDL_SQUEDL_HPP
#define SQUEDL_SQUEDL_HPP

#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <set>
#include <string>
#include <thread>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include "squedl/detail/expected.hpp"
namespace squedl {

int add();

using bytes = std::vector<std::byte>;
using kind = std::string;

class error : public std::exception {};

template <typename T = void>
using expected = detail::expected<T, error>;

using unexpected = detail::unexpected<error>;

template <typename T, typename TArgs = typename T::args>
inline constexpr bool is_serializable_v =
    std::is_same_v<decltype(T::serialize(std::declval<const TArgs&>())), bytes>;

template <typename T>
inline constexpr bool has_kind_v = std::is_same_v<decltype(T::kind()), kind>;

template <typename T, typename TArgs = typename T::args>
inline constexpr bool is_workable_v =
    std::is_same_v<TArgs, decltype(T::deserialize(std::declval<const bytes&>()))> &&
    std::is_same_v<std::optional<error>, decltype(std::declval<T>()(std::declval<TArgs>()))>;

template <typename Clock = std::chrono::system_clock>
class test_bus {
public:
    using id_t = std::uint64_t;
    using time_point = typename Clock::time_point;
    using duration = typename Clock::duration;
    using clock = Clock;

    explicit test_bus(duration ack_timeout = std::chrono::minutes{1}, bool auto_ack = false,
                      duration tick_duration = std::chrono::seconds{1})
        : data{std::make_shared<state>(ack_timeout, auto_ack, tick_duration)} {};

    expected<id_t> put(const kind& kind, bytes&& payload, duration after = duration::zero()) {
        std::lock_guard<std::mutex> _(data->mtx);
        const auto id{data->next_id()};
        auto now{Clock::now()};
        auto at{now + after};

        if (at > now) {
            data->delayed[kind].emplace(std::piecewise_construct, std::forward_as_tuple(at),
                                        std::forward_as_tuple(id, std::move(payload), at));
        } else {
            data->enqueued[kind].emplace(id, std::move(payload));
        }

        data->cv.notify_one();

        return expected{id};
    };

    expected<id_t> put(const kind& kind, const bytes& payload, duration after = duration::zero()) {
        return put(kind, bytes{payload}, after);
    };

    std::optional<std::vector<std::pair<id_t, std::shared_ptr<const bytes>>>>
    next(const kind& kind, size_t count, duration timeout = duration::zero()) {
        std::unique_lock lock{data->mtx};

        auto now{Clock::now()};
        data->put_due(kind, now);

        auto& enqueued{data->enqueued[kind]};
        auto& unacked{data->unacked[kind]};
        auto& unacked_time_points{data->unacked_time_points[kind]};

        if (count == 0)
            return opt_with_empty_vec();

        auto waited_for_condition{true};
        if (timeout == duration::zero())
            data->cv.wait(lock, [this, &enqueued] { return !enqueued.empty() || data->stopping; });
        else
            waited_for_condition = data->cv.wait_for(
                lock, timeout, [this, &enqueued] { return !enqueued.empty() || data->stopping; });

        if (data->stopping)
            return std::nullopt;

        if (!waited_for_condition)
            return opt_with_empty_vec();

        std::vector<std::pair<id_t, std::shared_ptr<const bytes>>> result{};
        result.reserve(count);
        message msg{};
        while (count && !enqueued.empty()) {
            msg = std::move(enqueued.front());
            auto id{msg.id};

            --count;
            enqueued.pop();

            if (data->auto_ack) {
                result.push_back(std::make_pair(id, std::move(msg.payload)));

                continue;
            }

            auto unacked_due{now + data->ack_timeout};
            auto payload{msg.payload};
            unacked_time_points[msg.id] = unacked_due;
            unacked.emplace(unacked_due, std::move(msg));

            result.push_back(std::make_pair(id, std::move(payload)));
        }

        return std::optional{std::move(result)};
    };

    void ack(kind kind, id_t id) {
        if (data->auto_ack)
            return;

        std::lock_guard<std::mutex> _{data->mtx};
        auto& unacked{data->unacked[kind]};
        auto& unacked_time_points{data->unacked_time_points[kind]};

        auto time_point_it{unacked_time_points.find(id)};
        if (time_point_it == unacked_time_points.end())
            return;

        auto range = unacked.equal_range(time_point_it->second);
        for (auto unacked_it = range.first; unacked_it != range.second; ++unacked_it) {
            if (unacked_it->second.id != id)
                continue;

            unacked.erase(unacked_it);
            break;
        }

        unacked_time_points.erase(time_point_it);
    };

    void reject(kind kind, id_t id) { ack(kind, id); };

    void nack(kind kind, id_t id) {
        if (data->auto_ack)
            return;

        std::lock_guard<std::mutex> _{data->mtx};

        auto& enqueued = data->enqueued[kind];
        auto& unacked = data->unacked[kind];
        auto& unacked_time_points = data->unacked_time_points[kind];

        auto time_point_it{unacked_time_points.find(id)};
        if (time_point_it == unacked_time_points.end())
            return;

        auto range = unacked.equal_range(time_point_it->second);
        for (auto unacked_it = range.first; unacked_it != range.second; ++unacked_it) {
            if (unacked_it->second.id != id)
                continue;

            enqueued.emplace(unacked_it->second.id, std::move(unacked_it->second.payload));
            unacked.erase(unacked_it);
            break;
        }
    };

    void stop() {
        if (data->stopping)
            return;
        {
            std::lock_guard<std::mutex> _{data->mtx};
            data->stopping = true;
            data->cv.notify_all();
        }
    };

    size_t enqueued_size(kind kind) {
        std::lock_guard<std::mutex> _{data->mtx};
        return data->enqueued[kind].size();
    };
    size_t delayed_size(kind kind) {
        std::lock_guard<std::mutex> _{data->mtx};
        return data->delayed[kind].size();
    };
    size_t unacked_size(kind kind) {
        std::lock_guard<std::mutex> _{data->mtx};
        return data->unacked[kind].size();
    };

    bool empty() {
        std::lock_guard<std::mutex> _{data->mtx};
        return std::all_of(data->enqueued.cbegin(), data->enqueued.cend(),
                           [](auto x) { return x.second.empty(); }) &&
               std::all_of(data->delayed.cbegin(), data->delayed.cend(),
                           [](auto x) { return x.second.empty(); }) &&
               std::all_of(data->unacked.cbegin(), data->unacked.cend(),
                           [](auto x) { return x.second.empty(); });
    }

private:
    struct message {
        id_t id{};
        std::shared_ptr<const bytes> payload;
        std::optional<time_point> after;

        message() = default;
        message(id_t id, bytes&& payload)
            : id{id}, payload{std::make_shared<bytes>(std::move(payload))}, after{std::nullopt} {};
        message(id_t id, bytes&& payload, std::optional<time_point> after)
            : id{id}, payload{std::make_shared<bytes>(std::move(payload))}, after{after} {};
        message(id_t id, std::shared_ptr<const bytes>&& payload)
            : id{id}, payload{std::move(payload)}, after{std::nullopt} {};
        message(id_t id, const std::shared_ptr<const bytes>&& payload)
            : id{id}, payload{payload}, after{std::nullopt} {};
        message(id_t id, std::shared_ptr<const bytes>&& payload, std::optional<time_point> after)
            : id{id}, payload{std::move(payload)}, after{after} {};
        message(id_t id, const std::shared_ptr<const bytes>&& payload,
                std::optional<time_point> after)
            : id{id}, payload{payload}, after{after} {};
    };

    struct state {
        std::mutex mtx;
        std::condition_variable cv;
        std::atomic<bool> stopping{};

        std::map<kind, std::queue<message>> enqueued;
        std::map<kind, std::multimap<time_point, message>> delayed;
        std::map<kind, std::map<id_t, time_point>> unacked_time_points;
        std::map<kind, std::multimap<time_point, message>> unacked;

        std::atomic<id_t> id{1};
        id_t next_id() { return id++; };

        duration ack_timeout{};
        bool auto_ack{};
        std::thread tick;
        duration tick_dur;

        explicit state(duration ack_timeout, bool auto_ack, duration tick_duration)
            : auto_ack{auto_ack}, ack_timeout{ack_timeout}, tick_dur{tick_duration} {
            tick = std::thread{[this] {
                while (!stopping) {
                    std::this_thread::sleep_for(tick_dur);

                    std::lock_guard<std::mutex> _{mtx};
                    auto now{Clock::now()};
                    std::set<kind> kinds{};
                    for (const auto& [k, _] : enqueued)
                        kinds.insert(k);
                    for (const auto& [k, _] : delayed)
                        kinds.insert(k);
                    for (const auto& k : kinds)
                        if (put_due(k, now))
                            cv.notify_one();
                };
            }};
        };

        state(state const& other) = delete;
        state(state&& other) = delete;
        state& operator=(state const& other) = delete;
        state& operator=(state&& other) = delete;

        ~state() {
            if (!stopping) {
                std::lock_guard<std::mutex> _{mtx};
                stopping = true;
                cv.notify_all();
            }

            if (tick.joinable())
                tick.join();
        };

        size_t put_due(const kind& kind, time_point now) {
            auto& k_enqueued{enqueued[kind]};
            auto& k_delayed{delayed[kind]};
            auto& k_unacked{unacked[kind]};
            auto& k_unacked_time_points{unacked_time_points[kind]};

            size_t result{};

            auto delayed_due{k_delayed.lower_bound(now)};
            for (auto delayed_it{k_delayed.cbegin()}; delayed_it != delayed_due; ++delayed_it) {
                k_enqueued.emplace(delayed_it->second.id, std::move(delayed_it->second.payload));
                ++result;
            }
            k_delayed.erase(k_delayed.begin(), delayed_due);

            auto unacked_due{k_unacked.lower_bound(now)};
            for (auto unacked_it{k_unacked.cbegin()}; unacked_it != unacked_due; ++unacked_it) {
                k_enqueued.emplace(unacked_it->second.id, std::move(unacked_it->second.payload));
                k_unacked_time_points.erase(unacked_it->second.id);
                ++result;
            }
            k_unacked.erase(k_unacked.begin(), unacked_due);

            return result;
        };
    };

    std::shared_ptr<state> data;

    static std::optional<std::vector<std::pair<id_t, std::shared_ptr<const bytes>>>>
    opt_with_empty_vec() {
        return std::optional{std::vector<std::pair<id_t, std::shared_ptr<const bytes>>>{}};
    }
}; // test_bus

template <typename Bus>
class scheduler {
    Bus bus;

public:
    using clock = typename Bus::clock;
    using duration = typename Bus::clock::duration;
    using time_point = typename Bus::clock::time_point;
    using id_t = typename Bus::id_t;

    explicit scheduler<Bus>(Bus bus) : bus{bus} {};

    template <typename T, typename Args = typename T::args>
    expected<id_t> try_schedule(const Args& task, duration after = duration::zero()) {
        static_assert(is_serializable_v<T, Args> && has_kind_v<T>);

        auto kind{T::kind()};

        return bus.put(kind, std::move(T::serialize(task)), after);
    };

    template <typename T, typename Args = typename T::args>
    expected<id_t> try_schedule(const Args& task, time_point after) {
        return try_schedule<T, Args>(task, after - clock::now());
    };

    template <typename T, typename Args = typename T::args>
    id_t schedule(const Args& task, duration after = duration::zero()) {
        auto result{try_schedule<T, Args>(task, after)};
        if (result.has_value())
            return result.value();

        throw result.error();
    };

    template <typename T, typename Args = typename T::args>
    id_t schedule(const Args& task, time_point after) {
        return schedule<T, Args>(task, after - clock::now());
    };
}; // scheduler

template <typename Bus>
class worker_pool {
public:
    using id_t = typename Bus::id_t;
    using duration = typename Bus::duration;
    using job = std::pair<typename Bus::id_t, std::shared_ptr<const bytes>>;

    static constexpr duration default_polling_interval = std::chrono::milliseconds(100);

    explicit worker_pool<Bus>(Bus bus, duration polling_interval = default_polling_interval)
        : data{std::make_shared<state>(bus, polling_interval)} {};

    worker_pool(worker_pool const& other) = delete;
    worker_pool(worker_pool&& other) = delete;
    worker_pool& operator=(worker_pool const& other) = delete;
    worker_pool& operator=(worker_pool&& other) = delete;
    ~worker_pool() { stop(); }

    template <typename T, typename TArgs = typename T::args>
    void work_on(T task, size_t pool_size) {
        data->workers.try_emplace(T::kind(), data->bus, data->polling_interval, task, pool_size);
    };

    void stop() {
        for (auto& [_, worker] : data->workers)
            worker.stop();
    };

private:
    class worker {
        Bus bus;
        duration polling_interval;
        kind kind;
        size_t size{};

        std::vector<std::thread> threads;
        std::mutex mtx;
        std::vector<job> jobs;
        size_t ready_at{};
        size_t ready_count{};
        std::atomic<bool> stopping{};
        std::set<id_t> seen_ids;

    public:
        template <typename T, typename TArgs = typename T::args>
        worker(Bus bus, duration polling_interval, T task, size_t size)
            : bus{bus}, polling_interval(polling_interval), kind{T::kind()}, size{size} {
            std::lock_guard<std::mutex> _{mtx};

            threads.reserve(size);
            jobs.reserve(size);

            for (size_t i{}; i < size; ++i) {
                jobs.emplace_back();
                threads.emplace_back([this, task]() mutable {
                    std::optional<job> opt_job{};
                    while ((opt_job = next()).has_value() && !stopping) {
                        auto& [id, payload]{opt_job.value()};
                        try {
                            if (task(T::deserialize(*payload)) == std::nullopt)
                                ack(id);
                            else
                                nack(id);
                        } catch (std::exception& e) {
                            nack(id);
                        }
                    }
                });
            }
        };

        worker(worker const& other) = delete;
        worker(worker&& other) = delete;
        worker& operator=(worker const& other) = delete;
        worker& operator=(worker&& other) = delete;

        ~worker() {
            stop();
            for (auto& thread : threads)
                if (thread.joinable())
                    thread.join();
        }

        void stop() {
            if (stopping)
                return;
            std::lock_guard<std::mutex> _{mtx};
            stopping = true;
        }

    private:
        std::optional<job> next() {
            std::lock_guard<std::mutex> _{mtx};
            if (size == 0 || stopping)
                return std::nullopt;

            if (ready_count > 0) {
                auto was_ready_at{ready_at};
                --ready_count;
                ready_at = (was_ready_at + 1) % size;
                return jobs[was_ready_at];
            }

            auto new_jobs{bus.next(kind, size, polling_interval)};
            if (!new_jobs.has_value() || new_jobs.value().empty()) {
                stopping = !new_jobs.has_value();
                return std::nullopt;
            }

            for (size_t i{}; i < std::min(new_jobs.value().size(), size) && i < size; ++i) {
                jobs[(ready_at + i) % size] = new_jobs.value()[i];
                ++ready_count;
            }

            auto was_ready_at{ready_at};
            --ready_count;
            ready_at = (was_ready_at + 1) % size;

            return jobs[was_ready_at];
        };

        void ack(id_t id) { bus.ack(kind, id); };
        void nack(id_t id) { bus.nack(kind, id); };
    };

    struct state {
        Bus bus;
        duration polling_interval;
        std::map<kind, worker> workers;

        explicit state(Bus bus, duration polling_interval)
            : bus{bus}, polling_interval(polling_interval) {};
    };

    std::shared_ptr<state> data;
}; // worker_pool

} // namespace squedl

#endif // SQUEDL_SQUEDL_HPP
