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

#include <cassert>
#include <iomanip>
#include <ostream>
#include <vector>

#include "../Type.hpp"

namespace meteor::runtime
{
	class Memory
	{
	public:
		explicit Memory()
			: m_data(dataSize, 0)
		{
		}

		explicit Memory(std::vector<Word> data)
			: m_data(std::move(data))
		{
			m_data.resize(dataSize);
		}

		// Uncopyable, movable.
		Memory(const Memory&) =delete;
		Memory(Memory&&) =default;

		Memory& operator=(const Memory&) =delete;
		Memory& operator=(Memory&&) =default;

		~Memory() =default;

		[[nodiscard]]
		std::size_t size() const noexcept
		{
			return m_data.size();
		}

		[[nodiscard]]
		Word read(std::size_t position) const
		{
			assert(position < size());

			return m_data[position];
		}

		void write(std::size_t position, Word value)
		{
			assert(position < size());

			m_data[position] = value;
		}

		void dump(std::ostream& stream)
		{
			dump(stream, 0, size());
		}

		void dump(std::ostream& stream, std::size_t begin, std::size_t end)
		{
			assert(begin <= size());
			assert(end <= size());

			constexpr auto width = std::size_t {16};

			// Save the stream states.
			const auto flags = stream.flags(std::ios::hex | std::ios::uppercase);
			const auto fill = stream.fill('0');
			const auto fillWidth = stream.width();

			// Restore the stream states.
			const auto restore = [&]()
			{
				stream.flags(flags);
				stream.fill(fill);
				stream.width(fillWidth);
			};

			try
			{
				// Output header.
				stream << "    |";

				for (std::size_t row = 0; row < width; row++)
				{
					stream << " " << std::setw(4) << row;
				}

				stream << "\n";
				stream << "----+";

				for (std::size_t row = 0; row < width; row++)
				{
					stream << "-----";
				}

				// Output data.
				const auto rowOffset = begin % width;
				const auto firstColumn = begin / width;

				for (std::size_t i = begin; i < end; i++)
				{
					const auto row = i % width;
					const auto column = i / width;

					if (column == firstColumn && row == rowOffset)
					{
						stream << "\n";
						stream << std::setw(4) << column * width << "|";

						for (std::size_t i = 0; i < rowOffset; i++)
						{
							stream << " " << "    ";
						}
					}
					else if (row == 0)
					{
						stream << "\n";
						stream << std::setw(4) << column * width << "|";
					}

					stream << " " << std::setw(4) << read(i);
				}

				stream << std::endl;
			}
			catch (...)
			{
				restore();
				throw;
			}

			restore();
		}

	private:
		constexpr static std::size_t dataSize = 65536;

		std::vector<Word> m_data;
	};
}
