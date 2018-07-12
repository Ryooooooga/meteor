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
#include "Token.hpp"

namespace meteor::cc
{
	class Semantics
	{
	public:
		explicit Semantics(std::string_view name)
			: m_name(name)
			, m_intTypeInfo(std::make_shared<PrimitiveTypeInfo>(TypeCategory::integer, u8"int"))
		{
		}

		// Uncopyable, unmovable.
		Semantics(const Semantics&) =delete;
		Semantics(Semantics&&) =delete;

		Semantics& operator=(const Semantics&) =delete;
		Semantics& operator=(Semantics&&) =delete;

		~Semantics() =default;

		// function-declaration
		[[nodiscard]]
		std::unique_ptr<FunctionDeclarationNode> actOnFunctionDeclaration(const std::shared_ptr<Token>& name, std::unique_ptr<TypeNode>&& returnType)
		{
			auto typeInfo = std::make_shared<FunctionTypeInfo>(returnType->typeInfo());

			return std::make_unique<FunctionDeclarationNode>(name->line(), typeInfo, std::string {name->text()}, std::move(returnType));
		}

		// variable-declaration
		[[nodiscard]]
		std::unique_ptr<DeclarationNode> actOnVariableDeclaration(const std::shared_ptr<Token>& name, std::unique_ptr<TypeNode>&& type, std::unique_ptr<ExpressionNode>&& initializer)
		{
			return std::make_unique<VariableDeclarationNode>(name->line(), type->typeInfo(), std::string {name->text()}, std::move(type), std::move(initializer));
		}

		// paren-expression
		[[nodiscard]]
		std::unique_ptr<ExpressionNode> actOnParenExpression(const std::shared_ptr<Token>& token, std::unique_ptr<ExpressionNode>&& expression)
		{
			return std::make_unique<ParenExpressionNode>(token->line(), expression->typeInfo(), std::move(expression));
		}

		// integer-expression
		[[nodiscard]]
		std::unique_ptr<ExpressionNode> actOnIntegerExpression(const std::shared_ptr<Token>& token)
		{
			return std::make_unique<IntegerExpressionNode>(token->line(), m_intTypeInfo, token->integer());
		}

		// integer-type
		[[nodiscard]]
		std::unique_ptr<TypeNode> actOnIntegerType(const std::shared_ptr<Token>& token)
		{
			return std::make_unique<IntegerTypeNode>(token->line(), m_intTypeInfo);
		}

	private:
		std::string_view m_name;

		std::shared_ptr<ITypeInfo> m_intTypeInfo;
	};
}
