# Description

Provides variant class and visit function in 'std-like' manner. 
It's delevoped mainly for educational purposes
and ability to write something interesting in C++11.

It can also perform checks at compile-time whether all contained types:
1. Have not explicitly deleted move assignment operator;
2. Have copy assignment operator;
3. Have both.

By default variant is checking both.

With checks turned off, you will get std::logic_error exception with some debug info of type dependent on your compiler if you:
1. Try to copy variant currently containing non-copyable type;
2. Try to move variant currently containing non-movable type.

In release mode debug info is not included by default.
If you still need it, define VAR_THROW_RUNTIME_DEBUG_INFO
before including var_stuff.h

## Dependencies

Standard library for c++11

## Variant compile-time checks

If you want to turn off checks for:
1. Move operator= -> `var_stuff::mcheck(VALUE);`
2. Copy operator= -> `var_stuff::ccheck(VALUE);`
3. Both -> `var_stuff::nocheck(VALUE);

## Visit

If you want to return something from var_stuff::visit, the return type must have either copy constructor or move constructor.

## Compilation

Compiles with g++ and clang++, MSVC is not tested.
