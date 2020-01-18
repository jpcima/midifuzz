#include "midifuzz.h"
#include "midifuzz_forge_write.h"
#include <lv2/lv2plug.in/ns/lv2core/lv2.h>
#include <lv2/lv2plug.in/ns/ext/atom/atom.h>
#include <lv2/lv2plug.in/ns/ext/atom/forge.h>
#include <lv2/lv2plug.in/ns/ext/urid/urid.h>
#include <lv2/lv2plug.in/ns/ext/midi/midi.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

typedef struct LV2_midifuzz {
    midifuzz_t *mf;
    double sample_rate;
    struct {
        float *active;
        float *rate_limit;
        float *enable_notes;
        float *enable_controller_changes;
        float *channel;
    } controls;
    LV2_Atom_Sequence *events;
    LV2_URID_Map *map;
    LV2_Atom_Forge forge;
    struct {
        LV2_URID midi_event;
    } urid;
} LV2_midifuzz_t;

enum {
    midifuzz_port_events,
    midifuzz_port_active,
    midifuzz_port_channel,
    midifuzz_port_rate_limit,
    midifuzz_port_enable_notes,
    midifuzz_port_enable_controller_changes,
};

static LV2_Handle
instantiate(const LV2_Descriptor *descriptor,
            double sample_rate,
            const char * bundle_path,
            const LV2_Feature *const *features)
{
    LV2_midifuzz_t *effect = malloc(sizeof(LV2_midifuzz_t));
    effect->mf = NULL;
    effect->sample_rate = sample_rate;
    effect->events = NULL;
    effect->map = NULL;

    for (const LV2_Feature *const *featp = features; *featp; ++featp) {
        if (!strcmp((*featp)->URI, LV2_URID__map))
            effect->map = (LV2_URID_Map *)(*featp)->data;
    }

    if (!effect->map) {
        free(effect);
        return NULL;
    }

    lv2_atom_forge_init(&effect->forge, effect->map);

    effect->urid.midi_event = effect->map->map(
        effect->map->handle, LV2_MIDI__MidiEvent);

    return (LV2_Handle)effect;
}

static void
connect_port(LV2_Handle instance,
             uint32_t port,
             void *data_location)
{
    LV2_midifuzz_t *effect = (LV2_midifuzz_t *)instance;
    switch (port) {
    case midifuzz_port_events:
        effect->events = (LV2_Atom_Sequence *)data_location;
        break;
    case midifuzz_port_active:
        effect->controls.active = (float *)data_location;
        break;
    case midifuzz_port_channel:
        effect->controls.channel = (float *)data_location;
        break;
    case midifuzz_port_rate_limit:
        effect->controls.rate_limit = (float *)data_location;
        break;
    case midifuzz_port_enable_notes:
        effect->controls.enable_notes = (float *)data_location;
        break;
    case midifuzz_port_enable_controller_changes:
        effect->controls.enable_controller_changes = (float *)data_location;
        break;
    default:
        assert(false);
    }
}

static void
activate(LV2_Handle instance)
{
    LV2_midifuzz_t *effect = (LV2_midifuzz_t *)instance;
    effect->mf = midifuzz_new(effect->sample_rate);
}

static void
run(LV2_Handle instance,
    uint32_t sample_count)
{
    LV2_midifuzz_t *effect = (LV2_midifuzz_t *)instance;
    midifuzz_t *mf = effect->mf;

    midifuzz_start(mf, *effect->controls.active > 0);

    midifuzz_enable_kind(mf, mfk_notes, *effect->controls.enable_notes > 0);
    midifuzz_enable_kind(mf, mfk_controllers, *effect->controls.enable_controller_changes > 0);

    midifuzz_set_channel(mf, (int)*effect->controls.channel);
    midifuzz_set_rate_limit(mf, (int)*effect->controls.rate_limit);

    mf_forge_t forge;
    forge.forge = &effect->forge;
    forge.urid.midi_event = effect->urid.midi_event;
    lv2_atom_forge_set_buffer(
        forge.forge, (uint8_t *)effect->events, effect->events->atom.size);

    LV2_Atom_Forge_Frame seq;
    if (lv2_atom_forge_sequence_head(forge.forge, &seq, 0) != 0) {
        midifuzz_set_write_callback(mf, &midifuzz_forge_write, &forge);

        uint32_t sample_index = 0;
        const uint32_t granularity = 32;

        while (sample_index < sample_count) {
            uint32_t samples_current = sample_count - sample_index;
            if (samples_current > granularity)
                samples_current = granularity;
            forge.frame_time = sample_index;
            midifuzz_process(mf, samples_current);
            sample_index += samples_current;
        }

        lv2_atom_forge_pop(forge.forge, &seq);
    }
}

static void
deactivate(LV2_Handle instance)
{
    LV2_midifuzz_t *effect = (LV2_midifuzz_t *)instance;
    midifuzz_free(effect->mf);
    effect->mf = NULL;
}

static void
cleanup(LV2_Handle instance)
{
    LV2_midifuzz_t *effect = (LV2_midifuzz_t *)instance;
    midifuzz_free(effect->mf);
    free(effect);
}

static const void *
extension_data(const char *uri)
{
    return NULL;
}

static const LV2_Descriptor gDescriptor =
{
    "http://jpcima.sdf1.org/lv2/midi-fuzz",
    &instantiate,
    &connect_port,
    &activate,
    &run,
    &deactivate,
    &cleanup,
    &extension_data,
};

LV2_SYMBOL_EXPORT const LV2_Descriptor *
lv2_descriptor(uint32_t index)
{
    if (index > 0)
        return NULL;
    return &gDescriptor;
}
