#include <gtest/gtest.h>
#include "sutolist.hpp"
#include "ctor_dtor_counter.hpp"

TEST(Sutolist, name) {
    Sutolist<int> l;
    ASSERT_STREQ(l.name(), "Sutolist");
}

TEST(Sutolist, initializer_list_ctor) {
    Sutolist<int> l {1, 2, 3, 4};
    ASSERT_EQ(l.size(), 4);
    ASSERT_EQ(l[0], 1);
    ASSERT_EQ(l[3], 4);
}

TEST(Sutolist, copy_ctor) {
    Sutolist<int> l1 {1, 2, 3, 4};
    Sutolist<int> l2 {l1};
    l2[3] = 10;
    l2.push_back(20);
    ASSERT_EQ(l1.size(), 4);
    ASSERT_EQ(l2.size(), 5);
    ASSERT_EQ(l1[3], 4);
    ASSERT_EQ(l2[3], 10);
}

TEST(Sutolist, copy_assign) {
    Sutolist<int> l1 {1, 2, 3, 4};
    Sutolist<int> l2 = l1;
    l2[3] = 10;
    l2.push_back(20);
    ASSERT_EQ(l1.size(), 4);
    ASSERT_EQ(l2.size(), 5);
    ASSERT_EQ(l1[3], 4);
    ASSERT_EQ(l2[3], 10);
}

TEST(Sutolist, move_ctor) {
    Sutolist<int> l1 {1, 2, 3, 4};
    Sutolist<int> l2 { std::move(l1) };
    ASSERT_EQ(l1.size(), 0);
    ASSERT_EQ(l2.size(), 4);
}

TEST(Sutolist, push_back) {
    Sutolist<int> l;
    l.push_back(10);
    l.push_back(20);

    ASSERT_EQ(l.size(), 2);
    ASSERT_EQ(l[0], 10);
    ASSERT_EQ(l[1], 20);
}

TEST(Sutolist, insert_begin) {
    Sutolist<int> l {1, 2, 3, 4};
    l.insert(l.begin(), 100);

    ASSERT_EQ(l.size(), 5);
    ASSERT_EQ(l[0], 100);
}

TEST(Sutolist, insert_middle) {
    Sutolist<int> l {1, 2, 3, 4};
    l.insert(2, 100);

    ASSERT_EQ(l.size(), 5);
    ASSERT_EQ(l[2], 100);
}

TEST(Sutolist, erase_begin) {
    Sutolist<int> l {1, 2, 3, 4};
    l.erase(l.begin());

    ASSERT_EQ(l.size(), 3);
    ASSERT_EQ(l[0], 2);
}

TEST(Sutolist, erase_middle) {
    Sutolist<int> l {1, 2, 3, 4};
    l.erase(2);

    ASSERT_EQ(l.size(), 3);
    ASSERT_EQ(l[2], 4);
}

TEST(Sutolist, erase_back) {
    Sutolist<int> v {1, 2, 3, 4};
    v.erase(v.end()-1);

    ASSERT_EQ(v.size(), 3);
    ASSERT_EQ(v[2], 3);
}

TEST(Sutolist, dtor) {
    CtorDtorCounter::reset();
    {
        Sutolist<CtorDtorCounter> l {};
        l.push_back(CtorDtorCounter{});
        l.push_back(CtorDtorCounter{});
        l.push_back(CtorDtorCounter{});

        ASSERT_EQ(3, l.size());
        ASSERT_GT(CtorDtorCounter::ctored, 0);
    }

    ASSERT_EQ(CtorDtorCounter::ctored, CtorDtorCounter::dtored);
    ASSERT_GT(CtorDtorCounter::dtored, 0);
}
