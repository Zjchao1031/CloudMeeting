/**
 * @file main.cpp
 * @brief 定义客户端程序入口。
 */
#include "app/Application.h"
#include "app/AppContext.h"
#include "ui/MainWindow.h"
#include "ui/AppStyle.h"
#include "media/MediaEngine.h"
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

    // 初始化应用上下文与业务服务。
    AppContext::instance().setup();

    // 应用全局深色主题。
    app.setStyleSheet(AppStyle::GLOBAL_QSS);

    // 在事件循环退出前提前停止所有媒体/网络资源，确保多媒体后端线程在
    // QApplication 销毁前得到清理；静态单例析构器中的同名调用届时为空操作。
    QObject::connect(&app, &QApplication::aboutToQuit, []() {
        AppContext::instance().mediaEngine()->stopAll();
    });

    MainWindow w;
    w.show();
    return app.exec();
}
