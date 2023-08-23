#pragma once
#ifdef ENABLE_BAP_FRAMES

typedef struct GB_trace_state_s GB_trace_state_t;

void GB_trace_open(GB_gameboy_t *gb, const char *filename);
void GB_trace_close(GB_gameboy_t *gb);
void GB_trace_frame_begin(GB_gameboy_t *gb, uint16_t pc, uint8_t *op, size_t sz);
void GB_trace_frame_end(GB_gameboy_t *gb);
void GB_trace_push_reg(GB_gameboy_t *gb, bool post, const char *name, uint16_t v, size_t bits);
void GB_trace_push_mem(GB_gameboy_t *gb, bool post, uint16_t addr, uint8_t val);

#endif
