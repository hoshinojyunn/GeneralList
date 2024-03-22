#include "GeneralList.h"
#include <cstdlib>
#include <iostream>
#include <gtest/gtest.h>

struct X{
    X()=default;
    virtual void func(){
        std::cout << "hello X\n";
    }
};

template<class T>
struct Y:public X{
    T val;
    Y(T i_) : val(i_){}
    void func() {
        std::cout << "hello Y\n";
    }
};

TEST(Basic, ElementConstructTest){
    X x{};
    tool::Element e{1.2};
    tool::Element e1{x};
    tool::Element e2{std::string{"abcdefg"}};
    tool::Element e3{1.5};
}

TEST(Basic, GeneralListBasicTest){
    X x{};
    tool::Element e{1.2};
    tool::Element e1{x};
    tool::Element e2{std::string{"abcdefg"}};
    tool::Element e3{1.5};

    tool::GeneralList list0 {1,e1,e2, "abc"};
    tool::GeneralList list {"1236546", X{}, Y{10}};
    list.insert(list.begin(), list0);

    std::cout << list << '\n';
}

TEST(Basic, ElementCastTest){
    Y<std::string>y{"hello"};
    tool::Element e{1.2};
    tool::Element e1{y};
    tool::Element e2{std::string{"abcdefg"}};
    tool::Element e3{10};

    double e_b = e.ElementCast<double>();
    Y<std::string>&e1_b = e1.ElementCast<Y<std::string>&>();
    std::string e2_b = e2.ElementCast<std::string>();
    int e3_b = e3.ElementCast<int>();
    ASSERT_EQ(e1_b.val, "hello");
    e1_b.val = "world";

    ASSERT_LE(std::abs(e_b-1.2), 1e-5);
    ASSERT_EQ(e1_b.val, "world");
    ASSERT_EQ(e2_b, "abcdefg");
    ASSERT_EQ(e3_b, 10);

    X x{};
    tool::Element e4{X{}};
    ASSERT_NO_THROW(e4.ElementCast<X>());

    tool::Element e5{x};

    ASSERT_ANY_THROW(e5.ElementCast<X>());
    ASSERT_ANY_THROW(e4.ElementCast<int>());
}

TEST(Basic, CompareTest){
    X x{};
    tool::Element e{1.2};
    tool::Element e1{x};
    tool::Element e2{std::string{"abcdefg"}};
    tool::Element e3{10};
    tool::Element e4{10};
    tool::Element e5{1.5};

    // 底层类型不同的比较 出错
    ASSERT_THROW((e > e3), exception::compare_exception);
    ASSERT_THROW((e < e3), exception::compare_exception);
    ASSERT_THROW((e >= e3), exception::compare_exception);
    ASSERT_THROW((e <= e3), exception::compare_exception);
    ASSERT_THROW((e == e3), exception::compare_exception);
    ASSERT_THROW((e != e3), exception::compare_exception);
    
    // 底层类型相同的比较
    ASSERT_EQ((e > e5), false);
    ASSERT_EQ((e != e5), true);
    ASSERT_EQ((e3 >= e4), true);
    ASSERT_EQ((e3 != e4), false);
}

