#pragma once

/**
 * @file AppContext.h
 * @brief 声明应用上下文单例。
 */

/**
 * @class AppContext
 * @brief 管理应用级初始化流程的全局上下文。
 */
class AppContext
{
public:
    /**
     * @brief 获取应用上下文单例实例。
     * @return 应用上下文的全局唯一实例引用。
     */
    static AppContext& instance();

    /**
     * @brief 执行应用上下文初始化。
     */
    void setup();
};
