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
#include <vector>

#include "Symbol.hpp"

namespace meteor::cc
{
	class Scope;
	class Parser;

#define METEOR_CC_NODE(name) class name;
#include "Node.def.hpp"

	class IVisitor
	{
	public:
		virtual ~IVisitor() =default;

#define METEOR_CC_NODE(name) virtual void visit(name& node) =0;
#include "Node.def.hpp"
	};

	class Node
	{
	public:
		explicit Node(std::size_t line)
			: m_line(line)
			, m_children()
		{
			m_children.reserve(8);
		}

		// Uncopyable, .
		Node(const Node&) =delete;
		Node(Node&&) =delete;

		Node& operator=(const Node&) =delete;
		Node& operator=(Node&&) =delete;

		virtual ~Node() =default;

		[[nodiscard]]
		std::size_t line() const noexcept
		{
			return m_line;
		}

		[[nodiscard]]
		const std::vector<std::unique_ptr<Node>>& children() const noexcept
		{
			return m_children;
		}

		virtual void accept(IVisitor& visitor) =0;

	protected:
		void addChild(std::unique_ptr<Node>&& node)
		{
			m_children.emplace_back(std::move(node));
		}

	private:
		std::size_t m_line;
		std::vector<std::unique_ptr<Node>> m_children;
	};

	class StatementNode
		: public Node
	{
	public:
		using Node::Node;
	};

	class DeclarationNode
		: public StatementNode
	{
	public:
		using StatementNode::StatementNode;

		[[nodiscard]]
		virtual std::shared_ptr<Symbol> symbol() const =0;
	};

	class DeclaratorNode
		: public Node
	{
	public:
		using Node::Node;

		[[nodiscard]]
		virtual std::shared_ptr<Symbol> symbol() const =0;
	};

	class ExpressionNode
		: public Node
	{
	public:
		using Node::Node;

		[[nodiscard]]
		std::shared_ptr<ITypeInfo> typeInfo() const noexcept
		{
			return m_typeInfo;
		}

		[[nodiscard]]
		bool isLvalue() const noexcept
		{
			return m_isLvalue;
		}

		void typeInfo(Passkey<SymbolAnalyzer>, const std::shared_ptr<ITypeInfo>& typeInfo, bool isLvalue)
		{
			assert(typeInfo);

			m_typeInfo = typeInfo;
			m_isLvalue = isLvalue;
		}

	private:
		std::shared_ptr<ITypeInfo> m_typeInfo;
		bool m_isLvalue;
	};

	class TypeNode
		: public Node
	{
	public:
		using Node::Node;

		[[nodiscard]]
		virtual std::shared_ptr<ITypeInfo> typeInfo() const =0;
	};

	// root:
	//     external-declaration*
	// external-declaration:
	//     function-declaration
	//     variable-declaration
	class RootNode
		: public Node
	{
	public:
		explicit RootNode(std::string_view filename)
			: Node(0)
			, m_filename(filename)
			, m_scope(nullptr)
		{
		}

		[[nodiscard]]
		std::string_view filename() const noexcept
		{
			return m_filename;
		}

		void addChild(Passkey<Parser>, std::unique_ptr<DeclarationNode>&& node)
		{
			assert(node);

			Node::addChild(std::move(node));
		}

		void accept(IVisitor& visitor) override
		{
			visitor.visit(*this);
		}

		void scope(Passkey<SymbolAnalyzer>, const std::shared_ptr<Scope>& scope)
		{
			assert(scope);

			m_scope = scope;
		}

	private:
		std::string m_filename;
		std::shared_ptr<Scope> m_scope;
	};

	// --- statement ---

	// empty-statement:
	//     ';'
	class EmptyStatementNode
		: public StatementNode
	{
	public:
		using StatementNode::StatementNode;

		void accept(IVisitor& visitor) override
		{
			visitor.visit(*this);
		}
	};

	// compound-statement:
	//     '{' statement* '}'
	class CompoundStatementNode
		: public StatementNode
	{
	public:
		explicit CompoundStatementNode(std::size_t line)
			: StatementNode(line)
			, m_scope(nullptr)
		{
		}

		void addChild(Passkey<Parser>, std::unique_ptr<StatementNode>&& node)
		{
			assert(node);

			Node::addChild(std::move(node));
		}

		void accept(IVisitor& visitor) override
		{
			visitor.visit(*this);
		}

		void scope(Passkey<SymbolAnalyzer>, const std::shared_ptr<Scope>& scope)
		{
			assert(scope);

			m_scope = scope;
		}

	private:
		std::shared_ptr<Scope> m_scope;
	};

