#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setWindowIcon(QIcon(":/icon/RWX.png")); // Using a Qt resource

    MainWindow w;
    w.show();
    return app.exec();
}

