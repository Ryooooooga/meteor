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

#include "Node.hpp"
#include "TokenStream.hpp"

namespace meteor::cc
{
	class Parser
	{
	public:
		explicit Parser(std::string_view name, std::string_view code)
			: m_stream(Lexer { name, code })
		{
		}

		// Uncopyable, movable.
		Parser(const Parser&) =delete;
		Parser(Parser&&) =default;

		Parser& operator=(const Parser&) =delete;
		Parser& operator=(Parser&&) =default;

		~Parser() =default;

		[[nodiscard]]
		std::unique_ptr<RootNode> parse()
		{
			// root
			return parseRoot();
		}

	private:
		// root:
		//     declaration*
		[[nodiscard]]
		std::unique_ptr<RootNode> parseRoot()
		{
			auto node = std::make_unique<RootNode>(m_stream.name());

			// declaration*
			while (peekToken()->kind() != TokenKind::endOfFile)
			{
				// declaration
				node->addChild(parseDeclaration());
			}

			return node;
		}

		// --- statement ---

		// statement:
		//     empty-statement
		[[nodiscard]]
		std::unique_ptr<StatementNode> parseStatement()
		{
			switch (peekToken()->kind())
			{
				case TokenKind::semicolon:
					// empty-statement
					return parseEmptyStatement();

				case TokenKind::leftBrace:
					// compound-statement
					return parseCompoundStatement();

				default:
					// TODO:
					throw std::runtime_error { "not implemented" };
			}
		}

		// empty-statement:
		//     ';'
		[[nodiscard]]
		std::unique_ptr<StatementNode> parseEmptyStatement()
		{
			// ';'
			const auto token = matchToken(TokenKind::semicolon);

			return std::make_unique<EmptyStatementNode>(token->line());
		}

		// compound-statement:
		//     '{' statement* '}'
		[[nodiscard]]
		std::unique_ptr<StatementNode> parseCompoundStatement()
		{
			// '{'
			matchToken(TokenKind::leftBrace);

			throw std::runtime_error {"not implemented parseCompoundStatement"};
		}

		// --- declaration ---

		// declaration:
		//     function-declaration
		[[nodiscard]]
		std::unique_ptr<DeclarationNode> parseDeclaration()
		{
			return parseFunctionDeclaration();
		}

		// function-declaration:
		//     type declarator compound-statement
		[[nodiscard]]
		std::unique_ptr<DeclarationNode> parseFunctionDeclaration()
		{
			// type
			auto typeSpecifier = parseType();

			// declarator
			auto declarator = parseDeclarator();

			// compound-statement
			auto body = parseCompoundStatement();

			// return std::make_unique<FunctionDeclarationNode>(declarator->line(), std::move(typeSpecifier), std::move(declarator), std::move(body));
			throw std::runtime_error {"not implemented"};
		}

		// parameter-declaration:
		//     type declarator
		[[nodiscard]]
		std::unique_ptr<DeclarationNode> parseParameterDeclaration()
		{
			// type
			auto typeSpecifier = parseType();

			// declarator
			auto declarator = parseDeclarator();

			return std::make_unique<ParameterDeclarationNode>(declarator->line(), std::move(typeSpecifier), std::move(declarator));
		}

		// --- declarator ---

		// declarator:
		//     direct-declarator
		[[nodiscard]]
		std::unique_ptr<DeclaratorNode> parseDeclarator()
		{
			return parseDirectDeclarator();
		}

		// direct-declarator:
		//     primary-declarator declarator-postfix*
		[[nodiscard]]
		std::unique_ptr<DeclaratorNode> parseDirectDeclarator()
		{
			// primary-declarator
			auto declarator = parsePrimaryDeclarator();

			// declarator-postfix*
			while (true)
			{
				switch (peekToken()->kind())
				{
					case TokenKind::leftParen:
						// function-declarator
						declarator = parseFunctionDeclarator(std::move(declarator));
						return declarator; // TODO: repeat
						break;

					default:
						return declarator;
				}
			}
		}

		// primary-declarator:
		//     identifier-declarator
		[[nodiscard]]
		std::unique_ptr<DeclaratorNode> parsePrimaryDeclarator()
		{
			// identifier-declarator
			return parseIdentifierDeclarator();
		}

		// function-declarator:
		//     direct-declarator parameter-list
		[[nodiscard]]
		std::unique_ptr<DeclaratorNode> parseFunctionDeclarator(std::unique_ptr<DeclaratorNode>&& declarator)
		{
			// parameter-list
			auto parameters = parseParameterList();

			(void)declarator; throw std::runtime_error {"not implemented parseFunctionDeclarator"}; // TODO:
		}

		// parameter-list:
		//     '(' 'void' ')'
		//     '(' parameter-declaration {',' parameter-declaration}* ')'
		[[nodiscard]]
		std::unique_ptr<ParameterListNode> parseParameterList()
		{
			// '('
			const auto token = matchToken(TokenKind::leftParen);

			auto node = std::make_unique<ParameterListNode>(token->line());

			// 'void'?
			if (!consumeTokenIf(TokenKind::keyword_void))
			{
				// parameter-declaration
				node->addChild(parseParameterDeclaration());

				// {',' parameter-declaration}*
				while (consumeTokenIf(TokenKind::comma))
				{
					node->addChild(parseParameterDeclaration());
				}
			}

			// ')'
			matchToken(TokenKind::rightParen);

			return node;
		}

		// identifier-declarator
		//     identifier
		[[nodiscard]]
		std::unique_ptr<DeclaratorNode> parseIdentifierDeclarator()
		{
			// identifier
			const auto token = matchToken(TokenKind::identifier);

			return std::make_unique<IdentifierDeclaratorNode>(token->line(), token->text());
		}

		// --- type ---

		// type:
		//     integer-type
		[[nodiscard]]
		std::unique_ptr<TypeNode> parseType()
		{
			switch (const auto token = peekToken(); token->kind())
			{
				case TokenKind::keyword_int:
					// integer-type
					return parseIntegerType();

				default:
					// error
					reportError(boost::format(u8"unexpected token `%1%', expected type.") % token->text());
			}
		}

		// integer-type:
		//     'int'
		[[nodiscard]]
		std::unique_ptr<TypeNode> parseIntegerType()
		{
			// 'int'
			const auto token = matchToken(TokenKind::keyword_int);

			return std::make_unique<IntegerTypeNode>(token->line());
		}

		[[nodiscard]]
		std::shared_ptr<Token> peekToken()
		{
			return m_stream.peek(0);
		}

		std::shared_ptr<Token> consumeToken()
		{
			return m_stream.consume();
		}

		[[nodiscard]]
		std::shared_ptr<Token> consumeTokenIf(TokenKind acceptable)
		{
			if (peekToken()->kind() == acceptable)
			{
				return consumeToken();
			}

			return nullptr;
		}

		std::shared_ptr<Token> matchToken(TokenKind expected)
		{
			if (const auto token = peekToken(); token->kind() != expected)
			{
				reportError(boost::format(u8"unexpected token `%1%', expected %2%.") % token->text() % expected);
			}

			return consumeToken();
		}

		template <typename Message>
		[[noreturn]]
		void reportError(Message&& message)
		{
			throw std::runtime_error { (boost::format(u8"%1%(%2%): %3%") % m_stream.name() % peekToken()->line() % std::forward<Message>(message)).str() };
		}

		TokenStream m_stream;
	};
}
