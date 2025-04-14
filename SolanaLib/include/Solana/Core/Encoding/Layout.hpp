#pragma once
#include <functional>
#include "Solana/Core/Types/Types.hpp"
#include "Solana/Core/Encoding/Serializer.hpp"

#define EVAL0(...) __VA_ARGS__
#define EVAL1(...) EVAL0(EVAL0(EVAL0(__VA_ARGS__)))
#define EVAL2(...) EVAL1(EVAL1(EVAL1(__VA_ARGS__)))
#define EVAL3(...) EVAL2(EVAL2(EVAL2(__VA_ARGS__)))
#define EVAL4(...) EVAL3(EVAL3(EVAL3(__VA_ARGS__)))
#define EVAL5(...) EVAL4(EVAL4(EVAL4(__VA_ARGS__)))
#define EVAL6(...) EVAL5(EVAL5(EVAL5(__VA_ARGS__)))
#define EVAL7(...) EVAL6(EVAL6(EVAL6(__VA_ARGS__)))
#define EVAL(...)  EVAL7(EVAL7(EVAL7(__VA_ARGS__)))

#define MAP_END(...)
#define MAP_OUT
#define MAP_COMMA ,

#define MAP_GET_END2() 0, MAP_END
#define MAP_GET_END1(...) MAP_GET_END2
#define MAP_GET_END(...) MAP_GET_END1
#define MAP_NEXT0(test, next, ...) next MAP_OUT
#define MAP_NEXT1(test, next) MAP_NEXT0(test, next, 0)
#define MAP_NEXT(test, next)  MAP_NEXT1(MAP_GET_END test, next)

#define MAP0(f, x, peek, ...) f(x) MAP_NEXT(peek, MAP1)(f, peek, __VA_ARGS__)
#define MAP1(f, x, peek, ...) f(x) MAP_NEXT(peek, MAP0)(f, peek, __VA_ARGS__)

#define MAP_LIST_NEXT1(test, next) MAP_NEXT0(test, MAP_COMMA next, 0)
#define MAP_LIST_NEXT(test, next)  MAP_LIST_NEXT1(MAP_GET_END test, next)

#define MAP_LIST0(f, x, peek, ...) f(x) MAP_LIST_NEXT(peek, MAP_LIST1)(f, peek, __VA_ARGS__)
#define MAP_LIST1(f, x, peek, ...) f(x) MAP_LIST_NEXT(peek, MAP_LIST0)(f, peek, __VA_ARGS__)

/**
 * Applies the function macro `f` to each of the remaining parameters.
 */
#define MAP(f, ...) EVAL(MAP1(f, __VA_ARGS__, ()()(), ()()(), ()()(), 0))

#define DECL_VAR(type,var) type var{};

#define GET_VAR_NAME(type, var) var,
#define GET_VAR_NAME_PAIR(pair) GET_VAR_NAME pair
#define DECL_VAR_PAIR(pair) DECL_VAR pair

#define GET_BYTES_NEEDED(type, var) Solana::BytesNeeded<type>::value +
#define GET_BYTES_NEEDED_PAIR(pair) GET_BYTES_NEEDED pair

#define LAYOUT(S, ...) \
  struct S {             \
    bool operator==(const S&) const = default;                    \
    void decode(const Solana::Buffer & buffer) { \
        return LayoutDecoder().Decode(buffer, MAP(GET_VAR_NAME_PAIR, __VA_ARGS__) std::placeholders::_1);   \
    }                   \
    Solana::Buffer encode() const {     \
        return LayoutEncoder().Encode(MAP(GET_VAR_NAME_PAIR, __VA_ARGS__) std::placeholders::_1).getBuffer();  \
    }                   \
    static int space() { return MAP(GET_BYTES_NEEDED_PAIR, __VA_ARGS__) Solana::BytesNeeded<std::in_place_t>::value; }                    \
    MAP(DECL_VAR_PAIR, __VA_ARGS__)      \
  };



