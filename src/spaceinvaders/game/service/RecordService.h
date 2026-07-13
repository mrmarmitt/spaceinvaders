#pragma once

#include <memory>
#include <vector>

#include "spaceinvaders/game/Record.h"

class RecordRepository;

// Regras de recordes: ranking por pontuacao e corte do top-N (mesmo desenho
// do RecordService do 8puzzle, com uma metrica so — pontos).
class RecordService
{
    std::shared_ptr<RecordRepository> m_recordRepository;
    std::vector<Record>               m_records;

public:
    explicit RecordService(std::shared_ptr<RecordRepository> recordRepository);

    void addRecord(const Record& record);

    [[nodiscard]] std::vector<Record>        listByScore() const;
    [[nodiscard]] const std::vector<Record>& getAll() const;

    /// O recorde entra no top-minPosition?
    [[nodiscard]] bool isNewRecord(const Record& newRecord, size_t minPosition) const;
};
