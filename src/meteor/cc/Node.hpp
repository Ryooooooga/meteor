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

#include <vector>

#include <boost/format.hpp>

#include "../Type.hpp"
#include "Scope.hpp"
#include "TypeInfo.hpp"

namespace meteor::cc
{
	class Node;
	class StatementNode;
	class DeclarationNode;
	class ExpressionNode;
	class TypeNode;

	class RootNode;
	class FunctionDeclarationNode;
	class FunctionDefinitionNode;
	class VariableDeclarationNode;
	class EmptyStatementNode;
	class CompoundStatementNode;
	class IfStatementNode;
	class ExpressionStatementNode;
	class ParenExpressionNode;
	class IntegerExpressionNode;
	class IntegerTypeNode;

	class IVisitor
	{
	public:
		virtual ~IVisitor() =default;

		virtual void visit(RootNode& node) =0;
		virtual void visit(FunctionDeclarationNode& node) =0;
		virtual void visit(FunctionDefinitionNode& node) =0;
		virtual void visit(VariableDeclarationNode& node) =0;
		virtual void visit(EmptyStatementNode& node) =0;
		virtual void visit(CompoundStatementNode& node) =0;
		virtual void visit(IfStatementNode& node) =0;
		virtual void visit(ExpressionStatementNode& node) =0;
		virtual void visit(ParenExpressionNode& node) =0;
		virtual void visit(IntegerExpressionNode& node) =0;
		virtual void visit(IntegerTypeNode& node) =0;
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

		// Uncopyable, unmovable.
		Node(const Node&) =delete;
		Node(Node&&) =delete;

		Node& operator=(const Node&) =delete;
		Node& operator=(Node&&) =delete;

		~Node() =default;

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
		explicit DeclarationNode(std::size_t line, const std::shared_ptr<Symbol>& symbol)
			: StatementNode(line), m_symbol(symbol)
		{
			assert(m_symbol);
		}

		[[nodiscard]]
		std::shared_ptr<Symbol> symbol() const noexcept
		{
			return m_symbol;
		}

		[[nodiscard]]
		std::shared_ptr<ITypeInfo> typeInfo() const noexcept
		{
			return m_symbol->typeInfo();
		}

		[[nodiscard]]
		std::string_view name() const noexcept
		{
			return m_symbol->name();
		}

	private:
		std::shared_ptr<Symbol> m_symbol;
	};

	class ExpressionNode
		: public Node
	{
	public:
		explicit ExpressionNode(std::size_t line, const std::shared_ptr<ITypeInfo>& typeInfo)
			: Node(line), m_typeInfo(typeInfo)
		{
			assert(m_typeInfo);
		}

		[[nodiscard]]
		std::shared_ptr<ITypeInfo> typeInfo() const noexcept
		{
			return m_typeInfo;
		}

	private:
		std::shared_ptr<ITypeInfo> m_typeInfo;
	};

	class TypeNode
		: public Node
	{
	public:
		explicit TypeNode(std::size_t line, const std::shared_ptr<ITypeInfo>& typeInfo)
			: Node(line), m_typeInfo(typeInfo)
		{
			assert(m_typeInfo);
		}

		[[nodiscard]]
		std::shared_ptr<ITypeInfo> typeInfo() const noexcept
		{
			return m_typeInfo;
		}

	private:
		std::shared_ptr<ITypeInfo> m_typeInfo;
	};

	// root:
	//     external-declaration
	class RootNode
		: public Node
	{
	public:
		explicit RootNode(std::string name, const std::shared_ptr<Scope>& scope)
			: Node(0)
			, m_name(std::move(name))
			, m_scope(scope)
		{
			assert(m_scope);
		}

		[[nodiscard]]
		std::string_view name() const noexcept
		{
			return m_name;
		}

		void addChild(std::unique_ptr<DeclarationNode>&& node)
		{
			Node::addChild(std::move(node));
		}

		void accept(IVisitor& visitor) override
		{
			visitor.visit(*this);
		}

	private:
		std::string m_name;
		std::shared_ptr<Scope> m_scope;
	};

