#pragma once

#include <string>

namespace Logger {

    enum class Severity { Info = 0, Low, Medium, High };

    void log(Severity severity, const std::string& msg);

    void registerGlLogger();
}