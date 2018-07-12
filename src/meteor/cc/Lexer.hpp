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

#include <memory>
#include <boost/format.hpp>

#include "Token.hpp"

namespace meteor::cc
{
	class Lexer
	{
	public:
		explicit Lexer(std::string name, std::string code)
			: m_name(std::move(name))
			, m_code(std::move(code))
			, m_pos(0)
			, m_line(1)
		{
		}

		// Uncopyable, movable.
		Lexer(const Lexer&) =delete;
		Lexer(Lexer&&) =default;

		Lexer& operator=(const Lexer&) =delete;
		Lexer& operator=(Lexer&&) =default;

		~Lexer() =default;

		[[nodiscard]]
		std::string_view name() const noexcept
		{
			return m_name;
		}

		[[nodiscard]]
		std::string_view code() const noexcept
		{
			return m_code;
		}

		[[nodiscard]]
		std::unique_ptr<Token> read()
		{
			// [EOF]
			return formToken(TokenKind::endOfFile, u8"[EOF]");
		}

	private:
		[[nodiscard]]
		bool eof() const noexcept
		{
			return m_pos >= m_code.size();
		}

		[[nodiscard]]
		char peek(std::size_t offset) const noexcept
		{
			return m_pos + offset < m_code.size()
				? m_code[m_pos + offset] : u8'\0';
		}

		char consume() noexcept
		{
			if (eof()) return u8'\0';

			const auto c = peek(0);

			m_pos++;
			if (c == u8'\n') m_line++;

			return c;
		}

		template <typename... Args>
		[[nodiscard]]
		std::unique_ptr<Token> formToken(TokenKind kind, std::string text, Args&&... args) const
		{
			return std::make_unique<Token>(kind, std::move(text), m_line, std::forward<Args>(args)...);
		}

		std::string m_name;
		std::string m_code;
		std::size_t m_pos;
		std::size_t m_line;
	};
}
