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
		explicit Parser(std::string name, std::string code)
			: m_stream(Lexer { std::move(name), std::move(code) })
		{
		}

		// Uncopyable, movable.
		Parser(const Parser&) =delete;
		Parser(Parser&&) =default;

		Parser& operator=(const Parser&) =delete;
		Parser& operator=(Parser&&) =default;

		~Parser() =default;

		[[nodiscard]]
		std::string_view name() const noexcept
		{
			return m_stream.name();
		}

		[[nodiscard]]
		std::string_view code() const noexcept
		{
			return m_stream.code();
		}

		[[nodiscard]]
		std::unique_ptr<RootNode> parse()
		{
			return parseRoot();
		}

	private:
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
		std::shared_ptr<Token> consumeTokenIf(TokenKind kind)
		{
			if (peekToken()->kind() == kind)
			{
				return consumeToken();
			}

			return nullptr;
		}

		std::shared_ptr<Token> matchToken(TokenKind kind)
		{
			if (auto t = consumeTokenIf(kind)) return t;

			reportError(peekToken()->line(), boost::format(u8"unexpected token `%1%', expected `%2%'.") % peekToken()->text() % kind);
		}

		template <typename Message>
		[[noreturn]]
		void reportError(size_t line, Message&& message)
		{
			throw std::runtime_error { (boost::format(u8"%1%(%2%): %3%") % name() % line % std::forward<Message>(message)).str() };
		}

		// root:
		//     statement*
		std::unique_ptr<RootNode> parseRoot()
		{
			auto node = std::make_unique<RootNode>(std::string(name()));

			// statement*
			while (peekToken()->kind() != TokenKind::endOfFile)
			{
				// statement
				node->addChild(parseStatement());
			}

			return node;
		}

		// statement:
		//     empty-statement
		//     if-statement
		//     expression-statement
		std::unique_ptr<StatementNode> parseStatement()
		{
			switch (peekToken()->kind())
			{
				case TokenKind::semicolon:
					// empty-statement
					return parseEmptyStatement();

				case TokenKind::keyword_if:
					// if-statement
					return parseIfStatement();

				default:
					// expression-statement
					return parseExpressionStatement();
			}
		}

		// empty-statement:
		//     ';'
		std::unique_ptr<StatementNode> parseEmptyStatement()
		{
			// ';'
			const auto token = matchToken(TokenKind::semicolon);

			return std::make_unique<EmptyStatementNode>(token->line());
		}

		// if-statement:
		//     'if' '(' expression ')' statement
		//     'if' '(' expression ')' statement 'else' statement
		std::unique_ptr<StatementNode> parseIfStatement()
		{
			// 'if'
			const auto token = matchToken(TokenKind::keyword_if);

			// '('
			matchToken(TokenKind::leftParen);

			// expression
			auto condition = parseExpression();

			// ')'
			matchToken(TokenKind::rightParen);

			// statement
			auto then = parseStatement();

			// 'else'
			if (!consumeTokenIf(TokenKind::keyword_else))
			{
				return std::make_unique<IfStatementNode>(token->line(), std::move(condition), std::move(then), nullptr);
			}

			// statement
			auto otherwise = parseStatement();

			return std::make_unique<IfStatementNode>(token->line(), std::move(condition), std::move(then), std::move(otherwise));
		}

		// expression-statement:
		//     expression ';'
		std::unique_ptr<StatementNode> parseExpressionStatement()
		{
			// expression
			auto expression = parseExpression();

			// ';'
			matchToken(TokenKind::semicolon);

			return std::make_unique<ExpressionStatementNode>(expression->line(), std::move(expression));
		}

		// expression:
		//     primary-expression
		std::unique_ptr<ExpressionNode> parseExpression()
		{
			// primary-expression
			return parsePrimaryExpression();
		}

		// primary-expression:
		//     paren-expression
		//     integer-expression
		std::unique_ptr<ExpressionNode> parsePrimaryExpression()
		{
			switch (peekToken()->kind())
			{
				case TokenKind::leftParen:
					// paren-expression
					return parseParenExpression();

				case TokenKind::integerLiteral:
					// integer-expression
					return parseIntegerExpression();

				default:
					// error
					reportError(peekToken()->line(), boost::format(u8"unexpected token `%1%', expected expression.") % peekToken()->text());
			}
		}

		// paren-expression:
		//     '(' expression ')'
		std::unique_ptr<ExpressionNode> parseParenExpression()
		{
			// '('
			const auto token = matchToken(TokenKind::leftParen);

			// expression
			auto expression = parseExpression();

			// ')'
			matchToken(TokenKind::rightParen);

			return std::make_unique<ParenExpressionNode>(token->line(), std::move(expression));
		}

		// integer-expression:
		//     integer-literal
		std::unique_ptr<ExpressionNode> parseIntegerExpression()
		{
			// integer-literal
			const auto token = matchToken(TokenKind::integerLiteral);

			return std::make_unique<IntegerExpressionNode>(token->line(), token->integer());
		}

		TokenStream m_stream;
	};
}
