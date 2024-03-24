#include "GeneralList.h"
#include <any>
#include <cstdlib>
#include <iostream>
#include <gtest/gtest.h>
#include <typeinfo>

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
    std::cout << "Element typeid=" << typeid(tool::Element).name() << '\n';
}

TEST(Basic, GeneralListBasicTest){
    X x{};
    tool::Element e{1.2};
    tool::Element e1{x};
    tool::Element e2{std::string{"abcdefg"}};
    tool::Element e3{1.5};

    // list0[3]: char* const
    tool::GeneralList list0 {1,e1,e2, "abc"};
    tool::GeneralList list {"1236546", X{}, Y{10}};
    list.insert(list.begin(), list0);

    const auto &l = list[0].ElementCast<tool::GeneralList&>();
    std::cout << list << '\n';
    std::cout << l << '\n';
    const tool::Element& e4 = 1.5;
    ASSERT_EQ(e4, e3);

    ASSERT_EQ(list[1], "1236546");
    
    // Equal(const Element&=l[0], const Element&=1)
    // l[0]是1 存的是右值 而1也是右值
    if(l[0] == 1){
        std::cout << "success\n";
    }

    // const int a = 1;
    // const tool::Element&e5{1};
    // if(a == e5){
    //     std::cout << "success1\n";
    // }

    // Equal(const Element&=l[0], const Element&=const int&1)
    // l[0]是1 存的是右值 而第二个参数是一个引用
    // ASSERT_EQ(l[0], 1);
    const std::string&s = "abc"; 

    ASSERT_THROW(auto x=(l[1]==e1), exception::compare_exception);
    ASSERT_EQ(l[2], e2);
    // char *const与std::string的比较(无法将std::string cast到char*const)
    ASSERT_THROW(auto x = (l[3] == s), std::bad_cast);

}

TEST(Basic, ElementCastTest){
    Y<std::string>y{"hello"};
    tool::Element e{1.2};
    tool::Element e1{y};
    tool::Element e2{std::string{"abcdefg"}};
    tool::Element e3{10};

    double e_b = e.ElementCast<double>();
    Y<std::string>e1_b = e1.ElementCast<Y<std::string>&>();
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

    ASSERT_NO_THROW(e5.ElementCast<X>());
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

    // 底层类型不同的比较 出错 出现bad_cast 会把第二个参数cast到第一个参数的类型
    ASSERT_THROW(auto x = (e < e3), std::bad_cast);
    ASSERT_THROW(auto x = (e5<e4), std::bad_cast);
    
    // 底层类型相同的比较
    ASSERT_EQ((e > e5), false);
    ASSERT_EQ((e != e5), true);
    ASSERT_EQ((e3 >= e4), true);
    ASSERT_EQ((e3 != e4), false);
}

