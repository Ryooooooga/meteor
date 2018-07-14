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
#include "meteor/cc/Printer.hpp"
#include "meteor/cc/Parser.hpp"
#include "meteor/cc/SymbolAnalyzer.hpp"

#include "meteor/runtime/Processor.hpp"

#include <iostream>

int main()
{
	try
	{
		constexpr char source[] = u8R"(
			int x;
			int y;
			int z;
			int w;

			int f(int a, int b) {
				w = a - b;
			}

			int main(void) {
				f(5, 8);
			}
		)";

		auto parser = meteor::cc::Parser { "test.c", source };
		auto ast = parser.parse();
		auto compiler = meteor::cc::Compiler {};

		meteor::cc::SymbolAnalyzer {}.resolve(*ast);

		auto program = compiler.compile(*ast);

		meteor::cc::Printer {std::cout}.print(*ast);

		for (meteor::Word addr = 0; addr < program.size(); addr++)
		{
			std::cout << boost::format(u8"%1$04X: %2$04X") % addr % program[addr] << std::endl;
		}

		auto memory = std::make_shared<meteor::runtime::Memory>(program);
		auto processor = meteor::runtime::Processor(memory);

		std::size_t steps = 0;

		while (steps++ < 100 && processor.step())
		{
		}

		std::cout << "steps: " << steps << std::endl;
		memory->dump(std::cout, 0x0000, 0x0030);
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
