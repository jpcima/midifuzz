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

#pragma once
#include "midifuzz.h"
#include "rate_follower.h"
#include "utility/pl_list/pl_list.hpp"
#include <bitset>
#include <random>

struct midifuzz {
    double sampleRate = 0;
    midifuzz_write_callback_t *cb = nullptr;
    void *cbdata = nullptr;
    bool fuzzing = false;
    unsigned channel = 0;
    double ratelimit = 1;
    uint32_t numFramesThisCycle = 0;
    uint32_t bytesWrittenThisCycle = 0;
    RateFollower rateFollower;
    std::bitset<mf_kind_count> kindmask;
    std::minstd_rand prng;
    unsigned countNoteOn = 0; // total note-on count over all channels
    uint8_t notesOn[16 * 128] = {}; // number of note-on sent per note number
};

static int midifuzz_dummy_write(void *, int, const uint8_t *, uint32_t)
{
    return 0;
}

static uint32_t midifuzz_roll_dice(midifuzz_t *mf, uint32_t num);
static uint32_t midifuzz_pick_random_channel(midifuzz_t *mf);
static uint32_t midifuzz_pick_random_note_on(midifuzz_t *mf);
static void midifuzz_cleanup_notes(midifuzz_t *mf);
static bool midifuzz_send_message(midifuzz_t *mf, int kind, const uint8_t *data, uint32_t size);
static bool midifuzz_have_space_for_message(midifuzz_t *mf, uint32_t size);
