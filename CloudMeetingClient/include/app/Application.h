#pragma once

/**
 * @file Application.h
 * @brief 声明应用生命周期管理类。
 */

/**
 * @class Application
 * @brief 封装客户端应用的初始化与关闭流程。
 */
class Application
{
public:
    /**
     * @brief 构造应用对象。
     */
    Application();

    /**
     * @brief 初始化应用运行环境。
     */
    void init();

    /**
     * @brief 关闭应用并释放相关资源。
     */
    void shutdown();
};
