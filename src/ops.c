#include "ops.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdnoreturn.h>
#include "bus.h"
#include "cpu.h"

static uint8_t read_r(gb_t *gb, uint8_t idx);
static uint8_t read_n(gb_t *gb);
static int8_t read_e(gb_t *gb);
static uint8_t read_ind(gb_t *gb, uint8_t idx);
static uint16_t read_rr(gb_t *gb, uint8_t idx);
static uint16_t read_rr_stk(gb_t *gb, uint8_t idx);
static uint16_t read_nn(gb_t *gb);

static void write_r(gb_t *gb, uint8_t idx, uint8_t val);
static void write_ind(gb_t *gb, uint8_t idx, uint8_t val);
static void write_rr(gb_t *gb, uint8_t idx, uint16_t val);
static void write_rr_stk(gb_t *gb, uint8_t idx, uint16_t val);

static bool check_cc(gb_t *gb, uint8_t idx);

static noreturn void op_unimplemented(gb_t *gb, uint8_t opcode);

static uint8_t op_nop(gb_t *gb);
static uint8_t op_jp_nn(gb_t *gb);
static uint8_t op_xor_r(gb_t *gb, uint8_t opcode);
static uint8_t op_ld_rr_nn(gb_t *gb, uint8_t opcode);
static uint8_t op_ld_r_n(gb_t *gb, uint8_t opcode);
static uint8_t op_ld_ind_a(gb_t *gb, uint8_t opcode);
static uint8_t op_ld_a_ind(gb_t *gb, uint8_t opcode);
static uint8_t op_dec_r(gb_t *gb, uint8_t opcode);
static uint8_t op_jr_cc_e(gb_t *gb, uint8_t opcode);
static uint8_t op_di(gb_t *gb);
static uint8_t op_ldh_n_a(gb_t *gb);
static uint8_t op_ldh_a_n(gb_t *gb);
static uint8_t op_ldh_c_a(gb_t *gb);
static uint8_t op_ldh_a_c(gb_t *gb);
static uint8_t op_cp_n(gb_t *gb);
static uint8_t op_ld_nn_a(gb_t *gb);
static uint8_t op_ld_a_nn(gb_t *gb);
static uint8_t op_inc_r(gb_t *gb, uint8_t opcode);
static uint8_t op_call_nn(gb_t *gb);
static uint8_t op_ret(gb_t *gb);
static uint8_t op_dec_rr(gb_t *gb, uint8_t opcode);
static uint8_t op_inc_rr(gb_t *gb, uint8_t opcode);
static uint8_t op_ld_r_r(gb_t *gb, uint8_t opcode);
static uint8_t op_or_r(gb_t *gb, uint8_t opcode);
static uint8_t op_and_r(gb_t *gb, uint8_t opcode);
static uint8_t op_sub_r(gb_t *gb, uint8_t opcode);
static uint8_t op_add_r(gb_t *gb, uint8_t opcode);
static uint8_t op_ei(gb_t *gb);
static uint8_t op_cpl(gb_t *gb);
static uint8_t op_and_n(gb_t *gb);
static uint8_t op_rst_n(gb_t *gb, uint8_t opcode);
static uint8_t op_pop_rr(gb_t *gb, uint8_t opcode);
static uint8_t op_push_rr(gb_t *gb, uint8_t opcode);
static uint8_t op_add_hl_rr(gb_t *gb, uint8_t opcode);
static uint8_t op_jp_hl(gb_t *gb);
static uint8_t op_jp_cc_nn(gb_t *gb, uint8_t opcode);
static uint8_t op_ret_cc(gb_t *gb, uint8_t opcode);
static uint8_t op_jr_e(gb_t *gb);
static uint8_t op_reti(gb_t *gb);
static uint8_t op_add_n(gb_t *gb);
static uint8_t op_or_n(gb_t *gb);
static uint8_t op_rlca(gb_t *gb);
static uint8_t op_adc_r(gb_t *gb, uint8_t opcode);
static uint8_t op_cp_r(gb_t *gb, uint8_t opcode);
static uint8_t op_sub_n(gb_t *gb);
static uint8_t op_daa(gb_t *gb);
static uint8_t op_xor_n(gb_t *gb);

static uint8_t cpu_execute_cb(gb_t *gb, uint8_t opcode);

static noreturn void op_unimplemented_cb(gb_t *gb, uint8_t opcode);

static uint8_t op_sla_r(gb_t *gb, uint8_t opcode);
static uint8_t op_swap_r(gb_t *gb, uint8_t opcode);
static uint8_t op_srl_r(gb_t *gb, uint8_t opcode);
static uint8_t op_res_b_r(gb_t *gb, uint8_t opcode);
static uint8_t op_set_b_r(gb_t *gb, uint8_t opcode);
static uint8_t op_bit_b_r(gb_t *gb, uint8_t opcode);

