/*
 *	Project-based Learning II (CPU)
 *
 *	Program:	instruction set simulator of the Educational CPU Board
 *	File Name:	cpuboard.c
 *	Descrioption:	simulation(emulation) of an instruction
 */

#include "cpuboard.h"

#include <stdio.h>

enum Instruction_code {
    NOP = 0x00,
    HLT = 0x0f,
    OUT = 0x10,
    IN = 0x1f,
    RCF = 0x20,
    SCF = 0x2f,
    LD = 0x60,
    ST = 0x70,
    ADD = 0xB0,
    ADC = 0x90,
    SUB = 0xa0,
    SBC = 0x80,
    CMP = 0xf0,
    AND = 0xe0,
    EOR = 0xC0,
    BBC = 0x30,
};

enum Operand_B_3bits {
    ACC = 0x00,
    IX = 0x01,
    IMMEDIATE_ADDRESS = 0x02,
    ABSOLUTE_PROGRAM_ADDRESS = 0x04,
    ABSOLUTE_DATA_ADDRESS = 0x05,
    IX_MODIFICATION_PROGRAM_ADDRESS = 0x06,
    IX_MODIFICATION_DATA_ADDRESS = 0x07
};

int step_OUT();
int step_IN();
int step_RCF();
int step_SCF();
int step_ST();
int step_LD();
int step_ADD();
int step_ADC();
int step_SUB();
int step_SBC();
int step_CMP();
int step_AND();
int step_EOR();
int step_BBC();
void set_flag(Bit cf, Bit vf, Bit nf, Bit zf);
Bit chk_carry_flag(int ans);
Bit chk_overflow_flag(Uword num1, Uword num2, int ans);
Bit chk_negative_flag(int ans);
Bit chk_zero_flag(char ans);
Uword decrypt_operand_a(const Uword code);
Uword decrypt_operand_b(const Uword code);
Uword get_operand_b_value(const Uword OPERAND_B);
Uword get_operand_a_value(const Uword OPERAND_A);
void store_value_to_register(const Uword OPERAND_A, const Uword value);
void unknown_instruction_code(const Uword code);
void bad_oprand_B(const Uword code);

Uword MAR;
Uword IR;

Cpub *cpub;

int step(Cpub *cpub_) {
    cpub = cpub_;

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
        case ADD:
            return_status = step_ADD();
            break;
        case ADC:
            return_status = step_ADC();
            break;
        case SUB:
            return_status = step_SUB();
            break;
        case SBC:
            return_status = step_SBC();
            break;
        case CMP:
            return_status = step_CMP();
            break;
        case AND:
            return_status = step_AND();
            break;
        case EOR:
            return_status = step_EOR();
            break;
        case BBC:
            return_status = step_BBC();
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

    Uword operand_b_value;

    operand_b_value = get_operand_b_value(OPERAND_B);

    store_value_to_register(OPERAND_A, operand_b_value);

    return RUN_STEP;
}

