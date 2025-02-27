#pragma once
#include "defs.h"
#include <stdint.h>

#ifndef GB_DISABLE_DEBUGGER
void GB_cpu_disassemble(GB_gameboy_t *gb, uint16_t pc, uint16_t count);
uint16_t GB_cpu_opsize(GB_gameboy_t *gb, uint16_t pc);
#endif
#ifdef GB_INTERNAL
internal void GB_cpu_run(GB_gameboy_t *gb);
#endif
