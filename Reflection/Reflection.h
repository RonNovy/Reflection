/*
	Written by Ron S. Novy

	If you like this software please consider donating a buck or two through PayPal.
	Email: ron (dot) novy69 (at) hotmail (dot) com

	Copyright (C) 2018 Ron S. Novy

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files(the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions :

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.
*/
#pragma once

// STL headers.
#include <array>
#include <vector>


// Boost headers.
#include <boost/preprocessor/variadic.hpp>
#include <boost/preprocessor/seq/for_each_i.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <boost/type_traits/add_const.hpp>
#include <boost/bind.hpp>


// ********************************
// **** namespace recurse
namespace recurse
{
	// ********************************
	// **** range_t - A simple range type for use with recurse::for_each<>
	template <typename _Type, _Type _Start, _Type _Finish>
	struct range_t
	{
		typedef _Type value_type;
		typedef range_t type;
		typedef range_t *pointer;

		typedef std::integral_constant<_Type, _Start>		begin;
		typedef std::integral_constant<_Type, _Finish>		end;
		typedef std::integral_constant<_Type, _Start + 1>	next_val;
		typedef range_t<_Type, _Start + 1, _Finish>			next;
		typedef range_t<_Type, _Start - 1, _Finish>			prior;

		static range_t * cast() { return nullptr; };
		static _Type const value = _Start;
		static bool const is_done = std::is_same<next_val, end>::value;
	};
	// ********************************


	// ********************************
	// **** Auxiliary functions for recurse.
	namespace aux
	{
		// Actual recursive part of recurse::for_each<>
		template< bool done = true >
		struct static_recurse_impl
		{
			template <typename _Range, typename _Func, typename... _Args>
			static int execute(_Range*, _Func, _Args... args) { return 0; };
		};

		// Recurse, accumulate and return.
		template<>
		struct static_recurse_impl<false>
		{
			template <typename _Range, typename _Func, typename... _Args>
			static int execute(_Range*, _Func func, _Args... args)
			{
				typedef typename _Range::type	type;
				typedef typename _Range::next	next;
				int ret = func(type::cast(), args...);
				ret += static_recurse_impl< _Range::is_done >::execute(next::cast(), func, args...);
				return ret;
			};
		};
	}
	// **** End of namespace aux
	// ********************************


	// ********************************
	// **** for_each - Recursively iterate at compile time.
	// This function is like boost::for_each<> except with a variable number of arguments an accumulated return value.
	template <typename _Range, typename _Func, typename... _Args>
	inline int for_each(_Range*, _Func func, _Args... args)
	{
		typedef typename _Range::type	type;
		typedef typename _Range::next	next;
		return aux::static_recurse_impl< _Range::is_done >::execute(type::cast(), func, args...);
	};
	// ********************************
}
// **** End of namespace recurse
// ********************************


// ********************************
// ********************************
// **** Macros for manipulating preprocessor macro arguments.
#define _remove_paran(...) __VA_ARGS__
#define _get_first(a, b) _remove_paran a
#define _get_second(a, b) b
#define _define_member(a, b) _remove_paran a b


# /* BOOST_PP_STRINGIZE2 - Has a preprocessor depth of 2 so that it processes any macros that may be used as arguments */
# /*  Note: This is good enough for this Reflection.h code, ymmv */
# if !defined(BOOST_PP_STRINGIZE2)
#   if BOOST_PP_CONFIG_FLAGS() & BOOST_PP_CONFIG_MSVC()
#      define BOOST_PP_STRINGIZE2(...)		BOOST_PP_STRINGIZE2_A((__VA_ARGS__))
#      define BOOST_PP_STRINGIZE2_A(arg)	BOOST_PP_STRINGIZE2_B arg
#      define BOOST_PP_STRINGIZE2_B(...)	BOOST_PP_STRINGIZE2_C((__VA_ARGS__))
#      define BOOST_PP_STRINGIZE2_C(arg)	BOOST_PP_STRINGIZE2_I arg
#   elif BOOST_PP_CONFIG_FLAGS() & BOOST_PP_CONFIG_MWCC()
#      define BOOST_PP_STRINGIZE2(text)		BOOST_PP_STRINGIZE2_OOA((text))
#      define BOOST_PP_STRINGIZE2_OOA(par)	BOOST_PP_STRINGIZE2_B ## par
#      define BOOST_PP_STRINGIZE2_B(text)	BOOST_PP_STRINGIZE2_OOC((text))
#      define BOOST_PP_STRINGIZE2_OOC(par)	BOOST_PP_STRINGIZE2_I ## par
#   else
#      define BOOST_PP_STRINGIZE2(text)		BOOST_PP_STRINGIZE2_I(text)
#   endif
#   define BOOST_PP_STRINGIZE2_I(...) #__VA_ARGS__
# endif
// **** End macros for manipulating preprocessor macro arguments.
// ********************************


// ********************************
// **** This macro is expanded once for each reflected element inside a class.
#define __REFLECT_EACH(r, data, i, x)																	\
	_define_member x ;																					\
	template<class Self> struct field_data<i, Self> {													\
		Self & self;																					\
		field_data(Self & self) : self(self) {};														\
		typedef _get_first x typeof;																	\
		typename reflection::make_const<Self, typeof>::type & get() { return self._get_second x; };		\
		typename boost::add_const<typeof>::type & get() const { return self._get_second x; };			\
		auto type_name() const { return BOOST_PP_STRINGIZE2(_get_first x); };							\
		auto var_name() const { return BOOST_PP_STRINGIZE(_get_second x); };							\
	};
