This is a comprehensive guide to the Motorola 68000 (M68k) family architecture, specifically tailored to assist a coding LLM in reverse-engineering and understanding 68k assembly code. It synthesizes the programming model, addressing modes, stack mechanics, and instruction semantics derived from the provided Programmer's Reference Manual.

### 1. Architecture Overview

#### 1.1 Data Organization and Endianness

* **Endianness:** The M68000 family uses **Big-Endian** byte ordering. The lowest memory address corresponds to the highest-order (most significant) byte.


* **Data Sizes:**
* Byte: 8 bits 


* Word: 16 bits 


* Long Word: 32 bits 


* Quad Word: 64 bits (typically represented across two registers) 


* Floating Point: Single (32-bit), Double (64-bit), Extended (96-bit in memory, 80-bit logical).





#### 1.2 Integer Programming Model

The user-level integer model consists of 16 general-purpose 32-bit registers, a 32-bit Program Counter (PC), and an 8-bit Condition Code Register (CCR).

* **Data Registers (D0 - D7):** Used for bit, byte, word, and long-word operations. They can also act as index registers. Word/byte operations on Data Registers only modify the lower 16/8 bits; the upper bits remain completely unchanged.


* 
**Address Registers (A0 - A7):** Used as software stack pointers, index registers, or base address registers. Address registers do *not* support byte-sized operands. When an address register is a destination for a word operation, the source operand is **sign-extended to 32 bits** before altering the entire 32-bit address register. Address register operations typically do *not* alter the Condition Code Register (CCR).


* 
**A7 (Stack Pointer):** A7 is implicitly used by hardware for stacking during subroutine calls (JSR/BSR) and exception handling.


* 
**Program Counter (PC):** Contains the address of the currently executing instruction.



#### 1.3 Floating-Point Model (Coprocessor)

* 
**Data Registers (FP0 - FP7):** 8 registers, always holding 80-bit extended-precision values internally.


* 
**Control/Status:** FPCR (16-bit Control), FPSR (32-bit Status), FPIAR (32-bit Instruction Address).



---

### 2. Condition Code Register (CCR) & Branching

The CCR occupies the lower byte of the Status Register (SR) and holds 5 flags evaluated for conditional branches:

* **X (Extend):** Used for multi-precision arithmetic (like `ADDX`, `SUBX`). Mirrors the C-bit for most arithmetic operations.


* 
**N (Negative):** Set if the Most Significant Bit (MSB) of the result is 1.


* 
**Z (Zero):** Set if the result is exactly 0.


* 
**V (Overflow):** Set if an arithmetic overflow occurs (sign bit inversion out of bounds).


* 
**C (Carry):** Set if a carry-out of the MSB occurs during addition, or a borrow occurs during subtraction.



**Branching Conditions (`Bcc`, `Scc`, `DBcc`):**
M68k provides numerous conditional predicates tested by instructions:

* `EQ` (Equal: Z=1) / `NE` (Not Equal: Z=0)
* `CS` or `LO` (Carry Set/Lower: C=1) / `CC` or `HI` (Carry Clear/Higher: C=0 and Z=0)
* `MI` (Minus: N=1) / `PL` (Plus: N=0)
* `VS` (Overflow Set: V=1) / `VC` (Overflow Clear: V=0)
* `GE` (Greater/Equal), `GT` (Greater Than), `LE` (Less/Equal), `LT` (Less Than) — Evaluated via signed combinations of N, V, and Z flags.

---

### 3. Addressing Modes (Effective Address)

Understanding Effective Address (`<ea>`) calculation is essential for reversing M68k. The syntax implies how data is fetched or stored.

#### 3.1 Basic Modes

* 
**Data Register Direct:** `Dn` (Operand is the contents of Dn).


* 
**Address Register Direct:** `An` (Operand is the contents of An).


* 
**Absolute Short:** `(xxx).W` (16-bit address sign-extended to 32 bits).


* 
**Absolute Long:** `(xxx).L` (Full 32-bit fixed memory address).


* 
**Immediate:** `#<data>` (Literal value embedded in the instruction stream).



#### 3.2 Address Register Indirect Modes

* 
**Indirect:** `(An)` (Address is in An).


* 
**Postincrement:** `(An)+` (Access memory at An, then An = An + size).


* 
**Predecrement:** `-(An)` (An = An - size, then access memory at An). *(Note: If An is A7 (SP) and size is byte, SP is adjusted by 2 to maintain word alignment)*.


* 
**Indirect with Displacement:** `(d16, An)` (Address = An + sign-extended 16-bit integer).


* 
**Indirect with Index (8-bit):** `(d8, An, Xn.SIZE*SCALE)` (Address = An + Xn + sign-extended 8-bit integer).



#### 3.3 Advanced / Memory Indirect Modes (68020+)

* 
**Base Displacement:** `(bd, An, Xn.SIZE*SCALE)` (Allows 16/32-bit displacements).


* 
**Memory Indirect Postindexed:** `([bd, An], Xn.SIZE*SCALE, od)` (Fetches a pointer from `An + bd`, adds Index and outer displacement `od`).


* 
**Memory Indirect Preindexed:** `([bd, An, Xn.SIZE*SCALE], od)` (Adds Index before fetching the pointer).



