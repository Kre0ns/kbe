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

static uint8_t op_nop(gb_t *gb, uint8_t opcode);
static uint8_t op_jp_nn(gb_t *gb, uint8_t opcode);
static uint8_t op_xor_r(gb_t *gb, uint8_t opcode);
static uint8_t op_ld_rr_nn(gb_t *gb, uint8_t opcode);
static uint8_t op_ld_r_n(gb_t *gb, uint8_t opcode);
static uint8_t op_ld_ind_a(gb_t *gb, uint8_t opcode);
static uint8_t op_ld_a_ind(gb_t *gb, uint8_t opcode);
static uint8_t op_dec_r(gb_t *gb, uint8_t opcode);
static uint8_t op_jr_cc_e(gb_t *gb, uint8_t opcode);
static uint8_t op_di(gb_t *gb, uint8_t opcode);
static uint8_t op_ldh_n_a(gb_t *gb, uint8_t opcode);
static uint8_t op_ldh_a_n(gb_t *gb, uint8_t opcode);
static uint8_t op_ldh_c_a(gb_t *gb, uint8_t opcode);
static uint8_t op_ldh_a_c(gb_t *gb, uint8_t opcode);
static uint8_t op_cp_n(gb_t *gb, uint8_t opcode);
static uint8_t op_ld_nn_a(gb_t *gb, uint8_t opcode);
static uint8_t op_ld_a_nn(gb_t *gb, uint8_t opcode);
static uint8_t op_inc_r(gb_t *gb, uint8_t opcode);
static uint8_t op_call_nn(gb_t *gb, uint8_t opcode);
static uint8_t op_ret(gb_t *gb, uint8_t opcode);
static uint8_t op_dec_rr(gb_t *gb, uint8_t opcode);
static uint8_t op_inc_rr(gb_t *gb, uint8_t opcode);
static uint8_t op_ld_r_r(gb_t *gb, uint8_t opcode);
static uint8_t op_or_r(gb_t *gb, uint8_t opcode);
static uint8_t op_and_r(gb_t *gb, uint8_t opcode);
static uint8_t op_sub_r(gb_t *gb, uint8_t opcode);
static uint8_t op_add_r(gb_t *gb, uint8_t opcode);
static uint8_t op_ei(gb_t *gb, uint8_t opcode);
static uint8_t op_cpl(gb_t *gb, uint8_t opcode);
static uint8_t op_and_n(gb_t *gb, uint8_t opcode);
static uint8_t op_rst_n(gb_t *gb, uint8_t opcode);
static uint8_t op_pop_rr(gb_t *gb, uint8_t opcode);
static uint8_t op_push_rr(gb_t *gb, uint8_t opcode);
static uint8_t op_add_hl_rr(gb_t *gb, uint8_t opcode);
static uint8_t op_jp_hl(gb_t *gb, uint8_t opcode);
static uint8_t op_jp_cc_nn(gb_t *gb, uint8_t opcode);
static uint8_t op_ret_cc(gb_t *gb, uint8_t opcode);
static uint8_t op_jr_e(gb_t *gb, uint8_t opcode);
static uint8_t op_reti(gb_t *gb, uint8_t opcode);
static uint8_t op_add_n(gb_t *gb, uint8_t opcode);
static uint8_t op_or_n(gb_t *gb, uint8_t opcode);
static uint8_t op_rlca(gb_t *gb, uint8_t opcode);
static uint8_t op_adc_r(gb_t *gb, uint8_t opcode);
static uint8_t op_cp_r(gb_t *gb, uint8_t opcode);
static uint8_t op_sub_n(gb_t *gb, uint8_t opcode);
static uint8_t op_daa(gb_t *gb, uint8_t opcode);
static uint8_t op_xor_n(gb_t *gb, uint8_t opcode);
static uint8_t op_ld_nn_sp(gb_t *gb, uint8_t opcode);
static uint8_t op_rrca(gb_t *gb, uint8_t opcode);
static uint8_t op_rla(gb_t *gb, uint8_t opcode);
static uint8_t op_rra(gb_t *gb, uint8_t opcode);
static uint8_t op_scf(gb_t *gb, uint8_t opcode);
static uint8_t op_ccf(gb_t *gb, uint8_t opcode);
static uint8_t op_stop(gb_t *gb, uint8_t opcode);
static uint8_t op_halt(gb_t *gb, uint8_t opcode);
static uint8_t op_sbc_r(gb_t *gb, uint8_t opcode);
static uint8_t op_call_cc_nn(gb_t *gb, uint8_t opcode);
static uint8_t op_adc_n(gb_t *gb, uint8_t opcode);
static uint8_t op_sbc_n(gb_t *gb, uint8_t opcode);
static uint8_t op_add_sp_e(gb_t *gb, uint8_t opcode);
static uint8_t op_ld_hl_spe(gb_t *gb, uint8_t opcode);
static uint8_t op_ld_sp_hl(gb_t *gb, uint8_t opcode);

