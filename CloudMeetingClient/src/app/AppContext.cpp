#include "app/AppContext.h"

AppContext &AppContext::instance()
{
    static AppContext ctx;
    return ctx;
}

void AppContext::setup() {}
