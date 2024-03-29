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
#include "../util/Passkey.hpp"

namespace meteor::cc
{
	class SymbolAnalyzer;
	class Compiler;
	class ITypeInfo;

	class Symbol
	{
	public:
		explicit Symbol(std::string_view name, const std::shared_ptr<ITypeInfo>& typeInfo)
			: m_name(name)
			, m_typeInfo(typeInfo)
			, m_isGlobal()
			, m_address(0xffff)
		{
			assert(m_typeInfo);
		}

		// Uncopyable, unmovable.
		Symbol(const Symbol&) =delete;
		Symbol(Symbol&&) =delete;

		Symbol& operator=(const Symbol&) =delete;
		Symbol& operator=(Symbol&&) =delete;

		~Symbol() =default;

		[[nodiscard]]
		std::string_view name() const noexcept
		{
			return m_name;
		}

		[[nodiscard]]
		std::shared_ptr<ITypeInfo> typeInfo() const noexcept
		{
			return m_typeInfo;
		}

		[[nodiscard]]
		Word address() const noexcept
		{
			return m_address;
		}

		[[nodiscard]]
		bool isGlobal() const noexcept
		{
			return m_isGlobal;
		}

		void address(Passkey<Compiler>, bool isGlobal, Word address)
		{
			m_isGlobal = isGlobal;
			m_address = address;
		}

	private:
		std::string m_name;
		std::shared_ptr<ITypeInfo> m_typeInfo;
		bool m_isGlobal;
		Word m_address;
	};
}
