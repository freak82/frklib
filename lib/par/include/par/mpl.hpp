#pragma once

#include <string_view>
#include <type_traits>

namespace freak::mpl
{

template <typename T>
consteval std::string_view type_name() noexcept
{
    std::string_view ret  = __PRETTY_FUNCTION__;
    std::string_view mark = "T = ";
    const auto mlen       = mark.size();
    const auto pos0       = ret.find(mark);
    if (pos0 == std::string_view::npos) throw -1;
    const auto pos1 = ret.find(';', pos0 + mlen);
    if (pos1 == std::string_view::npos) throw -1;
    ret = ret.substr(pos0 + mlen, pos1 - pos0 - mlen);
    return ret;
}

////////////////////////////////////////////////////////////////////////////////
#define FREAK_DEFINE_HAS_METHOD(method)                                   \
    template <typename, typename>                                         \
    class has_##method;                                                   \
    template <typename C, typename R, typename... A>                      \
    class has_##method<C, R(A...)>                                        \
    {                                                                     \
        template <typename T>                                             \
        static auto check(T*) -> typename std::is_same<                   \
            decltype(std::declval<T>().method(std::declval<A>()...)),     \
            R>::type;                                                     \
        template <typename>                                               \
        static std::false_type check(...);                                \
                                                                          \
    public:                                                               \
        static constexpr bool value = decltype(check<C>(nullptr))::value; \
    }

#define FREAK_HAS_METHOD(class_name, method, signature) \
    has_##method<class_name, signature>::value
#define FREAK_HAS_METHOD_NMSP(nmsp, class_name, method, signature) \
    nmsp::has_##method<class_name, signature>::value

} // namespace freak::mpl