uint8_t cpu_execute(gb_t *gb, uint8_t opcode)
{
    switch (opcode)
    {
    case 0x00: return op_nop(gb);
    case 0xC3: return op_jp_nn(gb);
    case 0xA8: case 0xA9: case 0xAA: case 0xAB: case 0xAC: case 0xAD: case 0xAE: case 0xAF: return op_xor_r(gb, opcode);
    case 0x01: case 0x11: case 0x21: case 0x31: return op_ld_rr_nn(gb, opcode);
    case 0x06: case 0x0E: case 0x16: case 0x1E: case 0x26: case 0x2E: case 0x36: case 0x3E: return op_ld_r_n(gb, opcode);
    case 0x02: case 0x12: case 0x22: case 0x32: return op_ld_ind_a(gb, opcode);
    case 0x0A: case 0x1A: case 0x2A: case 0x3A: return op_ld_a_ind(gb, opcode);
    case 0x05: case 0x0D: case 0x15: case 0x1D: case 0x25: case 0x2D: case 0x35: case 0x3D: return op_dec_r(gb, opcode);
    case 0x20: case 0x28: case 0x30: case 0x38: return op_jr_cc_e(gb, opcode);
    case 0xF3: return op_di(gb);
    case 0xE0: return op_ldh_n_a(gb);
    case 0xF0: return op_ldh_a_n(gb);
    case 0xE2: return op_ldh_c_a(gb);
    case 0xF2: return op_ldh_a_c(gb);
    case 0xFE: return op_cp_n(gb);
    case 0xEA: return op_ld_nn_a(gb);
    case 0xFA: return op_ld_a_nn(gb);
    case 0x04: case 0x0c: case 0x14: case 0x1c: case 0x24: case 0x2c: case 0x34: case 0x3c: return op_inc_r(gb, opcode);
    case 0xCD: return op_call_nn(gb);
    case 0xC9: return op_ret(gb);
    case 0x0B: case 0x1B: case 0x2B: case 0x3B: return op_dec_rr(gb, opcode);
    case 0x03: case 0x13: case 0x23: case 0x33: return op_inc_rr(gb, opcode);
    case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x46: case 0x47: case 0x48: case 0x49: case 0x4A: case 0x4B: case 0x4C: case 0x4D: case 0x4E: case 0x4F: return op_ld_r_r(gb, opcode);
    case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x56: case 0x57: case 0x58: case 0x59: case 0x5A: case 0x5B: case 0x5C: case 0x5D: case 0x5E: case 0x5F: return op_ld_r_r(gb, opcode);
    case 0x60: case 0x61: case 0x62: case 0x63: case 0x64: case 0x65: case 0x66: case 0x67: case 0x68: case 0x69: case 0x6A: case 0x6B: case 0x6C: case 0x6D: case 0x6E: case 0x6F: return op_ld_r_r(gb, opcode);
    case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x77: case 0x78: case 0x79: case 0x7A: case 0x7B: case 0x7C: case 0x7D: case 0x7E: case 0x7F: return op_ld_r_r(gb, opcode);
    case 0xB0: case 0xB1: case 0xB2: case 0xB3: case 0xB4: case 0xB5: case 0xB6: case 0xB7: return op_or_r(gb, opcode);
    case 0xA0: case 0xA1: case 0xA2: case 0xA3: case 0xA4: case 0xA5: case 0xA6: case 0xA7: return op_and_r(gb, opcode);
    case 0x90: case 0x91: case 0x92: case 0x93: case 0x94: case 0x95: case 0x96: case 0x97: return op_sub_r(gb, opcode);
    case 0x80: case 0x81: case 0x82: case 0x83: case 0x84: case 0x85: case 0x86: case 0x87: return op_add_r(gb, opcode);
    case 0xFB: return op_ei(gb);
    case 0x2F: return op_cpl(gb);
    case 0xE6: return op_and_n(gb);
    case 0xC7: case 0xCF: case 0xD7: case 0xDF: case 0xE7: case 0xEF: case 0xF7: case 0xFF: return op_rst_n(gb, opcode);
    case 0xC1: case 0xD1: case 0xE1: case 0xF1: return op_pop_rr(gb, opcode);
    case 0xC5: case 0xD5: case 0xE5: case 0xF5: return op_push_rr(gb, opcode);
    case 0x09: case 0x19: case 0x29: case 0x39: return op_add_hl_rr(gb, opcode);
    case 0xE9: return op_jp_hl(gb);
    case 0xC2: case 0xCA: case 0xD2: case 0xDA: return op_jp_cc_nn(gb, opcode);
    case 0xC0: case 0xC8: case 0xD0: case 0xD8: return op_ret_cc(gb, opcode);
    case 0x18: return op_jr_e(gb);
    case 0xD9: return op_reti(gb);
    case 0xC6: return op_add_n(gb);
    case 0xF6: return op_or_n(gb);
    case 0x07: return op_rlca(gb);
    case 0x88: case 0x89: case 0x8A: case 0x8B: case 0x8C: case 0x8D: case 0x8E: case 0x8F: return op_adc_r(gb, opcode);
    case 0xB8: case 0xB9: case 0xBA: case 0xBB: case 0xBC: case 0xBD: case 0xBE: case 0xBF: return op_cp_r(gb, opcode);
    case 0xD6: return op_sub_n(gb);
    case 0x27: return op_daa(gb);
    case 0xEE: return op_xor_n(gb);
    
    case 0xCB: return cpu_execute_cb(gb, read_n(gb));

    default: op_unimplemented(gb, opcode);
    }
}

