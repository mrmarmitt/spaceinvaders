#include "RecordService.h"

#include <algorithm>
#include <utility>

#include "repository/RecordRepository.h"

RecordService::RecordService(std::shared_ptr<RecordRepository> recordRepository):
    m_recordRepository(std::move(recordRepository)), m_records(m_recordRepository->loadAll())
{
}

void RecordService::addRecord(const Record& record)
{
    m_records.push_back(record);
    m_recordRepository->saveAll(m_records);
}

std::vector<Record> RecordService::listByScore() const
{
    std::vector<Record> sorted = m_records;
    std::sort(sorted.begin(), sorted.end(), [](const Record& a, const Record& b) { return a.scoresHigherThan(b); });
    return sorted;
}

const std::vector<Record>& RecordService::getAll() const { return m_records; }

bool RecordService::isNewRecord(const Record& newRecord, const size_t minPosition) const
{
    if (m_records.size() < minPosition)
    {
        return true;
    }
    const auto sorted = listByScore();
    return newRecord.scoresHigherThan(sorted[minPosition - 1]);
}
