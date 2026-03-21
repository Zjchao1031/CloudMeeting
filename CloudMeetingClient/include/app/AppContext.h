#pragma once

class AppContext
{
public:
    static AppContext& instance();
    void setup();
};