static uint8_t cpu_execute_cb(gb_t *gb, uint8_t opcode)
{
    switch (opcode)
    {
    case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27: return op_sla_r(gb, opcode);
    case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37: return op_swap_r(gb, opcode);
    case 0x38: case 0x39: case 0x3A: case 0x3B: case 0x3C: case 0x3D: case 0x3E: case 0x3F: return op_srl_r(gb, opcode);
    case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x46: case 0x47: case 0x48: case 0x49: case 0x4A: case 0x4B: case 0x4C: case 0x4D: case 0x4E: case 0x4F: return op_bit_b_r(gb, opcode);
    case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x56: case 0x57: case 0x58: case 0x59: case 0x5A: case 0x5B: case 0x5C: case 0x5D: case 0x5E: case 0x5F: return op_bit_b_r(gb, opcode);
    case 0x60: case 0x61: case 0x62: case 0x63: case 0x64: case 0x65: case 0x66: case 0x67: case 0x68: case 0x69: case 0x6A: case 0x6B: case 0x6C: case 0x6D: case 0x6E: case 0x6F: return op_bit_b_r(gb, opcode);
    case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77: case 0x78: case 0x79: case 0x7A: case 0x7B: case 0x7C: case 0x7D: case 0x7E: case 0x7F: return op_bit_b_r(gb, opcode);
    case 0x80: case 0x81: case 0x82: case 0x83: case 0x84: case 0x85: case 0x86: case 0x87: case 0x88: case 0x89: case 0x8A: case 0x8B: case 0x8C: case 0x8D: case 0x8E: case 0x8F: return op_res_b_r(gb, opcode);
    case 0x90: case 0x91: case 0x92: case 0x93: case 0x94: case 0x95: case 0x96: case 0x97: case 0x98: case 0x99: case 0x9A: case 0x9B: case 0x9C: case 0x9D: case 0x9E: case 0x9F: return op_res_b_r(gb, opcode);
    case 0xA0: case 0xA1: case 0xA2: case 0xA3: case 0xA4: case 0xA5: case 0xA6: case 0xA7: case 0xA8: case 0xA9: case 0xAA: case 0xAB: case 0xAC: case 0xAD: case 0xAE: case 0xAF: return op_res_b_r(gb, opcode);
    case 0xB0: case 0xB1: case 0xB2: case 0xB3: case 0xB4: case 0xB5: case 0xB6: case 0xB7: case 0xB8: case 0xB9: case 0xBA: case 0xBB: case 0xBC: case 0xBD: case 0xBE: case 0xBF: return op_res_b_r(gb, opcode);
    case 0xC0: case 0xC1: case 0xC2: case 0xC3: case 0xC4: case 0xC5: case 0xC6: case 0xC7: case 0xC8: case 0xC9: case 0xCA: case 0xCB: case 0xCC: case 0xCD: case 0xCE: case 0xCF: return op_set_b_r(gb, opcode);
    case 0xD0: case 0xD1: case 0xD2: case 0xD3: case 0xD4: case 0xD5: case 0xD6: case 0xD7: case 0xD8: case 0xD9: case 0xDA: case 0xDB: case 0xDC: case 0xDD: case 0xDE: case 0xDF: return op_set_b_r(gb, opcode);
    case 0xE0: case 0xE1: case 0xE2: case 0xE3: case 0xE4: case 0xE5: case 0xE6: case 0xE7: case 0xE8: case 0xE9: case 0xEA: case 0xEB: case 0xEC: case 0xED: case 0xEE: case 0xEF: return op_set_b_r(gb, opcode);
    case 0xF0: case 0xF1: case 0xF2: case 0xF3: case 0xF4: case 0xF5: case 0xF6: case 0xF7: case 0xF8: case 0xF9: case 0xFA: case 0xFB: case 0xFC: case 0xFD: case 0xFE: case 0xFF: return op_set_b_r(gb, opcode);

    default: op_unimplemented_cb(gb, opcode);
    }
    
}


