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

#include "Node.hpp"
#include "TokenStream.hpp"

namespace meteor::cc
{
	class Parser
	{
	public:
		explicit Parser(std::string_view name, std::string_view code)
			: m_stream(Lexer { name, code })
		{
		}

		// Uncopyable, movable.
		Parser(const Parser&) =delete;
		Parser(Parser&&) =default;

		Parser& operator=(const Parser&) =delete;
		Parser& operator=(Parser&&) =default;

		~Parser() =default;

		[[nodiscard]]
		std::unique_ptr<RootNode> parse()
		{
			// root
			return parseRoot();
		}

	private:
		// root:
		//     TODO
		[[nodiscard]]
		std::unique_ptr<RootNode> parseRoot()
		{
			return std::make_unique<RootNode>(m_stream.name());
		}

		TokenStream m_stream;
	};
}
