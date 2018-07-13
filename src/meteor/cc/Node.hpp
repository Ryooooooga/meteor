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

#include <boost/format.hpp>

#include "../util/Passkey.hpp"

namespace meteor::cc
{
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
	};

	class DeclaratorNode
		: public Node
	{
	public:
		using Node::Node;
	};

	class ExpressionNode
		: public Node
	{
	public:
		using Node::Node;
	};

	class TypeNode
		: public Node
	{
	public:
		using Node::Node;
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
		{
		}

		[[nodiscard]]
		std::string_view filename() const noexcept
		{
			return m_filename;
		}

		void addChild(std::unique_ptr<DeclarationNode>&& node)
		{
			assert(node);

			Node::addChild(std::move(node));
		}

		void accept(IVisitor& visitor) override
		{
			visitor.visit(*this);
		}

	private:
		std::string m_filename;
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
		using StatementNode::StatementNode;

		void addChild(std::unique_ptr<StatementNode>&& node)
		{
			assert(node);

			Node::addChild(std::move(node));
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
		{
			assert(typeSpecifier);
			assert(declarator);
			assert(body);

			addChild(std::move(typeSpecifier));
			addChild(std::move(declarator));
			addChild(std::move(body));
		}

		void accept(IVisitor& visitor) override
		{
			visitor.visit(*this);
		}
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
		{
		}

		[[nodiscard]]
		std::string_view name() const noexcept
		{
			return m_name;
		}

		void accept(IVisitor& visitor) override
		{
			visitor.visit(*this);
		}

	private:
		std::string m_name;
	};

	// parameter-list:
	//     '(' 'void' ')'
	//     '(' parameter-declaration {',' parameter-declaration}* ')'
	class ParameterListNode
		: public Node
	{
	public:
		using Node::Node;

		void addChild(std::unique_ptr<DeclarationNode>&& node)
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

		void accept(IVisitor& visitor) override
		{
			visitor.visit(*this);
		}
	};

	// --- expression ---

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

		void accept(IVisitor& visitor) override
		{
			visitor.visit(*this);
		}

	private:
		std::string m_name;
	};

	// --- type ---

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

	// Node printer.
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

		void visit(RootNode& node)
		{
			write(u8"RootNode %1%", node.filename());
			visitChildren(node);
		}

		void visit(ParameterListNode& node)
		{
			write(u8"ParameterListNode");
			visitChildren(node);
		}

		void visit(EmptyStatementNode& node)
		{
			write(u8"EmptyStatementNode");
			visitChildren(node);
		}

		void visit(CompoundStatementNode& node)
		{
			write(u8"CompoundStatementNode");
			visitChildren(node);
		}

		void visit(ExpressionStatementNode& node)
		{
			write(u8"ExpressionStatementNode");
			visitChildren(node);
		}

		void visit(FunctionDeclarationNode& node)
		{
			write(u8"FunctionDeclarationNode");
			visitChildren(node);
		}

		void visit(VariableDeclarationNode& node)
		{
			write(u8"VariableDeclarationNode");
			visitChildren(node);
		}

		void visit(ParameterDeclarationNode& node)
		{
			write(u8"ParameterDeclarationNode");
			visitChildren(node);
		}

		void visit(IdentifierDeclaratorNode& node)
		{
			write(u8"IdentifierDeclaratorNode `%1%'", node.name());
			visitChildren(node);
		}

		void visit(FunctionDeclaratorNode& node)
		{
			write(u8"FunctionDeclaratorNode");
			visitChildren(node);
		}

		void visit(IdentifierExpressionNode& node)
		{
			write(u8"IdentifierExpressionNode `%1%'", node.name());
			visitChildren(node);
		}

		void visit(IntegerTypeNode& node)
		{
			write(u8"IntegerTypeNode");
			visitChildren(node);
		}

		void visitChildren(Node& node)
		{
			Printer printer { m_stream, m_depth + 1 };

			for (const auto& child : node.children())
			{
				child->accept(printer);
			}
		}

		template <typename... Args>
		void write(const std::string& format, Args&&... args)
		{
			for (std::size_t i = 0; i < m_depth; i++)
			{
				m_stream << u8"    ";
			}

			m_stream << (boost::format(format) % ... % std::forward<Args>(args)) << std::endl;
		}

		std::ostream& m_stream;
		std::size_t m_depth;
	};
}
