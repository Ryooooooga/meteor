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
#include <unordered_map>

#include "Symbol.hpp"

namespace meteor::cc
{
	class Scope
	{
	public:
		explicit Scope(const std::shared_ptr<Scope>& parentScope)
			: m_parentScope(parentScope)
		{
		}

		// Uncopyable, unmovable.
		Scope(const Scope&) =delete;
		Scope(Scope&&) =delete;

		Scope& operator=(const Scope&) =delete;
		Scope& operator=(Scope&&) =delete;

		~Scope() =default;

		[[nodiscard]]
		std::shared_ptr<Scope> parentScope() const noexcept
		{
			return m_parentScope;
		}

		[[nodiscard]]
		bool tryRegister(const std::shared_ptr<Symbol>& symbol)
		{
			return m_table.try_emplace(symbol->name(), symbol).second;
		}

		[[nodiscard]]
		std::shared_ptr<Symbol> find(std::string_view name, bool recursively) const
		{
			if (const auto it = m_table.find(name); it != std::end(m_table))
			{
				return it->second;
			}
			else if (recursively && m_parentScope)
			{
				return m_parentScope->find(name, recursively);
			}

			return nullptr;
		}

	private:
		std::shared_ptr<Scope> m_parentScope;
		std::unordered_map<std::string_view, std::shared_ptr<Symbol>> m_table;
	};
}
