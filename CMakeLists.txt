cmake_minimum_required(VERSION 3.16)
project(BranchDocs VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Widgets Test)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets Test)

option(ENABLE_COVERAGE  "Enable coverage reporting (GCC/Clang only)" OFF)
option(ENABLE_WARNINGS  "Enable strict compiler warnings"            ON)
option(ENABLE_SANITIZERS "Enable ASan+UBSan (Debug builds)"          OFF)

if(ENABLE_WARNINGS)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        add_compile_options(-Wall -Wextra -Wpedantic -Wshadow)
    elseif(MSVC)
        add_compile_options(/W4)
    endif()
endif()

if(ENABLE_COVERAGE)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        add_compile_options(--coverage -O0 -g)
        add_link_options(--coverage)
    endif()
endif()

if(ENABLE_SANITIZERS)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
        add_link_options(-fsanitize=address,undefined)
    endif()
endif()

add_library(branchdocs_domain STATIC
    src/domain/Uuid.h
    src/domain/Uuid.cpp
    src/domain/Version.h
    src/domain/Version.cpp
    src/domain/VersionHistory.h
    src/domain/VersionHistory.cpp
    src/domain/Document.h
    src/domain/Document.cpp
)
target_include_directories(branchdocs_domain PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

add_library(branchdocs_app STATIC
    src/app/EditorController.h
    src/app/EditorController.cpp
)
target_include_directories(branchdocs_app PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(branchdocs_app
    PUBLIC
        branchdocs_domain
        Qt${QT_VERSION_MAJOR}::Core
)

add_executable(BranchDocs
    src/main.cpp
    src/ui/MainWindow.h
    src/ui/MainWindow.cpp
)
target_link_libraries(BranchDocs
    PRIVATE
        branchdocs_app
        Qt${QT_VERSION_MAJOR}::Widgets
)

enable_testing()

add_executable(domain_tests tests/domain_tests.cpp)
target_link_libraries(domain_tests
    PRIVATE
        branchdocs_domain
        Qt${QT_VERSION_MAJOR}::Test
)
add_test(NAME domain_tests COMMAND domain_tests)

add_executable(controller_tests tests/controller_tests.cpp)
target_link_libraries(controller_tests
    PRIVATE
        branchdocs_app
        Qt${QT_VERSION_MAJOR}::Test
)
add_test(NAME controller_tests COMMAND controller_tests)

add_executable(integration_tests tests/integration_tests.cpp)
target_link_libraries(integration_tests
    PRIVATE
        branchdocs_domain
        Qt${QT_VERSION_MAJOR}::Test
)
add_test(NAME integration_tests COMMAND integration_tests)

if(ENABLE_COVERAGE)
    find_program(LCOV_EXECUTABLE    lcov)
    find_program(GENHTML_EXECUTABLE genhtml)

    if(LCOV_EXECUTABLE AND GENHTML_EXECUTABLE)
        add_custom_target(coverage
            COMMAND ${LCOV_EXECUTABLE}
                    --capture
                    --directory ${CMAKE_BINARY_DIR}
                    --output-file ${CMAKE_BINARY_DIR}/coverage.info
                    --rc branch_coverage=1
                    --rc geninfo_unexecuted_blocks=1
                    --ignore-errors inconsistent,mismatch
            COMMAND ${LCOV_EXECUTABLE}
                    --remove ${CMAKE_BINARY_DIR}/coverage.info
                    '/usr/*'
                    '*/tests/*'
                    '*/build/*'
                    '*_autogen/*'
                    '*/autogen/*'
                    '*.moc'
                    'moc_*.cpp'
                    --output-file ${CMAKE_BINARY_DIR}/coverage_filtered.info
                    --rc branch_coverage=1
                    --ignore-errors unused
            COMMAND ${GENHTML_EXECUTABLE}
                    ${CMAKE_BINARY_DIR}/coverage_filtered.info
                    --output-directory ${CMAKE_BINARY_DIR}/coverage_html
                    --branch-coverage
                    --rc branch_coverage=1
                    --ignore-errors inconsistent
            COMMAND ${LCOV_EXECUTABLE}
                    --list ${CMAKE_BINARY_DIR}/coverage_filtered.info
                    --rc branch_coverage=1
                    --ignore-errors unused,empty
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        )
    endif()
endif()