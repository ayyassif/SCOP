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
	DEBUG::INFO << "======================= CONTROLS =======================" << std::endl;
DEBUG::INFO << "  W / A / S / D   Move the camera" << std::endl;
DEBUG::INFO << "  SPACE / SHIFT   Move up / down" << std::endl;
DEBUG::INFO << "  CTRL            Toggle camera speed boost" << std::endl;
DEBUG::INFO << "  MOUSE           Look around" << std::endl;
DEBUG::INFO << "  M               Toggle Color / Texture mode" << std::endl;
DEBUG::INFO << "  N               Toggle Grey / Chart mode" << std::endl;
DEBUG::INFO << "  C               Switch between Orbit and Free Camera modes" << std::endl;
DEBUG::INFO << "  TAB             Toggle mouse capture" << std::endl;
DEBUG::INFO << "  ESC             Exit application" << std::endl;
DEBUG::INFO << "========================================================" << std::endl;
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
