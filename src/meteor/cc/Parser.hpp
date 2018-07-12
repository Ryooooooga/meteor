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

#include "Semantics.hpp"
#include "TokenStream.hpp"

namespace meteor::cc
{
	class Parser
	{
	public:
		explicit Parser(std::string name, std::string code)
			: m_stream(Lexer { std::move(name), std::move(code) })
			, m_sema()
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
		//     external-declaration*
		std::unique_ptr<RootNode> parseRoot()
		{
			auto node = m_sema.actOnRootBegan(name());

			// external-declaration*
			while (peekToken()->kind() != TokenKind::endOfFile)
			{
				// external-declaration
				node->addChild(parseExternalDeclaration());
			}

			m_sema.actOnRootEnded();

			return node;
		}

		// external-declaration:
		//     function-declaration
		//     variable-declaration
		std::unique_ptr<DeclarationNode> parseExternalDeclaration()
		{
			// type
			auto type = parseType();

			// identifier
			const auto name = matchToken(TokenKind::identifier);

			if (peekToken()->kind() == TokenKind::leftParen)
			{
				// function-declaration
				return parseFunctionDeclaration(std::move(type), name);
			}
			else
			{
				// variable-declaration
				return parseVariableDeclaration(std::move(type), name);
			}
		}

		// function-declaration:
		//     type identifier parameter-list ';'
		//     type identifier parameter-list compound-statement
		std::unique_ptr<DeclarationNode> parseFunctionDeclaration(std::unique_ptr<TypeNode>&& returnType, const std::shared_ptr<Token>& name)
		{
			// TODO: parameter-list
			matchToken(TokenKind::leftParen);
			matchToken(TokenKind::rightParen);

			auto declaration = m_sema.actOnFunctionDeclaration(name, std::move(returnType));

			// ';'?
			if (consumeTokenIf(TokenKind::semicolon))
			{
				return declaration;
			}

			// compound-statement
			auto body = parseCompoundStatement();

			return m_sema.actOnFunctionBodyEnded(std::move(declaration), std::move(body));
		}

		// variable-declaration:
		//     type identifier ';'
		//     type identifier '=' expression ';'
		std::unique_ptr<DeclarationNode> parseVariableDeclaration()
		{
			// type
			auto type = parseType();

			// identifier
			const auto name = matchToken(TokenKind::identifier);

			return parseVariableDeclaration(std::move(type), name);
		}

		std::unique_ptr<DeclarationNode> parseVariableDeclaration(std::unique_ptr<TypeNode>&& type, const std::shared_ptr<Token>& name)
		{
			std::unique_ptr<ExpressionNode> initializer;

			// '='
			if (consumeTokenIf(TokenKind::assign))
			{
				// expression
				initializer = parseExpression();
			}

			// ';'
			matchToken(TokenKind::semicolon);

			return m_sema.actOnVariableDeclaration(name, std::move(type), std::move(initializer));
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

				case TokenKind::leftBrace:
					// compound-statement
					return parseCompoundStatement();

				case TokenKind::keyword_if:
					// if-statement
					return parseIfStatement();

				case TokenKind::keyword_int:
					// variable-declaration
					return parseVariableDeclaration();

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

			return m_sema.actOnEmptyStatement(token);
		}

		// compound-statement:
		//     '{' statement* '}'
		std::unique_ptr<StatementNode> parseCompoundStatement()
		{
			// '{'
			auto node = m_sema.actOnCompoundStatementBegan(matchToken(TokenKind::leftBrace));

			// statement*
			while (peekToken()->kind() != TokenKind::rightBrace)
			{
				// statement
				node->addChild(parseStatement());
			}

			// '}'
			matchToken(TokenKind::rightBrace);

			return node;
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
				return m_sema.actOnIfStatementEnded(token, std::move(condition), std::move(then), nullptr);
			}

			// statement
			auto otherwise = parseStatement();

			return m_sema.actOnIfStatementEnded(token, std::move(condition), std::move(then), std::move(otherwise));
		}

		// expression-statement:
		//     expression ';'
		std::unique_ptr<StatementNode> parseExpressionStatement()
		{
			// expression
			auto expression = parseExpression();

			// ';'
			matchToken(TokenKind::semicolon);

			return m_sema.actOnExpressionStatement(std::move(expression));
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

			return m_sema.actOnParenExpression(token, std::move(expression));
		}

		// integer-expression:
		//     integer-literal
		std::unique_ptr<ExpressionNode> parseIntegerExpression()
		{
			// integer-literal
			const auto token = matchToken(TokenKind::integerLiteral);

			return m_sema.actOnIntegerExpression(token);
		}

		// type:
		//     primary-type
		std::unique_ptr<TypeNode> parseType()
		{
			// primary-type
			return parsePrimaryType();
		}

		// primary-type:
		//     integer-type
		std::unique_ptr<TypeNode> parsePrimaryType()
		{
			switch (peekToken()->kind())
			{
				case TokenKind::keyword_int:
					// integer-type
					return parseIntegerType();

				default:
					// error
					reportError(peekToken()->line(), boost::format(u8"unexpected token `%1%', expected type.") % peekToken()->text());
			}
		}

		// integer-type:
		//     'int'
		std::unique_ptr<TypeNode> parseIntegerType()
		{
			// 'int'
			const auto token = matchToken(TokenKind::keyword_int);

			return m_sema.actOnIntegerType(token);
		}

		TokenStream m_stream;
		Semantics m_sema;
	};
}