	// if-statement:
	//     'if' paren-expression compound-statement
	//     'if' paren-expression compound-statement 'else' compound-statement
	class IfStatementNode
		: public StatementNode
	{
	public:
		explicit IfStatementNode(std::size_t line, std::unique_ptr<ExpressionNode>&& condition, std::unique_ptr<StatementNode>&& then, std::unique_ptr<StatementNode>&& otherwise)
			: StatementNode(line)
		{
			assert(condition);

			addChild(std::move(condition));
			addChild(std::move(then));
			addChild(std::move(otherwise));
		}

		[[nodiscard]]
		ExpressionNode& condition() const noexcept
		{
			return static_cast<ExpressionNode&>(*children()[0]);
		}

		[[nodiscard]]
		StatementNode& then() const noexcept
		{
			return static_cast<StatementNode&>(*children()[1]);
		}

		[[nodiscard]]
		StatementNode* otherwise() const noexcept
		{
			return static_cast<StatementNode*>(children()[2].get());
		}

		void accept(IVisitor& visitor) override
		{
			visitor.visit(*this);
		}
	};

	// while-statement:
	//     'while' paren-expression compound-statement
	class WhileStatementNode
		: public StatementNode
	{
	public:
		explicit WhileStatementNode(std::size_t line, std::unique_ptr<ExpressionNode>&& condition, std::unique_ptr<StatementNode>&& body)
			: StatementNode(line)
		{
			assert(condition);
			assert(body);

			addChild(std::move(condition));
			addChild(std::move(body));
		}

		[[nodiscard]]
		ExpressionNode& condition() const noexcept
		{
			return static_cast<ExpressionNode&>(*children()[0]);
		}

		[[nodiscard]]
		StatementNode& body() const noexcept
		{
			return static_cast<StatementNode&>(*children()[1]);
		}

		void accept(IVisitor& visitor) override
		{
			visitor.visit(*this);
		}
	};

	// return-statement:
	//     'return' expression? ';'
	class ReturnStatementNode
		: public StatementNode
	{
	public:
		explicit ReturnStatementNode(std::size_t line, std::unique_ptr<ExpressionNode>&& expression)
			: StatementNode(line)
		{
			addChild(std::move(expression));
		}

		[[nodiscard]]
		ExpressionNode* expression() const noexcept
		{
			return static_cast<ExpressionNode*>(children()[0].get());
		}

		void accept(IVisitor& visitor) override
		{
			visitor.visit(*this);
		}
	};

	// expression-statement:
	//     expression ';'
	class ExpressionStatementNode
		: public StatementNode
	{
	public:
		explicit ExpressionStatementNode(std::size_t line, std::unique_ptr<ExpressionNode>&& expression)
			: StatementNode(line)
		{
			assert(expression);

			addChild(std::move(expression));
		}

		[[nodiscard]]
		ExpressionNode& expression() const noexcept
		{
			return static_cast<ExpressionNode&>(*children()[0]);
		}

		void accept(IVisitor& visitor) override
		{
			visitor.visit(*this);
		}
	};

	// --- declaration ---

	// function-declaration:
	//     type declarator compound-statement
	class FunctionDeclarationNode
		: public DeclarationNode
	{
	public:
		explicit FunctionDeclarationNode(std::size_t line, std::unique_ptr<TypeNode>&& typeSpecifier, std::unique_ptr<DeclaratorNode>&& declarator, std::unique_ptr<StatementNode>&& body)
			: DeclarationNode(line)
			, m_scope(nullptr)
		{
			assert(typeSpecifier);
			assert(declarator);
			assert(body);

			addChild(std::move(typeSpecifier));
			addChild(std::move(declarator));
			addChild(std::move(body));
		}

		[[nodiscard]]
		TypeNode& typeSpecifier() const noexcept
		{
			return static_cast<TypeNode&>(*children()[0]);
		}

		[[nodiscard]]
		DeclaratorNode& declarator() const noexcept
		{
			return static_cast<DeclaratorNode&>(*children()[1]);
		}

		[[nodiscard]]
		StatementNode& body() const noexcept
		{
			return static_cast<StatementNode&>(*children()[2]);
		}

		[[nodiscard]]
		std::shared_ptr<Symbol> symbol() const override
		{
			return declarator().symbol();
		}

		void accept(IVisitor& visitor) override
		{
			visitor.visit(*this);
		}

		void scope(Passkey<SymbolAnalyzer>, const std::shared_ptr<Scope>& scope)
		{
			assert(scope);

			m_scope = scope;
		}

	private:
		std::shared_ptr<Scope> m_scope;
	};

