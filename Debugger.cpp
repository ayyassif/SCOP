#include "Debugger.hpp"

Debugger::Debugger(std::string_view label,
				   std::string_view color,
				   std::ostream& output)
	: m_label(label), m_color(color), m_output(output)
{
}

Debugger& Debugger::operator<<(char value)
{
	if (value == '\n')
		finishLine();
	else
	{
		startLine();
		m_output.put(value);
	}
	return *this;
}

Debugger& Debugger::operator<<(StreamManipulator manipulator)
{
	if (manipulator == endlManipulator())
	{
		if (m_lineStarted)
			m_output << RESET;
		manipulator(m_output);
		m_lineStarted = false;
	}
	else
	{
		startLine();
		manipulator(m_output);
	}
	return *this;
}

Debugger::StreamManipulator Debugger::endlManipulator()
{
	return static_cast<StreamManipulator>(std::endl<char, std::char_traits<char>>);
}

void Debugger::startLine()
{
	if (m_lineStarted)
		return;

	m_output << m_color << '[' << m_label << "] ";
	m_lineStarted = true;
}

void Debugger::finishLine()
{
	if (m_lineStarted)
		m_output << RESET;
	m_output.put('\n');
	m_lineStarted = false;
}

void Debugger::writeText(std::string_view text)
{
	for (char character : text)
	{
		if (character == '\n')
			finishLine();
		else
		{
			startLine();
			m_output.put(character);
		}
	}
}

void Debugger::printWelcome()
{
	DEBUG::INFO << "========================================" << std::endl;
	DEBUG::INFO << "          SCOP - 3D Model Viewer" << std::endl;
	DEBUG::INFO << "========================================" << std::endl;
}

void Debugger::printControls()
{
	printControls({
		{"W / A / S / D", "Move the camera"},
		{"Mouse", "Look around"},
		{"M", "Change the render mode"},
		{"C", "Change camera mode"},
		{"TAB", "toggle the mouse on/off"},
		{"ESC", "Exit the application"},
	});
}

void Debugger::printControls(std::initializer_list<ControlInfo> controls)
{
	DEBUG::INFO << "Controls:" << std::endl;
	for (const ControlInfo& control : controls)
		DEBUG::INFO << "  " << control.keys << " - " << control.description << std::endl;
}

void Debugger::printAvailableModes()
{
	DEBUG::INFO << "Available render modes:" << std::endl;
	DEBUG::INFO << "  1. Color" << std::endl;
	DEBUG::INFO << "  2. Texture" << std::endl;
	DEBUG::INFO << "  3. Shades of Grey" << std::endl;
	DEBUG::INFO << "  4. Chart Colors" << std::endl;
    DEBUG::INFO << std::endl;
    DEBUG::INFO << "Available camera modes:" << std::endl;
    DEBUG::INFO << "  1. Orbit Camera" << std::endl;
	DEBUG::INFO << "  2. Free Camera" << std::endl;
}

void Debugger::cameraModeChanged(std::string_view modeName)
{
	DEBUG::CAMERA << "Camera mode changed to: " << modeName << std::endl;
}

void Debugger::renderModeChanged(std::string_view modeName)
{
	DEBUG::RENDER << "Render mode changed to: " << modeName << std::endl;
}