static uint8_t cpu_execute_cb(gb_t *gb, uint8_t opcode);

static noreturn void op_unimplemented_cb(gb_t *gb, uint8_t opcode);

static uint8_t op_rlc_r(gb_t *gb, uint8_t opcode);
static uint8_t op_rrc_r(gb_t *gb, uint8_t opcode);
static uint8_t op_rl_r(gb_t *gb, uint8_t opcode);
static uint8_t op_rr_r(gb_t *gb, uint8_t opcode);
static uint8_t op_sla_r(gb_t *gb, uint8_t opcode);
static uint8_t op_sra_r(gb_t *gb, uint8_t opcode);
static uint8_t op_swap_r(gb_t *gb, uint8_t opcode);
static uint8_t op_srl_r(gb_t *gb, uint8_t opcode);
static uint8_t op_res_b_r(gb_t *gb, uint8_t opcode);
static uint8_t op_set_b_r(gb_t *gb, uint8_t opcode);
static uint8_t op_bit_b_r(gb_t *gb, uint8_t opcode);

static uint8_t (*const op_table[256])(gb_t*, uint8_t) = {
//       X0          X1           X2           X3         X4             X5          X6         X7         X8            X9            XA           XB              XC             XD          XE         XF
/* 0X */ op_nop,     op_ld_rr_nn, op_ld_ind_a, op_inc_rr, op_inc_r,      op_dec_r,   op_ld_r_n, op_rlca,   op_ld_nn_sp,  op_add_hl_rr, op_ld_a_ind, op_dec_rr,      op_inc_r,      op_dec_r,   op_ld_r_n, op_rrca,      
/* 1X */ op_stop,    op_ld_rr_nn, op_ld_ind_a, op_inc_rr, op_inc_r,      op_dec_r,   op_ld_r_n, op_rla,    op_jr_e,      op_add_hl_rr, op_ld_a_ind, op_dec_rr,      op_inc_r,      op_dec_r,   op_ld_r_n, op_rra,      
/* 2X */ op_jr_cc_e, op_ld_rr_nn, op_ld_ind_a, op_inc_rr, op_inc_r,      op_dec_r,   op_ld_r_n, op_daa,    op_jr_cc_e,   op_add_hl_rr, op_ld_a_ind, op_dec_rr,      op_inc_r,      op_dec_r,   op_ld_r_n, op_cpl,      
/* 3X */ op_jr_cc_e, op_ld_rr_nn, op_ld_ind_a, op_inc_rr, op_inc_r,      op_dec_r,   op_ld_r_n, op_scf,    op_jr_cc_e,   op_add_hl_rr, op_ld_a_ind, op_dec_rr,      op_inc_r,      op_dec_r,   op_ld_r_n, op_ccf,      
/* 4X */ op_ld_r_r,  op_ld_r_r,   op_ld_r_r,   op_ld_r_r, op_ld_r_r,     op_ld_r_r,  op_ld_r_r, op_ld_r_r, op_ld_r_r,    op_ld_r_r,    op_ld_r_r,   op_ld_r_r,      op_ld_r_r,     op_ld_r_r,  op_ld_r_r, op_ld_r_r, 
/* 5X */ op_ld_r_r,  op_ld_r_r,   op_ld_r_r,   op_ld_r_r, op_ld_r_r,     op_ld_r_r,  op_ld_r_r, op_ld_r_r, op_ld_r_r,    op_ld_r_r,    op_ld_r_r,   op_ld_r_r,      op_ld_r_r,     op_ld_r_r,  op_ld_r_r, op_ld_r_r, 
/* 6X */ op_ld_r_r,  op_ld_r_r,   op_ld_r_r,   op_ld_r_r, op_ld_r_r,     op_ld_r_r,  op_ld_r_r, op_ld_r_r, op_ld_r_r,    op_ld_r_r,    op_ld_r_r,   op_ld_r_r,      op_ld_r_r,     op_ld_r_r,  op_ld_r_r, op_ld_r_r,
/* 7X */ op_ld_r_r,  op_ld_r_r,   op_ld_r_r,   op_ld_r_r, op_ld_r_r,     op_ld_r_r,  op_halt,   op_ld_r_r, op_ld_r_r,    op_ld_r_r,    op_ld_r_r,   op_ld_r_r,      op_ld_r_r,     op_ld_r_r,  op_ld_r_r, op_ld_r_r,
/* 8X */ op_add_r,   op_add_r,    op_add_r,    op_add_r,  op_add_r,      op_add_r,   op_add_r,  op_add_r,  op_adc_r,     op_adc_r,     op_adc_r,    op_adc_r,       op_adc_r,      op_adc_r,   op_adc_r,  op_adc_r,
/* 9X */ op_sub_r,   op_sub_r,    op_sub_r,    op_sub_r,  op_sub_r,      op_sub_r,   op_sub_r,  op_sub_r,  op_sbc_r,     op_sbc_r,     op_sbc_r,    op_sbc_r,       op_sbc_r,      op_sbc_r,   op_sbc_r,  op_sbc_r,
/* AX */ op_and_r,   op_and_r,    op_and_r,    op_and_r,  op_and_r,      op_and_r,   op_and_r,  op_and_r,  op_xor_r,     op_xor_r,     op_xor_r,    op_xor_r,       op_xor_r,      op_xor_r,   op_xor_r,  op_xor_r,
/* BX */ op_or_r,    op_or_r,     op_or_r,     op_or_r,   op_or_r,       op_or_r,    op_or_r,   op_or_r,   op_cp_r,      op_cp_r,      op_cp_r,     op_cp_r,        op_cp_r,       op_cp_r,    op_cp_r,   op_cp_r,
/* CX */ op_ret_cc,  op_pop_rr,   op_jp_cc_nn, op_jp_nn,  op_call_cc_nn, op_push_rr, op_add_n,  op_rst_n,  op_ret_cc,    op_ret,       op_jp_cc_nn, cpu_execute_cb, op_call_cc_nn, op_call_nn, op_adc_n,  op_rst_n,
/* DX */ op_ret_cc,  op_pop_rr,   op_jp_cc_nn, NULL,      op_call_cc_nn, op_push_rr, op_sub_n,  op_rst_n,  op_ret_cc,    op_reti,      op_jp_cc_nn, NULL,           op_call_cc_nn, NULL,       op_sbc_n,  op_rst_n,
/* EX */ op_ldh_n_a, op_pop_rr,   op_ldh_c_a,  NULL,      NULL,          op_push_rr, op_and_n,  op_rst_n,  op_add_sp_e,  op_jp_hl,     op_ld_nn_a,  NULL,           NULL,          NULL,       op_xor_n,  op_rst_n,
/* FX */ op_ldh_a_n, op_pop_rr,   op_ldh_a_c,  op_di,     NULL,          op_push_rr, op_or_n,   op_rst_n,  op_ld_hl_spe, op_ld_sp_hl,  op_ld_a_nn,  op_ei,          NULL,          NULL,       op_cp_n,   op_rst_n,
};

