#ifdef ENABLE_BAP_FRAMES

// Include order is fragile! Not including stdint.h here makes some members of GB_gameboy_t align differently.
#include <stdint.h>
#include <trace.container.hpp>
extern "C" {
#include "gb.h"
};

struct GB_trace_state_s
{
    SerializedTrace::TraceContainerWriter *writer;
};

extern "C" void GB_trace_open(GB_gameboy_t *gb, const char *filename)
{
    if (gb->trace) {
       GB_log(gb, "Error: Trace was already opened.\n");
       return;
    }

    tracer *trac = new tracer();
    trac->set_name("sameboy");
    trac->set_version("");

    target *tgt = new target();
    char title[17];
    GB_get_rom_title(gb, title);
    tgt->set_path(title);
    tgt->set_md5sum("");

    fstats *fst = new fstats();
    fst->set_size(0);
    fst->set_atime(0.0);
    fst->set_mtime(0.0);
    fst->set_ctime(0.0);

    meta_frame meta;
    meta.set_allocated_tracer(trac);
    meta.set_allocated_target(tgt);
    meta.set_allocated_fstats(fst);
    meta.set_user("");
    meta.set_host("");
    meta.set_time(0.0);

    try {
        auto writer = new SerializedTrace::TraceContainerWriter(filename, meta, frame_arch_sm83, 0);
        GB_trace_state_t *t = new GB_trace_state_t;
        t->writer = writer;
        gb->trace = t;
    } catch (std::exception e) {
        GB_log(gb, "Error: Trace open failed: %s\n", e.what());
    }
}

extern "C" void GB_trace_close(GB_gameboy_t *gb)
{
    if (!gb->trace) {
        GB_log(gb, "Error: No trace was opened.\n");
        return;
    }
    gb->trace->writer->finish();
    delete gb->trace->writer;
    delete gb->trace;
    gb->trace = NULL;
}
#endif
