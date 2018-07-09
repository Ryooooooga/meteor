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

#include <array>
#include <iostream>
#include <memory>

#include "Memory.hpp"
#include "../Operation.hpp"
#include "../Register.hpp"
#include "../SystemCall.hpp"

namespace meteor::runtime
{
	class Processor
	{
	public:
		explicit Processor(std::shared_ptr<Memory> memory)
			: m_memory(std::move(memory))
			, m_registers()
		{
			assert(m_memory);
		}

		// Uncopyable, movable.
		Processor(const Processor&) =delete;
		Processor(Processor&&) =default;

		Processor& operator=(const Processor&) =delete;
		Processor& operator=(Processor&&) =default;

		~Processor() =default;

		bool step()
		{
			const auto instruction = fetchProgram();
			const auto operation = (instruction >> 8) & 0xff;
			const auto register1 = static_cast<Register>((instruction >> 4) & 0x07);
			const auto register2 = static_cast<Register>((instruction >> 0) & 0x07);

			switch (operation)
			{
				// 0x00 ~ 0x0f
				case operations::nop      : return executeNOP      ();
				// 0x10 ~ 0x1f
				case operations::ld_adr   : return executeLD_adr   (register1, fetchProgram(), register2);
				case operations::st       : return executeST       (register1, fetchProgram(), register2);
				case operations::lad      : return executeLAD      (register1, fetchProgram(), register2);
				case operations::ld_r     : return executeLD_r     (register1, register2);
				// 0x20 ~ 0x2f
				case operations::adda_adr : return executeADDA_adr (register1, fetchProgram(), register2);
				case operations::suba_adr : return executeSUBA_adr (register1, fetchProgram(), register2);
				case operations::addl_adr : return executeADDL_adr (register1, fetchProgram(), register2);
				case operations::subl_adr : return executeSUBL_adr (register1, fetchProgram(), register2);
				case operations::adda_r   : return executeADDA_r   (register1, register2);
				case operations::suba_r   : return executeSUBA_r   (register1, register2);
				case operations::addl_r   : return executeADDL_r   (register1, register2);
				case operations::subl_r   : return executeSUBL_r   (register1, register2);
				// 0x30 ~ 0x3f
				case operations::and_adr  : return executeAND_adr  (register1, fetchProgram(), register2);
				case operations::or_adr   : return executeOR_adr   (register1, fetchProgram(), register2);
				case operations::xor_adr  : return executeXOR_adr  (register1, fetchProgram(), register2);
				case operations::and_r    : return executeAND_r    (register1, register2);
				case operations::or_r     : return executeOR_r     (register1, register2);
				case operations::xor_r    : return executeXOR_r    (register1, register2);
				// 0x40 ~ 0x4f
				case operations::cpa_adr  : return executeCPA_adr  (register1, fetchProgram(), register2);
				case operations::cpl_adr  : return executeCPL_adr  (register1, fetchProgram(), register2);
				case operations::cpa_r    : return executeCPA_r    (register1, register2);
				case operations::cpl_r    : return executeCPL_r    (register1, register2);
				// 0x50 ~ 0x5f
				case operations::sla_adr  : return executeSLA_adr  (register1, fetchProgram(), register2);
				case operations::sra_adr  : return executeSRA_adr  (register1, fetchProgram(), register2);
				case operations::sll_adr  : return executeSLL_adr  (register1, fetchProgram(), register2);
				case operations::srl_adr  : return executeSRL_adr  (register1, fetchProgram(), register2);
				// 0x60 ~ 0x6f
				case operations::jmi      : return executeJMI      (fetchProgram(), register2);
				case operations::jnz      : return executeJNZ      (fetchProgram(), register2);
				case operations::jze      : return executeJZE      (fetchProgram(), register2);
				case operations::jump     : return executeJUMP     (fetchProgram(), register2);
				case operations::jpl      : return executeJPL      (fetchProgram(), register2);
				case operations::jov      : return executeJOV      (fetchProgram(), register2);
				// 0x70 ~ 0x7f
				case operations::push     : return executePUSH     (fetchProgram(), register2);
				case operations::pop      : return executePOP      (register1);
				// 0x80 ~ 0x8f
				case operations::call     : return executeCALL     (fetchProgram(), register2);
				case operations::ret      : return executeRET      ();
				// 0xf0 ~ 0xff
				case operations::svc      : return executeSVC      (fetchProgram(), register2);
				default                   : return executeError    (instruction);
			}
		}