static uint8_t (*const op_table_cb[256])(gb_t*, uint8_t) = {
//       X0          X1          X2          X3          X4          X5          X6          X7          X8          X9          XA          XB          XC          XD          XE          XF
/* 0X */ op_rlc_r,   op_rlc_r,   op_rlc_r,   op_rlc_r,   op_rlc_r,   op_rlc_r,   op_rlc_r,   op_rlc_r,   op_rrc_r,   op_rrc_r,   op_rrc_r,   op_rrc_r,   op_rrc_r,   op_rrc_r,   op_rrc_r,   op_rrc_r,   
/* 1X */ op_rl_r,    op_rl_r,    op_rl_r,    op_rl_r,    op_rl_r,    op_rl_r,    op_rl_r,    op_rl_r,    op_rr_r,    op_rr_r,    op_rr_r,    op_rr_r,    op_rr_r,    op_rr_r,    op_rr_r,    op_rr_r, 
/* 2X */ op_sla_r,   op_sla_r,   op_sla_r,   op_sla_r,   op_sla_r,   op_sla_r,   op_sla_r,   op_sla_r,   op_sra_r,   op_sra_r,   op_sra_r,   op_sra_r,   op_sra_r,   op_sra_r,   op_sra_r,   op_sra_r,
/* 3X */ op_swap_r,  op_swap_r,  op_swap_r,  op_swap_r,  op_swap_r,  op_swap_r,  op_swap_r,  op_swap_r,  op_srl_r,   op_srl_r,   op_srl_r,   op_srl_r,   op_srl_r,   op_srl_r,   op_srl_r,   op_srl_r,
/* 4X */ op_bit_b_r, op_bit_b_r, op_bit_b_r, op_bit_b_r, op_bit_b_r, op_bit_b_r, op_bit_b_r, op_bit_b_r, op_bit_b_r, op_bit_b_r, op_bit_b_r, op_bit_b_r, op_bit_b_r, op_bit_b_r, op_bit_b_r, op_bit_b_r,
/* 5X */ op_bit_b_r, op_bit_b_r, op_bit_b_r, op_bit_b_r, op_bit_b_r, op_bit_b_r, op_bit_b_r, op_bit_b_r, op_bit_b_r, op_bit_b_r, op_bit_b_r, op_bit_b_r, op_bit_b_r, op_bit_b_r, op_bit_b_r, op_bit_b_r,
/* 6X */ op_bit_b_r, op_bit_b_r, op_bit_b_r, op_bit_b_r, op_bit_b_r, op_bit_b_r, op_bit_b_r, op_bit_b_r, op_bit_b_r, op_bit_b_r, op_bit_b_r, op_bit_b_r, op_bit_b_r, op_bit_b_r, op_bit_b_r, op_bit_b_r,
/* 7X */ op_bit_b_r, op_bit_b_r, op_bit_b_r, op_bit_b_r, op_bit_b_r, op_bit_b_r, op_bit_b_r, op_bit_b_r, op_bit_b_r, op_bit_b_r, op_bit_b_r, op_bit_b_r, op_bit_b_r, op_bit_b_r, op_bit_b_r, op_bit_b_r,
/* 8X */ op_res_b_r, op_res_b_r, op_res_b_r, op_res_b_r, op_res_b_r, op_res_b_r, op_res_b_r, op_res_b_r, op_res_b_r, op_res_b_r, op_res_b_r, op_res_b_r, op_res_b_r, op_res_b_r, op_res_b_r, op_res_b_r,
/* 9X */ op_res_b_r, op_res_b_r, op_res_b_r, op_res_b_r, op_res_b_r, op_res_b_r, op_res_b_r, op_res_b_r, op_res_b_r, op_res_b_r, op_res_b_r, op_res_b_r, op_res_b_r, op_res_b_r, op_res_b_r, op_res_b_r,
/* AX */ op_res_b_r, op_res_b_r, op_res_b_r, op_res_b_r, op_res_b_r, op_res_b_r, op_res_b_r, op_res_b_r, op_res_b_r, op_res_b_r, op_res_b_r, op_res_b_r, op_res_b_r, op_res_b_r, op_res_b_r, op_res_b_r,
/* BX */ op_res_b_r, op_res_b_r, op_res_b_r, op_res_b_r, op_res_b_r, op_res_b_r, op_res_b_r, op_res_b_r, op_res_b_r, op_res_b_r, op_res_b_r, op_res_b_r, op_res_b_r, op_res_b_r, op_res_b_r, op_res_b_r,
/* CX */ op_set_b_r, op_set_b_r, op_set_b_r, op_set_b_r, op_set_b_r, op_set_b_r, op_set_b_r, op_set_b_r, op_set_b_r, op_set_b_r, op_set_b_r, op_set_b_r, op_set_b_r, op_set_b_r, op_set_b_r, op_set_b_r,
/* DX */ op_set_b_r, op_set_b_r, op_set_b_r, op_set_b_r, op_set_b_r, op_set_b_r, op_set_b_r, op_set_b_r, op_set_b_r, op_set_b_r, op_set_b_r, op_set_b_r, op_set_b_r, op_set_b_r, op_set_b_r, op_set_b_r,
/* EX */ op_set_b_r, op_set_b_r, op_set_b_r, op_set_b_r, op_set_b_r, op_set_b_r, op_set_b_r, op_set_b_r, op_set_b_r, op_set_b_r, op_set_b_r, op_set_b_r, op_set_b_r, op_set_b_r, op_set_b_r, op_set_b_r,
/* FX */ op_set_b_r, op_set_b_r, op_set_b_r, op_set_b_r, op_set_b_r, op_set_b_r, op_set_b_r, op_set_b_r, op_set_b_r, op_set_b_r, op_set_b_r, op_set_b_r, op_set_b_r, op_set_b_r, op_set_b_r, op_set_b_r,
};