#### 3.4 Program Counter (PC) Relative Modes

* Similar to Address Register modes, but use the `PC` as the base to generate Position Independent Code (PIC). Forms include `(d16, PC)` and `(d8, PC, Xn.SIZE*SCALE)`.



---

### 4. Stack & Subroutine Mechanics

The M68k uses a downward-growing stack (high memory to low memory) accessed via `A7` (`SP`).

* **Push:** `-(SP)`
* **Pop:** `(SP)+`

**Subroutine Calls:**

* 
`BSR <label>` / `JSR <ea>`: Pushes the 32-bit return address (PC) onto the stack (`SP - 4 -> SP`), then branches to the target.


* 
`RTS` (Return from Subroutine): Pops the 32-bit return address from the stack into the PC (`(SP) -> PC`, `SP + 4 -> SP`).



**Stack Frames & Local Variables:**

* `LINK An, #<displacement>`: Standard prologue instruction. Pushes the current frame pointer (`An`) to the stack, copies the new `SP` into `An`, and allocates local stack space by adding the (usually negative) displacement to `SP`.


* `UNLK An`: Standard epilogue instruction. Restores `SP` from `An` (deallocating local variables), and pops the old frame pointer back into `An`.



---

### 5. Key Instruction Classes & Reversing Heuristics

#### 5.1 Data Movement

* `MOVE <ea_src>, <ea_dst>`: Core copy instruction. Sets `N` and `Z` flags based on the value moved; clears `V` and `C`.


* `MOVEA <ea>, An`: Move to address register. **Flags are NOT affected.** Source is sign-extended to 32 bits if it is a word.


* `MOVEM <list>, <ea>` / `MOVEM <ea>, <list>`: Move Multiple Registers. Highly common in function prologues/epilogues for saving/restoring context. Does not affect flags.
* If addressing mode is `-(An)`, registers are pushed `A7` to `A0`, then `D7` to `D0`.


* If addressing mode is `(An)+` or control mode, registers are loaded `D0` to `D7`, then `A0` to `A7`.




* `MOVEQ #<data>, Dn`: Move Quick. Fast move of an 8-bit signed literal, which is immediately sign-extended to 32 bits in the register.


* `LEA <ea>, An`: Load Effective Address. Calculates the address of the operand (instead of fetching its value) and stores the pointer in `An`.


* `PEA <ea>`: Push Effective Address. Calculates the `<ea>` and pushes it to the stack (`SP - 4 -> SP`).



#### 5.2 Arithmetic & Logic

* `ADD`, `SUB`: Standard math. Sets all condition codes. `ADDA`/`SUBA` (address targets) do *not* affect CCs.


* `ADDQ`, `SUBQ`: Add/Subtract Quick. Inline 3-bit immediate (values 1-8). Very common for incrementing pointers or adjusting loop counters.


* 
`MULS` / `MULU`: 16x16 -> 32-bit multiplication (standard), or 32x32 -> 64-bit on 68020+.


* 
`DIVS` / `DIVU`: 32/16 division yields a 16-bit quotient in the lower word and 16-bit remainder in the upper word.


* 
`EXT`, `EXTB`: Sign-extends a byte to word, or word to long inside a data register.


* `AND`, `OR`, `EOR`, `NOT`: Standard bitwise logic. Set `N` and `Z`; clear `V` and `C`.



#### 5.3 Program Control & Testing

* `TST <ea>`: Compares the operand with zero, setting `N` and `Z` accordingly. Equivalent to `CMP #0, <ea>` without the immediate footprint.


* 
`CMP <ea>, Dn`: Subtracts source from destination (Dn) *without modifying the destination*, setting CCs. (Order: `Dn - <ea>`).


* `DBcc Dn, <label>`: Test condition, decrement, and branch. Crucial for `for`/`while` loops. The loop falls through (terminates) if the condition is TRUE. If FALSE, `Dn` (lower 16 bits) is decremented. It loops *until* `Dn` reaches `-1`.



#### 5.4 Bit & Bit Field Operations

* `BTST`, `BSET`, `BCLR`, `BCHG`: Tests a specific bit and sets the `Z` flag if the bit is 0, then optionally modifies it. Bit numbers are modulo 32 for Data Registers, and modulo 8 for memory operations.


* 
`BFEXTU`, `BFEXTS`: Extracts a bit-field of 1 to 32 bits from memory/register and zero/sign extends it into a Data Register.



---

### 6. Floating-Point Reversing Context

When decoding FPU operations (usually executed via Coprocessor ID `1` on systems without a native FPU, or directly on 68040):

* Prefix is generally `F` (e.g., `FADD`, `FMUL`, `FCMP`).


* The instruction's precision suffix (`.S`, `.D`, `.X`) defines how memory data is ingested. Internally, the FPU converts all values to 80-bit extended precision for computation.


* `FBcc` / `FDBcc`: Float branch evaluations require awareness of the `NAN` (Not-a-Number) flag. Be cautious of IEEE non-aware branch inversions (e.g., the exact opposite of `FBGT` is `FBNGT`, not `FBLE`, due to unordered logic).



Would you like me to construct a specific JSON or Regex-based Opcode mapping table from the binary encodings to aid automated disassembler tool creation?