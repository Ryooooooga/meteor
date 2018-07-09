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
				case operations::nop      : return executeNOP      ();
				case operations::ld_adr   : return executeLD_adr   (register1, fetchProgram(), register2);
				case operations::st       : return executeST       (register1, fetchProgram(), register2);
				case operations::lad      : return executeLAD      (register1, fetchProgram(), register2);
				case operations::ld_r     : return executeLD_r     (register1, register2);
				case operations::adda_adr : return executeADDA_adr (register1, fetchProgram(), register2);
				case operations::suba_adr : return executeSUBA_adr (register1, fetchProgram(), register2);
				case operations::addl_adr : return executeADDL_adr (register1, fetchProgram(), register2);
				case operations::subl_adr : return executeSUBL_adr (register1, fetchProgram(), register2);
				case operations::adda_r   : return executeADDA_r   (register1, register2);
				case operations::suba_r   : return executeSUBA_r   (register1, register2);
				case operations::addl_r   : return executeADDL_r   (register1, register2);
				case operations::subl_r   : return executeSUBL_r   (register1, register2);
				case operations::and_r    : return executeAND_r    (register1, register2);
				case operations::or_r     : return executeOR_r     (register1, register2);
				case operations::xor_r    : return executeXOR_r    (register1, register2);
				case operations::jnz      : return executeJNZ      (fetchProgram(), register2);
				case operations::jze      : return executeJZE      (fetchProgram(), register2);
				case operations::jump     : return executeJUMP     (fetchProgram(), register2);
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
		Word getRegister(Register reg) const noexcept
		{
			return m_registers[static_cast<Word>(reg)];
		}

		void setRegister(Register reg, Word value) noexcept
		{
			m_registers[static_cast<Word>(reg)] = value;
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

		// AND r1, r2
		bool executeAND_r(Register r1, Register r2)
		{
			// r1 <- r1 & r2
			const Word value = getRegister(r1) & getRegister(r2);

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
			const Word value = getRegister(r1) | getRegister(r2);

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
			const Word value = getRegister(r1) ^ getRegister(r2);

			setRegister(r1, value);

			overflowFlag(false);
			zeroFlag(value == 0);
			signFlag(msb(value));

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

		bool executeError(Word instruction)
		{
			std::cerr << boost::format("unknown instruction word #%1$04X.") % instruction << std::endl;

			return false;
		}

		std::shared_ptr<Memory> m_memory;

		std::array<Word, numRegisters> m_registers;
	};
}
