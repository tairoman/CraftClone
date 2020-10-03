#include "Logger.h"

#include <iostream>

#include <GL/glew.h>

namespace
{
using namespace Logger;

auto serializeSeverity(Severity severity)
{
    switch (severity) {
    case Severity::Info: return "Info";
    case Severity::Low: return "Low";
    case Severity::Medium: return "Medium";
    case Severity::High: return "High";
    default: return "UNKNOWN";
    }
}

void GLAPIENTRY openglDebugCallback( GLenum source,
            GLenum type,
            [[maybe_unused]] GLuint id,
            GLenum severity,
            [[maybe_unused]] GLsizei length,
            const GLchar* message,
            [[maybe_unused]] const void* userParam)
{
    std::string src;
    switch (source)
    {
        case GL_DEBUG_SOURCE_API:             src = "API"; break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   src = "Window System API"; break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: src = "Shader Compiler"; break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:     src = "Third Party"; break;
        case GL_DEBUG_SOURCE_APPLICATION:     src = "User Application"; break;
        case GL_DEBUG_SOURCE_OTHER:           src = "Other"; break;
    }

    std::string tp;
    switch (type)
    {
        case GL_DEBUG_TYPE_ERROR:               tp = "Error"; break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: tp = "Deprecated Behaviour"; break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  tp = "Undefined Behaviour"; break; 
        case GL_DEBUG_TYPE_PORTABILITY:         tp = "Portability"; break;
        case GL_DEBUG_TYPE_PERFORMANCE:         tp = "Performance"; break;
        case GL_DEBUG_TYPE_MARKER:              tp = "Marker"; break;
        case GL_DEBUG_TYPE_PUSH_GROUP:          tp = "Push Group"; break;
        case GL_DEBUG_TYPE_POP_GROUP:           tp = "Pop Group"; break;
        case GL_DEBUG_TYPE_OTHER:               tp = "Other"; break;
    }
    
    Severity sev = Severity::Info;
    switch (severity)
    {
        case GL_DEBUG_SEVERITY_HIGH:         sev = Severity::High; break;
        case GL_DEBUG_SEVERITY_MEDIUM:       sev = Severity::Medium; break;
        case GL_DEBUG_SEVERITY_LOW:          sev = Severity::Low; break;
        case GL_DEBUG_SEVERITY_NOTIFICATION: sev = Severity::Info; break;
    }
    
    const auto prefix = "OpenGL (Source = " + src + ", Type = " + tp + ")\n";
    log(sev, prefix + message);
}

} // anon namespace

void Logger::log(Severity severity, const std::string& msg)
{
    std::cout << "[" << serializeSeverity(severity) << "]\n";
    std::cout << msg << "\n";
    std::cout << std::endl;
}

void Logger::registerGlLogger()
{
    glEnable              ( GL_DEBUG_OUTPUT );
    glDebugMessageCallback( &openglDebugCallback, nullptr );
}