	// variable-declaration:
	//     type declarator
	class VariableDeclarationNode
		: public DeclarationNode
	{
	public:
		explicit VariableDeclarationNode(std::size_t line, std::unique_ptr<TypeNode>&& typeSpecifier, std::unique_ptr<DeclaratorNode>&& declarator)
			: DeclarationNode(line)
		{
			assert(typeSpecifier);
			assert(declarator);

			addChild(std::move(typeSpecifier));
			addChild(std::move(declarator));
		}

		[[nodiscard]]
		TypeNode& typeSpecifier() const noexcept
		{
			return static_cast<TypeNode&>(*children()[0]);
		}

		[[nodiscard]]
		DeclaratorNode& declarator() const noexcept
		{
			return static_cast<DeclaratorNode&>(*children()[1]);
		}

		[[nodiscard]]
		std::shared_ptr<Symbol> symbol() const override
		{
			return declarator().symbol();
		}

		void accept(IVisitor& visitor) override
		{
			visitor.visit(*this);
		}
	};

	// parameter-declaration:
	//     type declarator
	class ParameterDeclarationNode
		: public DeclarationNode
	{
	public:
		explicit ParameterDeclarationNode(std::size_t line, std::unique_ptr<TypeNode>&& typeSpecifier, std::unique_ptr<DeclaratorNode>&& declarator)
			: DeclarationNode(line)
		{
			assert(typeSpecifier);
			assert(declarator);

			addChild(std::move(typeSpecifier));
			addChild(std::move(declarator));
		}

		[[nodiscard]]
		TypeNode& typeSpecifier() const noexcept
		{
			return static_cast<TypeNode&>(*children()[0]);
		}

		[[nodiscard]]
		DeclaratorNode& declarator() const noexcept
		{
			return static_cast<DeclaratorNode&>(*children()[1]);
		}

		[[nodiscard]]
		std::shared_ptr<Symbol> symbol() const override
		{
			return declarator().symbol();
		}

		void accept(IVisitor& visitor) override
		{
			visitor.visit(*this);
		}
	};

	// --- declarator ---

	// identifier-declarator:
	//     identifier
	class IdentifierDeclaratorNode
		: public DeclaratorNode
	{
	public:
		explicit IdentifierDeclaratorNode(std::size_t line, std::string_view name)
			: DeclaratorNode(line)
			, m_name(name)
			, m_symbol(nullptr)
		{
		}

		[[nodiscard]]
		std::string_view name() const noexcept
		{
			return m_name;
		}

		[[nodiscard]]
		std::shared_ptr<Symbol> symbol() const noexcept override
		{
			return m_symbol;
		}

		void accept(IVisitor& visitor) override
		{
			visitor.visit(*this);
		}

		void symbol(Passkey<SymbolAnalyzer>, const std::shared_ptr<Symbol>& symbol)
		{
			assert(symbol);

			m_symbol = symbol;
		}

	private:
		std::string m_name;
		std::shared_ptr<Symbol> m_symbol;
	};

	// pointer-declarator:
	//     '*' direct-declarator
	class PointerDeclaratorNode
		: public DeclaratorNode
	{
	public:
		explicit PointerDeclaratorNode(std::size_t line, std::unique_ptr<DeclaratorNode>&& declarator)
			: DeclaratorNode(line)
		{
			assert(declarator);

			addChild(std::move(declarator));
		}

		[[nodiscard]]
		DeclaratorNode& declarator() const noexcept
		{
			return static_cast<DeclaratorNode&>(*children()[0]);
		}

		[[nodiscard]]
		std::shared_ptr<Symbol> symbol() const override
		{
			return declarator().symbol();
		}

		void accept(IVisitor& visitor) override
		{
			visitor.visit(*this);
		}
	};

