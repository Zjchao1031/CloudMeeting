/**
 * @file AppContext.cpp
 * @brief 实现应用上下文单例。
 */
#include "app/AppContext.h"

AppContext &AppContext::instance()
{
    static AppContext ctx;
    return ctx;
}

void AppContext::setup() {}
