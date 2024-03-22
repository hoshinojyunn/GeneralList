#pragma once

#include <functional>
#include <iostream>
#include <memory>
#include <optional>
#include <ostream>
#include <stdexcept>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <typeinfo>
#include <format>
#include <utility>
#include <vector>


namespace exception {
struct compare_exception : std::logic_error {
    compare_exception(
        const std::type_info&l_ti, 
        const std::type_info&r_ti, const char *opt) : std::logic_error(
            std::format("compare_exception: {} {} {}", l_ti.name(), opt, r_ti.name())
        ){}
};

struct element_access_exception : std::runtime_error {
    element_access_exception() : std::runtime_error("bad element access"){}
};

struct element_cast_exception : std::runtime_error{
    element_cast_exception() : std::runtime_error("element cast fail"){}
};
}


namespace tool {

// 当T类型没有重载<<  打印s
template<class T, class = std::void_t<>>
struct Printer{
    static std::ostream& DefaultPrint(std::ostream&out, const T&val, const std::string_view s){
        return out<< s;
    }
};

// 偏特化 当T重载了<<  则打印值
template<class T>
struct Printer<T, std::void_t<decltype(std::cout << std::declval<T>())>>{
    static std::ostream& DefaultPrint(std::ostream&out, const T&val, const std::string_view s){
        return out<< val;
    }
};

//比较要定义为类模板（方法模板无法偏特化）
#define DEFAULT_CMP(name, opt)                                              \
template<class T, class = std::void_t<>>                                    \
struct default_##name{                                                      \
    static bool compare(const T&t1, const T&t2){                            \
        throw exception::compare_exception{typeid(T), typeid(T), #opt};     \
    }                                                                       \
};                                                                          \
template<class T>                                                           \
struct default_##name<                                                      \
        T,                                                                  \
        std::void_t<decltype(std::declval<T>() opt std::declval<T>())>      \
    >{                                                                      \
    static bool compare(const T&t1, const T&t2){                            \
        return t1 opt t2;                                                   \
    }                                                                       \
};                                                                           

DEFAULT_CMP(equal, ==)
DEFAULT_CMP(greater, >)
DEFAULT_CMP(less, <)
DEFAULT_CMP(greater_equal, >=)
DEFAULT_CMP(less_equal, <=)
#undef DEFAULT_CMP

template<class T>
struct function_traits;
// 普通函数偏特化
template <class Ret, class...Args>
struct function_traits<Ret(*)(Args...)>{
    using function_ptr = Ret(*)(Args...);
    using function_type = std::function<Ret(Args...)>;
    using return_type = Ret;
    using params = std::tuple<Args...>;
    constexpr static bool is_class_func = false;
};
// 成员函数偏特化
template<class Ret, class Class, class...Args>
struct function_traits<Ret(Class::*)(Args...)>{
    using function_ptr = Ret(Class::*)(Args...);
    using function_type = std::function<Ret(Args...)>;
    using return_type = Ret;
    using params = std::tuple<Args...>;
    using clazz = Class;
    constexpr static bool is_class_func = true;
};

struct Element{
    
    struct Holder{
        virtual std::ostream &Print(std::ostream&) const = 0;
        virtual bool Equal(const Holder&) const = 0;
        virtual bool Greater(const Holder&) const = 0;
        virtual bool Less(const Holder&) const = 0;
        virtual bool GreaterEqual(const Holder&) const = 0;
        virtual bool LessEqual(const Holder&) const = 0;
        virtual std::string Id() const = 0;
        virtual const std::type_info& Type() const = 0;
        virtual std::shared_ptr<Holder> Clone() const = 0;
        // holder重载<< 后面重载Element要用
        friend std::ostream& operator<<(std::ostream&out, const Holder&h){
            // 多态
            return h.Print(out);
        }
    };

    template<class T>
    struct HolderImpl : public Holder {
        HolderImpl() = default;
        template<class...Args>
        explicit HolderImpl(Args&&...args) : val(std::forward<Args>(args)...){}
        const std::type_info& Type() const override{
            return typeid(T);
        }
        std::string Id() const override {
            return std::format("{}", Type().name());
        };
        std::ostream &Print(std::ostream&out) const override {
            return tool::Printer<T>::DefaultPrint(out, val, Id());
        }
        #define CMP_IMPL(name, cmp_name, opt) \
        bool name(const Holder&other) const override { \
            const auto&l_ti = Type();                   \
            const auto&r_ti = other.Type();             \
            return l_ti!=r_ti ? throw exception::compare_exception(l_ti, r_ti, #opt) : \
            tool::default_##cmp_name<T>::compare(val, dynamic_cast<const HolderImpl<T>&>(other).val); \
        }
        
        CMP_IMPL(Equal, equal, ==)
        CMP_IMPL(Greater, greater, >)
        CMP_IMPL(Less, less, <)
        CMP_IMPL(GreaterEqual, greater_equal, >=)
        CMP_IMPL(LessEqual, less_equal, <=)
        #undef CMP_IMPL

        std::shared_ptr<Holder> Clone() const override{
            return std::make_shared<HolderImpl<T>>(this->val);
        }
        T val;
    };

