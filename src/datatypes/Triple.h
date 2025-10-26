#pragma once

template <typename T1, typename T2, typename T3>
struct Triple {
    T1 first;
    T2 second;
    T3 third;

    Triple(const T1& a, const T2& b, const T3& c) : first(a), second(b), third(c) {}

    bool operator==(const Triple & triple) const = default;
};
