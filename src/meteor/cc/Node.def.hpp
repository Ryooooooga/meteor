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

#ifndef METEOR_CC_NODE
#	define METEOR_CC_NODE(name)
#endif

#ifndef METEOR_CC_STATEMENT_NODE
#	define METEOR_CC_STATEMENT_NODE(name) METEOR_CC_NODE(name)
#endif

#ifndef METEOR_CC_DECLARATION_NODE
#	define METEOR_CC_DECLARATION_NODE(name) METEOR_CC_STATEMENT_NODE(name)
#endif

#ifndef METEOR_CC_DECLARATOR_NODE
#	define METEOR_CC_DECLARATOR_NODE(name) METEOR_CC_NODE(name)
#endif

#ifndef METEOR_CC_EXPRESSION_NODE
#	define METEOR_CC_EXPRESSION_NODE(name) METEOR_CC_NODE(name)
#endif

#ifndef METEOR_CC_TYPE_NODE
#	define METEOR_CC_TYPE_NODE(name) METEOR_CC_NODE(name)
#endif

METEOR_CC_NODE(RootNode)
METEOR_CC_NODE(ParameterListNode)

METEOR_CC_STATEMENT_NODE(EmptyStatementNode)

METEOR_CC_DECLARATOR_NODE(ParameterDeclarationNode)

METEOR_CC_DECLARATOR_NODE(IdentifierDeclaratorNode)

METEOR_CC_TYPE_NODE(IntegerTypeNode)

#undef METEOR_CC_NODE
#undef METEOR_CC_STATEMENT_NODE
#undef METEOR_CC_DECLARATION_NODE
#undef METEOR_CC_DECLARATOR_NODE
#undef METEOR_CC_EXPRESSION_NODE
#undef METEOR_CC_TYPE_NODE