	// parameter-list:
	//     '(' 'void' ')'
	//     '(' parameter-declaration {',' parameter-declaration}* ')'
	class ParameterListNode
		: public Node
	{
	public:
		using Node::Node;

		void addChild(Passkey<Parser>, std::unique_ptr<DeclarationNode>&& node)
		{
			assert(node);

			Node::addChild(std::move(node));
		}

		void accept(IVisitor& visitor) override
		{
			visitor.visit(*this);
		}
	};

	// function-declarator:
	//     direct-declarator parameter-list
	class FunctionDeclaratorNode
		: public DeclaratorNode
	{
	public:
		explicit FunctionDeclaratorNode(std::size_t line, std::unique_ptr<DeclaratorNode>&& declarator, std::unique_ptr<ParameterListNode>&& parameters)
			: DeclaratorNode(line)
		{
			assert(declarator);
			assert(parameters);

			addChild(std::move(declarator));
			addChild(std::move(parameters));
		}

		[[nodiscard]]
		DeclaratorNode& declarator() const noexcept
		{
			return static_cast<DeclaratorNode&>(*children()[0]);
		}

		[[nodiscard]]
		ParameterListNode& parameters() const noexcept
		{
			return static_cast<ParameterListNode&>(*children()[1]);
		}

		[[nodiscard]]
		std::shared_ptr<Symbol> symbol() const override
		{
			return declarator().symbol();
		}

		void accept(IVisitor& visitor) override
		{
			visitor.visit(*this);
		}
	};

	// --- expression ---

	class BinaryExpressionNode
		: public ExpressionNode
	{
	public:
		explicit BinaryExpressionNode(std::size_t line, std::unique_ptr<ExpressionNode>&& left, std::unique_ptr<ExpressionNode>&& right)
			: ExpressionNode(line)
		{
			assert(left);
			assert(right);

			addChild(std::move(left));
			addChild(std::move(right));
		}

		[[nodiscard]]
		ExpressionNode& left() const noexcept
		{
			return static_cast<ExpressionNode&>(*children()[0]);
		}

		[[nodiscard]]
		ExpressionNode& right() const noexcept
		{
			return static_cast<ExpressionNode&>(*children()[1]);
		}
	};

	class UnaryExpressionNode
		: public ExpressionNode
	{
	public:
		explicit UnaryExpressionNode(std::size_t line, std::unique_ptr<ExpressionNode>&& operand)
			: ExpressionNode(line)
		{
			assert(operand);

			addChild(std::move(operand));
		}

		[[nodiscard]]
		ExpressionNode& operand() const noexcept
		{
			return static_cast<ExpressionNode&>(*children()[0]);
		}
	};

	// comma-expression:
	//     comma-expression ',' assignment-expression
	class CommaExpressionNode
		: public BinaryExpressionNode
	{
	public:
		using BinaryExpressionNode::BinaryExpressionNode;

		void accept(IVisitor& visitor) override
		{
			visitor.visit(*this);
		}
	};

	// assignment-expression:
	//     unary-expression '=' assignment-expression
	class AssignmentExpressionNode
		: public BinaryExpressionNode
	{
	public:
		using BinaryExpressionNode::BinaryExpressionNode;

		void accept(IVisitor& visitor) override
		{
			visitor.visit(*this);
		}
	};

	// addition-expression:
	//     additive-expression '+' mutiplicative-expression
	class AdditionExpressionNode
		: public BinaryExpressionNode
	{
	public:
		using BinaryExpressionNode::BinaryExpressionNode;

		void accept(IVisitor& visitor) override
		{
			visitor.visit(*this);
		}
	};

	// subtraction-expression:
	//     additive-expression '+' mutiplicative-expression
	class SubtractionExpressionNode
		: public BinaryExpressionNode
	{
	public:
		using BinaryExpressionNode::BinaryExpressionNode;

		void accept(IVisitor& visitor) override
		{
			visitor.visit(*this);
		}
	};

	// plus-expression:
	//     '+' unary-expression
	class PlusExpressionNode
		: public UnaryExpressionNode
	{
	public:
		using UnaryExpressionNode::UnaryExpressionNode;

		void accept(IVisitor& visitor) override
		{
			visitor.visit(*this);
		}
	};