uint8_t cpu_execute(gb_t *gb, uint8_t opcode)
{
    if (!op_table[opcode]) op_unimplemented(gb, opcode);
    return op_table[opcode](gb, opcode);
}

static uint8_t cpu_execute_cb(gb_t *gb, uint8_t opcode)
{
    (void) opcode;
    uint8_t cb = read_n(gb);
    if (!op_table_cb[cb]) op_unimplemented_cb(gb, cb);
    return op_table_cb[cb](gb, cb);
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

static uint8_t op_nop(gb_t *gb, uint8_t opcode)
{
    (void)gb; (void)opcode;
    return 4;
}

static uint8_t op_jp_nn(gb_t *gb, uint8_t opcode)
{
    (void)opcode;
    
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

static uint8_t op_di(gb_t *gb, uint8_t opcode)
{
    (void)opcode;

    gb->cpu.ime = false;
    gb->cpu.ime_pending = false;

    return 4;
}

static uint8_t op_ldh_n_a(gb_t *gb, uint8_t opcode)
{
    (void)opcode;

    uint16_t addr = 0xFF00 | read_n(gb);

    bus_write(gb, addr, gb->cpu.a);

    return 12;
}

static uint8_t op_ldh_a_n(gb_t *gb, uint8_t opcode)
{
    (void)opcode;

    uint16_t addr = 0xFF00 | read_n(gb);

    gb->cpu.a = bus_read(gb, addr);

    return 12;
}

static uint8_t op_ldh_c_a(gb_t *gb, uint8_t opcode)
{
    (void)opcode;

    uint16_t addr = 0xFF00 | gb->cpu.c;

    bus_write(gb, addr, gb->cpu.a);

    return 8;
}

static uint8_t op_ldh_a_c(gb_t *gb, uint8_t opcode)
{
    (void)opcode;

    uint16_t addr = 0xFF00 | gb->cpu.c;

    gb->cpu.a = bus_read(gb, addr);

    return 8;
}

static uint8_t op_cp_n(gb_t *gb, uint8_t opcode)
{
    (void)opcode;

    uint8_t n = read_n(gb);
    uint8_t a = gb->cpu.a;
    uint8_t result = a - n;

    bool z = (result == 0);
    bool h = (a & 0x0F) < (n & 0x0F);
    bool c = a < n;

    gb->cpu.f = (z ? FLAG_Z : 0) | FLAG_N | (h ? FLAG_H : 0) | (c ? FLAG_C : 0);

    return 8;
}

static uint8_t op_ld_nn_a(gb_t *gb, uint8_t opcode)
{
    (void)opcode;

    bus_write(gb, read_nn(gb), gb->cpu.a);

    return 16;
}

static uint8_t op_ld_a_nn(gb_t *gb, uint8_t opcode)
{
    (void)opcode;

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

static uint8_t op_call_nn(gb_t *gb, uint8_t opcode)
{
    (void)opcode;

    uint16_t nn = read_nn(gb);

    push_u16(gb, gb->cpu.pc);

    gb->cpu.pc = nn;

    return 24;
}

static uint8_t op_ret(gb_t *gb, uint8_t opcode)
{
    (void)opcode;

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

static uint8_t op_ei(gb_t *gb, uint8_t opcode)
{
    (void)opcode;

    gb->cpu.ime_pending = true;

    return 4;
}

static uint8_t op_cpl(gb_t *gb, uint8_t opcode)
{
    (void)opcode;

    gb->cpu.a = ~gb->cpu.a;

    gb->cpu.f = (gb->cpu.f & FLAG_Z) | FLAG_N | FLAG_H | (gb->cpu.f & FLAG_C);

    return 4;
}

static uint8_t op_and_n(gb_t *gb, uint8_t opcode)
{
    (void)opcode;

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

static uint8_t op_jp_hl(gb_t *gb, uint8_t opcode)
{
    (void)opcode;

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

static uint8_t op_jr_e(gb_t *gb, uint8_t opcode)
{
    (void)opcode;

    int8_t e = read_e(gb);
    gb->cpu.pc += e;

    return 12;
}

static uint8_t op_reti(gb_t *gb, uint8_t opcode)
{
    (void)opcode;

    gb->cpu.pc = pop_u16(gb);
    gb->cpu.ime = true;

    return 16;
}

static uint8_t op_add_n(gb_t *gb, uint8_t opcode)
{
    (void)opcode;

    uint8_t val = read_n(gb);
    uint8_t result = gb->cpu.a + val;

    bool z = (result == 0);
    bool h = (gb->cpu.a & 0x0F) + (val & 0x0F) > 0x0F;
    bool c = (uint16_t)gb->cpu.a + (uint16_t)val > 0xFF;

    gb->cpu.a = result;

    gb->cpu.f = (z ? FLAG_Z : 0) | (h ? FLAG_H : 0) | (c ? FLAG_C : 0);

    return 8;
}

static uint8_t op_or_n(gb_t *gb, uint8_t opcode)
{
    (void)opcode;

    gb->cpu.a |= read_n(gb);
    gb->cpu.f = (gb->cpu.a == 0 ? FLAG_Z : 0);

    return 8;
}

static uint8_t op_rlca(gb_t *gb, uint8_t opcode)
{
    (void)opcode;

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

static uint8_t op_sub_n(gb_t *gb, uint8_t opcode)
{
    (void)opcode;

    uint8_t val = read_n(gb);
    uint8_t result = gb->cpu.a - val;

    bool z = (result == 0);
    bool h = (gb->cpu.a & 0x0F) < (val & 0x0F);
    bool c = gb->cpu.a < val;

    gb->cpu.a = result;

    gb->cpu.f = (z ? FLAG_Z : 0) | FLAG_N | (h ? FLAG_H : 0) | (c ? FLAG_C : 0);

    return 8;
}

static uint8_t op_daa(gb_t *gb, uint8_t opcode)
{
    (void)opcode;

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

static uint8_t op_xor_n(gb_t *gb, uint8_t opcode)
{
    (void)opcode;

    gb->cpu.a ^= read_n(gb);
    gb->cpu.f = (gb->cpu.a == 0) ? FLAG_Z : 0;

    return 8;
}

static uint8_t op_ld_nn_sp(gb_t *gb, uint8_t opcode)
{
    (void)opcode;

    uint16_t nn = read_nn(gb);

    bus_write(gb, nn, gb->cpu.sp & 0xFF);
    bus_write(gb, nn + 1, gb->cpu.sp >> 8);

    return 20;
}

static uint8_t op_rrca(gb_t *gb, uint8_t opcode)
{
    (void)opcode;

    gb->cpu.a = (gb->cpu.a >> 1) | (gb->cpu.a << 7);
    gb->cpu.f = (gb->cpu.a & 0x80) ? FLAG_C : 0;

    return 4;
}

static uint8_t op_rla(gb_t *gb, uint8_t opcode)
{
    (void)opcode;

    bool c = gb->cpu.a & 0x80;
    
    gb->cpu.a = (gb->cpu.a << 1) | ((gb->cpu.f & FLAG_C) >> 4);
    gb->cpu.f = c ? FLAG_C : 0;

    return 4;
}

static uint8_t op_rra(gb_t *gb, uint8_t opcode)
{
    (void)opcode;

    bool c = gb->cpu.a & 0x01;

    gb->cpu.a = (gb->cpu.a >> 1) | ((gb->cpu.f & FLAG_C) << 3);
    gb->cpu.f = c ? FLAG_C : 0;

    return 4;
}

static uint8_t op_scf(gb_t *gb, uint8_t opcode)
{
    (void)opcode;

    gb->cpu.f = (gb->cpu.f & FLAG_Z) | FLAG_C;

    return 4;
}

static uint8_t op_ccf(gb_t *gb, uint8_t opcode)
{
    (void)opcode;

    gb->cpu.f = (gb->cpu.f & FLAG_Z) | ((gb->cpu.f & FLAG_C) ^ FLAG_C);

    return 4;
}

static uint8_t op_stop(gb_t *gb, uint8_t opcode)
{
    // Not a full stop implementation.

    (void)opcode;

    read_n(gb);

    return 4;
}

static uint8_t op_halt(gb_t *gb, uint8_t opcode)
{
    (void)opcode;

    gb->cpu.halted = true;

    return 4;
}

static uint8_t op_sbc_r(gb_t *gb, uint8_t opcode)
{
    uint8_t idx = opcode & 0x07;
    uint8_t val = read_r(gb, idx);
    uint8_t c_bit = (gb->cpu.f & FLAG_C) >> 4;
    uint8_t result = gb->cpu.a - val - c_bit;

    bool z = (result == 0);
    bool h = (gb->cpu.a & 0x0F) < (val & 0x0F) + c_bit;
    bool c = gb->cpu.a < val + c_bit;

    gb->cpu.a = result;

    gb->cpu.f = (z ? FLAG_Z : 0) | FLAG_N | (h ? FLAG_H : 0) | (c ? FLAG_C : 0);

    return idx == 6 ? 8 : 4;
}

static uint8_t op_call_cc_nn(gb_t *gb, uint8_t opcode)
{
    uint8_t idx = (opcode & 0x18) >> 3;
    uint16_t nn = read_nn(gb);
    bool cc = check_cc(gb, idx);

    if (cc) 
    {
        push_u16(gb, gb->cpu.pc);
        gb->cpu.pc = nn;
    }

    return cc ? 24 : 12;
}

static uint8_t op_adc_n(gb_t *gb, uint8_t opcode)
{
    (void)opcode;
    
    uint8_t val = read_n(gb);
    uint8_t c_bit = (gb->cpu.f & FLAG_C) >> 4;
    uint8_t result = gb->cpu.a + val + c_bit;

    bool z = (result == 0);
    bool h = (gb->cpu.a & 0x0F) + (val & 0x0F) + c_bit > 0x0F;
    bool c = (uint16_t)gb->cpu.a + (uint16_t)val + (uint16_t)c_bit > 0xFF;

    gb->cpu.a = result;

    gb->cpu.f = (z ? FLAG_Z : 0) | (h ? FLAG_H : 0) | (c ? FLAG_C : 0);

    return 8;
}

static uint8_t op_sbc_n(gb_t *gb, uint8_t opcode)
{
    (void)opcode;

    uint8_t val = read_n(gb);
    uint8_t c_bit = (gb->cpu.f & FLAG_C) >> 4;
    uint8_t result = gb->cpu.a - val - c_bit;

    bool z = (result == 0);
    bool h = (gb->cpu.a & 0x0F) < (val & 0x0F) + c_bit;
    bool c = gb->cpu.a < val + c_bit;

    gb->cpu.a = result;

    gb->cpu.f = (z ? FLAG_Z : 0) | FLAG_N | (h ? FLAG_H : 0) | (c ? FLAG_C : 0);

    return 8;
}

static uint8_t op_add_sp_e(gb_t *gb, uint8_t opcode)
{
    (void)opcode;

    int8_t val = read_e(gb);

    bool h = (gb->cpu.sp & 0x0F) + ((uint8_t)val & 0x0F) > 0x0F;
    bool c = (gb->cpu.sp & 0xFF) + (uint8_t)val > 0xFF;

    gb->cpu.sp += val;

    gb->cpu.f = (h ? FLAG_H : 0) | (c ? FLAG_C : 0);

    return 16;   
}

static uint8_t op_ld_hl_spe(gb_t *gb, uint8_t opcode)
{
    (void)opcode;

    int8_t val = read_e(gb);

    bool h = (gb->cpu.sp & 0x0F) + ((uint8_t)val & 0x0F) > 0x0F;
    bool c = (gb->cpu.sp & 0xFF) + (uint8_t)val > 0xFF;

    gb->cpu.hl = gb->cpu.sp + val;

    gb->cpu.f = (h ? FLAG_H : 0) | (c ? FLAG_C : 0);

    return 12;
}

static uint8_t op_ld_sp_hl(gb_t *gb, uint8_t opcode)
{
    (void)opcode;

    gb->cpu.sp = gb->cpu.hl;
    
    return 8;
}

#pragma endregion


#pragma region CB Ops

static noreturn void op_unimplemented_cb(gb_t *gb, uint8_t opcode)
{
    fprintf(stderr, "Unimplemented CB opcode 0x%02X at 0x%04X\n", opcode, gb->cpu.pc - 2);
    exit(1);
}

static uint8_t op_rlc_r(gb_t *gb, uint8_t opcode)
{
    uint8_t idx = opcode & 0x07;
    uint8_t r = read_r(gb, idx);
    uint8_t result = (r << 1) | (r >> 7);

    bool z = result == 0;
    bool c = r & 0x80;
    
    write_r(gb, idx, result);
    gb->cpu.f = (z ? FLAG_Z : 0)| (c ? FLAG_C : 0);

    return idx == 6 ? 16 : 8;
}

static uint8_t op_rrc_r(gb_t *gb, uint8_t opcode)
{
    uint8_t idx = opcode & 0x07;
    uint8_t r = read_r(gb, idx);
    uint8_t result = (r >> 1) | (r << 7);

    bool z = result == 0;
    bool c = r & 0x01;
    
    write_r(gb, idx, result);
    gb->cpu.f = (z ? FLAG_Z : 0)| (c ? FLAG_C : 0);

    return idx == 6 ? 16 : 8;
}

static uint8_t op_rl_r(gb_t *gb, uint8_t opcode)
{
    uint8_t idx = opcode & 0x07;
    uint8_t r = read_r(gb, idx);
    uint8_t result = (r << 1) | ((gb->cpu.f & FLAG_C) >> 4);

    bool z = result == 0;
    bool c = r & 0x80;
    
    write_r(gb, idx, result);
    gb->cpu.f = (z ? FLAG_Z : 0)| (c ? FLAG_C : 0);

    return idx == 6 ? 16 : 8;
}

static uint8_t op_rr_r(gb_t *gb, uint8_t opcode)
{
    uint8_t idx = opcode & 0x07;
    uint8_t r = read_r(gb, idx);
    uint8_t result = (r >> 1) | ((gb->cpu.f & FLAG_C) << 3);

    bool z = result == 0;
    bool c = r & 0x01;
    
    write_r(gb, idx, result);
    gb->cpu.f = (z ? FLAG_Z : 0)| (c ? FLAG_C : 0);

    return idx == 6 ? 16 : 8;
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

static uint8_t op_sra_r(gb_t *gb, uint8_t opcode)
{
    uint8_t idx = opcode & 0x07;
    uint8_t r = read_r(gb, idx);
    uint8_t result = (r >> 1) | (r & 0x80);
    
    bool z = (result == 0);
    bool c = r & 0x01;

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