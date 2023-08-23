#ifdef ENABLE_BAP_FRAMES

#include <trace.container.hpp>

struct GB_trace_state_t
{
    SerializedTrace::TraceContainerWriter *writer;
    bool frame_started = false;
    uint16_t pc;
    uint8_t op[4];
    size_t op_size;
    operand_value_list *pre;
    operand_value_list *post;
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
    t->pre = new operand_value_list();
    t->post = new operand_value_list();
    t->frame_started = true;
}

extern "C" void GB_trace_frame_end(GB_gameboy_t *gb)
{
    auto t = *GB_trace_member(gb);
    if (!t->frame_started) {
        GB_log(gb, "No frame started!\n");
        return;
    }

    std_frame *sf = new std_frame();
    sf->set_address(t->pc);
    sf->set_thread_id(0);
    sf->set_rawbytes(std::string((const char *)t->op, t->op_size));
    sf->set_allocated_operand_pre_list(t->pre);
    sf->set_allocated_operand_post_list(t->post);

    frame f;
    f.set_allocated_std_frame(sf);

    t->writer->add(f);
    t->frame_started = false;
}

extern "C" void GB_trace_push_reg(GB_gameboy_t *gb, bool post, const char *name, uint16_t v, size_t bits)
{
    auto t = *GB_trace_member(gb);
    if (!t->frame_started) {
        GB_log(gb, "No frame started!\n");
        return;
    }
    auto l = post ? t->post : t->pre;

    reg_operand *ro = new reg_operand();
    ro->set_name(name);
    operand_info_specific *s = new operand_info_specific();
    s->set_allocated_reg_operand(ro);

    operand_usage *u = new operand_usage();
    u->set_read(!post);
    u->set_written(post);
    u->set_index(false);
    u->set_base(false);

    taint_info *ti = new taint_info();

    operand_info *i = l->add_elem();
    i->set_allocated_operand_info_specific(s);
    i->set_bit_length((int32_t)bits);
    i->set_allocated_operand_usage(u);
    i->set_allocated_taint_info(ti);
    uint8_t va[2] = { (uint8_t)v, (uint8_t)(v >> 8) };
    i->set_value(std::string((const char *)va, bits / 8));
}

extern "C" void GB_trace_push_mem(GB_gameboy_t *gb, bool post, uint16_t addr, uint8_t val)
{
    auto t = *GB_trace_member(gb);
    if (!t->frame_started) {
        GB_log(gb, "No frame started!\n");
        return;
    }
    auto l = post ? t->post : t->pre;

    mem_operand *mo = new mem_operand();
    mo->set_address(addr);
    operand_info_specific *s = new operand_info_specific();
    s->set_allocated_mem_operand(mo);

    operand_usage *u = new operand_usage();
    u->set_read(!post);
    u->set_written(post);
    u->set_index(false);
    u->set_base(false);

    taint_info *ti = new taint_info();

    operand_info *i = l->add_elem();
    i->set_allocated_operand_info_specific(s);
    i->set_bit_length(8);
    i->set_allocated_operand_usage(u);
    i->set_allocated_taint_info(ti);
    i->set_value(std::string((const char *)&val, 1));
}

#endif
