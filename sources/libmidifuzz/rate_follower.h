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
