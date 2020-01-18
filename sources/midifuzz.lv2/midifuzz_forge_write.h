#pragma once
#include <lv2/atom/atom.h>
#include <lv2/atom/forge.h>

typedef struct mf_forge {
    LV2_Atom_Forge *forge;
    uint32_t frame_time;
    struct {
        LV2_URID midi_event;
    } urid;
} mf_forge_t;

int midifuzz_forge_write(void *cbdata, int kind, const uint8_t *data, uint32_t size);
