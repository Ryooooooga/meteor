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

#include "Register.hpp"

namespace meteor
{
	namespace operations
	{
		// 0x0000 ~ 0x0f00
		constexpr Word nop      = 0x0000;
		// 0x1000 ~ 0x1f00
		constexpr Word ld_adr   = 0x1000;
		constexpr Word st       = 0x1100;
		constexpr Word lad      = 0x1200;
		constexpr Word ld_r     = 0x1400;
		// 0x2000 ~ 0x2f00
		constexpr Word adda_adr = 0x2000;
		constexpr Word suba_adr = 0x2100;
		constexpr Word addl_adr = 0x2200;
		constexpr Word subl_adr = 0x2300;
		constexpr Word adda_r   = 0x2400;
		constexpr Word suba_r   = 0x2500;
		constexpr Word addl_r   = 0x2600;
		constexpr Word subl_r   = 0x2700;
		// 0x3000 ~ 0x3f00
		constexpr Word and_adr  = 0x3000;
		constexpr Word or_adr   = 0x3100;
		constexpr Word xor_adr  = 0x3200;
		constexpr Word and_r    = 0x3400;
		constexpr Word or_r     = 0x3500;
		constexpr Word xor_r    = 0x3600;
		// 0x4000 ~ 0x4f00
		constexpr Word cpa_adr  = 0x4000;
		constexpr Word cpl_adr  = 0x4100;
		constexpr Word cpa_r    = 0x4400;
		constexpr Word cpl_r    = 0x4500;
		// 0x5000 ~ 0x5f00
		constexpr Word sla_adr  = 0x5000;
		constexpr Word sra_adr  = 0x5100;
		constexpr Word sll_adr  = 0x5200;
		constexpr Word srl_adr  = 0x5300;
		// 0x6000 ~ 0x6f00
		constexpr Word jmi      = 0x6100;
		constexpr Word jnz      = 0x6200;
		constexpr Word jze      = 0x6300;
		constexpr Word jump     = 0x6400;
		constexpr Word jpl      = 0x6500;
		constexpr Word jov      = 0x6600;
		// 0x7000 ~ 0x7f00
		constexpr Word push     = 0x7000;
		constexpr Word pop      = 0x7100;
		// 0x8000 ~ 0x8f00
		constexpr Word call     = 0x8000;
		constexpr Word ret      = 0x8100;
		// 0xf000 ~ 0xff00
		constexpr Word svc      = 0xf000;

		[[nodiscard]]
		constexpr Word operationCode(Word code) noexcept
		{
			return code & 0xff00;
		}

		[[nodiscard]]
		constexpr std::pair<Register, Register> registers(Word code) noexcept
		{
			return { static_cast<Register>((code >> 4) & 0x07), static_cast<Register>((code >> 0) & 0x07) };
		}
	}
}
