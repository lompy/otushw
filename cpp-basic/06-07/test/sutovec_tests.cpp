#include <gtest/gtest.h>
#include "sutovec.hpp"
#include "ctor_dtor_counter.hpp"

TEST(Sutovec, name) {
    Sutovec<int> v;
    ASSERT_STREQ(v.name(), "Sutovec");
}

TEST(Sutovec, initializer_list_ctor) {
    Sutovec<int> v {1, 2, 3, 4};
    ASSERT_EQ(v.size(), 4);
    ASSERT_EQ(v[0], 1);
    ASSERT_EQ(v[3], 4);
}

TEST(Sutovec, copy_ctor) {
    Sutovec<int> v1 {1, 2, 3, 4};
    Sutovec<int> v2 {v1};
    v2[3] = 10;
    v2.push_back(20);
    ASSERT_EQ(v1.size(), 4);
    ASSERT_EQ(v2.size(), 5);
    ASSERT_EQ(v1[3], 4);
    ASSERT_EQ(v2[3], 10);
}

TEST(Sutovec, copy_assign) {
    Sutovec<int> v1 {1, 2, 3, 4};
    Sutovec<int> v2 = v1;
    v2[3] = 10;
    v2.push_back(20);
    ASSERT_EQ(v1.size(), 4);
    ASSERT_EQ(v2.size(), 5);
    ASSERT_EQ(v1[3], 4);
    ASSERT_EQ(v2[3], 10);
}

TEST(Sutovec, move_ctor) {
    Sutovec<int> v1 {1, 2, 3, 4};
    Sutovec<int> v2 { std::move(v1) };
    ASSERT_EQ(v1.size(), 0);
    ASSERT_EQ(v2.size(), 4);
}

TEST(Sutovec, push_back) {
    Sutovec<int> v;
    v.push_back(10);
    v.push_back(20);

    ASSERT_EQ(v.size(), 2);
    ASSERT_GT(v.capacity(), 1);
    ASSERT_EQ(v[0], 10);
    ASSERT_EQ(v[1], 20);
}

TEST(Sutovec, insert_begin) {
    Sutovec<int> v {1, 2, 3, 4};
    v.insert(v.begin(), 100);

    ASSERT_EQ(v.size(), 5);
    ASSERT_EQ(v[0], 100);
}

TEST(Sutovec, insert_middle) {
    Sutovec<int> v {1, 2, 3, 4};
    v.insert(2, 100);

    ASSERT_EQ(v.size(), 5);
    ASSERT_EQ(v[2], 100);
}

TEST(Sutovec, erase_begin) {
    Sutovec<int> v {1, 2, 3, 4};
    v.erase(v.begin());

    ASSERT_EQ(v.size(), 3);
    ASSERT_EQ(v[0], 2);
}

TEST(Sutovec, erase_middle) {
    Sutovec<int> v {1, 2, 3, 4};
    v.erase(2);

    ASSERT_EQ(v.size(), 3);
    ASSERT_EQ(v[2], 4);
}

TEST(Sutovec, erase_back) {
    Sutovec<int> v {1, 2, 3, 4};
    v.erase(v.end()-1);

    ASSERT_EQ(v.size(), 3);
    ASSERT_EQ(v[2], 3);
}

TEST(Sutovec, dtor) {
    CtorDtorCounter::reset();
    {
        Sutovec<CtorDtorCounter> v {};
        v.push_back(CtorDtorCounter{});
        v.push_back(CtorDtorCounter{});
        v.push_back(CtorDtorCounter{});

        ASSERT_EQ(3, v.size());
        ASSERT_GT(CtorDtorCounter::ctored, 0);
    }

    ASSERT_EQ(CtorDtorCounter::ctored, CtorDtorCounter::dtored);
    ASSERT_GT(CtorDtorCounter::dtored, 0);
}