// ********************************


// ********************************
// **** decl_reflections is used to declare a list of the reflectable elements inside a class.
//  [ namespace some_namespace { ]
//    decl_reflections(
//        StructName,
//        ((type), variable_name1) 
//        ((type), variable_name2)
//        ...
//    );
//  [}]
//  decl_reflectable([some_namespace::]StructName);
//
#define decl_reflections(sname, ...)															\
	struct sname {																				\
		auto struct_name(void) const { return BOOST_PP_STRINGIZE(sname); };						\
		static const int fields_n = BOOST_PP_VARIADIC_SIZE(__VA_ARGS__);						\
		friend struct reflection::reflector;													\
		template<int N, class Self> struct field_data {};										\
		BOOST_PP_SEQ_FOR_EACH_I(__REFLECT_EACH, data, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))	\
	}
// ********************************
// ********************************


// ********************************
// ********************************
// **** Macros for manipulating preprocessor macro arguments passed to decl_reflections_alias
#define _remove_paran_a(...) __VA_ARGS__
#define _get_first_a(a, b, c, d) _remove_paran a
#define _get_second_a(a, b, c, d) b
#define _get_third_a(a, b, c, d) c
#define _get_fourth_a(a, b, c, d) d
#define _define_member_a(a, b, c, d) _remove_paran a c


// ********************************
// **** This macro is expanded once for each reflected element inside a class.
#define __REFLECT_EACH_ALIAS(r, data, i, x)																\
	_define_member_a x ;																				\
	template<class Self> struct field_data<i, Self> {													\
		Self & self;																					\
		field_data(Self & self) : self(self) {};														\
		typedef _get_first_a x typeof;																	\
		typename reflection::make_const<Self, typeof>::type & get() { return self._get_third_a x; };	\
		typename boost::add_const<typeof>::type & get() const { return self._get_third_a x; };			\
		auto type_name() const { return _get_second_a x ; };											\
		auto var_name() const { return _get_fourth_a x ; };												\
	};
// ********************************


// ********************************
// **** decl_reflections_alias is used to declare a list of the reflectable elements inside a class with manually defined names.
//  [ namespace some_namespace { ]
//    decl_reflections_alias(
//        StructName, "Alias:name of structure"
//        ((type1), "AliasNameOfType1", variable_name1, "Alias name of variable 1"), 
//        ((type2), "Alias:Name:Of:Type:2", variable_name2, "Alias:name:of:variable:2"),
//        ...
//    );
//  [}]
//  decl_reflectable([some_namespace::]StructName);
//
#define decl_reflections_alias(sname, aname, ...)													\
	struct sname {																					\
		auto struct_name(void) const { return aname; };												\
		static const int fields_n = BOOST_PP_VARIADIC_SIZE(__VA_ARGS__);							\
		friend struct reflection::reflector;														\
		template<int N, class Self> struct field_data {};											\
		BOOST_PP_SEQ_FOR_EACH_I(__REFLECT_EACH_ALIAS, data, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))	\
	}
// ********************************


// ********************************
// **** is_reflectable is a global template that is used to determine if a class was declared as reflectable.
// **** Example: is_reflectable<MyType>::value will be 'true' if 'MyType' was declared reflectable with decl_reflectable(MyType).
namespace reflection { template <typename _Type> struct is_reflectable { static const bool value = false; }; }
// ********************************


// ********************************
// **** decl_reflectable is used to declare a class as being reflectable.
template <typename ...>
struct decl_reflectable__must_be_called_in_the_global_namespace;

#define decl_reflectable(_T)												\
template <>																	\
struct decl_reflectable__must_be_called_in_the_global_namespace<>;			\
namespace reflection { template <> struct is_reflectable<_T> { static const bool value = true; }; }
// ********************************


// ********************************
// **** namespace reflection
namespace reflection
{
	// ********************************
	// **** A helper meta-function for adding const to a type.
	template<class M, class T>
	struct make_const
	{
		typedef T type;
	};

	template<class M, class T>
	struct make_const<const M, T>
	{
		typedef typename boost::add_const<T>::type type;
	};
	// ********************************


	// ********************************
	// **** Reflector class to get field/element data from a class.
	struct reflector
	{
		// Get field_data at index
		template<int _Index, class _Type>
		static typename _Type::template field_data<_Index, _Type> get_field_data(_Type & x)
		{
			return typename _Type::template field_data<_Index, _Type>(x);
		}

		// Get the number of fields
		template<class _Type>
		struct fields
		{
			static const int n = _Type::fields_n;
		};
	};
	// ********************************


	// ********************************
	// **** namespace aux
	namespace aux
	{
		// ********************************
		// **** field_visitor - Handles an individual element/field.
		struct field_visitor
		{
			template <class _Range, class _Type, class _Func, typename... _Args>
			int operator()(_Range *, _Type & val, _Func func, _Args... args)
			{
				return func(reflector::get_field_data<_Range::value>(val), args...);
			};
		};
		// ********************************
	}
	// **** End of namespace aux
	// ********************************


	// ********************************
	// **** The primary loop unroller to visit each element/field.
	template<class _Class, class _Func, typename... _Args>
	int visit_each(_Class & Class, _Func func, _Args... args)
	{
		typedef recurse::range_t<int, 0, reflector::fields<_Class>::n> range;
		return recurse::for_each<range>(range::cast(), aux::field_visitor(), Class, func, args...);
	};
	// ********************************
}
// **** End of namespace reflection
// ********************************


// End of file.
