cmake_minimum_required(VERSION 3.16)
project(ImageCompressionTool LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (MSVC)
    add_compile_options(
        /Zc:__cplusplus
        /permissive-
        /utf-8
    )
endif()

find_package(OpenCV REQUIRED)

set(Qt_COMPONENTS Widgets)
find_package(Qt6 COMPONENTS ${Qt_COMPONENTS} QUIET)
if (NOT Qt6_FOUND)
    find_package(Qt5 COMPONENTS ${Qt_COMPONENTS} QUIET)
endif()

option(BUILD_GUI "Build Qt GUI application" ON)

set(CORE_SOURCES
    src/core/BitIO.cpp
    src/core/Compressor.cpp
    src/core/DCTCodec.cpp
    src/core/Decompressor.cpp
    src/core/Huffman.cpp
    src/core/ImageData.cpp
    src/core/LZW.cpp
    src/core/RLE.cpp
)

set(CORE_HEADERS
    src/core/BitIO.h
    src/core/Compressor.h
    src/core/DCTCodec.h
    src/core/Decompressor.h
    src/core/Huffman.h
    src/core/ImageData.h
    src/core/LZW.h
    src/core/RLE.h
)

add_executable(img_compress
    src/main.cpp
    ${CORE_SOURCES}
    ${CORE_HEADERS}
)

target_include_directories(img_compress PRIVATE
    ${OpenCV_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(img_compress PRIVATE
    ${OpenCV_LIBS}
)

if (BUILD_GUI AND (Qt6_FOUND OR Qt5_FOUND))
    if (Qt6_FOUND)
        set(QT_LIBS Qt6::Widgets)
    else()
        set(QT_LIBS Qt5::Widgets)
    endif()

    add_executable(img_compress_gui
        src/gui/main.cpp
        src/gui/mainwindow.cpp
        src/gui/mainwindow.h
        ${CORE_SOURCES}
        ${CORE_HEADERS}
    )

    set_target_properties(img_compress_gui PROPERTIES
        AUTOMOC ON
        AUTOUIC ON
        AUTORCC ON
    )

    target_include_directories(img_compress_gui PRIVATE
        ${OpenCV_INCLUDE_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    )

    target_link_libraries(img_compress_gui PRIVATE
        ${QT_LIBS}
        ${OpenCV_LIBS}
    )
else()
    if (BUILD_GUI)
        message(WARNING "Qt was not found; BUILD_GUI=ON but img_compress_gui will not be built.")
    endif()
endif()
