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

			// XOR GR1, GR1
			add_XOR(Register::general1, Register::general1);
			// SVC 1
			add_SVC(1);

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

		// if-statement:
		//     'if' '(' expression ')' statement
		//     'if' '(' expression ')' statement 'else' statement
		void visit(IfStatementNode& node)
		{
			// condition
			node.condition().accept(*this);

			// OR  GR1, GR1
			add_OR(Register::general1, Register::general1);
			// JZE .else
			auto& elseLabel = add_JZE();

			// then
			node.then().accept(*this);

			if (const auto otherwise = node.otherwise())
			{
				// JUMP .endif
				auto& endifLabel = add_JUMP();

				// .else:
				elseLabel = position();

				// else
				otherwise->accept(*this);

				// .endif:
				endifLabel = position();
			}
			else
			{
				// .else:
				elseLabel = position();
			}
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
			addWord(operations::nop);
		}

		// LAD r, adr, x
		void add_LAD(Register r, Word adr, Register x = Register::general0)
		{
			addWord(operations::instruction(operations::lad, r, x));
			addWord(adr);
		}

		// OR r1, r2
		void add_OR(Register r1, Register r2)
		{
			addWord(operations::instruction(operations::or_r, r1, r2));
		}

		// XOR r1, r2
		void add_XOR(Register r1, Register r2)
		{
			addWord(operations::instruction(operations::xor_r, r1, r2));
		}

		// JZE adr, x
		void add_JZE(Word adr, Register x = Register::general0)
		{
			addWord(operations::instruction(operations::jze, Register::general0, x));
			addWord(adr);
		}

		// JZE ?, x
		[[nodiscard]]
		Word& add_JZE(Register x = Register::general0)
		{
			add_JZE(0xffff, x);
			return m_program.back();
		}

		// JUMP adr, x
		void add_JUMP(Word adr, Register x = Register::general0)
		{
			addWord(operations::instruction(operations::jump, Register::general0, x));
			addWord(adr);
		}

		// JUMP ?, x
		[[nodiscard]]
		Word& add_JUMP(Register x = Register::general0)
		{
			add_JUMP(0xffff, x);
			return m_program.back();
		}

		// PUSH adr
		void add_PUSH(Word adr)
		{
			addWord(operations::instruction(operations::push, Register::general0, Register::general0));
			addWord(adr);
		}

		// POP r
		void add_POP(Register r)
		{
			addWord(operations::instruction(operations::pop, r, Register::general0));
		}

		// SVC adr, x
		void add_SVC(Word adr, Register x = Register::general0)
		{
			addWord(operations::instruction(operations::svc, Register::general0, x));
			addWord(adr);
		}

		void addWord(Word word)
		{
			if (m_program.size() >= 0xffff)
			{
				throw std::runtime_error { "too large program." };
			}

			m_program.emplace_back(word);
		}

		[[nodiscard]]
		Word position() const noexcept
		{
			return static_cast<Word>(m_program.size());
		}

		std::vector<Word> m_program;
	};
}