	// function-declaration:
	//     type name parameter-list ';'
	class FunctionDeclarationNode
		: public DeclarationNode
	{
	public:
		explicit FunctionDeclarationNode(std::size_t line, const std::shared_ptr<Symbol>& symbol, std::unique_ptr<TypeNode>&& type)
			: DeclarationNode(line, symbol)
		{
			assert(type);

			addChild(std::move(type));
		}

		[[nodiscard]]
		TypeNode& returnType() const noexcept
		{
			return static_cast<TypeNode&>(*children()[0]);
		}

		void accept(IVisitor& visitor) override
		{
			visitor.visit(*this);
		}
	};

	// function-definition:
	//     type name parameter-list compound-statement
	class FunctionDefinitionNode
		: public DeclarationNode
	{
	public:
		explicit FunctionDefinitionNode(std::unique_ptr<FunctionDeclarationNode>&& declaration, std::unique_ptr<StatementNode>&& body, const std::shared_ptr<Scope>& scope)
			: DeclarationNode(declaration->line(), declaration->symbol()), m_scope(scope)
		{
			assert(declaration);
			assert(body);
			assert(m_scope);

			addChild(std::move(declaration));
			addChild(std::move(body));
		}

		[[nodiscard]]
		TypeNode& returnType() const noexcept
		{
			return declaration().returnType();
		}

		[[nodiscard]]
		FunctionDeclarationNode& declaration() const noexcept
		{
			return static_cast<FunctionDeclarationNode&>(*children()[0]);
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

	private:
		std::shared_ptr<Scope> m_scope;
	};

	// variable-declaration:
	//     type identifier ';'
	//     type identifier '=' expression ';'
	class VariableDeclarationNode
		: public DeclarationNode
	{
	public:
		explicit VariableDeclarationNode(std::size_t line, const std::shared_ptr<Symbol>& symbol, std::unique_ptr<TypeNode>&& type)
			: DeclarationNode(line, symbol)
		{
			assert(type);

			addChild(std::move(type));
		}

		[[nodiscard]]
		TypeNode& type() const noexcept
		{
			return static_cast<TypeNode&>(*children()[0]);
		}

		void accept(IVisitor& visitor) override
		{
			visitor.visit(*this);
		}
	};

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
		explicit CompoundStatementNode(std::size_t line, const std::shared_ptr<Scope>& scope)
			: StatementNode(line), m_scope(scope)
		{
			assert(m_scope);
		}

		void addChild(std::unique_ptr<StatementNode>&& node)
		{
			assert(node);

			Node::addChild(std::move(node));
		}

		void accept(IVisitor& visitor) override
		{
			visitor.visit(*this);
		}

	private:
		std::shared_ptr<Scope> m_scope;
	};