#pragma region Helpers

static uint8_t read_r(gb_t *gb, uint8_t idx)
{
    switch (idx)
    {
    case 0: return gb->cpu.b;
    case 1: return gb->cpu.c;
    case 2: return gb->cpu.d;
    case 3: return gb->cpu.e;
    case 4: return gb->cpu.h;
    case 5: return gb->cpu.l;
    case 6: return bus_read(gb, gb->cpu.hl);
    case 7: return gb->cpu.a;
    
    default:
        fprintf(stderr, "read_r idx %u out of range\n", idx);
        exit(1);
    }

    return 0;
}

static uint8_t read_n(gb_t *gb)
{
    return bus_read(gb, gb->cpu.pc++);
}

static int8_t read_e(gb_t *gb)
{
    return (int8_t)bus_read(gb, gb->cpu.pc++);
}

static uint8_t read_ind(gb_t *gb, uint8_t idx)
{
    switch (idx)
    {
    case 0: return bus_read(gb, gb->cpu.bc);                
    case 1: return bus_read(gb, gb->cpu.de);                
    case 2: return bus_read(gb, gb->cpu.hl++);                
    case 3: return bus_read(gb, gb->cpu.hl--);                
    
    default:
        fprintf(stderr, "read_ind idx %u out of range\n", idx);
        exit(1);
    }
}

static uint16_t read_rr(gb_t *gb, uint8_t idx)
{
    switch (idx)
    {
    case 0: return gb->cpu.bc;
    case 1: return gb->cpu.de;
    case 2: return gb->cpu.hl;
    case 3: return gb->cpu.sp;
    
    default:
        fprintf(stderr, "read_rr idx %u out of range\n", idx);
        exit(1);
    }
}

static uint16_t read_rr_stk(gb_t *gb, uint8_t idx)
{
    switch (idx)
    {
    case 0: return gb->cpu.bc;
    case 1: return gb->cpu.de;
    case 2: return gb->cpu.hl;
    case 3: return gb->cpu.af;
    
    default:
        fprintf(stderr, "read_rr_stk idx %u out of range\n", idx);
        exit(1);
    }
}

static uint16_t read_nn(gb_t *gb)
{
    uint8_t lsb = bus_read(gb, gb->cpu.pc++);
    uint8_t msb = bus_read(gb, gb->cpu.pc++);

    return (uint16_t)(msb << 8) | lsb;
}

static void write_r(gb_t *gb, uint8_t idx, uint8_t val)
{
    switch (idx)
    {
    case 0: gb->cpu.b = val; break; 
    case 1: gb->cpu.c = val; break;
    case 2: gb->cpu.d = val; break;
    case 3: gb->cpu.e = val; break;
    case 4: gb->cpu.h = val; break;
    case 5: gb->cpu.l = val; break;
    case 6: bus_write(gb, gb->cpu.hl, val); break;
    case 7: gb->cpu.a = val; break;
    
    default:
        fprintf(stderr, "write_r idx %u out of range\n", idx);
        exit(1);
    }
}

static void write_ind(gb_t *gb, uint8_t idx, uint8_t val)
{
    switch (idx)
    {
    case 0: bus_write(gb, gb->cpu.bc, val); break;
    case 1: bus_write(gb, gb->cpu.de, val); break;
    case 2: bus_write(gb, gb->cpu.hl++, val); break;
    case 3: bus_write(gb, gb->cpu.hl--, val); break;
    
    default:
        fprintf(stderr, "write_ind idx %u out of range\n", idx);
        exit(1);
    }
}

static void write_rr(gb_t *gb, uint8_t idx, uint16_t val)
{
    switch (idx)
    {
    case 0: gb->cpu.bc = val; break;
    case 1: gb->cpu.de = val; break;
    case 2: gb->cpu.hl = val; break;
    case 3: gb->cpu.sp = val; break;
    
    default:
        fprintf(stderr, "write_rr idx %u out of range\n", idx);
        exit(1);
    }
}

static void write_rr_stk(gb_t *gb, uint8_t idx, uint16_t val)
{
    switch (idx)
    {
    case 0: gb->cpu.bc = val; break;
    case 1: gb->cpu.de = val; break;
    case 2: gb->cpu.hl = val; break;
    case 3: gb->cpu.af = val; break;
    
    default:
        fprintf(stderr, "write_rr_stk idx %u out of range\n", idx);
        exit(1);
    }
}

