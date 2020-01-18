#include "midifuzz_forge_write.h"
#include <string.h>

int midifuzz_forge_write(void *cbdata, int kind, const uint8_t *data, uint32_t size)
{
    mf_forge_t *self = (mf_forge_t *)cbdata;
    LV2_Atom_Forge *forge = self->forge;

    if (!lv2_atom_forge_frame_time(forge, self->frame_time) ||
        !lv2_atom_forge_atom(forge, size, self->urid.midi_event) ||
        !lv2_atom_forge_raw(forge, data, size))
    {
        return -1;
    }

    lv2_atom_forge_pad(forge, size);

    return 0;
}
