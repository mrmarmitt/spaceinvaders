# Space Invaders — dominio C++ puro + testes (degrau 5 da PoC sprites).
#
# Mesmo desenho do 8puzzle: a lib do jogo (dominio + maquina de estados)
# compila sem NENHUMA plataforma; a plataforma The-Forge builda a parte via
# MSBuild (src/platform/theforge/PC_VS2019) contra a arvore do The-Forge.
# A cengine (via FetchContent) exige CMake >= 3.23.
cmake_minimum_required(VERSION 3.23)

project(SpaceInvaders)

# A cengine 0.5.0 exige C++23 e propaga o padrao via target_compile_features
# PUBLIC; alinhamos o projeto ao mesmo padrao (como no 8puzzle).
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(SPACEINVADERS_BUILD_TESTS "Build the test suite" ON)

# -----------------------------
# Dependencia: cengine (routing — GameRouter/StateGameFlow usam IState/IRouter)
# -----------------------------
include(FetchContent)

FetchContent_Declare(
  cengine
  GIT_REPOSITORY https://github.com/mrmarmitt/cengine.git
  GIT_TAG 0.5.0
)

# Sem a suite de testes da cengine (evita fetch do GoogleTest dela).
set(CENGINE_BUILD_TESTS OFF)
FetchContent_MakeAvailable(cengine)

# -----------------------------
# spaceinvaders_lib — o dominio do jogo, linkavel pela suite de testes.
# A plataforma theforge fica fora (buildada via MSBuild).
# -----------------------------
file(GLOB_RECURSE GAME_SOURCES
        "src/spaceinvaders/*.cpp" "src/spaceinvaders/*.h"
)

add_library(spaceinvaders_lib STATIC ${GAME_SOURCES})
target_include_directories(spaceinvaders_lib PUBLIC src)
target_link_libraries(spaceinvaders_lib PUBLIC
    cengine::core
    cengine::routing
)

# -----------------------------
# Testes (GoogleTest via CTest)
# -----------------------------
if(SPACEINVADERS_BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()
