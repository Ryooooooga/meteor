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

#include <deque>

#include "Lexer.hpp"

namespace meteor::cc
{
	class TokenStream
	{
	public:
		explicit TokenStream(Lexer&& lexer)
			: m_lexer(std::move(lexer))
			, m_queue()
		{
		}

		// Uncopyable, movable.
		TokenStream(const TokenStream&) =delete;
		TokenStream(TokenStream&&) =default;

		TokenStream& operator=(const TokenStream&) =delete;
		TokenStream& operator=(TokenStream&&) =default;

		~TokenStream() =default;

		[[nodiscard]]
		std::string_view name() const noexcept
		{
			return m_lexer.name();
		}

		[[nodiscard]]
		std::string_view code() const noexcept
		{
			return m_lexer.code();
		}

		void fill(std::size_t size)
		{
			while (m_queue.size() < size)
			{
				m_queue.emplace_back(m_lexer.read());
			}
		}

		[[nodiscard]]
		std::shared_ptr<Token> peek(std::size_t offset)
		{
			fill(offset + 1);
			return m_queue[offset];
		}

		std::shared_ptr<Token> consume()
		{
			auto t = peek(0);
			m_queue.pop_front();

			return t;
		}

	private:
		Lexer m_lexer;
		std::deque<std::shared_ptr<Token>> m_queue;
	};
}
