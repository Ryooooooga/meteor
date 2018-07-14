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
		//     external-declaration*
		[[nodiscard]]
		std::unique_ptr<RootNode> parseRoot()
		{
			auto node = std::make_unique<RootNode>(m_stream.name());

			// declaration*
			while (peekToken()->kind() != TokenKind::endOfFile)
			{
				// external-declaration
				node->addChild({}, parseExternalDeclaration());
			}

			return node;
		}

		// --- statement ---

		// statement:
		//     empty-statement
		//     compound-statement
		//     if-statement
		//     variable-statement
		//     expression-statement
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

				case TokenKind::keyword_if:
					// if-statement
					return parseIfStatement();

				case TokenKind::keyword_int:
					// variable-declaration
					return parseDeclaration(false);

				default:
					// expression-statement
					return parseExpressionStatement();
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
			const auto token = matchToken(TokenKind::leftBrace);

			auto node = std::make_unique<CompoundStatementNode>(token->line());

			// statement*
			while (peekToken()->kind() != TokenKind::rightBrace)
			{
				// statement
				node->addChild({}, parseStatement());
			}

			// '}'
			matchToken(TokenKind::rightBrace);

			return node;
		}

		// if-statement:
		//     'if' paren-expression compound-statement
		//     'if' paren-expression compound-statement 'else' compound-statement
		[[nodiscard]]
		std::unique_ptr<StatementNode> parseIfStatement()
		{
			// 'if'
			const auto token = matchToken(TokenKind::keyword_if);

			// paren-expression
			auto condition = parseParenExpression();

			// compound-statement
			auto then = parseCompoundStatement();

			// 'else'?
			if (!consumeTokenIf(TokenKind::keyword_else))
			{
				return std::make_unique<IfStatementNode>(token->line(), std::move(condition), std::move(then), nullptr);
			}

			// compound-statement
			auto otherwise = parseCompoundStatement();

			return std::make_unique<IfStatementNode>(token->line(), std::move(condition), std::move(then), std::move(otherwise));
		}

		// expression-statement:
		//     expression ';'
		[[nodiscard]]
		std::unique_ptr<StatementNode> parseExpressionStatement()
		{
			// expression
			auto expression = parseExpression();

			// ';'
			matchToken(TokenKind::semicolon);

			return std::make_unique<ExpressionStatementNode>(expression->line(), std::move(expression));
		}

		// --- declaration ---

		// external-declaration:
		//     function-declaration
		//     variable-declaration
		[[nodiscard]]
		std::unique_ptr<DeclarationNode> parseExternalDeclaration()
		{
			return parseDeclaration(true);
		}

		// declaration:
		//     function-declaration
		//     variable-declaration
		[[nodiscard]]
		std::unique_ptr<DeclarationNode> parseDeclaration(bool acceptFunction)
		{
			// type
			auto typeSpecifier = parseType();

			// declarator
			auto declarator = parseDeclarator();

			if (acceptFunction && peekToken()->kind() == TokenKind::leftBrace)
			{
				// function-declaration
				return parseFunctionDeclaration(std::move(typeSpecifier), std::move(declarator));
			}
			else
			{
				// variable-declaration
				return parseVariableDeclaration(std::move(typeSpecifier), std::move(declarator));
			}
		}

		// function-declaration:
		//     type declarator compound-statement
		[[nodiscard]]
		std::unique_ptr<DeclarationNode> parseFunctionDeclaration(std::unique_ptr<TypeNode>&& typeSpecifier, std::unique_ptr<DeclaratorNode>&& declarator)
		{
			// compound-statement
			auto body = parseCompoundStatement();

			return std::make_unique<FunctionDeclarationNode>(declarator->line(), std::move(typeSpecifier), std::move(declarator), std::move(body));
		}

		// variable-declaration:
		//     type declarator ';'
		[[nodiscard]]
		std::unique_ptr<DeclarationNode> parseVariableDeclaration(std::unique_ptr<TypeNode>&& typeSpecifier, std::unique_ptr<DeclaratorNode>&& declarator)
		{
			// ';'
			matchToken(TokenKind::semicolon);

			return std::make_unique<VariableDeclarationNode>(declarator->line(), std::move(typeSpecifier), std::move(declarator));
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

			return std::make_unique<FunctionDeclaratorNode>(declarator->line(), std::move(declarator), std::move(parameters));
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
				node->addChild({}, parseParameterDeclaration());

				// {',' parameter-declaration}*
				while (consumeTokenIf(TokenKind::comma))
				{
					node->addChild({}, parseParameterDeclaration());
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

		// --- expression ---

		// expression:
		//     comma-expression
		[[nodiscard]]
		std::unique_ptr<ExpressionNode> parseExpression()
		{
			return parseCommaExpression();
		}

		// comma-expression:
		//     assignment-expression {',' assignment-expression}*
		[[nodiscard]]
		std::unique_ptr<ExpressionNode> parseCommaExpression()
		{
			// assignment-expression
			auto expression = parseAssignmentExpression();

			// TODO: {',' assignment-expression}*

			return expression;
		}

		// assignment-expression:
		//     unary-expression assignment-operator assignment-expression
		//     conditional-expression
		// assignment-operator:
		//     '='
		[[nodiscard]]
		std::unique_ptr<ExpressionNode> parseAssignmentExpression()
		{
			// unary-expression
			auto left = parseUnaryExpression();

			//  assignment-operator assignment-expression
			switch (peekToken()->kind())
			{
				case TokenKind::assign:
					// '=' assignment-expression
					return parseAssignAssignmentExpression(std::move(left));

				default:
					// conditional-expression
					return parseLogicalOrExpression(std::move(left));
			}
		}

		// '=' assignment-expression
		[[nodiscard]]
		std::unique_ptr<ExpressionNode> parseAssignAssignmentExpression(std::unique_ptr<ExpressionNode>&& left)
		{
			// '='
			const auto token = matchToken(TokenKind::assign);

			// assignment-expression
			auto right = parseAssignmentExpression();

			return std::make_unique<AssignmentExpressionNode>(token->line(), std::move(left), std::move(right));
		}

		// conditional-expression:
		//     logical-or-expression '?' expression ':' conditional-expression
		//     logical-or-expression
		[[nodiscard]]
		std::unique_ptr<ExpressionNode> parseConditionalExpression(std::unique_ptr<ExpressionNode>&& left)
		{
			// logical-or-expression
			auto condition = parseLogicalOrExpression(std::move(left));

			// TODO: '?' expression ':' conditional-expression

			return condition;
		}

		// logical-or-expression:
		//     logical-and-expression {'||' logical-and-expression}*
		[[nodiscard]]
		std::unique_ptr<ExpressionNode> parseLogicalOrExpression(std::unique_ptr<ExpressionNode>&& left)
		{
			// logical-and-expression
			left = parseLogicalAndExpression(std::move(left));

			// TODO: {'||' logical-and-expression}*

			return std::move(left);
		}

		// logical-and-expression:
		//     bitwise-or-expression {'&&' bitwise-or-expression}*
		[[nodiscard]]
		std::unique_ptr<ExpressionNode> parseLogicalAndExpression(std::unique_ptr<ExpressionNode>&& left)
		{
			// bitwise-or-expression
			left = parseBitwiseOrExpression(std::move(left));

			// TODO: {'&&' bitwise-or-expression}*

			return std::move(left);
		}

		// bitwise-or-expression:
		//     bitwise-xor-expression {'|' bitwise-xor-expression}*
		[[nodiscard]]
		std::unique_ptr<ExpressionNode> parseBitwiseOrExpression(std::unique_ptr<ExpressionNode>&& left)
		{
			// bitwise-xor-expression
			left = parseBitwiseXorExpression(std::move(left));

			// TODO: {'|' bitwise-xor-expression}*

			return std::move(left);
		}

		// bitwise-xor-expression:
		//     bitwise-and-expression {'^' bitwise-and-expression}*
		[[nodiscard]]
		std::unique_ptr<ExpressionNode> parseBitwiseXorExpression(std::unique_ptr<ExpressionNode>&& left)
		{
			// bitwise-and-expression
			left = parseBitwiseAndExpression(std::move(left));

			// TODO: {'^' bitwise-and-expression}*

			return std::move(left);
		}

		// bitwise-and-expression:
		//     equality-expression {'&' equality-expression}*
		[[nodiscard]]
		std::unique_ptr<ExpressionNode> parseBitwiseAndExpression(std::unique_ptr<ExpressionNode>&& left)
		{
			// equality-expression
			left = parseEqualityExpression(std::move(left));

			// TODO: {'&' equality-expression}*

			return std::move(left);
		}

		// equality-expression:
		//     relational-expression {equality-operator relational-expression}*
		// equality-operator:
		//     '==' | '!='
		[[nodiscard]]
		std::unique_ptr<ExpressionNode> parseEqualityExpression(std::unique_ptr<ExpressionNode>&& left)
		{
			// relational-expression
			left = parseRelationalExpression(std::move(left));

			// TODO: {equality-operator relational-expression}*

			return std::move(left);
		}

		// relational-expression:
		//     shift-expression {relational-operator shift-expression}*
		// relational-operator:
		//     '<' | '<=' | '>' | '>='
		[[nodiscard]]
		std::unique_ptr<ExpressionNode> parseRelationalExpression(std::unique_ptr<ExpressionNode>&& left)
		{
			// shift-expression
			left = parseShiftExpression(std::move(left));

			// TODO: {relational-operator shift-expression}*

			return std::move(left);
		}

		// shift-expression:
		//     additive-expression {shift-operator additive-expression}*
		// shift-operator:
		//     '<<' | '>>'
		[[nodiscard]]
		std::unique_ptr<ExpressionNode> parseShiftExpression(std::unique_ptr<ExpressionNode>&& left)
		{
			// additive-expression
			left = parseAdditiveExpression(std::move(left));

			// TODO: {shift-operator additive-expression}*

			return std::move(left);
		}

		// additive-expression:
		//     multiplicative-expression {additive-operator multiplicative-expression}*
		// additive-operator:
		//     '+' | '-'
		[[nodiscard]]
		std::unique_ptr<ExpressionNode> parseAdditiveExpression(std::unique_ptr<ExpressionNode>&& left)
		{
			// multiplicative-expression
			left = parseMultiplicativeExpression(std::move(left));

			// TODO: {additive-operator multiplicative-expression}*

			return std::move(left);
		}

		// multiplicative-expression:
		//     unary-expression {multiplicative-operator unary-expression}*
		// multiplicative-operator:
		//     '*' | '/' | '%'
		[[nodiscard]]
		std::unique_ptr<ExpressionNode> parseMultiplicativeExpression(std::unique_ptr<ExpressionNode>&& left)
		{
			// TODO: {multiplicative-operator unary-expression}*

			return std::move(left);
		}

		// unary-expression:
		//     primary-expression
		//     '+' unary-expression
		//     '-' unary-expression
		//     '&' unary-expression
		//     '*' unary-expression
		//     '~' unary-expression
		//     '!' unary-expression
		[[nodiscard]]
		std::unique_ptr<ExpressionNode> parseUnaryExpression()
		{
			switch (const auto token = peekToken(); token->kind())
			{
				case TokenKind::plus:
					// plus-expression
					return parsePlusExpression();

				case TokenKind::minus:
					// minus-expression
					return parseMinusExpression();

				// TODO: unary-expression

				default:
					// primary-expression
					return parsePrimaryExpression();
			}
		}

		// plus-expression:
		//     '+' unary-expression
		[[nodiscard]]
		std::unique_ptr<ExpressionNode> parsePlusExpression()
		{
			// '+'
			const auto token = matchToken(TokenKind::plus);

			// unary-expression
			auto operand = parseUnaryExpression();

			return std::make_unique<PlusExpressionNode>(token->line(), std::move(operand));
		}

		// minus-expression:
		//     '-' unary-expression
		[[nodiscard]]
		std::unique_ptr<ExpressionNode> parseMinusExpression()
		{
			// '-'
			const auto token = matchToken(TokenKind::minus);

			// unary-expression
			auto operand = parseUnaryExpression();

			return std::make_unique<MinusExpressionNode>(token->line(), std::move(operand));
		}

		// primary-expression:
		//     paren-expression
		//     identifier-expression
		//     integer-expression
		[[nodiscard]]
		std::unique_ptr<ExpressionNode> parsePrimaryExpression()
		{
			switch (const auto token = peekToken(); token->kind())
			{
				case TokenKind::leftParen:
					// paren-expression
					return parseParenExpression();

				case TokenKind::identifier:
					// identifier-expression
					return parseIdentifierExpression();

				case TokenKind::integerLiteral:
					// integer-expression
					return parseIntegerExpression();

				default:
					// error
					reportError(boost::format(u8"unexpected token `%1%', expected expression.") % token->text());
			}
		}

		// paren-expression:
		//     '(' expression ')'
		[[nodiscard]]
		std::unique_ptr<ExpressionNode> parseParenExpression()
		{
			// '('
			matchToken(TokenKind::leftParen);

			// expression
			auto expression = parseExpression();

			// ')'
			matchToken(TokenKind::rightParen);

			return expression;
		}

		// identifier-expression:
		//     identifier
		[[nodiscard]]
		std::unique_ptr<ExpressionNode> parseIdentifierExpression()
		{
			// identifier
			const auto token = matchToken(TokenKind::identifier);

			return std::make_unique<IdentifierExpressionNode>(token->line(), token->text());
		}

		// integer-expression:
		//     integer-literal
		[[nodiscard]]
		std::unique_ptr<ExpressionNode> parseIntegerExpression()
		{
			// integer-literal
			const auto token = matchToken(TokenKind::integerLiteral);

			return std::make_unique<IntegerExpressionNode>(token->line(), token->integer());
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
