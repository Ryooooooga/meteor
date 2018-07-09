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

#include <array>
#include <memory>

#include "Memory.hpp"
#include "../Register.hpp"

namespace meteor::runtime
{
	class Processor
	{
	public:
		explicit Processor(std::shared_ptr<Memory> memory)
			: m_memory(std::move(memory))
			, m_registers()
		{
			assert(m_memory);
		}

		// Uncopyable, movable.
		Processor(const Processor&) =delete;
		Processor(Processor&&) =default;

		Processor& operator=(const Processor&) =delete;
		Processor& operator=(Processor&&) =default;

		~Processor() =default;

		[[nodiscard]]
		std::shared_ptr<Memory> memory() const noexcept
		{
			return m_memory;
		}

		void dumpRegisters(std::ostream& stream)
		{
			for (Word i = 0; i < numRegisters; i++)
			{
				stream << boost::format("%1$3s = #%2$04X = %2$d") % static_cast<Register>(i) % m_registers[i] << std::endl;
			}
		}

	private:
		std::shared_ptr<Memory> m_memory;

		std::array<Word, numRegisters> m_registers;
	};
}