static bool check_cc(gb_t *gb, uint8_t idx)
{
    switch (idx)
    {
    case 0: return !(gb->cpu.f & FLAG_Z);
    case 1: return gb->cpu.f & FLAG_Z;
    case 2: return !(gb->cpu.f & FLAG_C);
    case 3: return gb->cpu.f & FLAG_C;
    
    default:
        fprintf(stderr, "check_cc idx %u out of range\n", idx);
        exit(1);
    }
}

#pragma endregion


#pragma region Ops

static noreturn void op_unimplemented(gb_t *gb, uint8_t opcode)
{
    fprintf(stderr, "Unimplemented opcode 0x%02X at 0x%04X\n", opcode, gb->cpu.pc - 1);
    exit(1);
}

static uint8_t op_nop(gb_t *gb)
{
    (void)gb;
    return 4;
}

static uint8_t op_jp_nn(gb_t *gb)
{
    gb->cpu.pc = read_nn(gb);

    return 16;
}

static uint8_t op_xor_r(gb_t *gb, uint8_t opcode)
{
    uint8_t idx = opcode & 0x07;
    gb->cpu.a ^= read_r(gb, idx);
    gb->cpu.f = (gb->cpu.a == 0) ? FLAG_Z : 0;

    return idx == 6 ? 8 : 4;
}

static uint8_t op_ld_rr_nn(gb_t *gb, uint8_t opcode)
{
    uint8_t idx = (opcode & 0x30) >> 4;

    write_rr(gb, idx, read_nn(gb));

    return 12;
}

static uint8_t op_ld_r_n(gb_t *gb, uint8_t opcode)
{
    uint8_t idx = (opcode & 0x38) >> 3;

    write_r(gb, idx, read_n(gb));

    return idx == 6 ? 12 : 8;
}

static uint8_t op_ld_ind_a(gb_t *gb, uint8_t opcode)
{
    uint8_t idx = (opcode & 0x30) >> 4;
    
    write_ind(gb, idx, gb->cpu.a);

    return 8;
}

static uint8_t op_ld_a_ind(gb_t *gb, uint8_t opcode)
{
    uint8_t idx = (opcode & 0x30) >> 4;

    gb->cpu.a = read_ind(gb, idx);

    return 8;
}

static uint8_t op_dec_r(gb_t *gb, uint8_t opcode)
{
    uint8_t idx = (opcode & 0x38) >> 3;
    uint8_t val = read_r(gb, idx);
    uint8_t result = val - 1;

    bool h = (val & 0x0F) < 1;
    bool z = (result == 0);

    write_r(gb, idx, result);

    gb->cpu.f = (z ? FLAG_Z : 0) | FLAG_N | (h ? FLAG_H : 0) | (gb->cpu.f & FLAG_C);

    return idx == 6 ? 12 : 4;
}

static uint8_t op_jr_cc_e(gb_t *gb, uint8_t opcode)
{
    uint8_t idx = (opcode & 0x18) >> 3;
    int8_t e = read_e(gb);
    bool cc = check_cc(gb, idx);

    if (cc) gb->cpu.pc += e;

    return cc ? 12 : 8;
}

static uint8_t op_di(gb_t *gb)
{
    gb->cpu.ime = false;
    gb->cpu.ime_pending = false;

    return 4;
}

static uint8_t op_ldh_n_a(gb_t *gb)
{
    uint16_t addr = 0xFF00 | read_n(gb);

    bus_write(gb, addr, gb->cpu.a);

    return 12;
}

static uint8_t op_ldh_a_n(gb_t *gb)
{
    uint16_t addr = 0xFF00 | read_n(gb);

    gb->cpu.a = bus_read(gb, addr);

    return 12;
}

static uint8_t op_ldh_c_a(gb_t *gb)
{
    uint16_t addr = 0xFF00 | gb->cpu.c;

    bus_write(gb, addr, gb->cpu.a);

    return 8;
}

static uint8_t op_ldh_a_c(gb_t *gb)
{
    uint16_t addr = 0xFF00 | gb->cpu.c;

    gb->cpu.a = bus_read(gb, addr);

    return 8;
}

static uint8_t op_cp_n(gb_t *gb)
{
    uint8_t n = read_n(gb);
    uint8_t a = gb->cpu.a;
    uint8_t result = a - n;

    bool z = (result == 0);
    bool h = (a & 0x0F) < (n & 0x0F);
    bool c = a < n;

    gb->cpu.f = (z ? FLAG_Z : 0) | FLAG_N | (h ? FLAG_H : 0) | (c ? FLAG_C : 0);

    return 8;
}

static uint8_t op_ld_nn_a(gb_t *gb)
{
    bus_write(gb, read_nn(gb), gb->cpu.a);

    return 16;
}

