#include <QApplication>
#include "mainwindow.h"

// GUI entry. 启动Qt窗口，复用核心算法模块。
int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    MainWindow w;
    w.show();
    return app.exec();
}
