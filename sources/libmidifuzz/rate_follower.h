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
#include <memory>
#include <cstdint>

/**
   Estimates rate of MIDI bytes written.
 */
class RateFollower {
public:
    explicit RateFollower(uint32_t historySize = 1024)
        : fHistory(new HistoryItem[historySize]), fHistorySize(historySize)
    {
    }

    void addRecord(uint32_t time, uint32_t count)
    {
        uint32_t index = fHistoryIndex;
        HistoryItem &item = fHistory[index];
        fSum.time = fSum.time - item.time + time;
        fSum.count = fSum.count - item.count + count;
        item.time = time;
        item.count = count;
        fHistoryIndex = (index + 1 < fHistorySize) ? (index + 1) : 0;
    }

    double getCurrentRate()
    {
        return (double)fSum.count / (double)fSum.time;
    }

    uint32_t getTimeTotal() const
    {
        return fSum.time;
    }

    uint32_t getCountTotal() const
    {
        return fSum.count;
    }

private:
    struct HistoryItem {
        uint32_t time = 0;
        uint32_t count = 0;
    };

    std::unique_ptr<HistoryItem[]> fHistory;
    uint32_t fHistorySize = 0;
    uint32_t fHistoryIndex = 0;
    HistoryItem fSum;
};