static uint8_t op_ld_a_nn(gb_t *gb)
{
    gb->cpu.a = bus_read(gb, read_nn(gb));

    return 16;
}

static uint8_t op_inc_r(gb_t *gb, uint8_t opcode)
{
    uint8_t idx = (opcode & 0x38) >> 3;
    uint8_t val = read_r(gb, idx);
    uint8_t result = val + 1;

    bool z = result == 0;
    bool h = ((val & 0x0F) + 1) > 0x0F;

    write_r(gb, idx, result);

    gb->cpu.f = (z ? FLAG_Z : 0) | (h ? FLAG_H : 0) | (gb->cpu.f & FLAG_C);

    return idx == 6 ? 12 : 4; 
}

static uint8_t op_call_nn(gb_t *gb)
{
    uint16_t nn = read_nn(gb);

    push_u16(gb, gb->cpu.pc);

    gb->cpu.pc = nn;

    return 24;
}

static uint8_t op_ret(gb_t *gb)
{
    gb->cpu.pc = pop_u16(gb);

    return 16;
}

static uint8_t op_dec_rr(gb_t *gb, uint8_t opcode)
{
    uint8_t idx = (opcode & 0x30) >> 4;
    uint16_t val = read_rr(gb, idx);

    write_rr(gb, idx, val - 1);

    return 8;
}

static uint8_t op_inc_rr(gb_t *gb, uint8_t opcode)
{
    uint8_t idx = (opcode & 0x30) >> 4;
    uint16_t val = read_rr(gb, idx);

    write_rr(gb, idx, val + 1);

    return 8;
}

static uint8_t op_ld_r_r(gb_t *gb, uint8_t opcode)
{
    uint8_t idx_x = (opcode & 0x38) >> 3;
    uint8_t idx_y = (opcode & 0x07);

    write_r(gb, idx_x, read_r(gb, idx_y));

    return (idx_x == 6 || idx_y == 6) ? 8 : 4;
}

static uint8_t op_or_r(gb_t *gb, uint8_t opcode)
{
    uint8_t idx = opcode & 0x07;
    gb->cpu.a |= read_r(gb, idx);
    gb->cpu.f = (gb->cpu.a == 0) ? FLAG_Z : 0;

    return idx == 6 ? 8 : 4;
}

static uint8_t op_and_r(gb_t *gb, uint8_t opcode)
{
    uint8_t idx = opcode & 0x07;
    gb->cpu.a &= read_r(gb, idx);
    gb->cpu.f = (gb->cpu.a == 0 ? FLAG_Z : 0) | FLAG_H;

    return idx == 6 ? 8 : 4;
}

static uint8_t op_sub_r(gb_t *gb, uint8_t opcode)
{
    uint8_t idx = opcode & 0x07;
    uint8_t val = read_r(gb, idx);
    uint8_t result = gb->cpu.a - val;

    bool z = (result == 0);
    bool h = (gb->cpu.a & 0x0F) < (val & 0x0F);
    bool c = gb->cpu.a < val;

    gb->cpu.a = result;

    gb->cpu.f = (z ? FLAG_Z : 0) | FLAG_N | (h ? FLAG_H : 0) | (c ? FLAG_C : 0);

    return idx == 6 ? 8 : 4;
}

static uint8_t op_add_r(gb_t *gb, uint8_t opcode)
{
    uint8_t idx = opcode & 0x07;
    uint8_t val = read_r(gb, idx);
    uint8_t result = gb->cpu.a + val;

    bool z = (result == 0);
    bool h = (gb->cpu.a & 0x0F) + (val & 0x0F) > 0x0F;
    bool c = (uint16_t)gb->cpu.a + (uint16_t)val > 0xFF;

    gb->cpu.a = result;

    gb->cpu.f = (z ? FLAG_Z : 0) | (h ? FLAG_H : 0) | (c ? FLAG_C : 0);

    return idx == 6 ? 8 : 4;
}

static uint8_t op_ei(gb_t *gb)
{
    gb->cpu.ime_pending = true;

    return 4;
}

static uint8_t op_cpl(gb_t *gb)
{
    gb->cpu.a = ~gb->cpu.a;

    gb->cpu.f = (gb->cpu.f & FLAG_Z) | FLAG_N | FLAG_H | (gb->cpu.f & FLAG_C);

    return 4;
}

static uint8_t op_and_n(gb_t *gb)
{
    gb->cpu.a &= read_n(gb);
    gb->cpu.f = (gb->cpu.a == 0 ? FLAG_Z : 0) | FLAG_H;

    return 8;
}

