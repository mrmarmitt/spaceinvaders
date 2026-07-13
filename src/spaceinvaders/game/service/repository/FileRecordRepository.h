#pragma once

#include <string>

#include "RecordRepository.h"

// Persistencia em TSV (uma linha por recorde: nome, pontos, onda, data),
// relativo ao diretorio de trabalho — o diretorio do exe, como no 8puzzle.
class FileRecordRepository final: public RecordRepository
{
    std::string m_filePath;

public:
    explicit FileRecordRepository(std::string filePath);

    std::vector<Record> loadAll() override;
    void                saveAll(const std::vector<Record>& records) override;
};
