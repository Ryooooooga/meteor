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

#include <boost/format.hpp>

#include "Node.hpp"
#include "Scope.hpp"
#include "TypeInfo.hpp"

namespace meteor::cc
{
	class SymbolAnalyzer
		: private IVisitor
	{
	public:
		explicit SymbolAnalyzer() =default;

		// Uncopyable, unmovable.
		SymbolAnalyzer(const SymbolAnalyzer&) =delete;
		SymbolAnalyzer(SymbolAnalyzer&&) =delete;

		SymbolAnalyzer& operator=(const SymbolAnalyzer&) =delete;
		SymbolAnalyzer& operator=(SymbolAnalyzer&&) =delete;

		~SymbolAnalyzer() =default;

		void resolve(RootNode& node)
		{
			visit(node);
		}

	private:
		// root:
		//     external-declaration*
		void visit(RootNode& node)
		{
			m_name = node.filename();
			m_registerParams = false;

			// Generate the global scope.
			m_scope = std::make_shared<Scope>(m_scope);

			node.scope({}, m_scope);

			// external-declaration*
			for (const auto& child : node.children())
			{
				child->accept(*this);
			}

			// Pop the scope.
			m_scope = m_scope->parentScope();
		}

		// parameter-list:
		//     '(' 'void' ')'
		//     '(' parameter-declaration {',' parameter-declaration}* ')'
		void visit(ParameterListNode& node)
		{
			// parameter-declaration*
			for (const auto& child : node.children())
			{
				child->accept(*this);
			}
		}

		// empty-statement:
		//     ';'
		void visit([[maybe_unused]] EmptyStatementNode& node)
		{
		}

		// compound-statement:
		//     '{' statement* '}'
		void visit(CompoundStatementNode& node)
		{
			// Generate the local scope.
			m_scope = std::make_shared<Scope>(m_scope);

			node.scope({}, m_scope);

			// statement*
			for (const auto& child : node.children())
			{
				child->accept(*this);
			}

			// Pop the scope.
			m_scope = m_scope->parentScope();
		}

		// expression-statement:
		//     expression ';'
		void visit(ExpressionStatementNode& node)
		{
			// expression
			node.expression().accept(*this);
		}

		// function-declaration:
		//     type declarator compound-statement
		void visit(FunctionDeclarationNode& node)
		{
			// type
			node.typeSpecifier().accept(*this);

			m_baseType = node.typeSpecifier().typeInfo();

			// declarator
			node.declarator().accept(*this);

			// Register the function symbol.
			if (!m_scope->tryRegister(node.declarator().symbol()))
			{
				reportError(node, boost::format(u8"redefinition of function `%1%'.") % node.declarator().symbol()->name());
			}

			// Generate the function scope.
			m_scope = std::make_shared<Scope>(m_scope);

			node.scope({}, m_scope);

			// Register parameters.
			m_registerParams = true;
			node.declarator().accept(*this);
			m_registerParams = false;

			// compound-statement
			node.body().accept(*this);

			// Pop the scope.
			m_scope = m_scope->parentScope();
		}

		// variable-declaration:
		//     type declarator
		void visit(VariableDeclarationNode& node)
		{
			// TODO: forward declarations.

			// type
			node.typeSpecifier().accept(*this);

			m_baseType = node.typeSpecifier().typeInfo();

			// declarator
			node.declarator().accept(*this);

			// Register the symbol.
			if (!m_scope->tryRegister(node.symbol()))
			{
				reportError(node, boost::format(u8"redefinition of `%1%'.") % node.symbol()->name());
			}
		}

		// parameter-declaration:
		//     type declarator
		void visit(ParameterDeclarationNode& node)
		{
			// type
			node.typeSpecifier().accept(*this);

			m_baseType = node.typeSpecifier().typeInfo();

			// declarator
			node.declarator().accept(*this);

			if (m_registerParams)
			{
				// Check the parameter types.
				if (m_baseType->category() == TypeCategory::function)
				{
					reportError(node, u8"parameter type don't accept a function type.");
				}

				// Register the symbol.
				if (!m_scope->tryRegister(node.symbol()))
				{
					reportError(node, boost::format(u8"redefinition of `%1%'.") % node.symbol()->name());
				}

				m_registerParams = false;
			}
		}

		// identifier-declarator:
		//     identifier
		void visit(IdentifierDeclaratorNode& node)
		{
			// Set the symbol.
			if (node.symbol() == nullptr)
			{
				node.symbol({}, std::make_shared<Symbol>(node.name(), m_baseType));
			}
		}

		// function-declarator:
		//     direct-declarator parameter-list
		void visit(FunctionDeclaratorNode& node)
		{
			// parameter-list
			node.parameters().accept(*this);

			// Build the function type.
			std::vector<std::shared_ptr<ITypeInfo>> paramTypes;

			for (const auto& param : node.parameters().children())
			{
				paramTypes.emplace_back(static_cast<DeclarationNode&>(*param).symbol()->typeInfo());
			}

			m_baseType = std::make_shared<FunctionTypeInfo>(m_baseType, std::move(paramTypes));

			// declarator
			node.declarator().accept(*this);
		}

		// identifier-expression:
		//     identifier
		void visit(IdentifierExpressionNode& node)
		{
			// Resolve the symbol.
			if (const auto symbol = m_scope->find(node.name(), true))
			{
				node.symbol({}, symbol);
			}
			else
			{
				reportError(node, boost::format(u8"undeclared identifier `%1%'.") % node.name());
			}
		}

		// integer-type:
		//     'int'
		void visit(IntegerTypeNode& node)
		{
			m_baseType = std::make_shared<PrimitiveTypeInfo>(TypeCategory::integer, 1);

			node.typeInfo({}, m_baseType);
		}

		template <typename Message>
		[[noreturn]]
		void reportError(const Node& node, Message&& message)
		{
			throw std::runtime_error { (boost::format(u8"%1%(%2%): %3%") % m_name % node.line() % std::forward<Message>(message)).str() };
		}

		std::string_view m_name;
		std::shared_ptr<Scope> m_scope;
		std::shared_ptr<ITypeInfo> m_baseType;
		bool m_registerParams;
	};
}
