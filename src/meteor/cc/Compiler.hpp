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

			// TODO: call main.

			// Exit with status `0`.
			// LAD GR1, #0000
			add_LAD(Register::general1, 0x0000);
			// SVC #0001
			add_SVC(0x0001);

			// Compile functions.
			// external-declaration*
			m_isLocal = false;

			for (const auto& child : node.children())
			{
				child->accept(*this);
			}
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
			// Save the function address.
			node.symbol()->address({}, true, position());

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

		// identifier-expression:
		//     identifier
		void visit(IdentifierExpressionNode& node)
		{
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
		Word m_locals;
	};
}