		[[nodiscard]]
		std::shared_ptr<Memory> memory() const noexcept
		{
			return m_memory;
		}

		void dumpRegisters(std::ostream& stream)
		{
			for (Word i = 0; i < numRegisters; i++)
			{
				stream << boost::format("%1$3s = #%2$04X = %2$d") % static_cast<Register>(i) % m_registers[i] << std::endl;
			}
		}

	private:
		[[nodiscard]]
		constexpr static bool msb(Word value) noexcept
		{
			return (value & 0x8000) != 0;
		}

		[[nodiscard]]
		constexpr static bool lsb(Word value) noexcept
		{
			return (value & 0x0001) != 0;
		}

		[[nodiscard]]
		Word getRegister(Register reg) const noexcept
		{
			return m_registers[static_cast<Word>(reg)];
		}

		void setRegister(Register reg, Word value) noexcept
		{
			m_registers[static_cast<Word>(reg)] = value;
		}

		[[nodiscard]]
		Word stackPointer() const noexcept
		{
			return getRegister(Register::stackPointer);
		}

		void stackPointer(Word value) noexcept
		{
			setRegister(Register::stackPointer, value);
		}

		[[nodiscard]]
		Word programCounter() const noexcept
		{
			return getRegister(Register::programCounter);
		}

		void programCounter(Word value) noexcept
		{
			setRegister(Register::programCounter, value);
		}

		[[nodiscard]]
		bool overflowFlag() const noexcept
		{
			return (getRegister(Register::flags) & 0b001) != 0;
		}

		[[nodiscard]]
		bool zeroFlag() const noexcept
		{
			return (getRegister(Register::flags) & 0b010) != 0;
		}

		[[nodiscard]]
		bool signFlag() const noexcept
		{
			return (getRegister(Register::flags) & 0b100) != 0;
		}

		void overflowFlag(bool flag) noexcept
		{
			setRegister(Register::flags, (getRegister(Register::flags) & 0b110) | (flag ? 0b001 : 0b000));
		}

		void zeroFlag(bool flag) noexcept
		{
			setRegister(Register::flags, (getRegister(Register::flags) & 0b101) | (flag ? 0b010 : 0b000));
		}

		void signFlag(bool flag) noexcept
		{
			setRegister(Register::flags, (getRegister(Register::flags) & 0b011) | (flag ? 0b100 : 0b000));
		}

		[[nodiscard]]
		Word fetchProgram() noexcept
		{
			const auto value = m_memory->read(programCounter());
			programCounter(programCounter() + 1);

			return value;
		}

		void push(Word value) noexcept
		{
			stackPointer(stackPointer() - 1);
			m_memory->write(stackPointer(), value);
		}

		[[nodiscard]]
		Word pop() noexcept
		{
			const Word value = m_memory->read(stackPointer());
			stackPointer(stackPointer() + 1);

			return value;
		}

		// NOP
		bool executeNOP()
		{
			return true;
		}

		// LD r, adr, x
		bool executeLD_adr(Register r, Word adr, Register x)
		{
			// r <- m[address]
			const Word value = m_memory->read(adr + getRegister(x));

			setRegister(r, value);

			overflowFlag(false);
			zeroFlag(value == 0);
			signFlag(msb(value));

			return true;
		}

		// ST r, adr, x
		bool executeST(Register r, Word adr, Register x)
		{
			// address <- r
			const Word value = getRegister(r);

			m_memory->write(adr + getRegister(x), value);

			overflowFlag(false);
			zeroFlag(value == 0);
			signFlag(msb(value));

			return true;
		}

		// LAD r, adr, x
		bool executeLAD(Register r, Word adr, Register x)
		{
			// r <- address
			const Word value = adr + getRegister(x);

			setRegister(r, value);

			return true;
		}

