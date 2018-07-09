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

#include "Type.hpp"

namespace meteor
{
	namespace operations
	{
		// 0x00 ~ 0x0f
		constexpr Word nop      = 0x00;
		// 0x10 ~ 0x1f
		constexpr Word ld_adr   = 0x10;
		constexpr Word st       = 0x11;
		constexpr Word lad      = 0x12;
		constexpr Word ld_r     = 0x14;
		// 0x20 ~ 0x2f
		constexpr Word adda_adr = 0x20;
		constexpr Word suba_adr = 0x21;
		constexpr Word addl_adr = 0x22;
		constexpr Word subl_adr = 0x23;
		constexpr Word adda_r   = 0x24;
		constexpr Word suba_r   = 0x25;
		constexpr Word addl_r   = 0x26;
		constexpr Word subl_r   = 0x27;
		// 0x30 ~ 0x3f
		constexpr Word and_adr  = 0x30;
		constexpr Word or_adr   = 0x31;
		constexpr Word xor_adr  = 0x32;
		constexpr Word and_r    = 0x34;
		constexpr Word or_r     = 0x35;
		constexpr Word xor_r    = 0x36;
		// 0x40 ~ 0x4f
		constexpr Word cpa_adr  = 0x40;
		constexpr Word cpl_adr  = 0x41;
		constexpr Word cpa_r    = 0x44;
		constexpr Word cpl_r    = 0x45;
		// 0x50 ~ 0x5f
		constexpr Word sla_adr  = 0x50;
		constexpr Word sra_adr  = 0x51;
		constexpr Word sll_adr  = 0x52;
		constexpr Word srl_adr  = 0x53;
		// 0x60 ~ 0x6f
		constexpr Word jmi      = 0x61;
		constexpr Word jnz      = 0x62;
		constexpr Word jze      = 0x63;
		constexpr Word jump     = 0x64;
		constexpr Word jpl      = 0x65;
		constexpr Word jov      = 0x66;
		// 0x70 ~ 0x7f
		constexpr Word push     = 0x70;
		constexpr Word pop      = 0x71;
		// 0x80 ~ 0x8f
		constexpr Word call     = 0x80;
		constexpr Word ret      = 0x81;
		// 0xf0 ~ 0xff
		constexpr Word svc      = 0xf0;
	}
}