	// minus-expression:
	//     '-' unary-expression
	class MinusExpressionNode
		: public UnaryExpressionNode
	{
	public:
		using UnaryExpressionNode::UnaryExpressionNode;

		void accept(IVisitor& visitor) override
		{
			visitor.visit(*this);
		}
	};

	// address-expression:
	//     '&' unary-expression
	class AddressExpressionNode
		: public UnaryExpressionNode
	{
	public:
		using UnaryExpressionNode::UnaryExpressionNode;

		void accept(IVisitor& visitor) override
		{
			visitor.visit(*this);
		}
	};

	// dereference-expression:
	//     '*' unary-expression
	class DereferenceExpressionNode
		: public UnaryExpressionNode
	{
	public:
		using UnaryExpressionNode::UnaryExpressionNode;

		void accept(IVisitor& visitor) override
		{
			visitor.visit(*this);
		}
	};

	// argument-list:
	//     '(' ')'
	//     '(' assignment-expression {',' assignment-expression}* ')'
	class ArgumentListNode
		: public Node
	{
	public:
		using Node::Node;

		void addChild(Passkey<Parser>, std::unique_ptr<ExpressionNode>&& node)
		{
			assert(node);

			Node::addChild(std::move(node));
		}

		void accept(IVisitor& visitor) override
		{
			visitor.visit(*this);
		}
	};

	// call-expression:
	//     postfix-expression argument-list
	class CallExpressionNode
		: public ExpressionNode
	{
	public:
		explicit CallExpressionNode(std::size_t line, std::unique_ptr<ExpressionNode>&& callee, std::unique_ptr<ArgumentListNode>&& arguments)
			: ExpressionNode(line)
		{
			assert(callee);
			assert(arguments);

			addChild(std::move(callee));
			addChild(std::move(arguments));
		}

		[[nodiscard]]
		ExpressionNode& callee() const noexcept
		{
			return static_cast<ExpressionNode&>(*children()[0]);
		}

		[[nodiscard]]
		ArgumentListNode& arguments() const noexcept
		{
			return static_cast<ArgumentListNode&>(*children()[1]);
		}

		void accept(IVisitor& visitor) override
		{
			visitor.visit(*this);
		}
	};

	// identifier-expression:
	//     identifier
	class IdentifierExpressionNode
		: public ExpressionNode
	{
	public:
		explicit IdentifierExpressionNode(std::size_t line, std::string_view name)
			: ExpressionNode(line)
			, m_name(name)
		{
		}

		[[nodiscard]]
		std::string_view name() const noexcept
		{
			return m_name;
		}

		[[nodiscard]]
		std::shared_ptr<Symbol> symbol() const noexcept
		{
			return m_symbol;
		}

		void accept(IVisitor& visitor) override
		{
			visitor.visit(*this);
		}

		void symbol(Passkey<SymbolAnalyzer>, const std::shared_ptr<Symbol>& symbol)
		{
			assert(symbol);

			m_symbol = symbol;
		}

	private:
		std::string m_name;
		std::shared_ptr<Symbol> m_symbol;
	};

	// integer-expression:
	//     integer-literal
	class IntegerExpressionNode
		: public ExpressionNode
	{
	public:
		explicit IntegerExpressionNode(std::size_t line, Word value)
			: ExpressionNode(line)
			, m_value(value)
		{
		}

		[[nodiscard]]
		Word value() const noexcept
		{
			return m_value;
		}

		void accept(IVisitor& visitor) override
		{
			visitor.visit(*this);
		}

	private:
		Word m_value;
	};

	// --- type ---

	// integer-type:
	//     'int'
	class IntegerTypeNode
		: public TypeNode
	{
	public:
		using TypeNode::TypeNode;

		[[nodiscard]]
		std::shared_ptr<ITypeInfo> typeInfo() const noexcept override
		{
			return m_typeInfo;
		}

		void accept(IVisitor& visitor) override
		{
			visitor.visit(*this);
		}

		void typeInfo(Passkey<SymbolAnalyzer>, const std::shared_ptr<ITypeInfo>& typeInfo)
		{
			assert(typeInfo);

			m_typeInfo = typeInfo;
		}

	private:
		std::shared_ptr<ITypeInfo> m_typeInfo;
	};
}
