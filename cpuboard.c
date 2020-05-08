/*
 *	Project-based Learning II (CPU)
 *
 *	Program:	instruction set simulator of the Educational CPU Board
 *	File Name:	cpuboard.c
 *	Descrioption:	simulation(emulation) of an instruction
 */

#include "cpuboard.h"

#include <stdio.h>

int step_ST(Cpub *cpub);
int step_LD(Cpub *cpub);
Uword decrypt_operand_a(const Uword code);
Uword decrypt_operand_b(const Uword code);
void unknown_instruction_code(const Uword code);
void bad_oprand_B(const Uword code);

const Uword ACC = 0x00;
const Uword IX = 0x01;
const Uword IMMEDIATE_ADDRESS = 0x02;
const Uword ABSOLUTE_PROGRAM_ADDRESS = 0x04;
const Uword ABSOLUTE_DATA_ADDRESS = 0x05;
const Uword IX_MODIFICATION_PROGRAM_ADDRESS = 0x06;
const Uword IX_MODIFICATION_DATA_ADDRESS = 0x07;

Uword MAR;
Uword IR;

/*=============================================================================
 *   Simulation of a Single Instruction
 *===========================================================================*/
int step(Cpub *cpub) {
    int return_status = RUN_HALT;

    const Uword NOP = 0x00;
    const Uword HLT = 0x0f;
    const Uword LD = 0x60;
    const Uword ST = 0x70;

    const Uword MASK = 0xf0;

    MAR = cpub->pc;
    IR = cpub->mem[0x000 + MAR];
    const Uword INSTRUCTION_CODE = IR & MASK;
    cpub->pc++;

    if ((IR & 0xf8) == NOP) {
        return RUN_STEP;
    } else if (((IR & 0xfc) | 0x03) == HLT) {
        return RUN_HALT;
    }

    if (INSTRUCTION_CODE == LD) {
        return_status = step_LD(cpub);
    } else if (INSTRUCTION_CODE == ST) {
        return_status = step_ST(cpub);
    } else {
        unknown_instruction_code(IR & MASK);
        return RUN_HALT;
    }
    return return_status;
}

int step_LD(Cpub *cpub) {
    const Uword OPERAND_A = decrypt_operand_a(IR);
    const Uword OPERAND_B = decrypt_operand_b(IR);

    Uword second_word;
    Uword operand_b_value;

    MAR = cpub->pc;
    cpub->pc++;
    second_word = cpub->mem[0x000 + MAR];

    if (OPERAND_B == ACC) {
        operand_b_value = cpub->acc;
    } else if (OPERAND_B == IX) {
        operand_b_value = cpub->ix;
    } else if (OPERAND_B == IMMEDIATE_ADDRESS) {
        operand_b_value = second_word;
    } else if (OPERAND_B == ABSOLUTE_PROGRAM_ADDRESS) {
        operand_b_value = cpub->mem[0x000 + second_word];
    } else if (OPERAND_B == ABSOLUTE_DATA_ADDRESS) {
        operand_b_value = cpub->mem[0x100 + second_word];
    } else if (OPERAND_B == IX_MODIFICATION_PROGRAM_ADDRESS) {
        operand_b_value = cpub->mem[0x000 + cpub->ix + second_word];
    } else if (OPERAND_B == IX_MODIFICATION_DATA_ADDRESS) {
        operand_b_value = cpub->mem[0x100 + cpub->ix + second_word];
    } else {
        return RUN_HALT;
    }

    if (OPERAND_A == ACC) {
        cpub->acc = operand_b_value;
    } else {
        cpub->ix = operand_b_value;
    }

    return RUN_STEP;
}

int step_ST(Cpub *cpub) {
    int return_status = RUN_HALT;

    const Uword OPERAND_A = decrypt_operand_a(IR);
    const Uword OPERAND_B = decrypt_operand_b(IR);

    Uword operand_a_value;
    Uword second_word;

    if (OPERAND_A == ACC) {
        operand_a_value = cpub->acc;
    } else {
        operand_a_value = cpub->ix;
    }

    if (OPERAND_B == ACC) {
        fprintf(stderr, "ACC is Undefined operating(ST)\n");
        return RUN_HALT;
    } else if (OPERAND_B == IX) {
        fprintf(stderr, "IX is Undefined operating (ST)\n");
        return RUN_HALT;
    } else if (OPERAND_B == IMMEDIATE_ADDRESS) {
        fprintf(stderr, "IMMEDIATE_ADDRESS is Undefined operating in (ST)\n");
        return RUN_HALT;
    }

    MAR = cpub->pc;
    cpub->pc++;
    second_word = cpub->mem[0x000 + MAR];

    if (OPERAND_B == ABSOLUTE_PROGRAM_ADDRESS) {
        cpub->mem[0x000 + second_word] = operand_a_value;
        return_status = RUN_STEP;
    } else if (OPERAND_B == ABSOLUTE_DATA_ADDRESS) {
        cpub->mem[0x100 + second_word] = operand_a_value;
        return_status = RUN_STEP;
    } else if (OPERAND_B == IX_MODIFICATION_PROGRAM_ADDRESS) {
        cpub->mem[0x000 + cpub->ix + second_word] = operand_a_value;
        return_status = RUN_STEP;
    } else if (OPERAND_B == IX_MODIFICATION_DATA_ADDRESS) {
        cpub->mem[0x100 + cpub->ix + second_word] = operand_a_value;
        return_status = RUN_STEP;
    } else {
        return RUN_HALT;
    }

    return return_status;
}

Uword decrypt_operand_a(const Uword CODE) {
    const Uword MASK = 0x80;
    return CODE & MASK;
}

Uword decrypt_operand_b(const Uword CODE) {
    const Uword MASK = 0x07;
    Uword operand_b = CODE & MASK;

    if (operand_b == 0x02) {
        operand_b = 0x03;
    }
    return operand_b;
}

void unknown_instruction_code(const Uword code) {
    fprintf(stderr, "%#x is unknown instruction code.\n", code);
}

void bad_oprand_B(const Uword code) {
    fprintf(stderr, "%#x is bad operand B.\n", code);
}