		// LD r1, r2
		bool executeLD_r(Register r1, Register r2)
		{
			// r1 <- r2
			const Word value = getRegister(r2);

			setRegister(r1, value);

			overflowFlag(false);
			zeroFlag(value == 0);
			signFlag(msb(value));

			return true;
		}

		// ADDA r, adr, x
		bool executeADDA_adr(Register r, Word adr, Register x)
		{
			// r <- r + address
			const Word left = getRegister(r);
			const Word right = adr + getRegister(x);
			const Word value = left + right;

			setRegister(r, value);

			overflowFlag(msb(~(left ^ right) & (left ^ value)));
			zeroFlag(value == 0);
			signFlag(msb(value));

			return true;
		}

		// SUBA r, adr, x
		bool executeSUBA_adr(Register r, Word adr, Register x)
		{
			// r <- r - address
			const Word left = getRegister(r);
			const Word right = adr + getRegister(x);
			const Word value = left - right;

			setRegister(r, value);

			overflowFlag(msb((left ^ right) & (left ^ value)));
			zeroFlag(value == 0);
			signFlag(msb(value));

			return true;
		}

		// ADDL r, adr, x
		bool executeADDL_adr(Register r, Word adr, Register x)
		{
			// r <- r + address
			const Word left = getRegister(r);
			const Word right = adr + getRegister(x);
			const Word value = left + right;

			setRegister(r, value);

			overflowFlag(false);
			zeroFlag(value == 0);
			signFlag(msb(value));

			return true;
		}

		// SUBL r, adr, x
		bool executeSUBL_adr(Register r, Word adr, Register x)
		{
			// r <- r - address
			const Word left = getRegister(r);
			const Word right = adr + getRegister(x);
			const Word value = left - right;

			setRegister(r, value);

			overflowFlag(false);
			zeroFlag(value == 0);
			signFlag(msb(value));

			return true;
		}

		// ADDA r1, r2
		bool executeADDA_r(Register r1, Register r2)
		{
			// r1 <- r1 + r2
			const Word left = getRegister(r1);
			const Word right = getRegister(r2);
			const Word value = left + right;

			setRegister(r1, value);

			overflowFlag(msb(~(left ^ right) & (left ^ value)));
			zeroFlag(value == 0);
			signFlag(msb(value));

			return true;
		}

		// SUBA r1, r2
		bool executeSUBA_r(Register r1, Register r2)
		{
			// r1 <- r1 - r2
			const Word left = getRegister(r1);
			const Word right = getRegister(r2);
			const Word value = left - right;

			setRegister(r1, value);

			overflowFlag(msb((left ^ right) & (left ^ value)));
			zeroFlag(value == 0);
			signFlag(msb(value));

			return true;
		}

		// ADDL r1, r2
		bool executeADDL_r(Register r1, Register r2)
		{
			// r1 <- r1 + r2
			const Word left = getRegister(r1);
			const Word right = getRegister(r2);
			const Word value = left + right;

			setRegister(r1, value);

			overflowFlag(false);
			zeroFlag(value == 0);
			signFlag(msb(value));

			return true;
		}

		// SUBL r1, r2
		bool executeSUBL_r(Register r1, Register r2)
		{
			// r1 <- r1 - r2
			const Word left = getRegister(r1);
			const Word right = getRegister(r2);
			const Word value = left - right;

			setRegister(r1, value);

			overflowFlag(false);
			zeroFlag(value == 0);
			signFlag(msb(value));

			return true;
		}

		// AND r, adr, x
		bool executeAND_adr(Register r, Word adr, Register x)
		{
			// r1 <- r1 & r2
			const Word left = getRegister(r);
			const Word right = m_memory->read(adr + getRegister(x));
			const Word value = left & right;

			setRegister(r, value);

			overflowFlag(false);
			zeroFlag(value == 0);
			signFlag(msb(value));

			return true;
		}

		// OR r, adr, x
		bool executeOR_adr(Register r, Word adr, Register x)
		{
			// r1 <- r1 | r2
			const Word left = getRegister(r);
			const Word right = m_memory->read(adr + getRegister(x));
			const Word value = left | right;

			setRegister(r, value);

			overflowFlag(false);
			zeroFlag(value == 0);
			signFlag(msb(value));

			return true;
		}

