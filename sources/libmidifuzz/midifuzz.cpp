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

#include "midifuzz_private.h"
#include <cassert>

midifuzz_t *midifuzz_new(double sample_rate)
{
    midifuzz_t *mf = new midifuzz;
    mf->sampleRate = sample_rate;
    mf->cb = &midifuzz_dummy_write;
    return mf;
}

void midifuzz_free(midifuzz_t *mf)
{
    delete mf;
}

void midifuzz_start(midifuzz_t *mf, int start)
{
    mf->fuzzing = start;
}

void midifuzz_set_write_callback(midifuzz_t *mf, midifuzz_write_callback_t *cb, void *cbdata)
{
    mf->cb = cb ? cb : &midifuzz_dummy_write;
    mf->cbdata = cbdata;
}

void midifuzz_enable_kind(midifuzz_t *mf, int kind, int enable)
{
    mf->kindmask.set(kind, enable);
}

void midifuzz_set_channel(midifuzz_t *mf, int channel)
{
    mf->channel = channel;
}

void midifuzz_set_rate_limit(midifuzz_t *mf, double limit)
{
    const double min_limit = 64;
    mf->ratelimit = (limit > min_limit) ? limit : min_limit;
}

void midifuzz_process(midifuzz_t *mf, uint32_t nframes)
{
    mf->numFramesThisCycle = nframes;
    mf->bytesWrittenThisCycle = 0;

    if (!mf->fuzzing) {
        midifuzz_cleanup_notes(mf);
    }
    else {
        bool processMore = true;

        while (processMore) {
            int kindsAvailable[mf_kind_count];
            unsigned numKindsAvailable = 0;

            for (int k = 0; k < mf_kind_count; ++k) {
                if (mf->kindmask.test(k) ||
                    (k == mfk_notes && mf->countNoteOn > 0))
                    // permit note cleanup even if notes disabled
                {
                    kindsAvailable[numKindsAvailable++] = k;
                }
            }

            int kind = -1;
            if (numKindsAvailable > 0)
                kind = kindsAvailable[midifuzz_roll_dice(mf, numKindsAvailable)];

            switch (kind) {
            case mfk_notes: {
                uint8_t message[3];
                bool isNoteOn;

                if (mf->kindmask.test(mfk_notes))
                    isNoteOn = midifuzz_roll_dice(mf, 2);
                else
                    isNoteOn = false;

                if (isNoteOn) {
                    uint32_t ch = midifuzz_pick_random_channel(mf);
                    message[0] = 0x90|ch;
                    message[1] = midifuzz_roll_dice(mf, 128);
                    message[2] = 1 + midifuzz_roll_dice(mf, 127);
                }
                else {
                    uint32_t note = midifuzz_pick_random_note_on(mf);
                    if (note != ~0u) {
                        uint32_t ch = note / 128;
                        note %= 128;
                        message[0] = 0x90|ch;
                        message[1] = note;
                        message[2] = 0;
                    }
                }
                processMore = midifuzz_send_message(mf, kind, message, 3);
                break;
            }

            case mfk_controllers: {
                uint8_t message[3];
                unsigned ch = midifuzz_pick_random_channel(mf);
                message[0] = 0xb0|ch;
                message[1] = midifuzz_roll_dice(mf, 120);
                message[2] = midifuzz_roll_dice(mf, 128);
                processMore = midifuzz_send_message(mf, kind, message, 3);
                break;
            }

            default:
                processMore = false;
                break;
            }
        }
    }

    mf->rateFollower.addRecord(nframes, mf->bytesWrittenThisCycle);
}

static void midifuzz_cleanup_notes(midifuzz_t *mf)
{
    bool processMore = true;

    while (processMore) {
        uint32_t note = midifuzz_pick_random_note_on(mf);
        if (note == ~0u)
            processMore = false;
        else {
            uint32_t channel = note / 128;
            note %= 128;
            uint8_t message[3];
            message[0] = 0x80|channel;
            message[1] = note;
            message[2] = 0;
            processMore = midifuzz_send_message(mf, mfk_notes, message, 3);
        }
    }
}

static uint32_t midifuzz_roll_dice(midifuzz_t *mf, uint32_t num)
{
    std::uniform_int_distribution<uint32_t> dist{0, num - 1};
    return dist(mf->prng);
}

static uint32_t midifuzz_pick_random_channel(midifuzz_t *mf)
{
    if (mf->channel < 16)
        return mf->channel;
    else
        return midifuzz_roll_dice(mf, 16);
}

static uint32_t midifuzz_pick_random_note_on(midifuzz_t *mf)
{
    uint32_t total = mf->countNoteOn;
    if (total == 0)
        return ~0u;

    uint32_t ret = ~0u;
    uint32_t nthCurrent = 0;
    uint32_t nthRandom = midifuzz_roll_dice(mf, total);

    for (unsigned i = 0; ret == ~0u; ++i) {
        assert(i < 16 * 128);
        nthCurrent += mf->notesOn[i];
        if (nthRandom < nthCurrent)
            ret = i;
    }

    return ret;
}

static bool midifuzz_send_message(midifuzz_t *mf, int kind, const uint8_t *data, uint32_t size)
{
    if (!midifuzz_have_space_for_message(mf, size))
        return false;

    ///
    bool isNoteOff = false;
    bool isNoteOn = false;
    uint8_t *noteSlot = nullptr;

    {
        uint8_t status = (size > 0) ? data[0] : 0;
        uint8_t channel = status & 15;
        uint8_t data1 = (size > 1) ? (data[1] & 0x7f) : 0;
        uint8_t data2 = (size > 2) ? (data[2] & 0x7f) : 0;
        switch (status & 0xf0) {
        case 0x90:
            if (data2 == 0)
                goto L_NoteOff;
            isNoteOn = true;
            noteSlot = &mf->notesOn[channel * 128 + data1];
            break;
        L_NoteOff:
        case 0x80:
            isNoteOff = true;
            noteSlot = &mf->notesOn[channel * 128 + data1];
            break;
        }
    }

    ///
    if (isNoteOn && *noteSlot == 255)
        return false; // do not permit more note-on than max count

    //
    bool success = mf->cb(mf->cbdata, kind, data, size) == 0;

    //
    if (success) {
        if (isNoteOn) {
            ++(*noteSlot);
            ++(mf->countNoteOn);
        }
        else if (isNoteOff && *noteSlot > 0) {
            --(*noteSlot);
            --(mf->countNoteOn);
        }
        mf->bytesWrittenThisCycle += size;
    }

    return success;
}

static bool midifuzz_have_space_for_message(midifuzz_t *mf, uint32_t size)
{
    uint32_t count = mf->rateFollower.getCountTotal() + mf->bytesWrittenThisCycle + size;
    uint32_t time = mf->rateFollower.getTimeTotal() + mf->numFramesThisCycle;
    double rate = count * mf->sampleRate / time;
    return rate < mf->ratelimit;
}