	// if-statement:
	//     'if' '(' expression ')' statement
	//     'if' '(' expression ')' statement 'else' statement
	class IfStatementNode
		: public StatementNode
	{
	public:
		explicit IfStatementNode(std::size_t line, std::unique_ptr<ExpressionNode>&& condition, std::unique_ptr<StatementNode>&& then, std::unique_ptr<StatementNode>&& otherwise)
			: StatementNode(line)
		{
			assert(condition);
			assert(then);

			addChild(std::move(condition));
			addChild(std::move(then));
			addChild(std::move(otherwise));
		}

		[[nodiscard]]
		ExpressionNode& condition() noexcept
		{
			return static_cast<ExpressionNode&>(*children()[0]);
		}

		[[nodiscard]]
		StatementNode& then() noexcept
		{
			return static_cast<StatementNode&>(*children()[1]);
		}

		[[nodiscard]]
		StatementNode* otherwise() noexcept
		{
			return static_cast<StatementNode*>(children()[2].get());
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
		ExpressionNode& expression() noexcept
		{
			return static_cast<ExpressionNode&>(*children()[0]);
		}

		void accept(IVisitor& visitor) override
		{
			visitor.visit(*this);
		}
	};

	// paren-expression:
	//     '(' expression ')'
	class ParenExpressionNode
		: public ExpressionNode
	{
	public:
		explicit ParenExpressionNode(std::size_t line, const std::shared_ptr<ITypeInfo>& typeInfo, std::unique_ptr<ExpressionNode>&& expression)
			: ExpressionNode(line, typeInfo)
		{
			assert(expression);

			addChild(std::move(expression));
		}

		[[nodiscard]]
		ExpressionNode& expression() noexcept
		{
			return static_cast<ExpressionNode&>(*children()[0]);
		}

		void accept(IVisitor& visitor) override
		{
			visitor.visit(*this);
		}
	};

	// integer-expression:
	//     integer-literal
	class IntegerExpressionNode
		: public ExpressionNode
	{
	public:
		explicit IntegerExpressionNode(std::size_t line, const std::shared_ptr<ITypeInfo>& typeInfo, Word value)
			: ExpressionNode(line, typeInfo)
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

	// integer-type:
	//     'int'
	class IntegerTypeNode
		: public TypeNode
	{
	public:
		using TypeNode::TypeNode;

		void accept(IVisitor& visitor) override
		{
			visitor.visit(*this);
		}
	};

	class Printer
		: private IVisitor
	{
	public:
		explicit Printer(std::ostream& stream)
			: Printer(stream, 0) {}

		// Uncopyable, unmovable.
		Printer(const Printer&) =delete;
		Printer(Printer&&) =delete;

		Printer& operator=(const Printer&) =delete;
		Printer& operator=(Printer&&) =delete;

		~Printer() =default;

		void print(Node& node)
		{
			node.accept(*this);
		}

	private:
		explicit Printer(std::ostream& stream, std::size_t depth)
			: m_stream(stream), m_depth(depth) {}

		template <typename... Args>
		void print(const std::string& format, Args&&... args)
		{
			for (std::size_t i = 0; i < m_depth; i++)
			{
				m_stream << u8"    ";
			}

			m_stream << (boost::format(format) % ... % std::forward<Args>(args)) << std::endl;
		}

		void visitChildren(Node& node)
		{
			Printer printer {m_stream, m_depth + 1};

			for (const auto& child : node.children())
			{
				if (child) child->accept(printer);
			}
		}

		void visit(RootNode& node)
		{
			print(u8"RootNode %1%", node.name());
			visitChildren(node);
		}

		void visit(FunctionDeclarationNode& node)
		{
			print(u8"FunctionDeclarationNode <%1%> %2% (#%3$04X)", node.typeInfo()->name(), node.name(), node.symbol()->address());
			visitChildren(node);
		}

		void visit(FunctionDefinitionNode& node)
		{
			print(u8"FunctionDefinitionNode <%1%> %2% (#%3$04X)", node.typeInfo()->name(), node.name(), node.symbol()->address());
			visitChildren(node);
		}

		void visit(VariableDeclarationNode& node)
		{
			print(u8"VariableDeclarationNode <%1%> %2% (#%3$04X)", node.typeInfo()->name(), node.name(), node.symbol()->address());
			visitChildren(node);
		}

		void visit(EmptyStatementNode& node)
		{
			print(u8"EmptyStatementNode");
			visitChildren(node);
		}

		void visit(CompoundStatementNode& node)
		{
			print(u8"CompoundStatementNode");
			visitChildren(node);
		}

		void visit(IfStatementNode& node)
		{
			print(u8"IfStatementNode");
			visitChildren(node);
		}

		void visit(ExpressionStatementNode& node)
		{
			print(u8"ExpressionStatementNode");
			visitChildren(node);
		}

		void visit(ParenExpressionNode& node)
		{
			print(u8"ParenExpressionNode <%1%>", node.typeInfo()->name());
			visitChildren(node);
		}

		void visit(IntegerExpressionNode& node)
		{
			print(u8"IntegerExpressionNode <%1%> %2%", node.typeInfo()->name(), node.value());
			visitChildren(node);
		}

		void visit(IntegerTypeNode& node)
		{
			print(u8"IntegerTypeNode <%1%>", node.typeInfo()->name());
			visitChildren(node);
		}

		std::ostream& m_stream;
		std::size_t m_depth;
	};
}