int step_ST() {
    int return_status = RUN_HALT;

    const Uword OPERAND_A = decrypt_operand_a(IR);
    const Uword OPERAND_B = decrypt_operand_b(IR);

    Uword operand_a_value;
    Uword second_word;

    operand_a_value = get_operand_a_value(OPERAND_A);

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

int step_ADD() {
    int return_status = RUN_HALT;

    const Uword OPERAND_A = decrypt_operand_a(IR);
    const Uword OPERAND_B = decrypt_operand_b(IR);

    Uword operand_a_value;
    Uword operand_b_value;

    operand_a_value = get_operand_a_value(OPERAND_A);

    operand_b_value = get_operand_b_value(OPERAND_B);

    int sum = operand_a_value + operand_b_value;
    Bit cf = chk_carry_flag(sum);
    Bit vf = chk_overflow_flag(operand_a_value, operand_b_value, sum);
    Bit nf = chk_negative_flag(sum);
    Bit zf = chk_zero_flag(sum & 0xff);
    /* ADDはCFを使わない carryが出たらそれはoverflowである */
    set_flag(0, cf | vf, nf, zf);

    store_value_to_register(OPERAND_A, sum & 0xff);

    return_status = RUN_STEP;
    return return_status;
}

int step_ADC() {
    int return_status = RUN_HALT;

    const Uword OPERAND_A = decrypt_operand_a(IR);
    const Uword OPERAND_B = decrypt_operand_b(IR);

    Uword operand_a_value;
    Uword operand_b_value;

    operand_a_value = get_operand_a_value(OPERAND_A);

    operand_b_value = get_operand_b_value(OPERAND_B);

    int carry = cpub->cf;
    int sum = operand_a_value + operand_b_value + carry;
    Bit cf = chk_carry_flag(sum);
    Bit vf = chk_overflow_flag(operand_a_value, operand_b_value, sum);
    Bit nf = chk_negative_flag(sum);
    Bit zf = chk_zero_flag(sum & 0xff);
    set_flag(cf, vf, nf, zf);

    store_value_to_register(OPERAND_A, sum & 0xff);

    return_status = RUN_STEP;
    return return_status;
}

int step_SUB() {
    int return_status = RUN_HALT;

    const Uword OPERAND_A = decrypt_operand_a(IR);
    const Uword OPERAND_B = decrypt_operand_b(IR);

    Uword operand_a_value;
    Uword operand_b_value;

    operand_a_value = get_operand_a_value(OPERAND_A);

    operand_b_value = get_operand_b_value(OPERAND_B);
    operand_b_value = (~operand_b_value) + 1;

    int sum = operand_a_value + operand_b_value;
    Bit cf = 0;
    Bit vf = chk_overflow_flag(operand_a_value, operand_b_value, sum);
    Bit nf = chk_negative_flag(sum);
    Bit zf = chk_zero_flag(sum & 0xff);

    set_flag(cf, vf, nf, zf);

    store_value_to_register(OPERAND_A, sum & 0xff);

    return_status = RUN_STEP;
    return return_status;
}

int step_SBC() {
    int return_status = RUN_HALT;

    const Uword OPERAND_A = decrypt_operand_a(IR);
    const Uword OPERAND_B = decrypt_operand_b(IR);

    Uword operand_a_value;
    Uword operand_b_value;

    operand_a_value = get_operand_a_value(OPERAND_A);

    operand_b_value = get_operand_b_value(OPERAND_B);
    operand_b_value = (~operand_b_value) + 1;

    int sum = operand_a_value + operand_b_value;
    Bit cf = !chk_carry_flag(sum);
    Bit vf = chk_overflow_flag(operand_a_value, operand_b_value, sum);
    Bit nf = chk_negative_flag(sum);
    Bit zf = chk_zero_flag(sum & 0xff);

    set_flag(cf, vf, nf, zf);

    store_value_to_register(OPERAND_A, sum & 0xff);

    return_status = RUN_STEP;
    return return_status;
}

int step_CMP() {
    int return_status = RUN_HALT;

    const Uword OPERAND_A = decrypt_operand_a(IR);
    const Uword OPERAND_B = decrypt_operand_b(IR);

    Uword operand_a_value;
    Uword operand_b_value;

    operand_a_value = get_operand_a_value(OPERAND_A);

    // SUB命令と同じ処理
    // ZFが立っていたらA=Bであることがわかる
    operand_b_value = get_operand_b_value(OPERAND_B);
    operand_b_value = (~operand_b_value) + 1;

    int sum = operand_a_value + operand_b_value;
    Bit cf = 0;
    Bit vf = chk_overflow_flag(operand_a_value, operand_b_value, sum);
    Bit nf = chk_negative_flag(sum);
    Bit zf = chk_zero_flag(sum & 0xff);

    set_flag(cf, vf, nf, zf);

    return_status = RUN_STEP;
    return return_status;
}

int step_AND() {
    int return_status = RUN_HALT;

    const Uword OPERAND_A = decrypt_operand_a(IR);
    const Uword OPERAND_B = decrypt_operand_b(IR);

    Uword operand_a_value;
    Uword operand_b_value;

    operand_a_value = get_operand_a_value(OPERAND_A);

    operand_b_value = get_operand_b_value(OPERAND_B);

    Uword ans = operand_a_value & operand_b_value;
    Bit cf = cpub->cf;
    Bit vf = 0;
    Bit nf = chk_negative_flag(ans);
    Bit zf = chk_zero_flag(ans);
    set_flag(cf, vf, nf, zf);

    store_value_to_register(OPERAND_A, ans);

    return_status = RUN_STEP;
    return return_status;
}

int step_EOR() {
    int return_status = RUN_HALT;

    const Uword OPERAND_A = decrypt_operand_a(IR);
    const Uword OPERAND_B = decrypt_operand_b(IR);

    Uword operand_a_value;
    Uword operand_b_value;

    operand_a_value = get_operand_a_value(OPERAND_A);

    operand_b_value = get_operand_b_value(OPERAND_B);

    Uword ans = operand_a_value ^ operand_b_value;
    Bit cf = cpub->cf;
    Bit vf = 0;
    Bit nf = chk_negative_flag(ans);
    Bit zf = chk_zero_flag(ans);
    set_flag(cf, vf, nf, zf);

    store_value_to_register(OPERAND_A, ans);

    return_status = RUN_STEP;
    return return_status;
}

int step_BBC() {
    enum Branch {
        ALWAYS,
        NOT_ZERO,
        ZERO_OR_POSITIVE,
        POSITIVE,
        NO_INPUT,
        NO_CARRY,
        GREATER_THAN_OR_EQUAL,
        GREATER_THAN,
        OVERFLOW,
        ZERO,
        NEGATIVE,
        ZERO_OR_NEGATIVE,
        NO_OUTPUT,
        CARRY,
        LESS_THAN,
        LESS_THAN_OR_EQUAL
    };

    int return_status = RUN_HALT;

    const Uword MASK = 0x0f;
    const Uword BRANCH_CODE = IR & MASK;
    Uword second_word;

    MAR = cpub->pc;
    cpub->pc++;
    second_word = cpub->mem[0x000 + MAR];

    switch (BRANCH_CODE) {
        case NOT_ZERO:
            if (!cpub->zf) {
                cpub->pc = second_word;
            }
            break;
    }

    return_status = RUN_STEP;
    return return_status;
}

void set_flag(Bit cf, Bit vf, Bit nf, Bit zf) {
    if (cf) {
        cpub->cf = 1;
    } else {
        cpub->cf = 0;
    }

    if (vf) {
        cpub->vf = 1;
    } else {
        cpub->vf = 0;
    }

    if (nf) {
        cpub->nf = 1;
    } else {
        cpub->nf = 0;
    }

    if (zf) {
        cpub->zf = 1;
    } else {
        cpub->zf = 0;
    }
    return;
}

Bit chk_carry_flag(int ans) {
    if ((ans >> 8) & 1) {
        return 1;
    } else {
        return 0;
    }
}

Bit chk_overflow_flag(Uword num1, Uword num2, int ans) {
    char MSB_A = (num1 >> 7) & 1;
    char MSB_B = (num2 >> 7) & 1;
    char MSB_C = (ans >> 7) & 1;
    if ((MSB_A & MSB_B & !MSB_C) | (!MSB_A & !MSB_B & MSB_C)) {
        return 1;
    } else {
        return 0;
    }
}

Bit chk_negative_flag(int ans) {
    if ((ans >> 7) & 1) {
        return 1;
    } else {
        return 0;
    }
}

Bit chk_zero_flag(char ans) {
    if (ans) {
        return 0;
    } else {
        return 1;
    }
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

Uword get_operand_a_value(const Uword OPERAND_A) {
    Uword operand_a_value;
    if (OPERAND_A == ACC) {
        operand_a_value = cpub->acc;
    } else {
        operand_a_value = cpub->ix;
    }

    return operand_a_value;
}

Uword get_operand_b_value(const Uword OPERAND_B) {
    Uword operand_b_value;
    Uword second_word;
    switch (OPERAND_B) {
        case ACC:
            operand_b_value = cpub->acc;
            break;
        case IX:
            operand_b_value = cpub->ix;
            break;
        case IMMEDIATE_ADDRESS:
            MAR = cpub->pc;
            cpub->pc++;
            second_word = cpub->mem[0x000 + MAR];
            operand_b_value = second_word;
            break;
        case ABSOLUTE_PROGRAM_ADDRESS:
            MAR = cpub->pc;
            cpub->pc++;
            second_word = cpub->mem[0x000 + MAR];
            operand_b_value = cpub->mem[0x000 + second_word];
            break;
        case ABSOLUTE_DATA_ADDRESS:
            MAR = cpub->pc;
            cpub->pc++;
            second_word = cpub->mem[0x000 + MAR];
            operand_b_value = cpub->mem[0x100 + second_word];
            break;
        case IX_MODIFICATION_PROGRAM_ADDRESS:
            MAR = cpub->pc;
            cpub->pc++;
            second_word = cpub->mem[0x000 + MAR];
            operand_b_value = cpub->mem[0x000 + cpub->ix + second_word];
            break;
        case IX_MODIFICATION_DATA_ADDRESS:
            MAR = cpub->pc;
            cpub->pc++;
            second_word = cpub->mem[0x000 + MAR];
            operand_b_value = cpub->mem[0x100 + cpub->ix + second_word];
            break;
    }
    return operand_b_value;
}

void store_value_to_register(const Uword OPERAND_A, const Uword value) {
    if (OPERAND_A == ACC) {
        cpub->acc = value;
    } else {
        cpub->ix = value;
    }
    return;
}

void unknown_instruction_code(const Uword code) {
    fprintf(stderr, "%#x is unknown instruction code.\n", code);
}

void bad_oprand_B(const Uword code) {
    fprintf(stderr, "%#x is bad operand B.\n", code);
}
