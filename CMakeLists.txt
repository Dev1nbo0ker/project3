cmake_minimum_required(VERSION 3.16)
project(ImageCompressionTool LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV REQUIRED)
# Try Qt6 first, fallback to Qt5
find_package(Qt6 COMPONENTS Widgets QUIET)
if(Qt6_FOUND)
    set(QT_PACKAGE Qt6)
else()
    find_package(Qt5 COMPONENTS Widgets REQUIRED)
    set(QT_PACKAGE Qt5)
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

include_directories(${OpenCV_INCLUDE_DIRS})

set(CORE_SOURCES
    src/core/BitIO.cpp
    src/core/ImageData.cpp
    src/core/Huffman.cpp
    src/core/RLE.cpp
    src/core/LZW.cpp
    src/core/DCTCodec.cpp
    src/core/Compressor.cpp
    src/core/Decompressor.cpp
)
set(CORE_HEADERS
    src/core/BitIO.h
    src/core/ImageData.h
    src/core/Huffman.h
    src/core/RLE.h
    src/core/LZW.h
    src/core/DCTCodec.h
    src/core/Compressor.h
    src/core/Decompressor.h
)

add_executable(img_compress src/main.cpp ${CORE_SOURCES} ${CORE_HEADERS})
target_include_directories(img_compress PRIVATE src)
target_link_libraries(img_compress PRIVATE ${OpenCV_LIBS})

add_executable(img_compress_gui src/gui/main.cpp src/gui/mainwindow.cpp ${CORE_SOURCES} ${CORE_HEADERS})
target_include_directories(img_compress_gui PRIVATE src)
target_link_libraries(img_compress_gui PRIVATE ${OpenCV_LIBS} ${QT_PACKAGE}::Widgets)

target_compile_definitions(img_compress_gui PRIVATE QT_WIDGETS_LIB)
