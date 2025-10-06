#include "squedl/squedl.hpp"

#include <atomic>
#include <chrono>
#include <gtest/gtest.h>
#include <random>

namespace {
std::atomic<int64_t> sum_result{0};
std::atomic<int64_t> sub_result{0};

class single_serializable_value_task {
public:
    struct args {
        int64_t value{};
    };

    static std::vector<std::byte> serialize(const args& arg) {
        const auto* bytes{reinterpret_cast<const std::byte*>(&arg.value)};
        return {bytes, bytes + sizeof(arg.value)};
    }

    static args deserialize(const std::vector<std::byte>& data) {
        args args{};
        if (data.size() >= sizeof(int64_t))
            std::memcpy(&args.value, data.data(), sizeof(int64_t));
        return args;
    }
};

class sum_task : public single_serializable_value_task {
public:
    using args = single_serializable_value_task::args;

    static std::string kind() { return "sum_task"; }

    std::optional<squedl::error> operator()(args args) {
        sum_result += args.value;

        return std::nullopt;
    }
};

class sub_task : public single_serializable_value_task {
public:
    static std::string kind() { return "sub_task"; }

    std::optional<squedl::error> operator()(args args) {
        sub_result -= args.value;

        return std::nullopt;
    }
};
} // namespace

TEST(squedl, e2e_task_processing) {
    using namespace std::chrono_literals;
    const size_t NUM_TASKS{20};
    const size_t NUM_WORKERS{5};

    int64_t expected_sum{};
    int64_t expected_sub{};

    sum_result = 0;
    sub_result = 0;

    std::random_device rd;
    std::mt19937 gen{rd()};
    std::uniform_int_distribution<int64_t> value_dist{1, 10};
    std::uniform_int_distribution<int> delay_dist{0, 1};
    std::uniform_int_distribution<int> task_type_dist{0, 1};
    std::uniform_int_distribution<int> delay_ms_dist{100, 200};

    squedl::test_bus bus{1min};
    squedl::scheduler scheduler{bus};
    squedl::worker_pool pool{bus};

    for (size_t i = 0; i < NUM_TASKS; ++i) {
        int64_t value{value_dist(gen)};
        auto delay{delay_dist(gen) ? std::chrono::milliseconds(delay_ms_dist(gen)) : 0ms};

        switch (task_type_dist(gen)) {
        case 0: {
            scheduler.schedule<sum_task>(sum_task::args{value}, delay);
            expected_sum += value;
            break;
        }
        case 1: {
            scheduler.schedule<sub_task>(sub_task::args{value}, delay);
            expected_sub -= value;
            break;
        }
        }
    }

    size_t check_count{};

    pool.work_on(sum_task{}, NUM_WORKERS);
    pool.work_on(sub_task{}, NUM_WORKERS);

    while (!bus.empty())
        std::this_thread::sleep_for(1s);

    EXPECT_EQ(sum_result, expected_sum) << "sum result mismatch";
    EXPECT_EQ(sub_result, expected_sub) << "subtraction result mismatch";
    pool.stop();
    bus.stop();
}
