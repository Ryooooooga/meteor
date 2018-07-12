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

#include <boost/format.hpp>

namespace meteor::cc
{
	enum class TypeCategory
	{
		integer,
		function,
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

		virtual ~ITypeInfo() =default;

		[[nodiscard]]
		virtual TypeCategory category() const noexcept =0;

		[[nodiscard]]
		virtual std::string_view name() const noexcept =0;
	};

	class PrimitiveTypeInfo
		: public ITypeInfo
	{
	public:
		explicit PrimitiveTypeInfo(TypeCategory category, std::string name)
			: m_category(category), m_name(std::move(name)) {}

		[[nodiscard]]
		TypeCategory category() const noexcept override
		{
			return m_category;
		}

		[[nodiscard]]
		std::string_view name() const noexcept override
		{
			return m_name;
		}

	private:
		TypeCategory m_category;
		std::string m_name;
	};

	class FunctionTypeInfo
		: public ITypeInfo
	{
	public:
		explicit FunctionTypeInfo(const std::shared_ptr<ITypeInfo>& returnType)
			: m_name()
		{
			assert(returnType);

			m_name = (boost::format(u8"%1%()") % returnType->name()).str();
		}

		[[nodiscard]]
		TypeCategory category() const noexcept override
		{
			return TypeCategory::function;
		}

		[[nodiscard]]
		std::string_view name() const noexcept override
		{
			return m_name;
		}

	private:
		std::string m_name;
	};
}
