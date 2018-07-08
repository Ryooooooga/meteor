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

#pragma once

#include <cstddef>
#include <ostream>

#include "Type.hpp"

namespace meteor
{
	constexpr std::size_t numRegisters = 11;

	enum class Register: Word
	{
		general0 = 0,
		general1 = 1,
		general2 = 2,
		general3 = 3,
		general4 = 4,
		general5 = 5,
		general6 = 6,
		general7 = 7,
		stackPointer = 8,
		programCounter = 9,
		flags = 10,
	};

	[[nodiscard]]
	constexpr const char* toString(Register reg) noexcept
	{
		constexpr const char* names[numRegisters] =
		{
			"GR0", "GR1", "GR2", "GR3",
			"GR4", "GR5", "GR6", "GR7",
			"SP", "PC", "FR",
		};

		return names[static_cast<Word>(reg)];
	}

	inline std::ostream& operator <<(std::ostream& stream, Register reg)
	{
		return  stream << toString(reg);
	}
}
