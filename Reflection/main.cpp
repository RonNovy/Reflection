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
// Reflection - as simple as possible.
#include <iostream>
#include <vector>
#include <array>
#include <string>

// Project includes.
#include "Reflection.h"


// ********************************
// **** namespace somewhere
namespace somewhere
{
	decl_reflections_alias
	( Pet, "HoomansPet",
		((std::string), "std:stringy", name, "petsname"),
		((std::string), "std:stringy_t", species, "speciesname")
	);

	decl_reflections
	( Person,
		((std::string), name),
		((int), age),
		((Pet), pet)
	);
	// ********************************
}

decl_reflectable(somewhere::Pet);
decl_reflectable(somewhere::Person);
namespace somewhere
{

	// ********************************
	// **** This macro lets the _print_fields function grab the name of a variable automatically.
	#define print_fields(x) print_fields_alias(x, BOOST_PP_STRINGIZE(x))


	// ********************************
	template <typename _Type>
	int print_info_class(_Type &x, const char *name, const char *tname, int depth)
	{
		std::cout << std::string(depth, '\t')
			<< tname << "\t" << name << ";\t// ???\n";
		return 1;
	};

	template <>
	int print_info_class<std::string>(std::string &x, const char *name, const char *tname, int depth)
	{
		std::cout << std::string(depth, '\t')
			<< tname << "\t" << name << "= \"" << x << "\";\t// std::string\n";
		return 1;
	};
	// ********************************


	// ********************************
	// ********************************
	template <bool _Reflectable, bool _Fundamental>
	struct print_info
	{
		template <typename _Type, class _Func>
		static int func(_Type &x, _Func func, int depth)
		{
			std::cout << std::string(depth, '\t')
				<< x.type_name() << "\t" << x.var_name() << "= " << +x.get() << ";\t// is_fundamental\n";
			return 1;
		};
	};

	// Not fundamental and not reflectable.  Try and break it down further to supported STL types.
	template <>
	struct print_info<false, false>
	{
		template <typename _Type, class _Func>
		static int func(_Type &val, _Func func, int depth)
		{
			return print_info_class<typename _Type::typeof>(val.get(), val.var_name(), val.type_name(), depth);
		};
	};

	// Not fundamental but is reflectable.
	template <>
	struct print_info<true, false>
	{
		template <typename _Type, class _Func>
		static int func(_Type &val, _Func func, int depth)
		{
			return print_fields_alias(val.get(), val.var_name(), depth) + 1;
		};
	};
	// ********************************


	// ********************************
	// **** This gets called second.
	struct print_visitor
	{
		template<class _Type, class _Func>
		int operator()(_Type &&val, _Func func, int depth)
		{
			return print_info<reflection::is_reflectable<typename _Type::typeof>::value, std::is_fundamental<typename _Type::typeof>::value>::func(val, func, depth);
		};
	};
	// ********************************


	// ********************************
	// **** This gets called first.
	template<class _Class>
	int print_fields_alias(_Class & _class, const char *name, int depth = 0)
	{
		static_assert(reflection::is_reflectable<_Class>::value, "The type passed in must be defined as reflectable!");

		std::cout << std::string(depth, '\t')
			<< _class.struct_name() << "\t" << name << ";\t// is_reflectable\n";

		std::cout << std::string(depth, '\t') << "{\n";
		int ret = reflection::visit_each(_class, print_visitor(), print_visitor(), depth + 1);
		std::cout << std::string(depth, '\t') << "};\n";
		return ret;
	};
	// ********************************
}
// **** End namespace somewhere
// ********************************


// ********************************
int main()
{
	std::cout << std::boolalpha << "Testing C++ reflection.\n";


	somewhere::Person aatp{ "Jerry", 81, {"Ruff", "Dog"} };
	std::cout << "number of fields printed is " << somewhere::print_fields(aatp) << "\n\n";

	std::cout << "is_reflectable<int>::value        =" << reflection::is_reflectable<int>::value << "\n";
	std::cout << "is_reflectable<double>::value     =" << reflection::is_reflectable<double>::value << "\n";
	std::cout << "is_reflectable<somewhere::Person>::value=" << reflection::is_reflectable<somewhere::Person>::value << "\n\n";

	std::cout << "is_reflectable<std::string>::value        =" << reflection::is_reflectable<std::string>::value << "\n";
	std::cout << "is_reflectable<std::vector<double>>::value=" << reflection::is_reflectable<std::vector<double>>::value << "\n";
	std::cout << "is_reflectable<std::vector<somewhere::Person>>::value=" << reflection::is_reflectable<std::vector<somewhere::Person>>::value << "\n";

	std::cout << "Done.\n";
	return 0;
}

