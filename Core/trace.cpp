#ifdef ENABLE_BAP_FRAMES

#include <trace.container.hpp>

struct GB_trace_state_t
{
    SerializedTrace::TraceContainerWriter *writer;
    bool frame_started = false;
    uint16_t pc;
    uint8_t op[4];
    size_t op_size;
};

// If we would include gb.h, the GB_gameboy_t would have a different layout than in C for some reason.
// We work around it by only calling only functions.
typedef struct GB_gameboy_s GB_gameboy_t;
extern "C" GB_trace_state_t **GB_trace_member(GB_gameboy_t *gb);
extern "C" void GB_log(GB_gameboy_t *gb, const char *fmt, ...);
extern "C" void GB_get_rom_title(GB_gameboy_t *gb, char *title);

extern "C" void GB_trace_open(GB_gameboy_t *gb, const char *filename)
{
    GB_trace_state_t **tp = GB_trace_member(gb);
    if (*tp) {
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
        *tp = t;
    } catch (std::exception e) {
        GB_log(gb, "Error: Trace open failed: %s\n", e.what());
    }
}

extern "C" void GB_trace_close(GB_gameboy_t *gb)
{
    GB_trace_state_t **tp = GB_trace_member(gb);
    auto t = *tp;
    if (!t) {
        GB_log(gb, "Error: No trace was opened.\n");
        return;
    }
    if (t->frame_started) {
        GB_log(gb, "Warning: closing trace with unfinished frame.\n");
    }
    t->writer->finish();
    delete t->writer;
    delete t;
    *tp = NULL;
}

extern "C" void GB_trace_frame_begin(GB_gameboy_t *gb, uint16_t pc, uint8_t *op, size_t sz)
{
    auto t = *GB_trace_member(gb);
    if (t->frame_started) {
        GB_log(gb, "Frame already started!\n");
        return;
    }
    t->pc = pc;
    memcpy(t->op, op, sz);
    t->op_size = sz;
}

extern "C" void GB_trace_frame_end(GB_gameboy_t *gb)
{
    auto t = *GB_trace_member(gb);
    if (t->frame_started) {
        GB_log(gb, "No frame started!\n");
        return;
    }

    operand_value_list *pre = new operand_value_list();
    // push_regs(pre, &tf->pre.regs, true, false, nullptr);
    // push_mems(pre, tf->pre.mems, tf->pre.mems_count, true, false);

    operand_value_list *post = new operand_value_list();
    // push_regs(post, &tf->post.regs, false, true, &tf->pre.regs);
    // push_mems(post, tf->post.mems, tf->post.mems_count, false, true);

    std_frame *sf = new std_frame();
    sf->set_address(t->pc);
    sf->set_thread_id(0);
    sf->set_rawbytes(std::string((const char *)t->op, t->op_size));
    sf->set_allocated_operand_pre_list(pre);
    sf->set_allocated_operand_post_list(post);

    frame f;
    f.set_allocated_std_frame(sf);

    t->writer->add(f);
    t->frame_started = false;
}
#endif
