#include "GLSLShader.h"


GLSLShader::GLSLShader():
vs_object(0),
fs_object(0)
{
} 



GLSLShader::~GLSLShader()
{
	glDeleteShader(vs_object);
	glDeleteShader(fs_object);
	glDeleteProgram(prog_object);
}

void GLSLShader::load(const string& name)
{
	vs_object = createShader(GL_VERTEX_SHADER, name+string(".vert"));
	fs_object = createShader(GL_FRAGMENT_SHADER, name+string(".frag"));
	prog_object = glCreateProgram();
	
	if(vs_object)glAttachShader(prog_object, vs_object);
	if(fs_object)glAttachShader(prog_object, fs_object);
	glLinkProgram(prog_object);
	
	printProgramLog(prog_object);
}

void GLSLShader::compileFromSource(const char* vertexShaderSource, const char* fragmentShaderSource)
{
	vs_object = createShaderFromSource(GL_VERTEX_SHADER, string(vertexShaderSource));
	fs_object = createShaderFromSource(GL_FRAGMENT_SHADER, string(fragmentShaderSource));
	prog_object = glCreateProgram();
	
	if(vs_object)glAttachShader(prog_object, vs_object);
	if(fs_object)glAttachShader(prog_object, fs_object);
	glLinkProgram(prog_object);
	
	printProgramLog(prog_object);
}

void GLSLShader::bindShader()const
{
	glUseProgram(prog_object);
}

void GLSLShader::unbindShader()const
{
  glUseProgram(0);
}

void GLSLShader::setIntParam(const char* pname, const float& value)
{
	// Get from openGL the location of the variable pname
	// (lazy way, should be stored but this works)
	GLfloat id = glGetUniformLocation(this->prog_object, pname);
	// Set the value
	glUniform1i(id, value);
}

void GLSLShader::setFloatParam(const char* pname, const float& value)
{
	// Get from openGL the location of the variable pname
	// (lazy way, should be stored but this works)
	GLfloat id = glGetUniformLocation(this->prog_object, pname);
	// Set the value
	glUniform1f(id, value);
}

void GLSLShader::setVector2Param(const char* pname, const glm::vec2& value)
{
	// Get from openGL the location of the variable pname
	// (lazy way, should be stored but this works)
	GLfloat id = glGetUniformLocation(this->prog_object, pname);
	// Set the value
	glUniform2fv(id, 1, &value[0]);
}

void GLSLShader::setVector3Param(const char* pname, const glm::vec3& value)
{
	// Get from openGL the location of the variable pname
	// (lazy way, should be stored but this works)
	GLfloat id = glGetUniformLocation(this->prog_object, pname);
	// Set the value
	glUniform3fv(id, 1, &value[0]);
}

void GLSLShader::setVector4Param(const char* pname, const glm::vec4& value)
{
	// Get from openGL the location of the variable pname
	// (lazy way, should be stored but this works)
	GLfloat id = glGetUniformLocation(this->prog_object, pname);
	// Set the value
	glUniform4fv(id, 1, &value[0]);
}

void GLSLShader::setMatrix3Param(const char* pname, const glm::mat3& value)
{
	// Get from openGL the location of the variable pname
	// (lazy way, should be stored but this works)
	GLfloat id = glGetUniformLocation(this->prog_object, pname);
	// Set the value
	glUniformMatrix3fv(id, 1, false, &value[0][0]);
}

void GLSLShader::setMatrix4Param(const char* pname, const glm::mat4& value)
{
	// Get from openGL the location of the variable pname
	// (lazy way, should be stored but this works)
	GLfloat id = glGetUniformLocation(this->prog_object, pname);
	// Set the value
	glUniformMatrix4fv(id, 1, false, &value[0][0]);
}

void printProgramLog(GLuint program)
{
	GLint infoLog_length = 0;
	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLog_length);

	// Some drivers return 1 as infoLog_length when the infoLog is an empty string
	if(infoLog_length > 1) 
	{
		char* infoLog = new char[infoLog_length];
		glGetProgramInfoLog(program, infoLog_length, 0, infoLog);
		cerr << "InfoLog:" << endl << infoLog << endl << endl;
		delete [] infoLog;
	}
}

void printShaderLog(GLuint shader)
{
	GLint infoLog_length = 0;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLog_length);

	// Some drivers return 1 as infoLog_length when the infoLog is an empty string
	if(infoLog_length > 1)
	{
		char* infoLog = new char[infoLog_length];
		glGetShaderInfoLog(shader, infoLog_length, 0, infoLog);
		cerr << "InfoLog:" << endl << infoLog << endl << endl;
		delete [] infoLog;
	}
}

bool check(GLuint shader)
{
	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	return status == GL_TRUE;
}

const std::string readShaderSource(const std::string& file)
{
	ifstream is(file.c_str(), ios::binary);
	assert(is.is_open());
	
	string str, contents;
	
	while(getline(is,str)) 
	{
		contents += str;
		contents += "\n";
	}
	return contents;
}

GLuint createShaderFromSource(GLuint stype, const std::string& src)
{
	GLuint S = glCreateShader(stype);
	const GLchar* src_cstr = src.c_str();

	glShaderSource(S, 1, &src_cstr, 0);
	glCompileShader(S);

	if(!check(S))
	{
		printShaderLog(S);
		glDeleteShader(S);
		return 0;
	}
	return S;
}

GLuint createShader(GLuint stype, const std::string& file)
{
	string str = readShaderSource(file);
	assert(!str.empty());

	if(!str.empty())
	{
		GLuint shader = createShaderFromSource(stype, str);
		if(!check(shader))
		{
			cerr << file << endl;
			printShaderLog(shader);
			glDeleteShader(shader);
			return 0;
		}
		return shader;
	}

	return 0;
}

