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
#include "Symbol.hpp"
#include "TypeInfo.hpp"

namespace meteor::cc
{
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

		void visit(ArgumentListNode& node)
		{
			write(u8"ArgumentListNode");
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

		void visit(IfStatementNode& node)
		{
			write(u8"IfStatementNode");
			visitChildren(node);
		}

		void visit(WhileStatementNode& node)
		{
			write(u8"WhileStatementNode");
			visitChildren(node);
		}

		void visit(ReturnStatementNode& node)
		{
			write(u8"ReturnStatementNode");
			visitChildren(node);
		}

		void visit(ExpressionStatementNode& node)
		{
			write(u8"ExpressionStatementNode");
			visitChildren(node);
		}

		void visit(FunctionDeclarationNode& node)
		{
			if (node.symbol())
				write(u8"FunctionDeclarationNode <%1%>", node.symbol()->typeInfo()->name());
			else
				write(u8"FunctionDeclarationNode <?>");
			visitChildren(node);
		}

		void visit(VariableDeclarationNode& node)
		{
			if (node.symbol())
				write(u8"VariableDeclarationNode <%1%>", node.symbol()->typeInfo()->name());
			else
				write(u8"VariableDeclarationNode <?>");
			visitChildren(node);
		}

		void visit(ParameterDeclarationNode& node)
		{
			if (node.symbol())
				write(u8"ParameterDeclarationNode <%1%>", node.symbol()->typeInfo()->name());
			else
				write(u8"ParameterDeclarationNode <?>");
			visitChildren(node);
		}

		void visit(IdentifierDeclaratorNode& node)
		{
			if (node.symbol())
				write(u8"IdentifierDeclaratorNode `%1%' <%2%>", node.name(), node.symbol()->typeInfo()->name());
			else
				write(u8"IdentifierDeclaratorNode `%1%' <?>", node.name());
			visitChildren(node);
		}

		void visit(PointerDeclaratorNode& node)
		{
			if (node.symbol())
				write(u8"PointerDeclaratorNode <%1%>", node.symbol()->typeInfo()->name());
			else
				write(u8"PointerDeclaratorNode <?>");
			visitChildren(node);
		}

		void visit(FunctionDeclaratorNode& node)
		{
			if (node.symbol())
				write(u8"FunctionDeclaratorNode <%1%>", node.symbol()->typeInfo()->name());
			else
				write(u8"FunctionDeclaratorNode <?>");
			visitChildren(node);
		}

		void visit(AssignmentExpressionNode& node)
		{
			if (node.typeInfo())
				write(u8"AssignmentExpressionNode <%1% : %2%>", node.typeInfo()->name(), node.isLvalue() ? u8"lvalue" : u8"rvalue");
			else
				write(u8"AssignmentExpressionNode");
			visitChildren(node);
		}

		void visit(AdditionExpressionNode& node)
		{
			if (node.typeInfo())
				write(u8"AdditionExpressionNode <%1% : %2%>", node.typeInfo()->name(), node.isLvalue() ? u8"lvalue" : u8"rvalue");
			else
				write(u8"AdditionExpressionNode");
			visitChildren(node);
		}

		void visit(SubtractionExpressionNode& node)
		{
			if (node.typeInfo())
				write(u8"SubtractionExpressionNode <%1% : %2%>", node.typeInfo()->name(), node.isLvalue() ? u8"lvalue" : u8"rvalue");
			else
				write(u8"SubtractionExpressionNode");
			visitChildren(node);
		}

		void visit(PlusExpressionNode& node)
		{
			if (node.typeInfo())
				write(u8"PlusExpressionNode <%1% : %2%>", node.typeInfo()->name(), node.isLvalue() ? u8"lvalue" : u8"rvalue");
			else
				write(u8"PlusExpressionNode");
			visitChildren(node);
		}

		void visit(MinusExpressionNode& node)
		{
			if (node.typeInfo())
				write(u8"MinusExpressionNode <%1% : %2%>", node.typeInfo()->name(), node.isLvalue() ? u8"lvalue" : u8"rvalue");
			else
				write(u8"MinusExpressionNode");
			visitChildren(node);
		}

		void visit(AddressExpressionNode& node)
		{
			if (node.typeInfo())
				write(u8"AddressExpressionNode <%1% : %2%>", node.typeInfo()->name(), node.isLvalue() ? u8"lvalue" : u8"rvalue");
			else
				write(u8"AddressExpressionNode");
			visitChildren(node);
		}

		void visit(CallExpressionNode& node)
		{
			if (node.typeInfo())
				write(u8"CallExpressionNode <%1% : %2%>", node.typeInfo()->name(), node.isLvalue() ? u8"lvalue" : u8"rvalue");
			else
				write(u8"CallExpressionNode");
			visitChildren(node);
		}

		void visit(IdentifierExpressionNode& node)
		{
			if (node.typeInfo())
				write(u8"IdentifierExpressionNode `%1%' <%2% : %3%>", node.name(), node.typeInfo()->name(), node.isLvalue() ? u8"lvalue" : u8"rvalue");
			else
				write(u8"IdentifierExpressionNode `%1%'", node.name());
			visitChildren(node);
		}

		void visit(IntegerExpressionNode& node)
		{
			if (node.typeInfo())
				write(u8"IntegerExpressionNode #%1$04X <%2% : %3%>", node.value(), node.typeInfo()->name(), node.isLvalue() ? u8"lvalue" : u8"rvalue");
			else
				write(u8"IntegerExpressionNode #%1$04X", node.value());
			visitChildren(node);
		}

		void visit(IntegerTypeNode& node)
		{
			if (node.typeInfo())
				write(u8"IntegerTypeNode <%1%>", node.typeInfo()->name());
			else
				write(u8"IntegerTypeNode <?>");
			visitChildren(node);
		}

		void visitChildren(Node& node)
		{
			Printer printer { m_stream, m_depth + 1 };

			for (const auto& child : node.children())
			{
				if (child)
				{
					child->accept(printer);
				}
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