static uint8_t op_rst_n(gb_t *gb, uint8_t opcode)
{
    uint8_t tgt = opcode & 0x38;

    push_u16(gb, gb->cpu.pc);

    gb->cpu.pc = tgt;

    return 16;
}

static uint8_t op_pop_rr(gb_t *gb, uint8_t opcode)
{
    uint8_t idx = (opcode & 0x30) >> 4;

    write_rr_stk(gb, idx, pop_u16(gb));
    if (idx == 3) gb->cpu.f &= 0xF0;

    return 12;
}

static uint8_t op_push_rr(gb_t *gb, uint8_t opcode)
{
    uint8_t idx = (opcode & 0x30) >> 4;

    push_u16(gb, read_rr_stk(gb, idx));

    return 16;
}

static uint8_t op_add_hl_rr(gb_t *gb, uint8_t opcode)
{
    uint8_t idx = (opcode & 0x30) >> 4;
    uint16_t val = read_rr(gb, idx);
    uint16_t result = gb->cpu.hl + val;

    bool h = (gb->cpu.hl & 0x0FFF) + (val & 0x0FFF) > 0x0FFF;
    bool c = (uint32_t)gb->cpu.hl + (uint32_t)val > 0xFFFF;

    gb->cpu.hl = result;

    gb->cpu.f = (gb->cpu.f & FLAG_Z) | (h ? FLAG_H : 0) | (c ? FLAG_C : 0);

    return 8;
}

static uint8_t op_jp_hl(gb_t *gb)
{
    gb->cpu.pc = gb->cpu.hl;

    return 4;
}

static uint8_t op_jp_cc_nn(gb_t *gb, uint8_t opcode)
{
    uint8_t idx = (opcode & 0x18) >> 3;
    uint16_t nn = read_nn(gb);
    bool cc = check_cc(gb, idx);

    if (cc) gb->cpu.pc = nn;

    return cc ? 16 : 12;
}

static uint8_t op_ret_cc(gb_t *gb, uint8_t opcode)
{
    uint8_t idx = (opcode & 0x18) >> 3;
    bool cc = check_cc(gb, idx);

    if (cc) gb->cpu.pc = pop_u16(gb);

    return cc ? 20 : 8; 
}

static uint8_t op_jr_e(gb_t *gb)
{
    int8_t e = read_e(gb);
    gb->cpu.pc += e;

    return 12;
}

static uint8_t op_reti(gb_t *gb)
{
    gb->cpu.pc = pop_u16(gb);
    gb->cpu.ime = true;

    return 16;
}

static uint8_t op_add_n(gb_t *gb)
{
    uint8_t val = read_n(gb);
    uint8_t result = gb->cpu.a + val;

    bool z = (result == 0);
    bool h = (gb->cpu.a & 0x0F) + (val & 0x0F) > 0x0F;
    bool c = (uint16_t)gb->cpu.a + (uint16_t)val > 0xFF;

    gb->cpu.a = result;

    gb->cpu.f = (z ? FLAG_Z : 0) | (h ? FLAG_H : 0) | (c ? FLAG_C : 0);

    return 8;
}

static uint8_t op_or_n(gb_t *gb)
{
    gb->cpu.a |= read_n(gb);
    gb->cpu.f = (gb->cpu.a == 0 ? FLAG_Z : 0);

    return 8;
}

static uint8_t op_rlca(gb_t *gb)
{
    gb->cpu.a = (gb->cpu.a << 1) | (gb->cpu.a >> 7);
    gb->cpu.f = (gb->cpu.a & 0x1) ? FLAG_C : 0;

    return 4;
}

static uint8_t op_adc_r(gb_t *gb, uint8_t opcode)
{
    uint8_t idx = opcode & 0x07;
    uint8_t val = read_r(gb, idx);
    uint8_t c_bit = (gb->cpu.f & FLAG_C) >> 4;
    uint8_t result = gb->cpu.a + val + c_bit;

    bool z = (result == 0);
    bool h = (gb->cpu.a & 0x0F) + (val & 0x0F) + c_bit > 0x0F;
    bool c = (uint16_t)gb->cpu.a + (uint16_t)val + (uint16_t)c_bit > 0xFF;

    gb->cpu.a = result;

    gb->cpu.f = (z ? FLAG_Z : 0) | (h ? FLAG_H : 0) | (c ? FLAG_C : 0);

    return idx == 6 ? 8 : 4;
}

static uint8_t op_cp_r(gb_t *gb, uint8_t opcode)
{
    uint8_t idx = opcode & 0x07;
    uint8_t val = read_r(gb, idx);
    uint8_t result = gb->cpu.a - val;

    bool z = (result == 0);
    bool h = (gb->cpu.a & 0x0F) < (val & 0x0F);
    bool c = gb->cpu.a < val;

    gb->cpu.f = (z ? FLAG_Z : 0) | FLAG_N | (h ? FLAG_H : 0) | (c ? FLAG_C : 0);

    return idx == 6 ? 8 : 4;
}

