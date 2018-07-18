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
#include "TypeInfo.hpp"

namespace meteor::cc
{
	class Compiler
		: private IVisitor
	{
	public:
		explicit Compiler() =default;

		// Uncopyable, movable.
		Compiler(const Compiler&) =delete;
		Compiler(Compiler&&) =default;

		Compiler& operator=(const Compiler&) =delete;
		Compiler& operator=(Compiler&&) =default;

		~Compiler() =default;

		[[nodiscard]]
		std::vector<Word> compile(RootNode& node)
		{
			visit(node);

			return m_program;
		}

	private:
		constexpr static Register framePointer = Register::general7;

		// root:
		//     external-declaration*
		// external-declaration:
		//     function-declaration
		//     variable-declaration
		void visit(RootNode& node)
		{
			// Clear GR0.
			// LAD GR0, #0000
			add_LAD(Register::general0, 0x0000);

			// Set frame pointer.
			const auto fp = add_LAD(framePointer);

			// Call main.
			// CALL ?
			const auto mainAddress = add_CALL();

			// Exit with return value.
			// SVC #0001
			add_SVC(0x0001);

			// Compile functions.
			// external-declaration*
			m_isLocal = false;
			m_main = nullptr;

			for (const auto& child : node.children())
			{
				child->accept(*this);
			}

			if (m_main == nullptr)
			{
				throw std::runtime_error(std::string {node.filename()} + u8": function `main' is not defined.");
			}

			m_program[fp] = position();
			m_program[mainAddress] = m_main->address();
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
				// assignment-expression
				child->accept(*this);

				// ST GR1, n, FP
				add_ST(Register::general1, m_locals, framePointer);

				m_locals++;
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
			// Save the local address.
			const Word localsSaved = m_locals;

			// statement*
			for (const auto& child : node.children())
			{
				child->accept(*this);
			}

			// Restore the local address.
			m_locals = localsSaved;
		}

		// if-statement:
		//     'if' paren-expression compound-statement
		//     'if' paren-expression compound-statement 'else' compound-statement
		void visit(IfStatementNode& node)
		{
			// condition
			m_lvalue = false;
			node.condition().accept(*this);

			// CPA GR1, #0000
			add_CPA(Register::general1, 0x0000);

			if (const auto otherwise = node.otherwise())
			{
				// JZE .else
				const auto elseLabel = add_JZE();

				// then
				node.then().accept(*this);

				// JUMP .endif
				const auto endIfLabel = add_JUMP();

				// .else
				m_program[elseLabel] = position();

				// else
				otherwise->accept(*this);

				// .endif
				m_program[endIfLabel] = position();
			}
			else
			{
				// JZE .endif
				const auto endIfLabel = add_JZE();

				// then
				node.then().accept(*this);

				// .endif
				m_program[endIfLabel] = position();
			}
		}

		// while-statement:
		//     'while' paren-expression compound-statement
		void visit(WhileStatementNode& node)
		{
			// .startwhile
			const Word startPos = position();

			// condition
			m_lvalue = false;
			node.condition().accept(*this);

			// CPA GR1, #0000
			add_CPA(Register::general1, 0x0000);

			// JZE .endwhile
			const Word endWhileLabel = add_JZE();

			// body
			node.body().accept(*this);

			// JUMP .startwhile
			add_JUMP(startPos);

			// .endwhile
			m_program[endWhileLabel] = position();
		}

		// return-statement:
		//     'return' expression? ';'
		void visit(ReturnStatementNode& node)
		{
			// expression
			if (const auto expression = node.expression())
			{
				expression->accept(*this);
			}

			// RET
			add_RET();
		}

		// expression-statement:
		//     expression ';'
		void visit(ExpressionStatementNode& node)
		{
			// expression
			m_lvalue = false;
			node.expression().accept(*this);
		}

		// function-declaration:
		//     type declarator compound-statement
		void visit(FunctionDeclarationNode& node)
		{
			// Save the function address.
			node.symbol()->address({}, true, position());

			if (node.symbol()->name() == u8"main")
			{
				m_main = node.symbol();
			}

			// declarator
			m_isLocal = true;
			m_parameters = true;
			m_locals = 0x0000;

			node.declarator().accept(*this);

			// compound-statement
			node.body().accept(*this);

			// RET
			add_RET();
		}

		// variable-declaration:
		//     type declarator
		void visit(VariableDeclarationNode& node)
		{
			if (m_isLocal)
			{
				// Save the local variable address.
				node.symbol()->address({}, false, m_locals);

				m_locals += node.symbol()->typeInfo()->size();
			}
			else
			{
				// Save the global variable address.
				node.symbol()->address({}, true, position());

				for (Word i = 0; i < node.symbol()->typeInfo()->size(); i++)
				{
					addWord(0x0000);
				}
			}
		}

		// parameter-declaration:
		//     type declarator
		void visit(ParameterDeclarationNode& node)
		{
			// Save the local parameter address.
			node.symbol()->address({}, false, m_locals);

			m_locals += node.symbol()->typeInfo()->size();
		}

		// identifier-declarator:
		//     identifier
		void visit([[maybe_unused]] IdentifierDeclaratorNode& node)
		{
		}

		// pointer-declarator:
		//     '*' direct-declarator
		void visit(PointerDeclaratorNode& node)
		{
			// direct-declarator
			node.declarator().accept(*this);
		}

		// function-declarator:
		//     direct-declarator parameter-list
		void visit(FunctionDeclaratorNode& node)
		{
			// direct-declarator
			node.declarator().accept(*this);

			if (m_parameters)
			{
				// parameter-list
				node.parameters().accept(*this);

				m_parameters = false;
			}
		}

		// assignment-expression:
		//     unary-expression '=' assignment-expression
		void visit(AssignmentExpressionNode& node)
		{
			// right-hand-side
			node.right().accept(*this);

			// PUSH #0000, GR1
			add_PUSH(0x0000, Register::general1);

			// left-hand-side
			const auto lvalueSaved = std::exchange(m_lvalue, true);
			node.left().accept(*this);
			m_lvalue = lvalueSaved;

			// POP GR2
			add_POP(Register::general2);
			// ST GR2, #0000, GR1
			add_ST(Register::general2, 0x0000, Register::general1);
		}

		// addition-expression:
		//     additive-expression '+' mutiplicative-expression
		void visit(AdditionExpressionNode& node)
		{
			// right-hand-side
			node.right().accept(*this);

			// PUSH #0000, GR1
			add_PUSH(0x0000, Register::general1);

			// left-hand-side
			node.left().accept(*this);

			// POP GR2
			add_POP(Register::general2);
			// ADDA GR1, GR2
			add_ADDA(Register::general1, Register::general2);
		}

		// subtraction-expression:
		//     additive-expression '-' mutiplicative-expression
		void visit(SubtractionExpressionNode& node)
		{
			// right-hand-side
			node.right().accept(*this);

			// PUSH #0000, GR1
			add_PUSH(0x0000, Register::general1);

			// left-hand-side
			node.left().accept(*this);

			// POP GR2
			add_POP(Register::general2);
			// SUBA GR1, GR2
			add_SUBA(Register::general1, Register::general2);
		}

		// plus-expression:
		//     '+' unary-expression
		void visit(PlusExpressionNode& node)
		{
			// operand
			node.operand().accept(*this);
		}

		// minus-expression:
		//     '-' unary-expression
		void visit(MinusExpressionNode& node)
		{
			// operand
			node.operand().accept(*this);

			// LD GR2, GR1
			add_LD(Register::general2, Register::general1);
			// LAD GR1, #0000
			add_LAD(Register::general1, 0x0000);
			// SUBA GR1, GR2
			add_SUBA(Register::general1, Register::general2);
		}

		// address-expression:
		//     '&' unary-expression
		void visit(AddressExpressionNode& node)
		{
			// operand
			const auto lvalueSaved = std::exchange(m_lvalue, true);
			node.operand().accept(*this);
			m_lvalue = lvalueSaved;
		}

		// dereference-expression:
		//     '*' unary-expression
		void visit(DereferenceExpressionNode& node)
		{
			// operand
			const auto lvalueSaved = std::exchange(m_lvalue, false);
			node.operand().accept(*this);
			m_lvalue = lvalueSaved;

			if (!m_lvalue)
			{
				// rvalue
				add_LD(Register::general1, 0x0000, Register::general1);
			}
		}

		// call-expression:
		//     postfix-expression argument-list
		void visit(CallExpressionNode& node)
		{
			// argument-list
			const auto lvalueSaved = std::exchange(m_lvalue, false);
			const auto localsSaved = m_locals;
			node.arguments().accept(*this);

			// callee
			m_lvalue = true;
			node.callee().accept(*this);

			m_lvalue = lvalueSaved;
			m_locals = localsSaved;

			// ADDL FP, locals
			add_ADDL(framePointer, m_locals);
			// CALL
			add_CALL(0x0000, Register::general1);
			// SUBL FP, locals
			add_SUBL(framePointer, m_locals);
		}

		// identifier-expression:
		//     identifier
		void visit(IdentifierExpressionNode& node)
		{
			if (m_lvalue)
			{
				// Load addresses.
				if (node.symbol()->isGlobal())
				{
					// Load the global variable address.
					// LAD GR1, adr
					add_LAD(Register::general1, node.symbol()->address());
				}
				else
				{
					// Load the local variable address.
					// LAD GR1, adr, FP
					add_LAD(Register::general1, node.symbol()->address(), framePointer);
				}
			}
			else
			{
				// Load values.
				if (node.symbol()->isGlobal())
				{
					// Load the global variable.
					// LD GR1, adr
					add_LD(Register::general1, node.symbol()->address());
				}
				else
				{
					// Load the local variable.
					// LD GR1, adr, FP
					add_LD(Register::general1, node.symbol()->address(), framePointer);
				}
			}
		}

		// integer-expression:
		//     integer-literal
		void visit(IntegerExpressionNode& node)
		{
			// LAD GR1, value
			add_LAD(Register::general1, node.value());
		}

		// integer-type:
		//     'int'
		void visit([[maybe_unused]] IntegerTypeNode& node)
		{
		}

		[[nodiscard]]
		Word position() const noexcept
		{
			return static_cast<Word>(m_program.size());
		}

		void addWord(Word word)
		{
			assert(m_program.size() <= 0xffff);

			m_program.emplace_back(word);
		}

		// LD r1, r2
		void add_LD(Register r1, Register r2)
		{
			addWord(operations::instruction(operations::ld_r, r1, r2));
		}

		// LD r, adr, x
		void add_LD(Register r, Word adr, Register x = Register::general0)
		{
			addWord(operations::instruction(operations::ld_adr, r, x));
			addWord(adr);
		}

		// LAD r, adr, x
		void add_LAD(Register r, Word adr, Register x = Register::general0)
		{
			addWord(operations::instruction(operations::lad, r, x));
			addWord(adr);
		}

		// LAD r, ?, x
		[[nodiscard]]
		Word add_LAD(Register r, Register x = Register::general0)
		{
			add_LAD(r, 0xffff, x);

			return position() - 1;
		}

		// ST r, adr, x
		void add_ST(Register r, Word adr, Register x = Register::general0)
		{
			addWord(operations::instruction(operations::st, r, x));
			addWord(adr);
		}

		// ADDA r1, r2
		void add_ADDA(Register r1, Register r2)
		{
			addWord(operations::instruction(operations::adda_r, r1, r2));
		}

		// SUBA r1, r2
		void add_SUBA(Register r1, Register r2)
		{
			addWord(operations::instruction(operations::suba_r, r1, r2));
		}

		// ADDL r, adr, x
		void add_ADDL(Register r, Word adr, Register x = Register::general0)
		{
			addWord(operations::instruction(operations::addl_adr, r, x));
			addWord(adr);
		}

		// SUBL r, adr, x
		void add_SUBL(Register r, Word adr, Register x = Register::general0)
		{
			addWord(operations::instruction(operations::subl_adr, r, x));
			addWord(adr);
		}

		// CPA r, adr, x
		void add_CPA(Register r, Word adr, Register x = Register::general0)
		{
			addWord(operations::instruction(operations::cpa_adr, r, x));
			addWord(adr);
		}

		// JZE adr, x
		void add_JZE(Word adr, Register x = Register::general0)
		{
			addWord(operations::instruction(operations::jze, Register::general0, x));
			addWord(adr);
		}

		// JZE ?, x
		[[nodiscard]]
		Word add_JZE(Register x = Register::general0)
		{
			add_JZE(0xffff, x);

			return position() - 1;
		}

		// JUMP adr, x
		void add_JUMP(Word adr, Register x = Register::general0)
		{
			addWord(operations::instruction(operations::jump, Register::general0, x));
			addWord(adr);
		}

		// JUMP ?, x
		[[nodiscard]]
		Word add_JUMP(Register x = Register::general0)
		{
			add_JUMP(0xffff, x);

			return position() - 1;
		}

		// PUSH adr, x
		void add_PUSH(Word adr, Register x = Register::general0)
		{
			addWord(operations::instruction(operations::push, Register::general0, x));
			addWord(adr);
		}

		// POP r
		void add_POP(Register r)
		{
			addWord(operations::instruction(operations::pop, r, Register::general0));
		}

		// CALL adr, x
		void add_CALL(Word adr, Register x = Register::general0)
		{
			addWord(operations::instruction(operations::call, Register::general0, x));
			addWord(adr);
		}

		// CALL ?, x
		[[nodiscard]]
		Word add_CALL(Register x = Register::general0)
		{
			add_CALL(0xffff, x);

			return position() - 1;
		}

		// RET
		void add_RET()
		{
			addWord(operations::ret);
		}

		// SVC adr, x
		void add_SVC(Word adr, Register x = Register::general0)
		{
			addWord(operations::instruction(operations::svc, Register::general0, x));
			addWord(adr);
		}

		std::vector<Word> m_program;
		bool m_isLocal;
		bool m_parameters;
		bool m_lvalue;
		Word m_locals;
		std::shared_ptr<Symbol> m_main;
	};
}
