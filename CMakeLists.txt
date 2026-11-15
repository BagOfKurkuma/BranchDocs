cmake_minimum_required(VERSION 3.16)
project(BranchDocs LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets Test Sql REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets Test Sql REQUIRED)

option(ENABLE_COVERAGE "Enable coverage reporting" OFF)
if(ENABLE_COVERAGE)
    add_compile_options(--coverage -O0 -g)
    add_link_options(--coverage)
endif()

add_library(branchdocs_domain STATIC
    src/domain/StorageType.h
    src/domain/Uuid.h src/domain/Uuid.cpp
    src/domain/Version.h src/domain/Version.cpp
    src/domain/VersionHistory.h src/domain/VersionHistory.cpp
    src/domain/Document.h src/domain/Document.cpp
    src/domain/IRepository.h
    src/domain/commands/Command.h
    src/domain/commands/CommandStack.h src/domain/commands/CommandStack.cpp
    src/domain/commands/TextEditCommand.h src/domain/commands/TextEditCommand.cpp
    src/domain/commands/MergeCommitCommand.h src/domain/commands/MergeCommitCommand.cpp
)
target_include_directories(branchdocs_domain PUBLIC src)

add_library(branchdocs_infrastructure STATIC
    src/infrastructure/FileRepository.h src/infrastructure/FileRepository.cpp
    src/infrastructure/SqliteRepository.h src/infrastructure/SqliteRepository.cpp
)
target_link_libraries(branchdocs_infrastructure
    PUBLIC branchdocs_domain Qt${QT_VERSION_MAJOR}::Sql
    PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

add_executable(BranchDocs
    src/main.cpp
    src/app/EditorController.h src/app/EditorController.cpp
    src/app/RepositoryFactory.h
    src/ui/MainWindow.h src/ui/MainWindow.cpp
    src/ui/MergeDialog.h src/ui/MergeDialog.cpp
    src/ui/DiffDialog.h src/ui/DiffDialog.cpp
    src/ui/VersionViewerDialog.h src/ui/VersionViewerDialog.cpp
    src/ui/SentenceSplitter.h
)

target_link_libraries(BranchDocs
    PRIVATE branchdocs_domain branchdocs_infrastructure
            Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Sql)

enable_testing()

add_executable(domain_tests tests/domain_tests.cpp)
target_link_libraries(domain_tests PRIVATE branchdocs_domain Qt${QT_VERSION_MAJOR}::Test)
add_test(NAME domain_tests COMMAND domain_tests)

add_executable(memento_tests tests/memento_tests.cpp)
target_link_libraries(memento_tests PRIVATE branchdocs_domain Qt${QT_VERSION_MAJOR}::Test)
add_test(NAME memento_tests COMMAND memento_tests)

add_executable(controller_tests tests/controller_tests.cpp src/app/EditorController.cpp src/app/RepositoryFactory.h)
target_include_directories(controller_tests PRIVATE src)
target_link_libraries(controller_tests PRIVATE branchdocs_domain branchdocs_infrastructure Qt${QT_VERSION_MAJOR}::Test Qt${QT_VERSION_MAJOR}::Sql)
add_test(NAME controller_tests COMMAND controller_tests)

add_executable(command_tests tests/command_tests.cpp)
target_link_libraries(command_tests PRIVATE branchdocs_domain Qt${QT_VERSION_MAJOR}::Test)
add_test(NAME command_tests COMMAND command_tests)

add_executable(integration_tests tests/integration_tests.cpp)
target_link_libraries(integration_tests PRIVATE branchdocs_domain Qt${QT_VERSION_MAJOR}::Test)
add_test(NAME integration_tests COMMAND integration_tests)

add_executable(repository_tests tests/repository_tests.cpp)
target_link_libraries(repository_tests PRIVATE branchdocs_infrastructure Qt${QT_VERSION_MAJOR}::Test)
add_test(NAME repository_tests COMMAND repository_tests)

if(ENABLE_COVERAGE)
    add_custom_target(coverage
        COMMAND lcov --capture --directory ${CMAKE_BINARY_DIR} --output-file coverage.info --rc branch_coverage=1 --ignore-errors mismatch,gcov,inconsistent,unused,path
        COMMAND lcov --extract coverage.info "*src/domain/*" "*src/app/*" "*src/infrastructure/*" --output-file coverage_filtered.info --rc branch_coverage=1 --ignore-errors unused,inconsistent,corrupt,path
        COMMAND lcov --list coverage_filtered.info --rc branch_coverage=1 --ignore-errors inconsistent,corrupt,path
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        VERBATIM
    )
endif()