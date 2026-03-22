#include "app/Application.h"
#include "ui/MainWindow.h"
#include "ui/AppStyle.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    // High-DPI support
    QApplication::setHighDpiScaleFactorRoundingPolicy(
        Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);

    QApplication app(argc, argv);

    // Apply global dark theme
    app.setStyleSheet(AppStyle::GLOBAL_QSS);

    MainWindow w;
    w.show();
    return app.exec();
}
