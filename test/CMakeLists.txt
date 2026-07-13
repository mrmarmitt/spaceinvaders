# Configuracao dos testes (mesmo padrao do 8puzzle/cengine)

# -------------------------------------------------------------
# 1. GoogleTest via FetchContent
# -------------------------------------------------------------
include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.17.0
)

FetchContent_MakeAvailable(googletest)

# -------------------------------------------------------------
# 2. Executavel de testes — linka a lib do jogo
# -------------------------------------------------------------
add_executable(spaceinvaders_tests
    "${CMAKE_CURRENT_SOURCE_DIR}/spaceinvaders/game/WorldTest.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/spaceinvaders/game/service/RecordServiceTest.cpp"
)

target_link_libraries(spaceinvaders_tests
  PRIVATE
  spaceinvaders_lib
  gtest_main
)

# -------------------------------------------------------------
# 3. Registrar no CTest
# -------------------------------------------------------------
include(GoogleTest)

gtest_discover_tests(spaceinvaders_tests)