		// XOR r, adr, x
		bool executeXOR_adr(Register r, Word adr, Register x)
		{
			// r1 <- r1 ^ r2
			const Word left = getRegister(r);
			const Word right = m_memory->read(adr + getRegister(x));
			const Word value = left ^ right;

			setRegister(r, value);

			overflowFlag(false);
			zeroFlag(value == 0);
			signFlag(msb(value));

			return true;
		}

		// AND r1, r2
		bool executeAND_r(Register r1, Register r2)
		{
			// r1 <- r1 & r2
			const Word left = getRegister(r1);
			const Word right = getRegister(r2);
			const Word value = left & right;

			setRegister(r1, value);

			overflowFlag(false);
			zeroFlag(value == 0);
			signFlag(msb(value));

			return true;
		}

		// OR r1, r2
		bool executeOR_r(Register r1, Register r2)
		{
			// r1 <- r1 | r2
			const Word left = getRegister(r1);
			const Word right = getRegister(r2);
			const Word value = left | right;

			setRegister(r1, value);

			overflowFlag(false);
			zeroFlag(value == 0);
			signFlag(msb(value));

			return true;
		}

		// XOR r1, r2
		bool executeXOR_r(Register r1, Register r2)
		{
			// r1 <- r1 ^ r2
			const Word left = getRegister(r1);
			const Word right = getRegister(r2);
			const Word value = left ^ right;

			setRegister(r1, value);

			overflowFlag(false);
			zeroFlag(value == 0);
			signFlag(msb(value));

			return true;
		}

		// CPA r, adr, x
		bool executeCPA_adr(Register r, Word adr, Register x)
		{
			// r <- r - address
			const Word left = getRegister(r);
			const Word right = adr + getRegister(x);
			const Word value = left - right;

			overflowFlag(msb((left ^ right) & (left ^ value)));
			zeroFlag(value == 0);
			signFlag(msb(value));

			return true;
		}

		// CPL r, adr, x
		bool executeCPL_adr(Register r, Word adr, Register x)
		{
			// r <- r - address
			const Word left = getRegister(r);
			const Word right = adr + getRegister(x);
			const Word value = left - right;

			overflowFlag(false);
			zeroFlag(value == 0);
			signFlag(msb(value));

			return true;
		}

		// CPA r1, r2
		bool executeCPA_r(Register r1, Register r2)
		{
			// r1 <- r1 - r2
			const Word left = getRegister(r1);
			const Word right = getRegister(r2);
			const Word value = left - right;

			overflowFlag(msb((left ^ right) & (left ^ value)));
			zeroFlag(value == 0);
			signFlag(msb(value));

			return true;
		}

		// CPL r1, r2
		bool executeCPL_r(Register r1, Register r2)
		{
			// r1 <- r1 - r2
			const Word left = getRegister(r1);
			const Word right = getRegister(r2);
			const Word value = left - right;

			overflowFlag(false);
			zeroFlag(value == 0);
			signFlag(msb(value));

			return true;
		}

		// SLA r, adr, x
		bool executeSLA_adr(Register r, Word adr, Register x)
		{
			// r <- r << m[adr + x]
			const Word left = getRegister(r);
			const Word right = adr + getRegister(x);

			Word value = left;
			bool overflowBit = false;
			bool signBit = msb(left);

			for (Word i = 0; i < right; i++)
			{
				overflowBit = msb(value << 1);
				value <<= 1;
				value &= 0x7fff;
				value |= signBit ? 0x8000 : 0x0000;
			}

			setRegister(r, value);

			overflowFlag(overflowBit);
			zeroFlag(value == 0);
			signFlag(msb(value));

			return true;
		}

		// SRA r, adr, x
		bool executeSRA_adr(Register r, Word adr, Register x)
		{
			// r <- r >> m[adr + x]
			const Word left = getRegister(r);
			const Word right = adr + getRegister(x);

			Word value = left;
			bool overflowBit = false;
			bool signBit = msb(left);

			for (Word i = 0; i < right; i++)
			{
				overflowBit = lsb(value);
				value >>= 1;
				value &= 0x7fff;
				value |= signBit ? 0x8000 : 0x0000;
			}

			setRegister(r, value);

			overflowFlag(overflowBit);
			zeroFlag(value == 0);
			signFlag(msb(value));

			return true;
		}

