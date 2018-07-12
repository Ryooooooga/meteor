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

#include "../Operation.hpp"
#include "Node.hpp"

namespace meteor::cc
{
	class Compiler
		: private IVisitor
	{
	public:
		explicit Compiler()
			: m_program()
		{
			m_program.reserve(256);
		}

		// Uncopyable, movable.
		Compiler(const Compiler&) =delete;
		Compiler(Compiler&&) =default;

		Compiler& operator=(const Compiler&) =delete;
		Compiler& operator=(Compiler&&) =default;

		~Compiler() =default;

		[[nodiscard]]
		std::vector<Word> compile(RootNode& node)
		{
			// root
			visit(node);

			return m_program;
		}

	private:
		// root:
		//     statement*
		void visit(RootNode& node)
		{
			// statement*
			for (const auto& statement : node.children())
			{
				// statement
				statement->accept(*this);
			}
		}

		// empty-statement:
		//     ';'
		void visit([[maybe_unused]] EmptyStatementNode& node)
		{
			// NOP
			add_NOP();
		}

		// expression-statement:
		//     expression ';'
		void visit(ExpressionStatementNode& node)
		{
			// expression
			node.expression().accept(*this);
		}

		// integer-expression:
		//     integer-literal
		void visit(IntegerExpressionNode& node)
		{
			// LAD GR1, x
			add_LAD(Register::general1, node.value());
		}

		// NOP
		void add_NOP()
		{
			m_program.emplace_back(operations::nop);
		}

		// LAD r, adr, x
		void add_LAD(Register r, Word adr, Register x = Register::general0)
		{
			m_program.emplace_back(operations::instruction(operations::lad, r, x));
			m_program.emplace_back(adr);
		}

		// PUSH adr
		void add_PUSH(Word adr)
		{
			m_program.emplace_back(operations::instruction(operations::push, Register::general0, Register::general0));
			m_program.emplace_back(adr);
		}

		// POP r
		void add_POP(Register r)
		{
			m_program.emplace_back(operations::instruction(operations::pop, r, Register::general0));
		}

		std::vector<Word> m_program;
	};
}
