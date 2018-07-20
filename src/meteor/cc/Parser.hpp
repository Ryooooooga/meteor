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

				case TokenKind::keyword_while:
					// while-statement
					return parseWhileStatement();

				case TokenKind::keyword_return:
					// return-statement
					return parseReturnStatement();

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

		// while-statement:
		//     'while' paren-expression compound-statement
		[[nodiscard]]
		std::unique_ptr<StatementNode> parseWhileStatement()
		{
			// 'while'
			const auto token = matchToken(TokenKind::keyword_while);

			// paren-expression
			auto condition = parseParenExpression();

			// compound-statement
			auto body = parseCompoundStatement();

			return std::make_unique<WhileStatementNode>(token->line(), std::move(condition), std::move(body));
		}

		// return-statement:
		//     'return' expression? ';'
		[[nodiscard]]
		std::unique_ptr<StatementNode> parseReturnStatement()
		{
			// 'return'
			const auto token = matchToken(TokenKind::keyword_return);

			// ';'?
			if (consumeTokenIf(TokenKind::semicolon))
			{
				return std::make_unique<ReturnStatementNode>(token->line(), nullptr);
			}

			// expression
			auto expression = parseExpression();

			// ';'
			matchToken(TokenKind::semicolon);

			return std::make_unique<ReturnStatementNode>(token->line(), std::move(expression));
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
		//     pointer-declarator
		[[nodiscard]]
		std::unique_ptr<DeclaratorNode> parsePrimaryDeclarator()
		{
			switch (peekToken()->kind())
			{
				case TokenKind::leftParen:
					// paren-declarator
					return parseParenDeclarator();

				case TokenKind::star:
					// pointer-declarator
					return parsePointerDeclarator();

				default:
					// identifier-declarator
					return parseIdentifierDeclarator();
			}
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

		// paren-declarator:
		//     '(' declarator ')'
		[[nodiscard]]
		std::unique_ptr<DeclaratorNode> parseParenDeclarator()
		{
			// '('
			matchToken(TokenKind::leftParen);

			// declarator
			auto declarator = parseDeclarator();

			// ')'
			matchToken(TokenKind::rightParen);

			return declarator;
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

		// pointer-declarator:
		//     '*' direct-declarator
		[[nodiscard]]
		std::unique_ptr<DeclaratorNode> parsePointerDeclarator()
		{
			// '*'
			const auto token = matchToken(TokenKind::star);

			// direct-declarator
			auto declarator = parseDirectDeclarator();

			return std::make_unique<PointerDeclaratorNode>(token->line(), std::move(declarator));
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

			// {',' assignment-expression}*
			while (peekToken()->kind() == TokenKind::comma)
			{
				// ','
				const auto token = matchToken(TokenKind::comma);

				// assignment-expression
				auto right = parseAssignmentExpression();

				expression = std::make_unique<CommaExpressionNode>(token->line(), std::move(expression), std::move(right));
			}

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
					return parseAssignAssignmentExpressionRhs(std::move(left));

				default:
					// conditional-expression
					return parseConditionalExpression(std::move(left));
			}
		}

		// '=' assignment-expression
		[[nodiscard]]
		std::unique_ptr<ExpressionNode> parseAssignAssignmentExpressionRhs(std::unique_ptr<ExpressionNode>&& left)
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

			// {'|' bitwise-xor-expression}*
			while (peekToken()->kind() == TokenKind::verticalBar)
			{
				// '|'
				const auto token = matchToken(TokenKind::verticalBar);

				// bitwise-xor-expression
				auto right = parseBitwiseXorExpression(parseUnaryExpression());

				left = std::make_unique<BitwiseOrExpressionNode>(token->line(), std::move(left), std::move(right));
			}

			return std::move(left);
		}

		// bitwise-xor-expression:
		//     bitwise-and-expression {'^' bitwise-and-expression}*
		[[nodiscard]]
		std::unique_ptr<ExpressionNode> parseBitwiseXorExpression(std::unique_ptr<ExpressionNode>&& left)
		{
			// bitwise-and-expression
			left = parseBitwiseAndExpression(std::move(left));

			// {'^' bitwise-and-expression}*
			while (peekToken()->kind() == TokenKind::caret)
			{
				// '^'
				const auto token = matchToken(TokenKind::caret);

				// bitwise-and-expression
				auto right = parseBitwiseAndExpression(parseUnaryExpression());

				left = std::make_unique<BitwiseXorExpressionNode>(token->line(), std::move(left), std::move(right));
			}

			return std::move(left);
		}

		// bitwise-and-expression:
		//     equality-expression {'&' equality-expression}*
		[[nodiscard]]
		std::unique_ptr<ExpressionNode> parseBitwiseAndExpression(std::unique_ptr<ExpressionNode>&& left)
		{
			// equality-expression
			left = parseEqualityExpression(std::move(left));

			// {'&' equality-expression}*
			while (peekToken()->kind() == TokenKind::ampersand)
			{
				// '&'
				const auto token = matchToken(TokenKind::ampersand);

				// equality-expression
				auto right = parseEqualityExpression(parseUnaryExpression());

				left = std::make_unique<BitwiseAndExpressionNode>(token->line(), std::move(left), std::move(right));
			}

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
			left = parseMultiplicativeExpressionRhs(std::move(left));

			// {additive-operator multiplicative-expression}*
			while (true)
			{
				switch (peekToken()->kind())
				{
					case TokenKind::plus:
						// addtion-expression
						left = parseAdditionExpressionRhs(std::move(left));
						break;

					case TokenKind::minus:
						// subtraction-expression
						left = parseSubtractionExpressionRhs(std::move(left));
						break;

					default:
						return std::move(left);
				}
			}
		}

		// '+' mutiplicative-expression
		[[nodiscard]]
		std::unique_ptr<ExpressionNode> parseAdditionExpressionRhs(std::unique_ptr<ExpressionNode>&& left)
		{
			// '+'
			const auto token = matchToken(TokenKind::plus);

			// multiplicative-expression
			auto right = parseMultiplicativeExpression();

			return std::make_unique<AdditionExpressionNode>(token->line(), std::move(left), std::move(right));
		}

		// '-' mutiplicative-expression
		[[nodiscard]]
		std::unique_ptr<ExpressionNode> parseSubtractionExpressionRhs(std::unique_ptr<ExpressionNode>&& left)
		{
			// '-'
			const auto token = matchToken(TokenKind::minus);

			// multiplicative-expression
			auto right = parseMultiplicativeExpression();

			return std::make_unique<SubtractionExpressionNode>(token->line(), std::move(left), std::move(right));
		}

		// multiplicative-expression:
		//     unary-expression {multiplicative-operator unary-expression}*
		// multiplicative-operator:
		//     '*' | '/' | '%'
		[[nodiscard]]
		std::unique_ptr<ExpressionNode> parseMultiplicativeExpression()
		{
			// unary-expression
			auto left = parseUnaryExpression();

			return parseMultiplicativeExpressionRhs(std::move(left));
		}

		[[nodiscard]]
		std::unique_ptr<ExpressionNode> parseMultiplicativeExpressionRhs(std::unique_ptr<ExpressionNode>&& left)
		{
			// TODO: {multiplicative-operator unary-expression}*

			return std::move(left);
		}

		// unary-expression:
		//     postfix-expression
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

				case TokenKind::ampersand:
					// address-expression
					return parseAddressExpression();

				case TokenKind::star:
					// dereference-expression
					return parseDereferenceExpression();

				// TODO: unary-expression

				default:
					// postfix-expression
					return parsePostfixExpression();
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

		// address-expression:
		//     '&' unary-expression
		[[nodiscard]]
		std::unique_ptr<ExpressionNode> parseAddressExpression()
		{
			// '&'
			const auto token = matchToken(TokenKind::ampersand);

			// unary-expression
			auto operand = parseUnaryExpression();

			return std::make_unique<AddressExpressionNode>(token->line(), std::move(operand));
		}

		// dereference-expression:
		//     '*' unary-expression
		[[nodiscard]]
		std::unique_ptr<ExpressionNode> parseDereferenceExpression()
		{
			// '*'
			const auto token = matchToken(TokenKind::star);

			// unary-expression
			auto operand = parseUnaryExpression();

			return std::make_unique<DereferenceExpressionNode>(token->line(), std::move(operand));
		}

		// postfix-expression:
		//     primary-expression {postfix-expression-tail}*
		// postfix-expression-tail:
		//     call-expression-tail
		[[nodiscard]]
		std::unique_ptr<ExpressionNode> parsePostfixExpression()
		{
			// primary-expression
			auto node = parsePrimaryExpression();

			// {postfix-expression-tail}*
			while (true)
			{
				switch (peekToken()->kind())
				{
					case TokenKind::leftParen:
						// call-expression-tail
						node = parseCallExpression(std::move(node));
						break;

					default:
						return node;
				}
			}
		}

		// call-expression-tail:
		//     '(' ')'
		//     '(' assignment-expression {',' assignment-expression}* ')'
		[[nodiscard]]
		std::unique_ptr<ExpressionNode> parseCallExpression(std::unique_ptr<ExpressionNode>&& callee)
		{
			// '('
			const auto token = matchToken(TokenKind::leftParen);

			auto arguments = std::make_unique<ArgumentListNode>(token->line());

			if (peekToken()->kind() != TokenKind::rightParen)
			{
				// assignment-expression
				arguments->addChild({}, parseAssignmentExpression());

				// {',' assignment-expression}*
				while (consumeTokenIf(TokenKind::comma))
				{
					// assignment-expression
					arguments->addChild({}, parseAssignmentExpression());
				}
			}

			// ')'
			matchToken(TokenKind::rightParen);

			return std::make_unique<CallExpressionNode>(callee->line(), std::move(callee), std::move(arguments));
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
