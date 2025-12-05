# Image Compression Tool

C++17 project providing CLI and Qt GUI for Huffman, RLE, LZW, and a lossy DCT-based codec. Requires OpenCV and Qt5/Qt6 Widgets.

## Building
```bash
mkdir build && cd build
cmake ..
cmake --build .
```

## CLI usage examples
```bash
./img_compress huffman compress input.png output.huf
./img_compress huffman decompress output.huf restored.png
./img_compress rle compress input.png output.rle
./img_compress lzw compress input.png output.lzw
./img_compress dct compress input.png output.dct 75
./img_compress dct decompress output.dct restored.png
```

## GUI
Run the Qt GUI executable after building:
```bash
./img_compress_gui
```
It offers file pickers, algorithm/mode selection, and DCT quality slider. Logs show compression ratios and timing.
