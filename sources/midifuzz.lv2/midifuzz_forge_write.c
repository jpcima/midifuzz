/*
 * MIDI Fuzz
 * Copyright (C) 2020 Jean Pierre Cimalando <jp-dev@inbox.ru>
 *
 * Permission to use, copy, modify, and/or distribute this software for any purpose with
 * or without fee is hereby granted, provided that the above copyright notice and this
 * permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD
 * TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN
 * NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
 * IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

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
