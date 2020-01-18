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
#ifndef MIDIFUZZ_H_INCLUDED
#define MIDIFUZZ_H_INCLUDED

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct midifuzz midifuzz_t;

enum {
    mfk_notes,
    mfk_controllers,
    mf_kind_count,
};

/**
   Create a new instance of fuzzer.
 */
midifuzz_t *midifuzz_new(double sample_rate);

/**
   Destroy the fuzzer.
 */
void midifuzz_free(midifuzz_t *mf);

/**
   Resume or pause the fuzzing operation.
 */
void midifuzz_start(midifuzz_t *mf, int resume);

/**
   Writer callback for MIDI messages.
   Must return 0 on success, -1 otherwise.
 */
typedef int (midifuzz_write_callback_t)(void *cbdata, int kind, const uint8_t *data, uint32_t size);

/**
   Set the callback which receives MIDI messages.
 */
void midifuzz_set_write_callback(midifuzz_t *mf, midifuzz_write_callback_t *cb, void *cbdata);

/**
   Enables or disables a particular kind of fuzzing.
 */
void midifuzz_enable_kind(midifuzz_t *mf, int kind, int enable);

/**
   Set the target MIDI channel.
   If channel is 16, all channels are targeted.
 */
void midifuzz_set_channel(midifuzz_t *mf, int channel);

/**
   Set the maximum rate of sent data, expressed in bauds.
 */
void midifuzz_set_rate_limit(midifuzz_t *mf, double limit);

/**
   Processes a cycle of the MIDI fuzzer, with given duration in frames.
 */
void midifuzz_process(midifuzz_t *mf, uint32_t nframes);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // MIDIFUZZ_H_INCLUDED
