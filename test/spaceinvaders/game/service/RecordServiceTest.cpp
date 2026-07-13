#include <gtest/gtest.h>

#include <cstdio>
#include <memory>

#include "spaceinvaders/game/service/RecordService.h"
#include "spaceinvaders/game/service/repository/FileRecordRepository.h"

namespace {

constexpr const char* kTestFile = "test_records.tsv";

class RecordServiceTest: public ::testing::Test
{
protected:
    void SetUp() override { std::remove(kTestFile); }
    void TearDown() override { std::remove(kTestFile); }
};

Record makeRecord(const char* name, const int score, const uint32_t wave) { return { name, score, wave, "2026-07-13 10:00" }; }

} // namespace

TEST_F(RecordServiceTest, RankingPorPontuacao)
{
    RecordService service(std::make_shared<FileRecordRepository>(kTestFile));

    service.addRecord(makeRecord("Ana", 300, 2));
    service.addRecord(makeRecord("Bia", 900, 3));
    service.addRecord(makeRecord("Cao", 600, 2));

    const auto sorted = service.listByScore();
    ASSERT_EQ(sorted.size(), 3u);
    EXPECT_EQ(sorted[0].getName(), "Bia");
    EXPECT_EQ(sorted[1].getName(), "Cao");
    EXPECT_EQ(sorted[2].getName(), "Ana");
}

TEST_F(RecordServiceTest, TsvSobreviveAoReload)
{
    {
        RecordService service(std::make_shared<FileRecordRepository>(kTestFile));
        service.addRecord(makeRecord("Ana Maria", 1230, 4));
    }

    // nova instancia le do arquivo (como uma nova execucao do jogo)
    RecordService reloaded(std::make_shared<FileRecordRepository>(kTestFile));
    ASSERT_EQ(reloaded.getAll().size(), 1u);
    EXPECT_EQ(reloaded.getAll()[0].getName(), "Ana Maria");
    EXPECT_EQ(reloaded.getAll()[0].getScore(), 1230);
    EXPECT_EQ(reloaded.getAll()[0].getWave(), 4u);
    EXPECT_EQ(reloaded.getAll()[0].getPlayedAt(), "2026-07-13 10:00");
}

TEST_F(RecordServiceTest, TopDezDecideSeENovoRecorde)
{
    RecordService service(std::make_shared<FileRecordRepository>(kTestFile));

    // com menos de 10 registros, qualquer resultado entra
    EXPECT_TRUE(service.isNewRecord(makeRecord("", 10, 1), 10));

    for (int i = 0; i < 10; ++i)
    {
        service.addRecord(makeRecord("Vet", 100 * (i + 1), 1)); // 100..1000
    }

    EXPECT_TRUE(service.isNewRecord(makeRecord("", 150, 1), 10)) << "melhor que o 10o (100) entra";
    EXPECT_FALSE(service.isNewRecord(makeRecord("", 100, 1), 10)) << "empate com o 10o nao entra";
    EXPECT_FALSE(service.isNewRecord(makeRecord("", 50, 1), 10));
}

TEST_F(RecordServiceTest, ArquivoInexistenteComecaVazio)
{
    RecordService service(std::make_shared<FileRecordRepository>("nao_existe.tsv"));
    EXPECT_TRUE(service.getAll().empty());
}
