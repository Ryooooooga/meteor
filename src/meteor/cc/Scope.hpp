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
#include <memory>
#include <unordered_map>

namespace meteor::cc
{
	class DeclarationNode;

	class Scope
	{
	public:
		explicit Scope(const std::shared_ptr<Scope>& parentScope)
			: m_parent(parentScope)
			, m_table()
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
			return m_parent;
		}

		[[nodiscard]]
		const DeclarationNode* find(std::string_view name) const
		{
			if (const auto it = m_table.find(name); it != std::end(m_table))
			{
				return &it->second.get();
			}
			else if (m_parent)
			{
				return m_parent->find(name);
			}

			return nullptr;
		}

		[[nodiscard]]
		bool tryRegister(std::string_view name, const DeclarationNode& node)
		{
			return m_table.try_emplace(name, node).second;
		}

	private:
		std::shared_ptr<Scope> m_parent;
		std::unordered_map<std::string_view, std::reference_wrapper<const DeclarationNode>> m_table;
	};
}
