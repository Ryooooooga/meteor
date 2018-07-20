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

#ifndef METEOR_CC_TOKEN
#	define METEOR_CC_TOKEN(name, text)
#endif

#ifndef METEOR_CC_TOKEN_KEYWORD
#	define METEOR_CC_TOKEN_KEYWORD(name, text) METEOR_CC_TOKEN(name, text)
#endif

#ifndef METEOR_CC_TOKEN_KEYWORD1
#	define METEOR_CC_TOKEN_KEYWORD1(keyword) METEOR_CC_TOKEN_KEYWORD(keyword_ ## keyword, #keyword)
#endif

#ifndef METEOR_CC_TOKEN_PUNCTUATOR
#	define METEOR_CC_TOKEN_PUNCTUATOR(name, text) METEOR_CC_TOKEN(name, text)
#endif

METEOR_CC_TOKEN(endOfFile, "[EOF]")
METEOR_CC_TOKEN(identifier, "identifier")
METEOR_CC_TOKEN(integerLiteral, "integer literal")
METEOR_CC_TOKEN(characterLiteral, "character literal")
METEOR_CC_TOKEN(stringLiteral, "string literal")

METEOR_CC_TOKEN_KEYWORD1(auto)
METEOR_CC_TOKEN_KEYWORD1(break)
METEOR_CC_TOKEN_KEYWORD1(case)
METEOR_CC_TOKEN_KEYWORD1(char)
METEOR_CC_TOKEN_KEYWORD1(const)
METEOR_CC_TOKEN_KEYWORD1(continue)
METEOR_CC_TOKEN_KEYWORD1(default)
METEOR_CC_TOKEN_KEYWORD1(do)
METEOR_CC_TOKEN_KEYWORD1(double)
METEOR_CC_TOKEN_KEYWORD1(else)
METEOR_CC_TOKEN_KEYWORD1(enum)
METEOR_CC_TOKEN_KEYWORD1(extern)
METEOR_CC_TOKEN_KEYWORD1(float)
METEOR_CC_TOKEN_KEYWORD1(for)
METEOR_CC_TOKEN_KEYWORD1(goto)
METEOR_CC_TOKEN_KEYWORD1(if)
METEOR_CC_TOKEN_KEYWORD1(int)
METEOR_CC_TOKEN_KEYWORD1(long)
METEOR_CC_TOKEN_KEYWORD1(register)
METEOR_CC_TOKEN_KEYWORD1(return)
METEOR_CC_TOKEN_KEYWORD1(signed)
METEOR_CC_TOKEN_KEYWORD1(sizeof)
METEOR_CC_TOKEN_KEYWORD1(short)
METEOR_CC_TOKEN_KEYWORD1(static)
METEOR_CC_TOKEN_KEYWORD1(struct)
METEOR_CC_TOKEN_KEYWORD1(switch)
METEOR_CC_TOKEN_KEYWORD1(typedef)
METEOR_CC_TOKEN_KEYWORD1(union)
METEOR_CC_TOKEN_KEYWORD1(unsigned)
METEOR_CC_TOKEN_KEYWORD1(void)
METEOR_CC_TOKEN_KEYWORD1(volatile)
METEOR_CC_TOKEN_KEYWORD1(while)

METEOR_CC_TOKEN_PUNCTUATOR(plus, "+")
METEOR_CC_TOKEN_PUNCTUATOR(minus, "-")
METEOR_CC_TOKEN_PUNCTUATOR(star, "*")
METEOR_CC_TOKEN_PUNCTUATOR(slash, "/")
METEOR_CC_TOKEN_PUNCTUATOR(percent, "%")
METEOR_CC_TOKEN_PUNCTUATOR(tilde, "~")
METEOR_CC_TOKEN_PUNCTUATOR(ampersand, "&")
METEOR_CC_TOKEN_PUNCTUATOR(verticalBar, "|")
METEOR_CC_TOKEN_PUNCTUATOR(caret, "^")
METEOR_CC_TOKEN_PUNCTUATOR(increment, "++")
METEOR_CC_TOKEN_PUNCTUATOR(decrement, "--")
METEOR_CC_TOKEN_PUNCTUATOR(logicalAnd, "&&")
METEOR_CC_TOKEN_PUNCTUATOR(logicalOr, "||")
METEOR_CC_TOKEN_PUNCTUATOR(assign, "=")
METEOR_CC_TOKEN_PUNCTUATOR(addAssign, "+=")
METEOR_CC_TOKEN_PUNCTUATOR(subtractAssign, "-=")
METEOR_CC_TOKEN_PUNCTUATOR(multiplyAssign, "*=")
METEOR_CC_TOKEN_PUNCTUATOR(divideAssign, "/=")
METEOR_CC_TOKEN_PUNCTUATOR(moduloAssign, "%=")
METEOR_CC_TOKEN_PUNCTUATOR(andAssign, "&=")
METEOR_CC_TOKEN_PUNCTUATOR(orAssign, "|=")
METEOR_CC_TOKEN_PUNCTUATOR(xorAssign, "^=")
METEOR_CC_TOKEN_PUNCTUATOR(equal, "==")
METEOR_CC_TOKEN_PUNCTUATOR(notEqual, "!=")
METEOR_CC_TOKEN_PUNCTUATOR(lesserThan, "<")
METEOR_CC_TOKEN_PUNCTUATOR(lesserEqual, "<=")
METEOR_CC_TOKEN_PUNCTUATOR(greaterThan, ">")
METEOR_CC_TOKEN_PUNCTUATOR(greaterEqual, ">=")
METEOR_CC_TOKEN_PUNCTUATOR(exclamation, "!")
METEOR_CC_TOKEN_PUNCTUATOR(question, "?")
METEOR_CC_TOKEN_PUNCTUATOR(colon, ":")
METEOR_CC_TOKEN_PUNCTUATOR(semicolon, ";")
METEOR_CC_TOKEN_PUNCTUATOR(period, ".")
METEOR_CC_TOKEN_PUNCTUATOR(comma, ",")
METEOR_CC_TOKEN_PUNCTUATOR(leftParen, "(")
METEOR_CC_TOKEN_PUNCTUATOR(rightParen, ")")
METEOR_CC_TOKEN_PUNCTUATOR(leftBrace, "{")
METEOR_CC_TOKEN_PUNCTUATOR(rightBrace, "}")
METEOR_CC_TOKEN_PUNCTUATOR(leftBracket, "[")
METEOR_CC_TOKEN_PUNCTUATOR(rightBracket, "]")

#undef METEOR_CC_TOKEN
#undef METEOR_CC_TOKEN_KEYWORD
#undef METEOR_CC_TOKEN_KEYWORD1
#undef METEOR_CC_TOKEN_PUNCTUATOR
