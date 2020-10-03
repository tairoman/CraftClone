#ifndef CRAFTBONE_LOGGER_H
#define CRAFTBONE_LOGGER_H

#include <string>

namespace Logger {

    enum class Severity { Info = 0, Low, Medium, High };

    void log(Severity severity, const std::string& msg);

    void registerGlLogger();
}

#endif //CRAFTBONE_LOGGER_H