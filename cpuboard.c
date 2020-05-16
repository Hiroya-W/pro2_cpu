/*
 *	Project-based Learning II (CPU)
 *
 *	Program:	instruction set simulator of the Educational CPU Board
 *	File Name:	cpuboard.c
 *	Descrioption:	simulation(emulation) of an instruction
 */

#include "cpuboard.h"

#include <stdio.h>

#define NOP 0x00
#define HLT 0x0f
#define OUT 0x10
#define IN 0x1f
#define RCF 0x20
#define SCF 0x2f

#define LD 0x60
#define ST 0x70

#define ACC 0x00
#define IX 0x01
#define IMMEDIATE_ADDRESS 0x02
#define ABSOLUTE_PROGRAM_ADDRESS 0x04
#define ABSOLUTE_DATA_ADDRESS 0x05
#define IX_MODIFICATION_PROGRAM_ADDRESS 0x06
#define IX_MODIFICATION_DATA_ADDRESS 0x07

int step_ST();
int step_LD();
int step_OUT();
int step_IN();
int step_RCF();
int step_SCF();
Uword decrypt_operand_a(const Uword code);
Uword decrypt_operand_b(const Uword code);
void unknown_instruction_code(const Uword code);
void bad_oprand_B(const Uword code);

Uword MAR;
Uword IR;

Cpub *cpub;

int step(Cpub *step_cpub) {
    cpub = step_cpub;

    int return_status = RUN_HALT;

    const Uword MASK = 0xf0;

    MAR = cpub->pc;
    IR = cpub->mem[0x000 + MAR];
    const Uword INSTRUCTION_CODE = IR & MASK;
    cpub->pc++;

    switch (INSTRUCTION_CODE) {
        case 0x00:
            /*
             * NOP + HLT + JAL + JR
             */
            if ((IR & 0xf8) == NOP) {
                return_status = RUN_STEP;
            } else if (((IR & 0xfc) | 0x03) == HLT) {
                return_status = RUN_HALT;
            } else {
                unknown_instruction_code(IR);
                return_status = RUN_HALT;
            }
            break;
        case 0x10:
            /*
             * OUT + IN
             */
            if ((IR & 0xf8) == OUT) {
                return_status = step_OUT();
            } else {
                return_status = step_IN();
            }
            break;
        case 0x20:
            /*
             * RCF + SCF
             */
            if ((IR & 0xf8) == RCF) {
                return_status = step_RCF();
            } else {
                return_status = step_SCF();
            }
            break;
        case LD:
            return_status = step_LD();
            break;
        case ST:
            return_status = step_ST();
            break;
        default:
            unknown_instruction_code(IR);
            return_status = RUN_HALT;
            break;
    }

    return return_status;
}

int step_OUT() { return RUN_HALT; }

int step_IN() { return RUN_HALT; }

int step_RCF() {
    cpub->cf = 0;
    return RUN_STEP;
}

int step_SCF() {
    cpub->cf = 1;
    return RUN_STEP;
}

int step_LD() {
    const Uword OPERAND_A = decrypt_operand_a(IR);
    const Uword OPERAND_B = decrypt_operand_b(IR);

    Uword second_word;
    Uword operand_b_value;

    MAR = cpub->pc;
    cpub->pc++;
    second_word = cpub->mem[0x000 + MAR];

    switch (OPERAND_B) {
        case ACC:
            operand_b_value = cpub->acc;
            break;
        case IX:
            operand_b_value = cpub->ix;
            break;
        case IMMEDIATE_ADDRESS:
            operand_b_value = second_word;
            break;
        case ABSOLUTE_PROGRAM_ADDRESS:
            operand_b_value = cpub->mem[0x000 + second_word];
            break;
        case ABSOLUTE_DATA_ADDRESS:
            operand_b_value = cpub->mem[0x100 + second_word];
            break;
        case IX_MODIFICATION_PROGRAM_ADDRESS:
            operand_b_value = cpub->mem[0x000 + cpub->ix + second_word];
            break;
        case IX_MODIFICATION_DATA_ADDRESS:
            operand_b_value = cpub->mem[0x100 + cpub->ix + second_word];
            break;
        default:
            return RUN_HALT;
    }

    if (OPERAND_A == ACC) {
        cpub->acc = operand_b_value;
    } else {
        cpub->ix = operand_b_value;
    }

    return RUN_STEP;
}

int step_ST() {
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

    MAR = cpub->pc;
    cpub->pc++;
    second_word = cpub->mem[0x000 + MAR];

    switch (OPERAND_B) {
        case ACC:
            fprintf(stderr, "ACC is Undefined operating(ST)\n");
            return_status = RUN_HALT;
            break;
        case IX:
            fprintf(stderr, "IX is Undefined operating (ST)\n");
            return_status = RUN_HALT;
            break;
        case IMMEDIATE_ADDRESS:
            fprintf(stderr,
                    "IMMEDIATE_ADDRESS is Undefined operating in (ST)\n");
            return_status = RUN_HALT;
            break;
        case ABSOLUTE_PROGRAM_ADDRESS:
            cpub->mem[0x000 + second_word] = operand_a_value;
            return_status = RUN_STEP;
            break;
        case ABSOLUTE_DATA_ADDRESS:
            cpub->mem[0x100 + second_word] = operand_a_value;
            return_status = RUN_STEP;
            break;
        case IX_MODIFICATION_PROGRAM_ADDRESS:
            cpub->mem[0x000 + cpub->ix + second_word] = operand_a_value;
            return_status = RUN_STEP;
            break;
        case IX_MODIFICATION_DATA_ADDRESS:
            cpub->mem[0x100 + cpub->ix + second_word] = operand_a_value;
            return_status = RUN_STEP;
            break;
        default:
            return_status = RUN_HALT;
    }

    return return_status;
}

Uword decrypt_operand_a(const Uword CODE) {
    const Uword MASK = 0x08;
    return CODE & MASK;
}

Uword decrypt_operand_b(const Uword CODE) {
    const Uword MASK = 0x07;
    Uword operand_b = CODE & MASK;

    if (operand_b == 0x03) {
        operand_b = 0x02;
    }
    return operand_b;
}

void unknown_instruction_code(const Uword code) {
    fprintf(stderr, "%#x is unknown instruction code.\n", code);
}

void bad_oprand_B(const Uword code) {
    fprintf(stderr, "%#x is bad operand B.\n", code);
}
