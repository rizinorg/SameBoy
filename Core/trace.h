#pragma once
#ifdef ENABLE_BAP_FRAMES

typedef struct GB_trace_state_s GB_trace_state_t;

void GB_trace_open(GB_gameboy_t *gb, const char *filename);
void GB_trace_close(GB_gameboy_t *gb);

#endif
