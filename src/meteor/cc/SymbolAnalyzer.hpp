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

		// argument-list:
		//     '(' ')'
		//     '(' assignment-expression {',' assignment-expression}* ')'
		void visit(ArgumentListNode& node)
		{
			// assignment-expression*
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

		// if-statement:
		//     'if' paren-expression compound-statement
		//     'if' paren-expression compound-statement 'else' compound-statement
		void visit(IfStatementNode& node)
		{
			// condition
			node.condition().accept(*this);

			if (node.condition().typeInfo()->category() != TypeCategory::integer)
			{
				reportError(node.condition(), u8"condition of if statement must have type of 'int'.");
			}

			// then
			node.then().accept(*this);

			// else
			if (const auto otherwise = node.otherwise())
			{
				otherwise->accept(*this);
			}
		}

		// while-statement:
		//     'while' paren-expression compound-statement
		//     'while' paren-expression compound-statement 'else' compound-statement
		void visit(WhileStatementNode& node)
		{
			// condition
			node.condition().accept(*this);

			if (node.condition().typeInfo()->category() != TypeCategory::integer)
			{
				reportError(node.condition(), u8"condition of while statement must have type of 'int'.");
			}

			// body
			node.body().accept(*this);
		}

		// return-statement:
		//     'return' expression? ';'
		void visit(ReturnStatementNode& node)
		{
			// expression
			if (const auto expression = node.expression())
			{
				expression->accept(*this);

				// Check return value type.
				if (*expression->typeInfo() != *m_functionType->returnType())
				{
					reportError(*expression, u8"incompatible return value type.");
				}
			}
			else
			{
				// TODO: void
			}
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
			m_functionType = std::static_pointer_cast<FunctionTypeInfo>(node.symbol()->typeInfo());

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

		// pointer-declarator:
		//     '*' direct-declarator
		void visit(PointerDeclaratorNode& node)
		{
			// pointer type
			m_baseType = std::make_shared<PointerTypeInfo>(m_baseType);

			// declarator
			node.declarator().accept(*this);
		}

		// function-declarator:
		//     direct-declarator parameter-list
		void visit(FunctionDeclaratorNode& node)
		{
			// parameter-list
			node.parameters().accept(*this);

			m_registerParams = false;

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

		// comma-expression:
		//     assignment-expression {',' assignment-expression}*
		void visit(CommaExpressionNode& node)
		{
			// left-hand-side
			node.left().accept(*this);

			// right-hand-side
			node.right().accept(*this);

			// Resolve the type.
			node.typeInfo({}, node.right().typeInfo(), false);
		}

		// assignment-expression:
		//     unary-expression '=' assignment-expression
		void visit(AssignmentExpressionNode& node)
		{
			// left-hand-side
			node.left().accept(*this);

			if (!node.left().isLvalue())
			{
				reportError(node, u8"rvalue expression is not assignable.");
			}

			// right-hand-side
			node.right().accept(*this);

			if (*node.left().typeInfo() != *node.right().typeInfo())
			{
				reportError(node, u8"incompatible type.");
			}

			// Resolve the type.
			node.typeInfo({}, node.right().typeInfo(), false);
		}

		// bitwise-or-expression:
		//     bitwise-or-expression '|' bitwise-xor-expression
		void visit(BitwiseOrExpressionNode& node)
		{
			// left-hand-side
			node.left().accept(*this);
			// right-hand-side
			node.right().accept(*this);

			if (*node.left().typeInfo() != *node.right().typeInfo() || *node.left().typeInfo() != *m_intType)
			{
				reportError(node, u8"operands of binary operator '|' must have a type of 'int'.");
			}

			// Resolve the type.
			node.typeInfo({}, m_intType, false);
		}

		// bitwise-xor-expression:
		//     bitwise-xor-expression '|' bitwise-and-expression
		void visit(BitwiseXorExpressionNode& node)
		{
			// left-hand-side
			node.left().accept(*this);
			// right-hand-side
			node.right().accept(*this);

			if (*node.left().typeInfo() != *node.right().typeInfo() || *node.left().typeInfo() != *m_intType)
			{
				reportError(node, u8"operands of binary operator '^' must have a type of 'int'.");
			}

			// Resolve the type.
			node.typeInfo({}, m_intType, false);
		}

		// bitwise-and-expression:
		//     bitwise-and-expression '|' equality-expression
		void visit(BitwiseAndExpressionNode& node)
		{
			// left-hand-side
			node.left().accept(*this);
			// right-hand-side
			node.right().accept(*this);

			if (*node.left().typeInfo() != *node.right().typeInfo() || *node.left().typeInfo() != *m_intType)
			{
				reportError(node, u8"operands of binary operator '&' must have a type of 'int'.");
			}

			// Resolve the type.
			node.typeInfo({}, m_intType, false);
		}

		// addition-expression:
		//     additive-expression '+' mutiplicative-expression
		void visit(AdditionExpressionNode& node)
		{
			// left-hand-side
			node.left().accept(*this);
			// right-hand-side
			node.right().accept(*this);

			if (*node.left().typeInfo() != *node.right().typeInfo() || *node.left().typeInfo() != *m_intType)
			{
				reportError(node, u8"operands of binary operator '+' must have a type of 'int'.");
			}

			// Resolve the type.
			node.typeInfo({}, m_intType, false);
		}

		// subtraction-expression:
		//     additive-expression '-' mutiplicative-expression
		void visit(SubtractionExpressionNode& node)
		{
			// left-hand-side
			node.left().accept(*this);
			// right-hand-side
			node.right().accept(*this);

			if (*node.left().typeInfo() != *node.right().typeInfo() || *node.left().typeInfo() != *m_intType)
			{
				reportError(node, u8"operands of binary operator '-' must have a type of 'int'.");
			}

			// Resolve the type.
			node.typeInfo({}, m_intType, false);
		}

		// plus-expression:
		//     '+' unary-expression
		void visit(PlusExpressionNode& node)
		{
			// operand
			node.operand().accept(*this);

			if (*node.operand().typeInfo() != *m_intType)
			{
				reportError(node, u8"operand of unary operator '+' must have a type of 'int'.");
			}

			// Resolve the type.
			node.typeInfo({}, m_intType, false);
		}

		// minus-expression:
		//     '-' unary-expression
		void visit(MinusExpressionNode& node)
		{
			// operand
			node.operand().accept(*this);

			if (*node.operand().typeInfo() != *m_intType)
			{
				reportError(node, u8"operand of unary operator '-' must have a type of 'int'.");
			}

			// Resolve the type.
			node.typeInfo({}, m_intType, false);
		}

		// address-expression:
		//     '&' unary-expression
		void visit(AddressExpressionNode& node)
		{
			// operand
			node.operand().accept(*this);

			if (!node.operand().isLvalue())
			{
				reportError(node, u8"operand of unary operator '&' must be a lvalue.");
			}

			// Resolve the type.
			node.typeInfo({}, std::make_shared<PointerTypeInfo>(node.operand().typeInfo()), false);
		}

		// derefenrece-expression:
		//     '*' unary-expression
		void visit(DereferenceExpressionNode& node)
		{
			// operand
			node.operand().accept(*this);

			if (node.operand().typeInfo()->category() != TypeCategory::pointer)
			{
				reportError(node, u8"operand of unary operator '*' must have a pointer type.");
			}

			// Resolve the type.
			node.typeInfo({}, std::static_pointer_cast<PointerTypeInfo>(node.operand().typeInfo())->baseType(), true);
		}

		// call-expression:
		//     postfix-expression argument-list
		void visit(CallExpressionNode& node)
		{
			// callee
			node.callee().accept(*this);

			// arguments
			node.arguments().accept(*this);

			// Check callee types.
			if (node.callee().typeInfo()->category() != TypeCategory::function)
			{
				reportError(node, u8"operand is not a function.");
			}

			const auto functionType = std::static_pointer_cast<FunctionTypeInfo>(node.callee().typeInfo());

			// Check argument types.
			if (node.arguments().children().size() != functionType->parameterTypes().size())
			{
				reportError(node, u8"invalid number of arguments.");
			}

			if (!std::equal(
				std::begin(node.arguments().children()),
				std::end(node.arguments().children()),
				std::begin(functionType->parameterTypes()),
				std::end(functionType->parameterTypes()),
				[](const auto& arg, const auto& argType)
				{
					return *static_cast<ExpressionNode&>(*arg).typeInfo() == *argType;
				}))
			{
				reportError(node, u8"incompatible argument types.");
			}

			// Resolve the type.
			node.typeInfo({}, functionType->returnType(), false);
		}

		// identifier-expression:
		//     identifier
		void visit(IdentifierExpressionNode& node)
		{
			// Resolve the symbol.
			if (const auto symbol = m_scope->find(node.name(), true))
			{
				node.symbol({}, symbol);

				// Resolve the type.
				node.typeInfo({}, symbol->typeInfo(), true);
			}
			else
			{
				reportError(node, boost::format(u8"undeclared identifier `%1%'.") % node.name());
			}
		}

		// integer-expression:
		//     integer-literal
		void visit(IntegerExpressionNode& node)
		{
			// Resolve the type.
			node.typeInfo({}, m_intType, false);
		}

		// integer-type:
		//     'int'
		void visit(IntegerTypeNode& node)
		{
			m_baseType = m_intType;

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
		std::shared_ptr<FunctionTypeInfo> m_functionType;
		std::shared_ptr<ITypeInfo> m_intType = std::make_shared<PrimitiveTypeInfo>(TypeCategory::integer, 1);
		bool m_registerParams;
	};
}
