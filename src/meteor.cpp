/*================================================================================
 * The MIT License
 *
 * Copyright (c) 2018 Ryooooooga
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
================================================================================*/

#include "meteor/cc/Compiler.hpp"
#include "meteor/cc/Parser.hpp"

#include <iostream>

int main()
{
	try
	{
		constexpr char source[] = u8R"(
			42;
			;
			3;
		)";

		auto parser = meteor::cc::Parser { "test.c", source };

		auto ast = parser.parse();
		meteor::cc::Printer {std::cout}.print(*ast);

		auto compiler = meteor::cc::Compiler {};
		auto program = compiler.compile(*ast);

		for (const auto word : program)
		{
			std::cout << boost::format(u8"%1$04X") % word << std::endl;
		}
	}
	catch (const std::exception& e)
	{
		std::cerr
			<< "*** caught exception ***" << std::endl
			<< "type: " << typeid(e).name() << std::endl
			<< "what: " << e.what() << std::endl;
	}
	catch (...)
	{
		std::cerr
			<< "*** caught unknown exception ***" << std::endl;
	}
}
