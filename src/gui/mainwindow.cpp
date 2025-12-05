#include "mainwindow.h"
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFileDialog>
#include <QMessageBox>
#include <QDateTime>
#include <QFileInfo>
#include <chrono>
#include "../core/ImageData.h"
#include "../core/Compressor.h"
#include "../core/Decompressor.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    QWidget *central = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(central);

    auto makePathRow = [&](const QString &labelText, QLineEdit **edit, auto browseSlot) {
        QHBoxLayout *row = new QHBoxLayout();
        row->addWidget(new QLabel(labelText));
        *edit = new QLineEdit();
        (*edit)->setReadOnly(true);
        QPushButton *btn = new QPushButton("Browse...");
        connect(btn, &QPushButton::clicked, this, browseSlot);
        row->addWidget(*edit, 1);
        row->addWidget(btn);
        layout->addLayout(row);
    };

    makePathRow("Input:", &inputEdit, &MainWindow::browseInput);
    makePathRow("Output:", &outputEdit, &MainWindow::browseOutput);

    QHBoxLayout *algoRow = new QHBoxLayout();
    algoRow->addWidget(new QLabel("Algorithm:"));
    algoCombo = new QComboBox();
    algoCombo->addItems({"Huffman", "RLE", "LZW", "Lossy DCT"});
    connect(algoCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onAlgorithmChanged);
    algoRow->addWidget(algoCombo);

    algoRow->addWidget(new QLabel("Mode:"));
    modeCombo = new QComboBox();
    modeCombo->addItems({"Compress", "Decompress"});
    algoRow->addWidget(modeCombo);
    layout->addLayout(algoRow);

    QHBoxLayout *qRow = new QHBoxLayout();
    qRow->addWidget(new QLabel("Quality (DCT):"));
    qualitySpin = new QSpinBox();
    qualitySpin->setRange(1, 100);
    qualitySpin->setValue(75);
    qRow->addWidget(qualitySpin);
    layout->addLayout(qRow);

    QPushButton *runBtn = new QPushButton("Run");
    connect(runBtn, &QPushButton::clicked, this, &MainWindow::onRun);
    layout->addWidget(runBtn);

    logView = new QPlainTextEdit();
    logView->setReadOnly(true);
    layout->addWidget(logView, 1);

    setCentralWidget(central);
    setWindowTitle("Image Compression Tool");
    onAlgorithmChanged(0);
}

void MainWindow::logMessage(const QString &msg) {
    logView->appendPlainText(QDateTime::currentDateTime().toString("hh:mm:ss ") + msg);
}

void MainWindow::browseInput() {
    QString path;
    if (modeCombo->currentIndex() == 0) {
        path = QFileDialog::getOpenFileName(this, "Open Image", QString(), "Images (*.png *.bmp *.jpg *.jpeg)");
    } else {
        path = QFileDialog::getOpenFileName(this, "Open Compressed", QString(), "Compressed (*.huf *.rle *.lzw *.dct *.*)");
    }
    if (!path.isEmpty()) inputEdit->setText(path);
}

void MainWindow::browseOutput() {
    QString path = QFileDialog::getSaveFileName(this, "Save File", QString());
    if (!path.isEmpty()) outputEdit->setText(path);
}

void MainWindow::onAlgorithmChanged(int index) {
    bool isDct = (index == 3);
    qualitySpin->setEnabled(isDct);
}

void MainWindow::onRun() {
    if (inputEdit->text().isEmpty() || outputEdit->text().isEmpty()) {
        QMessageBox::warning(this, "Validation", "Please select input and output files.");
        return;
    }
    std::string algo;
    switch (algoCombo->currentIndex()) {
        case 0: algo = "huffman"; break;
        case 1: algo = "rle"; break;
        case 2: algo = "lzw"; break;
        case 3: algo = "dct"; break;
    }
    bool compress = modeCombo->currentIndex() == 0;
    try {
        auto start = std::chrono::steady_clock::now();
        if (compress) {
            cv::Mat img = ImageIO::loadImage(inputEdit->text().toStdString(), false);
            Compressor::compressImage(algo, img, outputEdit->text().toStdString(), qualitySpin->value());
            auto end = std::chrono::steady_clock::now();
            auto originalSize = static_cast<uint64_t>(img.total() * img.elemSize());
            uint64_t compressedSize = QFileInfo(outputEdit->text()).size();
            double ratio = compressedSize ? static_cast<double>(originalSize) / compressedSize : 0.0;
            logMessage(QString("Compression finished. ratio=%1, time(ms)=%2")
                       .arg(ratio).arg(std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count()));
        } else {
            cv::Mat img = Decompressor::decompressImage(algo, inputEdit->text().toStdString());
            ImageIO::saveImage(outputEdit->text().toStdString(), img);
            auto end = std::chrono::steady_clock::now();
            logMessage(QString("Decompression finished. time(ms)=%1")
                       .arg(std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count()));
        }
        QMessageBox::information(this, "Done", "Operation completed.");
    } catch (const std::exception &ex) {
        logMessage(QString("Error: %1").arg(ex.what()));
        QMessageBox::critical(this, "Error", ex.what());
    }
}
