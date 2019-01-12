/*
 *             Copyright Andrey Semashev 2018.
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          https://www.boost.org/LICENSE_1_0.txt)
 */
/*!
 * \file   uncaught_exceptions.hpp
 * \author Andrey Semashev
 * \date   2018-11-10
 *
 * \brief  This header provides an `uncaught_exception` function implementation, which was introduced in C++17.
 *
 * The code in this file is based on the implementation by Evgeny Panasyuk:
 *
 * https://github.com/panaseleus/stack_unwinding/blob/master/boost/exception/uncaught_exception_count.hpp
 */

#ifndef BOOST_CORE_UNCAUGHT_EXCEPTIONS_HPP_INCLUDED_
#define BOOST_CORE_UNCAUGHT_EXCEPTIONS_HPP_INCLUDED_

#include <exception>
#include <boost/config.hpp>

#if defined(BOOST_HAS_PRAGMA_ONCE)
#pragma once
#endif

namespace boost {

namespace core {

namespace detail {

// cxxabi.h availability macro
#if defined(__has_include) && (!defined(BOOST_GCC) || (__GNUC__ + 0) >= 5)
#   if __has_include(<cxxabi.h>)
#       define BOOST_CORE_HAS_CXXABI_H
#   endif
#elif defined(__GLIBCXX__) || defined(__GLIBCPP__)
#   define BOOST_CORE_HAS_CXXABI_H
#endif

#if defined(__cpp_lib_uncaught_exceptions) && __cpp_lib_uncaught_exceptions >= 201411
#define BOOST_CORE_HAS_UNCAUGHT_EXCEPTIONS
#elif defined(BOOST_CORE_HAS_CXXABI_H)
// MinGW GCC 4.4 seem to not work the same way the newer GCC versions do. As a result, __cxa_get_globals based implementation will always return 0.
// Just disable it for now and fall back to std::uncaught_exception().
#if !defined(__MINGW32__) || (defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 5)))
// Only GCC 4.7 declares __cxa_get_globals() in cxxabi.h, older compilers do not expose this function but it's there
#define BOOST_CORE_HAS_CXA_GET_GLOBALS
extern "C" void* __cxa_get_globals();
#endif
#elif defined(_MSC_VER)
#if _MSC_VER >= 1900
// Visual Studio 14 supports N4152 std::uncaught_exceptions()
#define BOOST_CORE_HAS_UNCAUGHT_EXCEPTIONS
#elif _MSC_VER >= 1400
#define BOOST_CORE_HAS_GETPTD
extern "C" void* _getptd();
#endif
#endif

} // namespace detail

#if !defined(BOOST_CORE_HAS_UNCAUGHT_EXCEPTIONS) && !defined(BOOST_CORE_HAS_CXA_GET_GLOBALS) && !defined(BOOST_CORE_HAS_GETPTD)
//! This macro is defined when `uncaught_exceptions` is not guaranteed to return values greater than 1 if multiple exceptions are pending
#define BOOST_CORE_UNCAUGHT_EXCEPTIONS_EMULATED
#endif

//! Returns the number of currently pending exceptions
inline unsigned int uncaught_exceptions() BOOST_NOEXCEPT
{
#if defined(BOOST_CORE_HAS_UNCAUGHT_EXCEPTIONS)
    // C++17 implementation
    return static_cast< unsigned int >(std::uncaught_exceptions());
#elif defined(BOOST_CORE_HAS_CXA_GET_GLOBALS)
    // Tested on {clang 3.2,GCC 3.5.6,GCC 4.1.2,GCC 4.4.6,GCC 4.4.7}x{x32,x64}
    return *(reinterpret_cast< const unsigned int* >(static_cast< const char* >(boost::core::detail::__cxa_get_globals()) + sizeof(void*))); // __cxa_eh_globals::uncaughtExceptions, x32 offset - 0x4, x64 - 0x8
#elif defined(BOOST_CORE_HAS_GETPTD)
    // MSVC specific. Tested on {MSVC2005SP1,MSVC2008SP1,MSVC2010SP1,MSVC2012}x{x32,x64}.
    return *(reinterpret_cast< const unsigned int* >(static_cast< const char* >(boost::core::detail::_getptd()) + (sizeof(void*) == 8 ? 0x100 : 0x90))); // _tiddata::_ProcessingThrow, x32 offset - 0x90, x64 - 0x100
#else
    // Portable C++03 implementation. Does not allow to detect multiple nested exceptions.
    return static_cast< unsigned int >(std::uncaught_exception());
#endif
}

#undef BOOST_CORE_HAS_CXXABI_H
#undef BOOST_CORE_HAS_CXA_GET_GLOBALS
#undef BOOST_CORE_HAS_UNCAUGHT_EXCEPTIONS
#undef BOOST_CORE_HAS_GETPTD

} // namespace core

} // namespace boost

#endif // BOOST_CORE_UNCAUGHT_EXCEPTIONS_HPP_INCLUDED_