		// SLL r, adr, x
		bool executeSLL_adr(Register r, Word adr, Register x)
		{
			// r <- r << m[adr + x]
			const Word left = getRegister(r);
			const Word right = adr + getRegister(x);

			Word value = left;
			bool overflowBit = false;

			for (Word i = 0; i < right; i++)
			{
				overflowBit = msb(value);
				value <<= 1;
			}

			setRegister(r, value);

			overflowFlag(overflowBit);
			zeroFlag(value == 0);
			signFlag(msb(value));

			return true;
		}

		// SRL r, adr, x
		bool executeSRL_adr(Register r, Word adr, Register x)
		{
			// r <- r >> m[adr + x]
			const Word left = getRegister(r);
			const Word right = adr + getRegister(x);

			Word value = left;
			bool overflowBit = false;

			for (Word i = 0; i < right; i++)
			{
				overflowBit = lsb(value);
				value >>= 1;
			}

			setRegister(r, value);

			overflowFlag(overflowBit);
			zeroFlag(value == 0);
			signFlag(msb(value));

			return true;
		}

		// JMI adr, x
		bool executeJMI(Word adr, Register x)
		{
			// SF == 1
			if (signFlag())
			{
				// pc <- address
				programCounter(adr + getRegister(x));
			}

			return true;
		}

		// JNZ adr, x
		bool executeJNZ(Word adr, Register x)
		{
			// ZF == 0
			if (!zeroFlag())
			{
				// pc <- address
				programCounter(adr + getRegister(x));
			}

			return true;
		}

		// JZE adr, x
		bool executeJZE(Word adr, Register x)
		{
			// ZF == 1
			if (zeroFlag())
			{
				// pc <- address
				programCounter(adr + getRegister(x));
			}

			return true;
		}

		// JUMP adr, x
		bool executeJUMP(Word adr, Register x)
		{
			// pc <- address
			programCounter(adr + getRegister(x));

			return true;
		}

		// JPL adr, x
		bool executeJPL(Word adr, Register x)
		{
			// ZF == 0 && SF == 0
			if (!zeroFlag() && !signFlag())
			{
				// pc <- address
				programCounter(adr + getRegister(x));
			}

			return true;
		}

		// JOV adr, x
		bool executeJOV(Word adr, Register x)
		{
			// OF == 1
			if (overflowFlag())
			{
				// pc <- address
				programCounter(adr + getRegister(x));
			}

			return true;
		}

		// PUSH adr, x
		bool executePUSH(Word adr, Register x)
		{
			// sp    <- sp - 1
			// m[sp] <- address
			push(adr + getRegister(x));

			return true;
		}

		// POP r
		bool executePOP(Register r)
		{
			// r  <- m[sp]
			// sp <- sp + 1
			setRegister(r, pop());

			return true;
		}

		// CALL adr, x
		bool executeCALL(Word adr, Register x)
		{
			// sp    <- sp - 1
			// m[sp] <- pc
			// pc    <- address
			push(programCounter());
			programCounter(adr + getRegister(x));

			return true;
		}

		// RET
		bool executeRET()
		{
			if (stackPointer() == 0x0000)
			{
				return false;
			}

			// r  <- m[sp]
			// sp <- sp + 1
			programCounter(pop());

			return true;
		}

		// SVC adr, x
		bool executeSVC(Word adr, Register x)
		{
			switch (adr + getRegister(x))
			{
				case system_calls::exit:
					// Exit system call.
					std::cout << boost::format("exit status %1$d") % getRegister(Register::general1) << std::endl;

					return false;

				default:
					// Error.
					std::cerr << boost::format("invalid system call #%1$04X.") % (adr + getRegister(x)) << std::endl;

					return false;
			}
		}

		bool executeError(Word instruction)
		{
			std::cerr << boost::format("unknown instruction word #%1$04X.") % instruction << std::endl;

			return false;
		}

		std::shared_ptr<Memory> m_memory;

		std::array<Word, numRegisters> m_registers;
	};
}
