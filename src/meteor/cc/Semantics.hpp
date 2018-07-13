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
		explicit Semantics()
			: m_name()
			, m_scope()
			, m_intTypeInfo(std::make_shared<PrimitiveTypeInfo>(TypeCategory::integer, u8"int"))
		{
		}

		// Uncopyable, unmovable.
		Semantics(const Semantics&) =delete;
		Semantics(Semantics&&) =delete;

		Semantics& operator=(const Semantics&) =delete;
		Semantics& operator=(Semantics&&) =delete;

		~Semantics() =default;

		// root
		[[nodiscard]]
		std::unique_ptr<RootNode> actOnRootBegan(std::string_view name)
		{
			m_name = name;
			m_scope = std::make_shared<Scope>(m_scope);

			return std::make_unique<RootNode>(m_name, m_scope);
		}

		void actOnRootEnded()
		{
			assert(m_scope);
			assert(m_scope->parentScope() == nullptr);

			m_scope = m_scope->parentScope();
		}

		// function-declaration
		[[nodiscard]]
		std::unique_ptr<FunctionDeclarationNode> actOnFunctionDeclaration(const std::shared_ptr<Token>& name, std::unique_ptr<TypeNode>&& returnType, std::unique_ptr<ParameterListNode>&& parameters)
		{
			// TODO: Build the function type.
			const auto typeInfo = std::make_shared<FunctionTypeInfo>(returnType->typeInfo());

			auto symbol = m_scope->find(name->text(), false);

			if (symbol)
			{
				// Check signatures.
				if (typeInfo->name() != symbol->typeInfo()->name())
				{
					reportError(name->line(), boost::format(u8"`%1%' redeclared as different type.") % name->text());
				}
			}
			else
			{
				symbol = std::make_shared<Symbol>(std::string {name->text()}, typeInfo, true);

				if (!m_scope->tryRegister(symbol))
				{
					assert(0 && "never reached");
				}
			}

			return std::make_unique<FunctionDeclarationNode>(name->line(), symbol, std::move(returnType), std::move(parameters));
		}

		// function-definition
		void actOnFunctionBegan(FunctionDeclarationNode& node)
		{
			m_scope = std::make_shared<Scope>(m_scope);

			// TODO: params
			(void)node;
		}

		[[nodiscard]]
		std::unique_ptr<DeclarationNode> actOnFunctionEnded(std::unique_ptr<FunctionDeclarationNode>&& declaration, std::unique_ptr<StatementNode>&& body)
		{
			auto node = std::make_unique<FunctionDefinitionNode>(std::move(declaration), std::move(body), m_scope);

			m_scope = m_scope->parentScope();

			return node;
		}

		// parameter-list
		[[nodiscard]]
		std::unique_ptr<ParameterListNode> actOnParameterListBegan(const std::shared_ptr<Token>& token)
		{
			return std::make_unique<ParameterListNode>(token->line());
		}

		// variable-declaration
		[[nodiscard]]
		std::unique_ptr<DeclarationNode> actOnVariableDeclaration(const std::shared_ptr<Token>& name, std::unique_ptr<TypeNode>&& type)
		{
			const auto symbol = std::make_shared<Symbol>(name->text(), type->typeInfo(), m_scope->parentScope() == nullptr);

			// Register to the scope.
			if (!m_scope->tryRegister(symbol))
			{
				reportError(name->line(), boost::format(u8"`%1%' is already declarared in this scope.") % symbol->name());
			}

			return std::make_unique<VariableDeclarationNode>(name->line(), symbol, std::move(type));
		}

		// parameter-declaration
		[[nodiscard]]
		std::unique_ptr<DeclarationNode> actOnParameterDeclaration(const std::shared_ptr<Token>& name, std::unique_ptr<TypeNode>&& type)
		{
			const auto symbol = std::make_shared<Symbol>(name->text(), type->typeInfo(), false);

			// Register to the scope.
			if (!m_scope->tryRegister(symbol))
			{
				reportError(name->line(), boost::format(u8"`%1%' is already declarared in this scope.") % symbol->name());
			}

			return std::make_unique<VariableDeclarationNode>(name->line(), symbol, std::move(type));
		}

		// empty-statement
		[[nodiscard]]
		std::unique_ptr<StatementNode> actOnEmptyStatement(const std::shared_ptr<Token>& token)
		{
			return std::make_unique<EmptyStatementNode>(token->line());
		}

		// compound-statement
		[[nodiscard]]
		std::unique_ptr<CompoundStatementNode> actOnCompoundStatementBegan(const std::shared_ptr<Token>& token)
		{
			m_scope = std::make_shared<Scope>(m_scope);

			return std::make_unique<CompoundStatementNode>(token->line(), m_scope);
		}

		void actOnCompoundStatementEnded()
		{
			assert(m_scope);

			m_scope = m_scope->parentScope();
		}

		// if-statement
		[[nodiscard]]
		std::unique_ptr<StatementNode> actOnIfStatement(const std::shared_ptr<Token>& token, std::unique_ptr<ExpressionNode>&& condition, std::unique_ptr<StatementNode>&& then, std::unique_ptr<StatementNode>&& otherwise)
		{
			return std::make_unique<IfStatementNode>(token->line(), std::move(condition), std::move(then), std::move(otherwise));
		}

		// expression-statement
		[[nodiscard]]
		std::unique_ptr<StatementNode> actOnExpressionStatement(std::unique_ptr<ExpressionNode>&& expression)
		{
			return std::make_unique<ExpressionStatementNode>(expression->line(), std::move(expression));
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
		template <typename Message>
		[[noreturn]]
		void reportError(size_t line, Message&& message)
		{
			throw std::runtime_error { (boost::format(u8"%1%(%2%): %3%") % m_name % line % std::forward<Message>(message)).str() };
		}

		std::string m_name;
		std::shared_ptr<Scope> m_scope;

		std::shared_ptr<ITypeInfo> m_intTypeInfo;
	};
}
