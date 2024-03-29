# Description

Provides variant class and visit function in 'std-like' manner. 
It's delevoped mainly for educational purposes
and ability to write something interesting in C++11.

It can also perform checks at compile-time whether all contained types:
1. Have not explicitly deleted move constructor;
2. Have copy constructor;
3. Have both.

By default variant is checking none.

With checks turned off, you will get std::logic_error exception with some debug type info, dependent on your compiler, if you:
1. Try to copy variant currently containing non-copyable type;
2. Try to move variant currently containing non-movable type.

In release mode debug info is not included by default.
If you still need it, define `VAR_THROW_RUNTIME_DEBUG_INFO`
before including var_stuff.h

## Dependencies

Standard library for c++11

## Variant compile-time checks

If you want to have checks only for:
1. Move constructor -> use `var_stuff::mcheck(VALUE);`
2. Copy constructor -> use `var_stuff::ccheck(VALUE);`
3. None -> use `var_stuff::nocheck(VALUE);`

## Visit

All operator()'s must have the same return type.
If you want to return something from var_stuff::visit, the return type must have either copy constructor or move constructor.
You can also return void.

## Compilation

Compiles with g++ and clang++, MSVC is not tested.