    Element()=default;
    ~Element() = default;
    // 去掉引用符与cv限定符 留下最纯粹的T
    template<class T>
    using BaseType = std::decay_t<std::remove_reference_t<T>>;

    // 当T类型与Element不同时 启用该构造函数 进行其他类型到Element的转换
    template<class T, class = std::enable_if_t<!std::is_same_v<BaseType<T>, Element>>>
    Element(T&&t) : holder_(std::make_shared<HolderImpl<T>>(std::forward<T>(t))){}
    
    template<class T, class = std::enable_if_t<!std::is_same_v<BaseType<T>, Element>>>
    Element& operator=(T&&t){
        Reset(), holder_ = std::make_shared<HolderImpl<T>>(std::forward<T>(t));
        return *this;
    }
    Element(const Element&other) : holder_(other.Clone()) {}
    Element& operator=(const Element&other){
        this->holder_ = other.Clone();
        return *this;
    }
    Element(Element&&other) {
        this->holder_ = other.holder_;
        other.holder_ = std::nullopt;
    }
    Element& operator=(Element&&other){
        this->holder_ = other.holder_;
        other.holder_ = std::nullopt;
        return *this;
    }

    bool HasValue() const noexcept {
        return holder_.has_value();
    }

    void Reset(){
        holder_ = std::nullopt;
    }

    auto Id() const -> std::string {
        return HasValue()?holder_.value()->Id() : "Null";
    }
    auto Type() const -> const std::type_info& {
        return HasValue()?holder_.value()->Type() : typeid(void);
    }

    // T不为Element时
    template<class T, class = 
        std::enable_if_t<!std::is_same_v<BaseType<T>, Element>>
    >
    auto ElementCast() const -> T {
        return GetHolderImpl<T>().val;
    }

    template<class T, class = 
        std::enable_if_t<std::is_same_v<BaseType<T>, Element>>
    >
    auto ElementCast() -> T {
        return *this;
    }

    // 显示类型转换
    template<class T>
    explicit operator T() const {
        return ElementCast<T>();
    }

    // 输出HolderImpl的内容 会调用HolderImpl的Print
    friend std::ostream& operator<<(std::ostream&out, const Element&elem){
        return elem.HasValue()?(out << elem.GetHolder()) : (out << "Null");
    }

    friend bool operator==(const Element&l, const Element&r){
        if(!l.HasValue() || !r.HasValue())
            throw exception::element_access_exception();
        return l.GetHolder().Equal(r.GetHolder());
    }

    friend bool operator <(const Element&l, const Element&r){
        if(!l.HasValue() || !r.HasValue())
            throw exception::element_access_exception();
        return l.GetHolder().Less(r.GetHolder());
    }

    friend bool operator >(const Element&l, const Element&r){
        if(!l.HasValue() || !r.HasValue())
            throw exception::element_access_exception();
        return l.GetHolder().Greater(r.GetHolder());
    }

    friend bool operator!=(const Element&l, const Element&r){
        return !(l == r);
    }

    friend bool operator>=(const Element&l, const Element&r){
        return l > r || l==r;
    }

    friend bool operator<=(const Element&l, const Element&r){
        return l < r || l==r;
    }

    
private:
    std::optional<std::shared_ptr<Holder>>holder_;
    auto Clone() const noexcept -> std::shared_ptr<Holder> {
        return HasValue()? holder_.value()->Clone() : nullptr;
    }
    template<class T>
    auto GetHolderImpl() const -> const HolderImpl<T> {
        if(!HasValue())
            throw exception::element_access_exception();
        
        auto holder_impl = std::dynamic_pointer_cast<HolderImpl<T>>(holder_.value());
        if(!holder_impl)
            throw exception::element_cast_exception();
        return std::move(*holder_impl);
    }

    auto GetHolder() const -> const Holder& {
        if(!HasValue())
            throw exception::element_access_exception();
        auto holder = holder_.value();
        if(!holder)
            throw std::runtime_error{"holder is empty"};
        return *holder;
    }
};


struct GeneralList : public std::vector<Element>{
    // 继承vector<Element>的构造函数 GeneralList可以直接使用其构造函数构造自己
    using std::vector<Element>::vector;
    friend std::ostream& operator <<(std::ostream&out, const GeneralList&list){
        if(list.empty())
            return out<<"[]";
        auto it = list.begin();
        out << "[" << *it++;
        for(; it !=list.end();++it){
            out << ", " << *it; 
        }
        out << "]";
        return out;
    }

private:
    
};

}

