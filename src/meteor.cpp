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

#include "meteor/runtime/Processor.hpp"

#include <iostream>

int main()
{
	try
	{
		const auto program = std::vector<meteor::Word>
		{
			0x3600,         //     XOR  GR0, GR0
			0x1410,         //     LD   GR1, GR0
			0x1220, 0x0001, //     LAD  GR2, #0001, GR0
			0x1230, 0x000a, //     LAD  GR3, #000a, GR0
			0x1441,         // .L  LD   GR4, GR1
			0x1412,         //     LD   GR1, GR2
			0x2424,         //     ADDA GR2, GR4
			0x2130, 0x0001, //     SUBA GR3, #0001, GR0
			0x7001, 0x0000, //     PUSH #0000, GR1
			0x6500, 0x0006, //     JPL  .L, GR0
			0x7170,         //     POP  GR7
			0x8100,         //     RET
		};

		const auto memory = std::make_shared<meteor::runtime::Memory>(program);
		const auto processor = std::make_unique<meteor::runtime::Processor>(memory);

		std::size_t steps = 0;

		while (steps++ < 100 && processor->step())
		{
			processor->memory()->dump(std::cout, 0x0000, 0x0020);
			processor->memory()->dump(std::cout, 0xfff0, 0x10000);
			processor->dumpRegisters(std::cout);
		}

		processor->memory()->dump(std::cout, 0x0000, 0x0020);
		processor->memory()->dump(std::cout, 0xfff0, 0x10000);
		processor->dumpRegisters(std::cout);

		std::cout << "steps: " << steps << std::endl;
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
