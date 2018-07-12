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

#include <string>
#include <string_view>

#include "../Type.hpp"
#include "TokenKind.hpp"

namespace meteor::cc
{
	class Token
	{
	public:
		explicit Token(TokenKind kind, std::string text, std::size_t line)
			: m_kind(kind)
			, m_text(std::move(text))
			, m_line(line)
			, m_integer()
			, m_string()
		{
		}

		explicit Token(TokenKind kind, std::string text, std::size_t line, Word integer)
			: m_kind(kind)
			, m_text(std::move(text))
			, m_line(line)
			, m_integer(integer)
			, m_string()
		{
		}

		explicit Token(TokenKind kind, std::string text, std::size_t line, std::string string)
			: m_kind(kind)
			, m_text(std::move(text))
			, m_line(line)
			, m_integer()
			, m_string(std::move(string))
		{
		}

		// Uncopyable, movable.
		Token(const Token&) =delete;
		Token(Token&&) =default;

		Token& operator=(const Token&) =delete;
		Token& operator=(Token&&) =default;

		~Token() =default;

		[[nodiscard]]
		TokenKind kind() const noexcept
		{
			return m_kind;
		}

		[[nodiscard]]
		std::string_view text() const noexcept
		{
			return m_text;
		}

		[[nodiscard]]
		std::size_t line() const noexcept
		{
			return m_line;
		}

		[[nodiscard]]
		Word integer() const noexcept
		{
			return m_integer;
		}

		[[nodiscard]]
		std::string_view string() const noexcept
		{
			return m_string;
		}

	private:
		TokenKind   m_kind;
		std::string m_text;
		std::size_t m_line;
		Word        m_integer;
		std::string m_string;
	};
}
