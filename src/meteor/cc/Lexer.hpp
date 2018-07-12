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
#include <boost/container/flat_map.hpp>
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
			while (!eof())
			{
				// \s
				constexpr auto isWhitespace = [](char c) noexcept
				{
					return c == u8'\t' || c == u8'\r' || c == u8'\n' || c == u8'\v' || c == u8'\f' || c == u8' ';
				};

				// [0-9]
				constexpr auto isDigit = [](char c) noexcept
				{
					return u8'0' <= c && c <= u8'9';
				};

				// // [A-Z_a-z]
				// constexpr auto isIdentifierStart = [](char c) noexcept
				// {
				// 	return (u8'A' <= c && c <= u8'Z') || (u8'a' <= c && c <= u8'z') || (c == u8'_');
				// };

				// // [0-9A-Z_a-z]
				// constexpr auto isIdentifierContinuation = [](char c) noexcept
				// {
				// 	return isIdentifierStart(c) || isDigit(c);
				// };

				// --- ignored ---

				// space:
				//     \s+
				if (isWhitespace(peek(0)))
				{
					while (isWhitespace(peek(0)))
					{
						consume();
					}

					continue;
				}

				// line-comment:
				//     '//' .*
				if (skipOver(u8"//"))
				{
					while (peek(0) != u8'\n')
					{
						consume();
					}

					continue;
				}

				// block-comment:
				//     '/*' .* '*/'
				if (skipOver(u8"/*"))
				{
					while (!skipOver(u8"*/"))
					{
						if (eof())
						{
							reportError(u8"unterminated block comment `/* ... */'.");
						}

						consume();
					}
				}

				// --- token ---

				// decimal-integer-literal:
				//     [1-9][0-9]*
				if (isDigit(peek(0)))
				{
					std::string text;

					while (isDigit(peek(0)))
					{
						text += consume();
					}

					try
					{
						if (const auto value = std::stoull(text); value <= 0xffff)
						{
							return formToken(TokenKind::integerLiteral, std::move(text), static_cast<Word>(value));
						}
					}
					catch (const std::out_of_range&)
					{
					}

					reportError(boost::format(u8"too large integer literal `%1%'.") % text);
				}

				// punctuator:
				//     '+' | '-' | ...
				static const auto punctuators = boost::container::flat_map<std::string, TokenKind, std::greater<>>
				{
#define METEOR_CC_TOKEN_PUNCTUATOR(name, text) { u8 ## text, TokenKind::name },
#include "Token.def.hpp"
				};

				for (const auto& [text, kind] : punctuators)
				{
					if (skipOver(text))
					{
						return formToken(kind, text);
					}
				}

				// error
				reportError(boost::format(u8"unexpected character `0x%1$02X'.") % int {peek(0)});
			}

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

		[[nodiscard]]
		bool startsWith(std::string_view s) const noexcept
		{
			return m_code.compare(m_pos, s.size(), s) == 0;
		}

		bool skipOver(std::string_view s) noexcept
		{
			if (!startsWith(s)) return false;

			for (size_t i = 0; i < s.size(); i++)
			{
				consume();
			}

			return true;
		}

		template <typename... Args>
		[[nodiscard]]
		std::unique_ptr<Token> formToken(TokenKind kind, std::string text, Args&&... args) const
		{
			return std::make_unique<Token>(kind, std::move(text), m_line, std::forward<Args>(args)...);
		}

		template <typename Message>
		[[noreturn]]
		void reportError(Message&& message)
		{
			throw std::runtime_error { (boost::format(u8"%1%(%2%): %3%") % m_name % m_line % std::forward<Message>(message)).str() };
		}

		std::string m_name;
		std::string m_code;
		std::size_t m_pos;
		std::size_t m_line;
	};
}
