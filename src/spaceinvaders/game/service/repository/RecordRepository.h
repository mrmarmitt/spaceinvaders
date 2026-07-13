#pragma once

#include <vector>

#include "spaceinvaders/game/Record.h"

// Porta de persistencia dos recordes (mesmo desenho do 8puzzle): o servico
// fala com esta interface; o arquivo TSV e um detalhe da implementacao.
class RecordRepository
{
public:
    virtual ~RecordRepository() = default;

    virtual std::vector<Record> loadAll() = 0;
    virtual void                saveAll(const std::vector<Record>& records) = 0;
};
