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

#include <algorithm>
#include <boost/format.hpp>

#include "../Type.hpp"
#include "../util/Passkey.hpp"

namespace meteor::cc
{
	class SymbolAnalyzer;

	enum class TypeCategory
	{
		function,
		integer,
	};

	class ITypeInfo
	{
	public:
		ITypeInfo() =default;

		// Uncopyable, unmovable.
		ITypeInfo(const ITypeInfo&) =delete;
		ITypeInfo(ITypeInfo&&) =delete;

		ITypeInfo& operator=(const ITypeInfo&) =delete;
		ITypeInfo& operator=(ITypeInfo&&) =delete;

		~ITypeInfo() =default;

		[[nodiscard]]
		virtual TypeCategory category() const =0;

		[[nodiscard]]
		virtual Word size() const =0;

		[[nodiscard]]
		virtual std::string name() const =0;

		[[nodiscard]]
		virtual bool equals(const ITypeInfo& type) const =0;
	};

	[[nodiscard]]
	inline bool operator ==(const ITypeInfo& a, const ITypeInfo& b)
	{
		return a.equals(b);
	}

	[[nodiscard]]
	inline bool operator !=(const ITypeInfo& a, const ITypeInfo& b)
	{
		return !(a == b);
	}

	class FunctionTypeInfo
		: public ITypeInfo
	{
	public:
		explicit FunctionTypeInfo(const std::shared_ptr<ITypeInfo>& returnType, std::vector<std::shared_ptr<ITypeInfo>>&& parameterTypes)
			: m_returnType(returnType)
			, m_parameterTypes(std::move(parameterTypes))
		{
			assert(m_returnType);
		}

		[[nodiscard]]
		TypeCategory category() const override
		{
			return TypeCategory::function;
		}

		[[nodiscard]]
		Word size() const override
		{
			return 0;
		}

		[[nodiscard]]
		std::string name() const override
		{
			std::string params;
			for (std::size_t i = 0; i < m_parameterTypes.size(); i++)
			{
				params += (i > 0) ? u8", " : u8"";
				params += m_parameterTypes[i]->name();
			}

			return (boost::format(u8"Func<%1%, (%2%)>") % m_returnType->name() % params).str();
		}

		[[nodiscard]]
		std::shared_ptr<ITypeInfo> returnType() const noexcept
		{
			return m_returnType;
		}

		[[nodiscard]]
		const std::vector<std::shared_ptr<ITypeInfo>>& parameterTypes() const noexcept
		{
			return m_parameterTypes;
		}

		[[nodiscard]]
		bool equals(const ITypeInfo& type) const override
		{
			if (const auto p = dynamic_cast<const FunctionTypeInfo*>(&type))
			{
				return *m_returnType == *p->m_returnType &&
					std::equal(
						std::begin(m_parameterTypes),
						std::end(m_parameterTypes),
						std::begin(p->m_parameterTypes),
						std::end(p->m_parameterTypes),
						[](const auto& a, const auto& b)
						{
							return *a == *b;
						});
			}

			return false;
		}

	private:
		std::shared_ptr<ITypeInfo> m_returnType;
		std::vector<std::shared_ptr<ITypeInfo>> m_parameterTypes;
	};

	class PrimitiveTypeInfo
		: public ITypeInfo
	{
	public:
		explicit PrimitiveTypeInfo(TypeCategory category, Word size)
			: m_category(category)
			, m_size(size)
		{
			assert(m_category != TypeCategory::function);
		}

		[[nodiscard]]
		TypeCategory category() const noexcept override
		{
			return m_category;
		}

		[[nodiscard]]
		Word size() const noexcept override
		{
			return m_size;
		}

		[[nodiscard]]
		std::string name() const override
		{
			switch (m_category)
			{
				case TypeCategory::integer:
					return "int";
				default:
					return "?";
			}
		}

		[[nodiscard]]
		bool equals(const ITypeInfo& type) const override
		{
			if (const auto p = dynamic_cast<const PrimitiveTypeInfo*>(&type))
			{
				return m_category == p->m_category && m_size == p->m_size;
			}

			return false;
		}

	private:
		TypeCategory m_category;
		Word m_size;
	};
}
