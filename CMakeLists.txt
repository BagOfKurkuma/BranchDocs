cmake_minimum_required(VERSION 3.16)
project(BranchDocs LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets Test REQUIRED)

add_library(branchdocs_domain STATIC
    src/domain/Uuid.h            src/domain/Uuid.cpp
    src/domain/Version.h         src/domain/Version.cpp
    src/domain/VersionHistory.h  src/domain/VersionHistory.cpp
    src/domain/Document.h        src/domain/Document.cpp
)
target_include_directories(branchdocs_domain PUBLIC src)

add_executable(BranchDocs
    src/main.cpp
    src/app/EditorController.h  src/app/EditorController.cpp
    src/ui/MainWindow.h         src/ui/MainWindow.cpp
)
target_include_directories(BranchDocs PRIVATE src)
target_link_libraries(BranchDocs
    PRIVATE
        branchdocs_domain
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