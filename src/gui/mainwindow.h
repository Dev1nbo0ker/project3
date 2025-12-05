#pragma once
#include <QMainWindow>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QPlainTextEdit>
#include <QSpinBox>

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    void browseInput();
    void browseOutput();
    void onRun();
    void onAlgorithmChanged(int index);

private:
    QLineEdit *inputEdit;
    QLineEdit *outputEdit;
    QComboBox *algoCombo;
    QComboBox *modeCombo;
    QSpinBox *qualitySpin;
    QPlainTextEdit *logView;
    void logMessage(const QString &msg);
};
