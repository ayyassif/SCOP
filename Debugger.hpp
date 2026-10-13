#ifndef SCOP_DEBUGGER_HPP
#define SCOP_DEBUGGER_HPP

#include <iostream>
#include <initializer_list>
#include <string_view>
#include <type_traits>

// End each log line with std::endl or '\n' to reset the terminal color.
class Debugger
{
public:
    struct ControlInfo
    {
        std::string_view keys;
        std::string_view description;
    };

    explicit Debugger(std::string_view label,
                      std::string_view color,
                      std::ostream& output = std::cout);

    Debugger(const Debugger&) = delete;
    Debugger& operator=(const Debugger&) = delete;

    // Supports strings, string_views, numbers, and other values accepted by ostream.
    template <typename T>
    Debugger& operator<<(const T& value)
    {
        if constexpr (std::is_convertible_v<const T&, std::string_view>)
        {
            writeText(std::string_view(value));
        }
        else
        {
            startLine();
            m_output << value;
        }
        return *this;
    }

    // Handle a newline character so the terminal color is reset at line endings.
    Debugger& operator<<(char value);

    // Special handling for std::endl; other ostream manipulators still work.
    using StreamManipulator = std::ostream& (*)(std::ostream&);
    Debugger& operator<<(StreamManipulator manipulator);

    static void printWelcome();
    static void printControls();
    static void printControls(std::initializer_list<ControlInfo> controls);
    static void printAvailableModes();
    static void cameraModeChanged(std::string_view modeName);
    static void renderModeChanged(std::string_view modeName);

private:
    static constexpr std::string_view RESET = "\033[0m";

    std::string_view m_label;
    std::string_view m_color;
    std::ostream& m_output;
    bool m_lineStarted = false;

    static StreamManipulator endlManipulator();

    void startLine();
    void finishLine();
    void writeText(std::string_view text);
};

namespace DEBUG
{
    // Informational messages
    inline Debugger INFO{"INFO", "\033[36m"};       // Cyan
    inline Debugger SUCCESS{"SUCCESS", "\033[32m"}; // Green

    // Problems and warnings
    inline Debugger WARNING{"WARNING", "\033[33m"}; // Yellow
    inline Debugger ERROR{"ERROR", "\033[31m", std::cerr}; // Red, stderr

    // SCOP-specific messages
    inline Debugger CAMERA{"CAMERA", "\033[35m"}; // Magenta
    inline Debugger RENDER{"RENDER", "\033[34m"}; // Blue
    inline Debugger INPUT{"INPUT", "\033[36m"};  // Cyan
    inline Debugger TRACE{"DEBUG", "\033[90m"};  // Gray
}

#endif // SCOP_DEBUGGER_HPP
