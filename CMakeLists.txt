# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.13)
project(SQIsign VERSION 1.0 LANGUAGES C ASM)

set(SQISIGN_SO_VERSION "0")
set(CMAKE_C_STANDARD 99)

set(SQISIGN_BUILD_TYPE "ref" CACHE STRING "Implementation: ref (modarith) or fiat_crypto")
set_property(CACHE SQISIGN_BUILD_TYPE PROPERTY STRINGS ref fiat_crypto)
# Keep previous optimized backend spellings as aliases.
if(SQISIGN_BUILD_TYPE MATCHES "^(opt|optimized)$")
    set(SQISIGN_BUILD_TYPE "ref" CACHE STRING "Implementation" FORCE)
endif()
if(NOT SQISIGN_BUILD_TYPE MATCHES "^(ref|fiat_crypto|broadwell|arm64crypto)$")
    message(FATAL_ERROR "Unsupported SQISIGN_BUILD_TYPE: ${SQISIGN_BUILD_TYPE}")
endif()
if(SQISIGN_BUILD_TYPE STREQUAL "ref")
    set(SQISIGN_GF_SOURCE_DIR "${PROJECT_SOURCE_DIR}/src/gf/ref")
else()
    set(SQISIGN_GF_SOURCE_DIR "${PROJECT_SOURCE_DIR}/src/gf/fiat_crypto")
endif()

if(SQISIGN_BUILD_TYPE STREQUAL "ref")
    add_compile_definitions(SQISIGN_BUILD_TYPE_REF)
elseif(SQISIGN_BUILD_TYPE STREQUAL "fiat_crypto")
    add_compile_definitions(SQISIGN_BUILD_TYPE_FIAT_CRYPTO)
elseif(SQISIGN_BUILD_TYPE STREQUAL "broadwell")
    add_compile_definitions(SQISIGN_BUILD_TYPE_BROADWELL)
elseif(SQISIGN_BUILD_TYPE STREQUAL "arm64crypto")
    add_compile_definitions(SQISIGN_BUILD_TYPE_ARM64CRYPTO)
endif()

include(CTest)

option(ENABLE_STRICT "Build with strict compile options." OFF)
option(ENABLE_TESTS  "Enable compilation of tests." ON)
option(ENABLE_CT_TESTING  "Enable compilation for constant time testing." OFF)
option(ENABLE_GMP_BUILD "Download and build external version of GMP" OFF)
option(ENABLE_DOCS "Enable Doxygen documentation target." OFF)

if(SQISIGN_BUILD_TYPE STREQUAL "broadwell")
	SET(SVARIANT_S "lvl1")
else()
	SET(SVARIANT_S "lvl1" "lvl3" "lvl5")
endif()

include(.cmake/flags.cmake)
include(.cmake/sanitizers.cmake)
include(.cmake/target.cmake)
include(.cmake/gmpconfig.cmake)

if(ENABLE_DOCS)
    include(.cmake/target_docs.cmake)
endif()


set(SELECT_IMPL_TYPE ${PROJECT_SOURCE_DIR}/.cmake/impl_type.cmake)
set(SELECT_SQISIGN_VARIANT ${PROJECT_SOURCE_DIR}/.cmake/sqisign_variant.cmake)

set(INC_PUBLIC ${PROJECT_SOURCE_DIR}/include)

add_subdirectory(src)
