/**
 * @file main.cpp
 * @brief 定义客户端程序入口。
 */
#include "app/Application.h"
#include "ui/MainWindow.h"
#include "ui/AppStyle.h"
#include <QApplication>

/**
 * @brief 启动 Qt 客户端应用程序。
 * @param[in] argc 命令行参数数量。
 * @param[in] argv 命令行参数数组。
 * @return 应用程序退出码。
 */
int main(int argc, char *argv[])
{
    // 启用高 DPI 缩放策略。
    QApplication::setHighDpiScaleFactorRoundingPolicy(
        Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);

    QApplication app(argc, argv);

    // 应用全局深色主题。
    app.setStyleSheet(AppStyle::GLOBAL_QSS);

    MainWindow w;
    w.show();
    return app.exec();
}