static uint8_t op_sub_n(gb_t *gb)
{
    uint8_t val = read_n(gb);
    uint8_t result = gb->cpu.a - val;

    bool z = (result == 0);
    bool h = (gb->cpu.a & 0x0F) < (val & 0x0F);
    bool c = gb->cpu.a < val;

    gb->cpu.a = result;

    gb->cpu.f = (z ? FLAG_Z : 0) | FLAG_N | (h ? FLAG_H : 0) | (c ? FLAG_C : 0);

    return 8;
}

static uint8_t op_daa(gb_t *gb)
{
    uint8_t a = gb->cpu.a;

    uint8_t offset = 0;
    bool subtraction = gb->cpu.f & FLAG_N;

    bool c = false;

    if ((!subtraction && (a & 0x0F) > 0x09) || gb->cpu.f & FLAG_H) offset |= 0x06;
    if ((!subtraction && a > 0x99) || gb->cpu.f & FLAG_C) 
    {
        offset |= 0x60;
        c = true;
    }

    gb->cpu.a = subtraction ? a - offset : a + offset;

    bool z = gb->cpu.a == 0;

    gb->cpu.f = (z ? FLAG_Z : 0) | (gb->cpu.f & FLAG_N) | (c ? FLAG_C : 0);

    return 4;
}

static uint8_t op_xor_n(gb_t *gb)
{
    gb->cpu.a ^= read_n(gb);
    gb->cpu.f = (gb->cpu.a == 0) ? FLAG_Z : 0;

    return 8;
}

#pragma endregion


#pragma region CB Ops

static noreturn void op_unimplemented_cb(gb_t *gb, uint8_t opcode)
{
    fprintf(stderr, "Unimplemented CB opcode 0x%02X at 0x%04X\n", opcode, gb->cpu.pc - 2);
    exit(1);
}

static uint8_t op_sla_r(gb_t *gb, uint8_t opcode)
{
    uint8_t idx = opcode & 0x07;
    uint8_t r = read_r(gb, idx);
    uint8_t result = r << 1;
    
    bool z = (result == 0);
    bool c = r & 0x80;

    write_r(gb, idx, result);

    gb->cpu.f = (z ? FLAG_Z : 0) | (c ? FLAG_C : 0);
    
    return idx == 6 ? 16 : 8;
}

static uint8_t op_swap_r(gb_t *gb, uint8_t opcode)
{
    uint8_t idx = opcode & 0x07;
    uint8_t r = read_r(gb, idx);
    uint8_t result = (r >> 4) | ((r & 0x0F) << 4);
    
    write_r(gb, idx, result);

    gb->cpu.f = result == 0 ? FLAG_Z : 0;
    
    return idx == 6 ? 16 : 8;
}

static uint8_t op_srl_r(gb_t *gb, uint8_t opcode)
{
    uint8_t idx = opcode & 0x07;
    uint8_t r = read_r(gb, idx);
    uint8_t result = r >> 1;
    
    bool z = (result == 0);
    bool c = r & 0x01;

    write_r(gb, idx, result);

    gb->cpu.f = (z ? FLAG_Z : 0) | (c ? FLAG_C : 0);
    
    return idx == 6 ? 16 : 8;
}

static uint8_t op_res_b_r(gb_t *gb, uint8_t opcode)
{
    uint8_t idx_r = opcode & 0x07;
    uint8_t idx_b = (opcode & 0x38) >> 3;

    uint8_t r = read_r(gb, idx_r);
    
    write_r(gb, idx_r, r & ~(1 << idx_b));

    return idx_r == 6 ? 16 : 8;
}

static uint8_t op_set_b_r(gb_t *gb, uint8_t opcode)
{
    uint8_t idx_r = opcode & 0x07;
    uint8_t idx_b = (opcode & 0x38) >> 3;

    uint8_t r = read_r(gb, idx_r);
    
    write_r(gb, idx_r, r | (1 << idx_b));

    return idx_r == 6 ? 16 : 8;
}

static uint8_t op_bit_b_r(gb_t *gb, uint8_t opcode)
{
    uint8_t idx_r = opcode & 0x07;
    uint8_t idx_b = (opcode & 0x38) >> 3;

    uint8_t r = read_r(gb, idx_r);
    bool b = r & (1 << idx_b);

    gb->cpu.f = (b ? 0 : FLAG_Z) | FLAG_H | (gb->cpu.f & FLAG_C);

    return idx_r == 6 ? 12 : 8;
}

#pragma endregion