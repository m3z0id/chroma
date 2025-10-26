#pragma once

template <typename T1, typename T2, typename T3, typename T4>
struct Quadruple {
    T1 first;
    T2 second;
    T3 third;
    T4 fourth;

    Quadruple(const T1& a, const T2& b, const T3& c, const T4& d) : first(a), second(b), third(c), fourth(d) {}

    bool operator==(const Quadruple & triple) const = default;
};
