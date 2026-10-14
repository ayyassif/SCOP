#include "Shader.hpp"
#include "Debugger.hpp"

#include <stdexcept>

static bool compileShader(const unsigned int &shader, const char *shaderStr, const char *&shaderCode)
{
    glShaderSource(shader, 1, &shaderCode, NULL);
    glCompileShader(shader);
    // start: error check
    int  success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if(!success)
    {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        DEBUG::ERROR << "Shader " << shaderStr << " compilation failed: "
                     << infoLog << std::endl;
        return false;
    }
    // end: error check
    return true;
}

Shader::Shader(const char* vertexPath, const char* fragmentPath) : ID(0)
{
    // 1. retrieve the vertex/fragment source code from filePath
    std::string vertexCode;
    std::string fragmentCode;
    std::ifstream vShaderFile;
    std::ifstream fShaderFile;
    // ensure ifstream objects can throw exceptions:
    vShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
    try 
    {
        // open files
        vShaderFile.open(vertexPath);
        fShaderFile.open(fragmentPath);
        std::stringstream vShaderStream, fShaderStream;
        // read file's buffer contents into streams
        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();		
        // close file handlers
        vShaderFile.close();
        fShaderFile.close();
        // convert stream into string
        vertexCode   = vShaderStream.str();
        fragmentCode = fShaderStream.str();		
    }
    catch(const std::ifstream::failure&)
    {
        throw std::runtime_error("Shader files could not be read");
    }
    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();

    // 2. compile shaders
    unsigned int vertex = glCreateShader(GL_VERTEX_SHADER);
    unsigned int fragment = 0;
    int success;
    char infoLog[512];
    // vertex Shader
    if (!compileShader(vertex, "VERTEX", vShaderCode))
    {
        glDeleteShader(vertex);
        throw std::runtime_error("Vertex shader compilation failed");
    }
    // similiar for Fragment Shader
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    if (!compileShader(fragment, "FRAGMENT", fShaderCode))
    {
        glDeleteShader(vertex);
        glDeleteShader(fragment);
        throw std::runtime_error("Fragment shader compilation failed");
    }
    // shader Program
    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    glLinkProgram(ID);
    // print linking errors if any
    glGetProgramiv(ID, GL_LINK_STATUS, &success);
    if(!success)
    {
        glGetProgramInfoLog(ID, 512, NULL, infoLog);
        glDeleteShader(vertex);
        glDeleteShader(fragment);
        glDeleteProgram(ID);
        ID = 0;
        throw std::runtime_error(std::string("Shader program linking failed: ") + infoLog);
    }
    else
        DEBUG::SUCCESS << "Shader program linked" << std::endl;
    // delete the shaders as they're linked into our program now and no longer necessary
    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

Shader::~Shader()
{
    if (ID != 0)
        glDeleteProgram(ID);
}

void Shader::use() 
{ 
    glUseProgram(ID);
} 

void Shader::setBool(const std::string &name, bool value) const
{         
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value); 
}
void Shader::setInt(const std::string &name, int value) const
{ 
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value); 
}
void Shader::setFloat(const std::string &name, float value) const
{ 
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value); 
} 

void Shader::setVec2(const std::string &name, const Vec2 &value) const
{ 
    glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]); 
}
void Shader::setVec2(const std::string &name, float x, float y) const
{ 
    glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y); 
}

void Shader::setVec3(const std::string &name, const Vec3 &value) const
{ 
    glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]); 
}
void Shader::setVec3(const std::string &name, float x, float y, float z) const
{ 
    glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z); 
}

void Shader::setVec4(const std::string &name, const Vec4 &value) const
{ 
    glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]); 
}
void Shader::setVec4(const std::string &name, float x, float y, float z, float w) const
{ 
    glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w); 
}

void Shader::setMat4(const std::string &name, const Mat4 &mat) const
{
    glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, mat.toColumnMajor());
}