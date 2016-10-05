#pragma once

#include "common/conversions.h"

#include "glm/glm.hpp"

namespace util {

template <typename t>
class range final {
public:
    using value_type = t;

    /// setters/getters

    constexpr range()
            : min_{0}
            , max_{0} {}

    constexpr range(const value_type& a, const value_type& b)
            : min_{a}
            , max_{b} {
        maintain_invariant();
    }

    constexpr value_type get_min() const { return min_; }
    constexpr value_type get_max() const { return max_; }

    /// mutators

    constexpr range& operator+=(const value_type& v) {
        min_ += v;
        max_ += v;
        return *this;
    }

    constexpr range& operator-=(const value_type& v) {
        min_ -= v;
        max_ -= v;
        return *this;
    }

    constexpr range& pad(const value_type& v) {
        min_ -= v;
        max_ += v;
        maintain_invariant();
        return *this;
    }

    template <typename archive>
    void serialize(archive&);

private:
    void maintain_invariant() {
        using glm::min;
        using glm::max;

        const auto a{min_};
        const auto b{max_};

        min_ = min(a, b);
        max_ = max(a, b);
    }

    value_type min_;
    value_type max_;
};

template <typename t>
range<t> make_range(const t& a, const t& b) {
    return range<t>{a, b};
}

//----------------------------------------------------------------------------//

template <typename t>
constexpr bool operator==(const range<t>& a, const range<t>& b) {
    return std::make_tuple(a.get_min(), a.get_max()) ==
           std::make_tuple(b.get_min(), b.get_max());
}

template <typename t>
constexpr bool operator!=(const range<t>& a, const range<t>& b) {
    return !(a == b);
}

template <typename t>
inline range<t> operator+(const range<t>& a, const t& b) {
    auto ret{a};
    return ret += b;
}

template <typename t>
inline range<t> operator-(const range<t>& a, const t& b) {
    auto ret{a};
    return ret -= b;
}

template <typename t>
inline auto padded(const range<t>& a, const t& b) {
    auto ret{a};
    return ret.pad(b);
}

template <typename t, typename u>
inline auto padded(const range<t>& a, const u& b) {
    return padded(a, t{b});
}

template <typename t>
inline auto inside(const range<t>& a, const t& b) {
    return glm::all(glm::lessThan(a.get_min(), b)) &&
           glm::all(glm::lessThan(b, a.get_max()));
}

template <typename t>
inline auto centre(const range<t>& a) {
    return (a.get_min() + a.get_max()) * t{0.5};
}

template <typename t>
inline auto dimensions(const range<t>& a) {
    return a.get_max() - a.get_min();
}

//----------------------------------------------------------------------------//

template <typename it>
inline auto min_max(it begin, it end) {
    if (begin == end) {
        throw std::runtime_error("can't minmax empty range");
    }

    using glm::min;
    using glm::max;

    auto mini{to_vec3(*begin)}, maxi{to_vec3(*begin)};
    for (auto i{begin + 1}; i != end; ++i) {
        const auto pt{to_vec3(*i)};
        mini = min(pt, mini);
        maxi = max(pt, maxi);
    }
    return make_range(mini, maxi);
}

namespace detail {
template <size_t dimensions>
struct range_value;

template <>
struct range_value<1> final {
    using type = float;
};
template <>
struct range_value<2> final {
    using type = glm::vec2;
};
template <>
struct range_value<3> final {
    using type = glm::vec3;
};

template <size_t n>
using range_value_t = typename range_value<n>::type;

template <size_t n>
using range_t = range<range_value_t<n>>;
}  // namespace detail

template <typename T, typename U, typename V>
constexpr auto map(T x, range<U> in, range<V> out) {
    return (((x - in.get_min()) * dimensions(out)) / dimensions(in)) +
           out.get_min();
}

}  // namespace util