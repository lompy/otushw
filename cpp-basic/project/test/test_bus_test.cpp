#include <chrono>
#include <cstddef>
#include <set>

#include <gtest/gtest.h>

#include "squedl/squedl.hpp"

TEST(squedl, test_bus) {
    const std::string kind{"test_kind"};
    const size_t total_messages{500};
    const auto ack_timeout{std::chrono::milliseconds{200}};
    const auto delay{std::chrono::milliseconds{1000}};

    squedl::test_bus<> bus{ack_timeout};
    std::set<squedl::test_bus<>::id_t> all_ids;
    std::set<squedl::test_bus<>::id_t> immediate_ids;
    std::set<squedl::test_bus<>::id_t> delayed_ids;
    std::set<squedl::test_bus<>::id_t> unacked_ids;

    for (size_t i = 0; i < total_messages; ++i) {
        auto val{static_cast<std::byte>(i % 256)};
        auto delayed{i % 10 == 0};
        auto result =
            bus.put(kind, std::vector{val}, delayed ? delay : squedl::test_bus<>::duration::zero());
        ASSERT_TRUE(result.has_value());
        all_ids.insert(result.value());
        if (delayed)
            delayed_ids.insert(result.value());
        else
            immediate_ids.insert(result.value());
    }
    ASSERT_EQ(all_ids.size(), immediate_ids.size() + delayed_ids.size());

    auto batch{bus.next(kind, total_messages / 10)};
    size_t final_count{};
    ASSERT_TRUE(batch.has_value());
    ASSERT_EQ(batch.value().size(), total_messages / 10);
    for (size_t i{}; i < batch.value().size(); ++i) {
        auto const& [id, _]{batch.value()[i]};
        ASSERT_TRUE(immediate_ids.count(id));
        if (i % 2 == 0) {
            bus.ack(kind, id);
            ++final_count;
        } else {
            unacked_ids.insert(id);
        };
    }

    batch = bus.next(kind, total_messages);
    ASSERT_TRUE(batch.has_value());
    ASSERT_EQ(batch.value().size(), total_messages * 8 / 10); // actually didn't get delayed
    for (const auto& [id, _] : batch.value()) {
        ASSERT_TRUE(immediate_ids.count(id));
        bus.ack(kind, id);
        ++final_count;
    }

    std::this_thread::sleep_for(ack_timeout * 2);

    batch = bus.next(kind, total_messages / 10); // try to get all
    ASSERT_TRUE(batch.has_value());
    ASSERT_EQ(batch.value().size(), total_messages / 20); // actually get only acked
    for (const auto& [id, _] : batch.value()) {
        ASSERT_TRUE(unacked_ids.count(id));
        bus.ack(kind, id);
        ++final_count;
    }

    // wait for delayed in a loop
    while (final_count < total_messages) {
        batch = bus.next(kind, total_messages);
        if (!batch.has_value())
            break;
        for (const auto& [id, _] : batch.value()) {
            ASSERT_TRUE(delayed_ids.count(id));
            bus.ack(kind, id);
            ++final_count;
        }
    }

    bus.stop();
    ASSERT_EQ(bus.enqueued_size(kind), 0);
    ASSERT_EQ(bus.delayed_size(kind), 0);
    ASSERT_EQ(bus.unacked_size(kind), 0);
}
