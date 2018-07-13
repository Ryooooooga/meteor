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

#include "../util/Passkey.hpp"

namespace meteor::cc
{
#define METEOR_CC_NODE(name) class name;
#include "Node.def.hpp"

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
	//     function-declaration*
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

		// TODO: Declarations only
		void addChild(std::unique_ptr</*DeclarationNode*/Node>&& node)
		{
			assert(node);

			Node::addChild(std::move(node));
		}

	private:
		std::string m_filename;
	};

	// --- statement ---

	// empty-statement
	class EmptyStatementNode
		: public StatementNode
	{
	public:
		using StatementNode::StatementNode;
	};

	// --- declaration ---

	// --- expression ---

	// --- type ---
}
