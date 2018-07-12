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

#include "../Type.hpp"

namespace meteor::cc
{
	class Node;
	class StatementNode;
	class ExpressionNode;

	class RootNode;
	class EmptyStatementNode;
	class ExpressionStatementNode;
	class IntegerExpressionNode;

	class IVisitor
	{
	public:
		virtual void visit(RootNode& node) =0;
		virtual void visit(EmptyStatementNode& node) =0;
		virtual void visit(ExpressionStatementNode& node) =0;
		virtual void visit(IntegerExpressionNode& node) =0;
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
			assert(node);

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

	class ExpressionNode
		: public Node
	{
	public:
		using Node::Node;
	};

	// root
	class RootNode
		: public Node
	{
	public:
		explicit RootNode(std::string name)
			: Node(0)
			, m_name(std::move(name))
		{
		}

		[[nodiscard]]
		std::string_view name() const noexcept
		{
			return m_name;
		}

		using Node::addChild;

		void accept(IVisitor& visitor) override
		{
			visitor.visit(*this);
		}

	private:
		std::string m_name;
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

	// expression-statement:
	//     expression ';'
	class ExpressionStatementNode
		: public StatementNode
	{
	public:
		explicit ExpressionStatementNode(std::size_t line, std::unique_ptr<ExpressionNode>&& expression)
			: StatementNode(line)
		{
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
				child->accept(printer);
			}
		}

		void visit(RootNode& node)
		{
			print(u8"RootNode %1%", node.name());
			visitChildren(node);
		}

		void visit(EmptyStatementNode& node)
		{
			print(u8"EmptyStatementNode");
			visitChildren(node);
		}

		void visit(ExpressionStatementNode& node)
		{
			print(u8"ExpressionStatementNode");
			visitChildren(node);
		}

		void visit(IntegerExpressionNode& node)
		{
			print(u8"IntegerExpressionNode %1%", node.value());
			visitChildren(node);
		}

		std::ostream& m_stream;
		std::size_t m_depth;
	};
}
