#include "danbo.h"
#include <GL\glut.h>
#include "tree.h"

GLfloat xRotated, yRotated, zRotated;
GLdouble radius=4;
GLfloat qaBlack[] = {0.0, 0.0, 0.0, 1.0}; //Black Color
GLfloat qaGreen[] = {1.0, 0.0, 0.0, 1.0}; //Green Color
GLfloat qaWhite[] = {1.0, 1.0, 1.0, 1.0}; //White Color
GLfloat qaRed[] = {1.0, 0.0, 0.0, 1.0}; //Red Color

    // Set lighting intensity and color
GLfloat qaAmbientLight[]    = {0.1, 0.1, 0.1, 1.0};
GLfloat qaDiffuseLight[]    = {1, 1, 1, 1.0};
GLfloat qaSpecularLight[]    = {1.0, 1.0, 1.0, 1.0};
GLfloat emitLight[] = {0.9, 0.9, 0.9, 0.01};
GLfloat Noemit[] = {0.0, 0.0, 0.0, 1.0};
    // Light source position
GLfloat qaLightPosition[]    = {0, 0, 2, 1};
GLfloat qaLightDirection[]    = {1, 1, 1, 0};
GLfloat dirVector0[]={ 1.0, 0.0, 0.0, 0.0}; //- See more at: http://www.codemiles.com/c-opengl-examples/add-spot-light-to-object-t9154.html#sthash.ObJtMYhU.dpuf
// ----------------------------------------------------------------------------------------------------------------------------












void DisplayError(char *ErrorText)
{
	MessageBox(NULL, ErrorText, "Error", MB_OK | MB_ICONERROR);
}

void DisplayInfo(char *InfoText)
{
	MessageBox(NULL, InfoText, "Info", MB_OK | MB_ICONINFORMATION);
}

bool DisplayQuestion(char *QuestionText)
{
	return MessageBox(NULL, QuestionText, "Question", MB_YESNO | MB_ICONQUESTION) == IDYES;
}

// ----------------------------------------------------------------------------------------------------------------------------

CString ModuleDirectory, ErrorLog;

bool wgl_context_forward_compatible = false;
int gl_version = 0, gl_max_texture_size = 0, gl_max_texture_max_anisotropy_ext = 0;

// ----------------------------------------------------------------------------------------------------------------------------

CTexture::CTexture()
{
	TextureID = 0;
}

CTexture::~CTexture()
{
}

CTexture::operator GLuint ()
{
	return TextureID;
}

void CTexture::Delete()
{
	glDeleteTextures(1, &TextureID);
	TextureID = 0;
}

bool CTexture::LoadTexture2D(char *Texture2DFileName)
{
	CString FileName = ModuleDirectory + Texture2DFileName;
	CString ErrorText = "Error loading file " + FileName + "! ->";

	FREE_IMAGE_FORMAT fif = FreeImage_GetFileType(FileName);

	if(fif == FIF_UNKNOWN)
	{
		fif = FreeImage_GetFIFFromFilename(FileName);
	}
	
	if(fif == FIF_UNKNOWN)
	{
		ErrorLog.Append(ErrorText + "fif is FIF_UNKNOWN" + "\r\n");
		return false;
	}

	FIBITMAP *dib = NULL;

	if(FreeImage_FIFSupportsReading(fif))
	{
		dib = FreeImage_Load(fif, FileName);
	}
	
	if(dib == NULL)
	{
		ErrorLog.Append(ErrorText + "dib is NULL" + "\r\n");
		return false;
	}

	int Width = FreeImage_GetWidth(dib), oWidth = Width;
	int Height = FreeImage_GetHeight(dib), oHeight = Height;
	int Pitch = FreeImage_GetPitch(dib);
	int BPP = FreeImage_GetBPP(dib);

	if(Width == 0 || Height == 0)
	{
		ErrorLog.Append(ErrorText + "Width or Height is 0" + "\r\n");
		return false;
	}

	if(Width > gl_max_texture_size) Width = gl_max_texture_size;
	if(Height > gl_max_texture_size) Height = gl_max_texture_size;

	if(!GLEW_ARB_texture_non_power_of_two)
	{
		Width = 1 << (int)floor((log((float)Width) / log(2.0f)) + 0.5f); 
		Height = 1 << (int)floor((log((float)Height) / log(2.0f)) + 0.5f);
	}

	if(Width != oWidth || Height != oHeight)
	{
		FIBITMAP *rdib = FreeImage_Rescale(dib, Width, Height, FILTER_BICUBIC);

		FreeImage_Unload(dib);

		if((dib = rdib) == NULL)
		{
			ErrorLog.Append(ErrorText + "rdib is NULL" + "\r\n");
			return false;
		}

		Pitch = FreeImage_GetPitch(dib);
	}

	BYTE *Data = FreeImage_GetBits(dib);

	if(Data == NULL)
	{
		ErrorLog.Append(ErrorText + "Data is NULL" + "\r\n");
		return false;
	}

	GLenum Format = 0;

	if(BPP == 32) Format = GL_BGRA;
	if(BPP == 24) Format = GL_BGR;

	if(Format == 0)
	{
		FreeImage_Unload(dib);
		ErrorLog.Append(ErrorText + "Format is 0" + "\r\n");
		return false;
	}

	if(gl_version < 12)
	{
		if(Format == GL_BGRA) Format = GL_RGBA;
		if(Format == GL_BGR) Format = GL_RGB;

		int bpp = BPP / 8;

		BYTE *line = Data;

		for(int y = 0; y < Height; y++)
		{
			BYTE *pixel = line;

			for(int x = 0; x < Width; x++)
			{
				BYTE Temp = pixel[0];
				pixel[0] = pixel[2];
				pixel[2] = Temp;

				pixel += bpp;
			}

			line += Pitch;
		}
	}

	glGenTextures(1, &TextureID);

	glBindTexture(GL_TEXTURE_2D, TextureID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_version >= 14 ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	if(GLEW_EXT_texture_filter_anisotropic)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, gl_max_texture_max_anisotropy_ext);
	}
	
	if(gl_version >= 14 && gl_version <= 21)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
	}
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, Width, Height, 0, Format, GL_UNSIGNED_BYTE, Data);
	
	if(gl_version >= 30)
	{
		glGenerateMipmap(GL_TEXTURE_2D);
	}

	glBindTexture(GL_TEXTURE_2D, 0);

	FreeImage_Unload(dib);

	return true;
}

// ----------------------------------------------------------------------------------------------------------------------------

CShaderProgram::CShaderProgram()
{
	SetDefaults();
}

CShaderProgram::~CShaderProgram()
{
}

CShaderProgram::operator GLuint ()
{
	return Program;
}

void CShaderProgram::Delete()
{
	delete [] UniformLocations;

	glDetachShader(Program, VertexShader);
	glDetachShader(Program, FragmentShader);

	glDeleteShader(VertexShader);
	glDeleteShader(FragmentShader);

	glDeleteProgram(Program);

	SetDefaults();
}

bool CShaderProgram::Load(char *VertexShaderFileName, char *FragmentShaderFileName)
{
	if(UniformLocations || VertexShader || FragmentShader || Program)
	{
		Delete();
	}

	bool Error = false;

	Error |= ((VertexShader = LoadShader(GL_VERTEX_SHADER, VertexShaderFileName)) == 0);

	Error |= ((FragmentShader = LoadShader(GL_FRAGMENT_SHADER, FragmentShaderFileName)) == 0);

	if(Error)
	{
		Delete();
		return false;
	}

	Program = glCreateProgram();
	glAttachShader(Program, VertexShader);
	glAttachShader(Program, FragmentShader);
	glLinkProgram(Program);

	int Param = 0;
	glGetProgramiv(Program, GL_LINK_STATUS, &Param);

	if(Param == GL_FALSE)
	{
		ErrorLog.Append("Error linking program (%s, %s)!\r\n", VertexShaderFileName, FragmentShaderFileName);

		int InfoLogLength = 0;
		glGetProgramiv(Program, GL_INFO_LOG_LENGTH, &InfoLogLength);
	
		if(InfoLogLength > 0)
		{
			char *InfoLog = new char[InfoLogLength];
			int CharsWritten  = 0;
			glGetProgramInfoLog(Program, InfoLogLength, &CharsWritten, InfoLog);
			ErrorLog.Append(InfoLog);
			delete [] InfoLog;
		}

		Delete();

		return false;
	}

	return true;
}

GLuint CShaderProgram::LoadShader(GLenum Type, char *ShaderFileName)
{
	CString FileName = ModuleDirectory + ShaderFileName;

	FILE *File;

	if((File = fopen(FileName, "rb")) == NULL)
	{
		ErrorLog.Append("Error loading file " + FileName + "!\r\n");
		return 0;
	}

	fseek(File, 0, SEEK_END);
	long Size = ftell(File);
	fseek(File, 0, SEEK_SET);
	char *Source = new char[Size + 1];
	fread(Source, 1, Size, File);
	fclose(File);
	Source[Size] = 0;

	GLuint Shader;

	Shader = glCreateShader(Type);
	glShaderSource(Shader, 1, (const char**)&Source, NULL);
	delete [] Source;
	glCompileShader(Shader);

	int Param = 0;
	glGetShaderiv(Shader, GL_COMPILE_STATUS, &Param);

	if(Param == GL_FALSE)
	{
		ErrorLog.Append("Error compiling shader %s!\r\n", ShaderFileName);

		int InfoLogLength = 0;
		glGetShaderiv(Shader, GL_INFO_LOG_LENGTH, &InfoLogLength);
	
		if(InfoLogLength > 0)
		{
			char *InfoLog = new char[InfoLogLength];
			int CharsWritten  = 0;
			glGetShaderInfoLog(Shader, InfoLogLength, &CharsWritten, InfoLog);
			ErrorLog.Append(InfoLog);
			delete [] InfoLog;
		}

		glDeleteShader(Shader);

		return 0;
	}

	return Shader;
}

void CShaderProgram::SetDefaults()
{
	UniformLocations = NULL;
	VertexShader = 0;
	FragmentShader = 0;
	Program = 0;
}

// ----------------------------------------------------------------------------------------------------------------------------

CCamera::CCamera()
{
	View = NULL;

	Reference = vec3(0.0f, 0.0f, 0.0f);
	Position = vec3(0.0f, 0.0f, 5.0f);

	X = vec3(1.0f, 0.0f, 0.0f);
	Y = vec3(0.0f, 1.0f, 0.0f);
	Z = vec3(0.0f, 0.0f, 1.0f);
}

CCamera::~CCamera()
{
}

void CCamera::CalculateViewMatrix()
{
	if(View)
	{
		*View = mat4x4(vec4(X.x, Y.x, Z.x, 0.0f), vec4(X.y, Y.y, Z.y, 0.0f), vec4(X.z, Y.z, Z.z, 0.0f), vec4(-dot(X, Position), -dot(Y, Position), -dot(Z, Position), 1.0f));
	}
}

void CCamera::LookAt(vec3 Reference, vec3 Position, bool RotateAroundReference)
{
	this->Reference = Reference;
	this->Position = Position;

	Z = normalize(Position - Reference);
	X = normalize(cross(vec3(0.0f, 1.0f, 0.0f), Z));
	Y = cross(Z, X);

	if(!RotateAroundReference)
	{
		this->Reference = this->Position;
		this->Position += Z * 0.05f;
	}

	CalculateViewMatrix();
}

void CCamera::Move(vec3 Movement)
{
	Reference += Movement;
	Position += Movement;

	CalculateViewMatrix();
}

vec3 CCamera::OnKeys(BYTE Keys, float FrameTime)
{
	float Speed = 5.0f;

	if(Keys & 0x40) // SHIFT
	{
		Speed *= 2.0f;
	}

	float Distance = Speed * FrameTime;

	vec3 Up(0.0f, 1.0f, 0.0f);
	vec3 Right = X;
	vec3 Forward = cross(Up, Right);

	Up *= Distance;
	Right *= Distance;
	Forward *= Distance;

	vec3 Movement;

	if(Keys & 0x01) // W
	{
		Movement += Forward;
	}

	if(Keys & 0x02) // S
	{
		Movement -= Forward;
	}

	if(Keys & 0x04) // A
	{
		Movement -= Right;
	}

	if(Keys & 0x08) // D
	{
		Movement += Right;
	}

	if(Keys & 0x10) // R
	{
		Movement += Up;
	}

	if(Keys & 0x20) // F
	{
		Movement -= Up;
	}

	return Movement;
}

void CCamera::OnMouseMove(int dx, int dy)
{
	float sensitivity = 0.25f;

	float hangle = (float)dx * sensitivity;
	float vangle = (float)dy * sensitivity;

	Position -= Reference;

	Y = rotate(Y, vangle, X);
	Z = rotate(Z, vangle, X);

	if(Y.y < 0.0f)
	{
		Z = vec3(0.0f, Z.y > 0.0f ? 1.0f : -1.0f, 0.0f);
		Y = cross(Z, X);
	}

	X = rotate(X, hangle, vec3(0.0f, 1.0f, 0.0f));
	Y = rotate(Y, hangle, vec3(0.0f, 1.0f, 0.0f));
	Z = rotate(Z, hangle, vec3(0.0f, 1.0f, 0.0f));

	Position = Reference + Z * length(Position);

	CalculateViewMatrix();
}

void CCamera::OnMouseWheel(short zDelta)
{
	Position -= Reference;

	if(zDelta < 0 && length(Position) < 500.0f)
	{
		Position += Position * 0.1f;
	}

	if(zDelta > 0 && length(Position) > 0.05f)
	{
		Position -= Position * 0.1f;
	}

	Position += Reference;

	CalculateViewMatrix();
}

void CCamera::SetViewMatrixPointer(mat4x4 *View)
{
	this->View = View;

	CalculateViewMatrix();
}

CCamera Camera;

// ----------------------------------------------------------------------------------------------------------------------------

COpenGLRenderer::COpenGLRenderer()
{
	//ShowAxisGrid = true;
    ShowAxisGrid = false;
	//Stop = false;
    Stop = true;
    
	Camera.SetViewMatrixPointer(&View);
}

COpenGLRenderer::~COpenGLRenderer()
{
}

bool COpenGLRenderer::Init()
{
	/*if(gl_version < 21)
	{
		ErrorLog.Set("OpenGL 2.1 not supported!");
		return false;
	}*/

/*    // Enable lighting
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

     // Set lighting intensity and color
       glLightfv(GL_LIGHT0, GL_AMBIENT, qaAmbientLight);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, qaDiffuseLight);
    glLightfv(GL_LIGHT0, GL_POSITION, qaLightPosition);
    glLightfv(GL_LIGHT0, GL_SPECULAR, qaSpecularLight);
    ////////////////////////////////////////////////


     glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, 30.0);// set cutoff angle
     glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, dirVector0); 
     glLightf(GL_LIGHT0, GL_SPOT_EXPONENT, 1); // set focusing strength - See more at: http://www.codemiles.com/c-opengl-examples/add-spot-light-to-object-t9154.html#sthash.ObJtMYhU.dpuf
*/

	bool Error = false;

	Error |= !Texture[1].LoadTexture2D("danbo1.png");
	Error |= !Texture[2].LoadTexture2D("danbo.png");
	Error |= !Texture[3].LoadTexture2D("csv2087t.jpg");
	Error |= !Texture[4].LoadTexture2D("batu1.jpg");
	Error |= !Texture[5].LoadTexture2D("rumput.jpg");
	Error |= !Texture[6].LoadTexture2D("kayu5.jpg");
	Error |= !Texture[7].LoadTexture2D("bata1.jpg");
	Error |= !Texture[8].LoadTexture2D("kayu4.jpg");  
	Error |= !Texture[9].LoadTexture2D("bata1.jpg"); 
	Error |= !Texture[10].LoadTexture2D("pintu1.jpg");   
    Error |= !Texture[11].LoadTexture2D("kayu4.jpg"); 
   	Error |= !Texture[12].LoadTexture2D("kayu4a.jpg"); 
   	Error |= !Texture[13].LoadTexture2D("jendela.jpg"); 
   	
   	Error |= !Texture[14].LoadTexture2D("kd1.jpg"); 
   	Error |= !Texture[15].LoadTexture2D("kd2.jpg");     
    Error |= !Texture[16].LoadTexture2D("kd3.jpg");     
    Error |= !Texture[17].LoadTexture2D("lubang.jpg");   
    
	Error |= !Texture[18].LoadTexture2D("cha.png");  
    
    Error |= !Texture[19].LoadTexture2D("jalan.jpg");  
     
    Error |= !Texture[20].LoadTexture2D("atap2.jpg"); 
     
    Error |= !Texture[21].LoadTexture2D("ball.jpg"); 
	if(gl_version >= 21)
	{
		Error |= !Shader.Load("glsl120shader.vs", "glsl120shader.fs");
	}

	if(Error)
	{
		return false;
	}


	GLfloat LightModelAmbient[] = {0.0f, 0.0f, 0.0f, 1.0f};
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, LightModelAmbient);

	GLfloat LightAmbient[] = {0.5f, 0.5f, 0.5f, 1.0f};
	glLightfv(GL_LIGHT0, GL_AMBIENT, LightAmbient);

	GLfloat LightDiffuse[] = {0.75f, 0.75f, 0.75f, 1.0f};
	glLightfv(GL_LIGHT0, GL_DIFFUSE, LightDiffuse);

	GLfloat MaterialAmbient[] = {1.0f, 1.0f, 1.0f, 1.0f};
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, MaterialAmbient);

	GLfloat MaterialDiffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, MaterialDiffuse);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHTING);
	glEnable(GL_COLOR_MATERIAL);
glClearColor(0.621f, 0.71f, 0.80f, 1.0f); //soft light blue
	//glClearColor(0.999f, 0.644f, 0.0f, 1.0f); //sunset
/*
       glLightfv(GL_LIGHT0, GL_AMBIENT, qaAmbientLight);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, qaDiffuseLight);
    glLightfv(GL_LIGHT0, GL_POSITION, qaLightPosition);
    glLightfv(GL_LIGHT0, GL_SPECULAR, qaSpecularLight);
    ////////////////////////////////////////////////


     glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, 30.0);// set cutoff angle
     glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, dirVector0); 
     glLightf(GL_LIGHT0, GL_SPOT_EXPONENT, 1);
*/




	TexCoords = new vec2[24];
	Normals = new vec3[24];

    int i;
    
//==DEFAULT
//depan
	TexCoords[0] = vec2(0.0f, 0.0f); Normals[0] = vec3( 0.0f, 0.0f, 1.0f); 
	TexCoords[1] = vec2(1.0f, 0.0f); Normals[1] = vec3( 0.0f, 0.0f, 1.0f); 
	TexCoords[2] = vec2(1.0f, 1.0f); Normals[2] = vec3( 0.0f, 0.0f, 1.0f); 
	TexCoords[3] = vec2(0.0f, 1.0f); Normals[3] = vec3( 0.0f, 0.0f, 1.0f); 
//belakang
	TexCoords[4] = vec2(0.0f, 0.0f); Normals[4] = vec3( 0.0f, 0.0f, -1.0f); 
	TexCoords[5] = vec2(1.0f, 0.0f); Normals[5] = vec3( 0.0f, 0.0f, -1.0f); 
	TexCoords[6] = vec2(1.0f, 1.0f); Normals[6] = vec3( 0.0f, 0.0f, -1.0f); 
	TexCoords[7] = vec2(0.0f, 1.0f); Normals[7] = vec3( 0.0f, 0.0f, -1.0f); 

//kanan
	TexCoords[8] = vec2(0.0f, 0.0f); Normals[8] = vec3(1.0f, 0.0f, 0.0f); 
	TexCoords[9] = vec2(1.0f, 0.0f); Normals[9] = vec3(1.0f, 0.0f, 0.0f); 
	TexCoords[10] = vec2(1.0f, 1.0f); Normals[10] = vec3(1.0f, 0.0f, 0.0f); 
	TexCoords[11] = vec2(0.0f, 1.0f); Normals[11] = vec3(1.0f, 0.0f, 0.0f); 
//kiri
	TexCoords[12] = vec2(0.0f, 0.0f); Normals[12] = vec3(-1.0f,  0.0f,  0.0f); 
	TexCoords[13] = vec2(1.0f, 0.0f); Normals[13] = vec3(-1.0f,  0.0f,  0.0f); 
	TexCoords[14] = vec2(1.0f, 1.0f); Normals[14] = vec3(-1.0f,  0.0f,  0.0f); 
	TexCoords[15] = vec2(0.0f, 1.0f); Normals[15] = vec3(-1.0f,  0.0f,  0.0f); 

//atas
	TexCoords[16] = vec2(0.0f, 0.0f); Normals[16] = vec3( 0.0f,  1.0f,  0.0f); 
	TexCoords[17] = vec2(1.0f, 0.0f); Normals[17] = vec3( 0.0f,  1.0f,  0.0f); 
	TexCoords[18] = vec2(1.0f, 1.0f); Normals[18] = vec3( 0.0f,  1.0f,  0.0f); 
	TexCoords[19] = vec2(0.0f, 1.0f); Normals[19] = vec3( 0.0f,  1.0f,  0.0f); 
//bawah
	TexCoords[20] = vec2(0.0f, 0.0f); Normals[20] = vec3( 0.0f,  -1.0f,  0.0f);
	TexCoords[21] = vec2(1.0f, 0.0f); Normals[21] = vec3( 0.0f,  -1.0f,  0.0f);
	TexCoords[22] = vec2(1.0f, 1.0f); Normals[22] = vec3( 0.0f,  -1.0f,  0.0f);
	TexCoords[23] = vec2(0.0f, 1.0f); Normals[23] = vec3( 0.0f,  -1.0f,  0.0f);

//=========


	Vertices = new vec3[24];
//==DANBO1====================================================================================
//depan
	Vertices[0] = vec3(-0.0f, -0.0f,  1.4f);
	Vertices[1] = vec3( 1.4f, -0.0f,  1.4f);
	Vertices[2] = vec3( 1.4f,  2.8f,  1.4f);
	Vertices[3] = vec3(-0.0f,  2.8f,  1.4f);
//belakang
	Vertices[4] = vec3( 1.4f, -0.0f, -0.0f);
	Vertices[5] = vec3(-0.0f, -0.0f, -0.0f);
	Vertices[6] = vec3(-0.0f,  2.8f, -0.0f);
	Vertices[7] = vec3( 1.4f,  2.8f, -0.0f);

//kanan
	Vertices[8] = vec3( 1.4f, -0.0f,  1.4f);
	Vertices[9] = vec3( 1.4f, -0.0f, -0.0f);
	Vertices[10] = vec3( 1.4f,  2.8f, -0.0f);
	Vertices[11] = vec3( 1.4f,  2.8f,  1.4f);
//kiri
	Vertices[12] = vec3(-0.0f, -0.0f, -0.0f);
	Vertices[13] = vec3(-0.0f, -0.0f,  1.4f);
	Vertices[14] = vec3(-0.0f,  2.8f,  1.4f);
	Vertices[15] = vec3(-0.0f,  2.8f, -0.0f);

//atas
	Vertices[16] = vec3(-0.0f,  2.8f,  1.4f);
	Vertices[17] = vec3( 1.4f,  2.8f,  1.4f);
	Vertices[18] = vec3( 1.4f,  2.8f, -0.0f);
	Vertices[19] = vec3(-0.0f,  2.8f, -0.0f);
//bawah
	Vertices[20] = vec3(-0.0f, -0.0f, -0.0f);
	Vertices[21] = vec3( 1.4f, -0.0f, -0.0f);
	Vertices[22] = vec3( 1.4f, -0.0f,  1.4f);
	Vertices[23] = vec3(-0.0f, -0.0f,  1.4f);

//--KAKI 2
	Vertices1 = new vec3[24];

//depan
	Vertices1[0] = vec3( 1.6f, -0.0f,  1.4f);
	Vertices1[1] = vec3( 3.0f, -0.0f,  1.4f);
	Vertices1[2] = vec3( 3.0f,  2.8f,  1.4f);
	Vertices1[3] = vec3( 1.6f,  2.8f,  1.4f);
//belakang
	Vertices1[4] = vec3( 3.0f, -0.0f, -0.0f);
	Vertices1[5] = vec3( 1.6f, -0.0f, -0.0f);
	Vertices1[6] = vec3( 1.6f,  2.8f, -0.0f);
	Vertices1[7] = vec3( 3.0f,  2.8f, -0.0f);

//kanan
	Vertices1[8] = vec3( 3.0f, -0.0f,  1.4f);
	Vertices1[9] = vec3( 3.0f, -0.0f, -0.0f);
	Vertices1[10] = vec3( 3.0f,  2.8f, -0.0f);
	Vertices1[11] = vec3( 3.0f,  2.8f,  1.4f);
//kiri
	Vertices1[12] = vec3( 1.6f, -0.0f, -0.0f);
	Vertices1[13] = vec3( 1.6f, -0.0f,  1.4f);
	Vertices1[14] = vec3( 1.6f,  2.8f,  1.4f);
	Vertices1[15] = vec3( 1.6f,  2.8f, -0.0f);

//atas
	Vertices1[16] = vec3( 1.6f,  2.8f,  1.4f);
	Vertices1[17] = vec3( 3.0f,  2.8f,  1.4f);
	Vertices1[18] = vec3( 3.0f,  2.8f, -0.0f);
	Vertices1[19] = vec3( 1.6f,  2.8f, -0.0f);
//bawah
	Vertices1[20] = vec3( 1.6f, -0.0f, -0.0f);
	Vertices1[21] = vec3( 3.0f, -0.0f, -0.0f);
	Vertices1[22] = vec3( 3.0f, -0.0f,  1.4f);
	Vertices1[23] = vec3( 1.6f, -0.0f,  1.4f);



//--Badan
	Vertices2 = new vec3[24];

//depan
	Vertices2[0] = vec3(-0.2f,  2.8f,  1.6f);
	Vertices2[1] = vec3( 3.2f,  2.8f,  1.6f);
	Vertices2[2] = vec3( 3.2f,  6.8f,  1.6f);
	Vertices2[3] = vec3(-0.2f,  6.8f,  1.6f);
//belakang
	Vertices2[4] = vec3( 3.2f,  2.8f, -0.2f);
	Vertices2[5] = vec3(-0.2f,  2.8f, -0.2f);
	Vertices2[6] = vec3(-0.2f,  6.8f, -0.2f);
	Vertices2[7] = vec3( 3.2f,  6.8f, -0.2f);

//kanan
	Vertices2[8] = vec3( 3.2f,  2.8f,  1.6f);
	Vertices2[9] = vec3( 3.2f,  2.8f, -0.2f);
	Vertices2[10] = vec3( 3.2f,  6.8f, -0.2f);
	Vertices2[11] = vec3( 3.2f,  6.8f,  1.6f);
//kiri
	Vertices2[12] = vec3(-0.2f,  2.8f, -0.2f);
	Vertices2[13] = vec3(-0.2f,  2.8f,  1.6f);
	Vertices2[14] = vec3(-0.2f,  6.8f,  1.6f);
	Vertices2[15] = vec3(-0.2f,  6.8f, -0.2f);

//atas
	Vertices2[16] = vec3(-0.2f,  6.8f,  1.6f);
	Vertices2[17] = vec3( 3.2f,  6.8f,  1.6f);
	Vertices2[18] = vec3( 3.2f,  6.8f, -0.2f);
	Vertices2[19] = vec3(-0.2f,  6.8f, -0.2f);
//bawah
	Vertices2[20] = vec3(-0.2f,  2.8f, -0.2f);
	Vertices2[21] = vec3( 3.2f,  2.8f, -0.2f);
	Vertices2[22] = vec3( 3.2f,  2.8f,  1.6f);
	Vertices2[23] = vec3(-0.2f,  2.8f,  1.6f);




//--KEPALA1
	Vertices3 = new vec3[24];
/*
//depan
	Vertices3[0] = vec3(-1.3f,  6.8f,  2.1f);
	Vertices3[1] = vec3( 4.3f,  6.8f,  2.1f);
	Vertices3[2] = vec3( 4.3f,  10.2f,  2.1f);
	Vertices3[3] = vec3(-1.3f,  10.2f,  2.1f);
	*/
//belakang
	Vertices3[4] = vec3( 4.3f,  6.8f, -0.7f);
	Vertices3[5] = vec3(-1.3f,  6.8f, -0.7f);
	Vertices3[6] = vec3(-1.3f,  10.2f, -0.7f);
	Vertices3[7] = vec3( 4.3f,  10.2f, -0.7f);

//kanan
	Vertices3[8] = vec3( 4.3f,  6.8f,  2.1f);
	Vertices3[9] = vec3( 4.3f,  6.8f, -0.7f);
	Vertices3[10] = vec3( 4.3f,  10.2f, -0.7f);
	Vertices3[11] = vec3( 4.3f,  10.2f,  2.1f);
//kiri
	Vertices3[12] = vec3(-1.3f,  6.8f, -0.7f);
	Vertices3[13] = vec3(-1.3f,  6.8f,  2.1f);
	Vertices3[14] = vec3(-1.3f,  10.2f,  2.1f);
	Vertices3[15] = vec3(-1.3f,  10.2f, -0.7f);

//atas
	Vertices3[16] = vec3(-1.3f,  10.2f,  2.1f);
	Vertices3[17] = vec3( 4.3f,  10.2f,  2.1f);
	Vertices3[18] = vec3( 4.3f,  10.2f, -0.7f);
	Vertices3[19] = vec3(-1.3f,  10.2f, -0.7f);
//bawah
	Vertices3[20] = vec3(-1.3f,  6.8f, -0.7f);
	Vertices3[21] = vec3( 4.3f,  6.8f, -0.7f);
	Vertices3[22] = vec3( 4.3f,  6.8f,  2.1f);
	Vertices3[23] = vec3(-1.3f,  6.8f,  2.1f);



//--KEPALA2
	Vertices4 = new vec3[24];

//depan
	Vertices4[0] = vec3(-1.3f,  6.8f,  2.1f);
	Vertices4[1] = vec3( 4.3f,  6.8f,  2.1f);
	Vertices4[2] = vec3( 4.3f,  10.2f,  2.1f);
	Vertices4[3] = vec3(-1.3f,  10.2f,  2.1f);




//--TANGAN KIRI
	Vertices5 = new vec3[24];

//depan
	Vertices5[0] = vec3(-1.0f,  2.3f,  1.3f);
	Vertices5[1] = vec3(-0.2f,  2.3f,  1.3f);
	Vertices5[2] = vec3( -0.2f,  6.3f,  1.3f);
	Vertices5[3] = vec3(-1.0f,  6.3f,  1.3f);
//belakang
	Vertices5[4] = vec3( -0.2f,  2.3f, 0.1f);
	Vertices5[5] = vec3(-1.0f,  2.3f, 0.1f);
	Vertices5[6] = vec3(-1.0f,  6.3f, 0.1f);
	Vertices5[7] = vec3( -0.2f,  6.3f, 0.1f);

//kanan
	Vertices5[8] = vec3( -0.2f,  2.3f,  1.3f);
	Vertices5[9] = vec3( -0.2f,  2.3f, 0.1f);
	Vertices5[10] = vec3( -0.2f,  6.3f, 0.1f);
	Vertices5[11] = vec3( -0.2f,  6.3f,  1.3f);
//kiri
	Vertices5[12] = vec3(-1.0f,  2.3f, 0.1f);
	Vertices5[13] = vec3(-1.0f,  2.3f,  1.3f);
	Vertices5[14] = vec3(-1.0f,  6.3f,  1.3f);
	Vertices5[15] = vec3(-1.0f,  6.3f, 0.1f);

//atas
	Vertices5[16] = vec3(-1.0f,  6.3f,  1.3f);
	Vertices5[17] = vec3( -0.2f,  6.3f,  1.3f);
	Vertices5[18] = vec3( -0.2f,  6.3f, 0.1f);
	Vertices5[19] = vec3(-1.0f,  6.3f,  0.1f);
//bawah
	Vertices5[20] = vec3(-1.0f,  2.3f, 0.1f);
	Vertices5[21] = vec3( -0.2f,  2.3f,0.1f);
	Vertices5[22] = vec3( -0.2f,  2.3f,  1.3f);
	Vertices5[23] = vec3(-1.0f,  2.3f,  1.3f);



//--TANGAN KANAN
	Vertices6 = new vec3[24];

//depan
	Vertices6[0] = vec3(3.2f,  2.3f,  1.3f);
	Vertices6[1] = vec3(4.0f,  2.3f,  1.3f);
	Vertices6[2] = vec3( 4.0f,  6.3f,  1.3f);
	Vertices6[3] = vec3(3.2f,  6.3f,  1.3f);
//belakang
	Vertices6[4] = vec3( 4.0f,  2.3f, 0.1f);
	Vertices6[5] = vec3(3.2f,  2.3f, 0.1f);
	Vertices6[6] = vec3(3.2f,  6.3f, 0.1f);
	Vertices6[7] = vec3(4.0f,  6.3f, 0.1f);

//kanan
	Vertices6[8] = vec3( 4.0f,  2.3f,  1.3f);
	Vertices6[9] = vec3( 4.0f,  2.3f, 0.1f);
	Vertices6[10] = vec3( 4.0f,  6.3f, 0.1f);
	Vertices6[11] = vec3( 4.0f,  6.3f,  1.3f);
//kiri
	Vertices6[12] = vec3(3.2f,  2.3f, 0.1f);
	Vertices6[13] = vec3(3.2f,  2.3f,  1.3f);
	Vertices6[14] = vec3(3.2f,  6.3f,  1.3f);
	Vertices6[15] = vec3(3.2f,  6.3f, 0.1f);

//atas
	Vertices6[16] = vec3(3.2f,  6.3f,  1.3f);
	Vertices6[17] = vec3(4.0f,  6.3f,  1.3f);
	Vertices6[18] = vec3(4.0f,  6.3f, 0.1f);
	Vertices6[19] = vec3(3.2f,  6.3f,  0.1f);
//bawah
	Vertices6[20] = vec3(3.2f,  2.3f, 0.1f);
	Vertices6[21] = vec3(4.0f,  2.3f,0.1f);
	Vertices6[22] = vec3(4.0f,  2.3f,  1.3f);
	Vertices6[23] = vec3(3.2f,  2.3f,  1.3f);
//=END DANBO1===========================================================================





//==DANBO2====================================================================================
	VerticesA = new vec3[24];
	//x=(int)(xxr+(xr[0]-xxr)*cos(sudut)-(yr[0]-yyr)*sin(sudut));
    //y=(int)(yyr+(xr[0]-xxr)*sin(sudut)+(yr[0]-yyr)*cos(sudut));
    
    
//depan
	VerticesA[0] = vec3(-0.0f+10.0f, -0.0f,  1.4f);
	VerticesA[1] = vec3( 1.4f+10.0f, -0.0f,  1.4f);
	VerticesA[2] = vec3( 1.4f+10.0f,  2.8f,  1.4f);
	VerticesA[3] = vec3(-0.0f+10.0f,  2.8f,  1.4f);
//belakang
	VerticesA[4] = vec3( 1.4f+10.0f, -0.0f, -0.0f);
	VerticesA[5] = vec3(-0.0f+10.0f, -0.0f, -0.0f);
	VerticesA[6] = vec3(-0.0f+10.0f,  2.8f, -0.0f);
	VerticesA[7] = vec3( 1.4f+10.0f,  2.8f, -0.0f);

//kanan
	VerticesA[8] = vec3( 1.4f+10.0f, -0.0f,  1.4f);
	VerticesA[9] = vec3( 1.4f+10.0f, -0.0f, -0.0f);
	VerticesA[10] = vec3( 1.4f+10.0f,  2.8f, -0.0f);
	VerticesA[11] = vec3( 1.4f+10.0f,  2.8f,  1.4f);
//kiri
	VerticesA[12] = vec3(-0.0f+10.0f, -0.0f, -0.0f);
	VerticesA[13] = vec3(-0.0f+10.0f, -0.0f,  1.4f);
	VerticesA[14] = vec3(-0.0f+10.0f,  2.8f,  1.4f);
	VerticesA[15] = vec3(-0.0f+10.0f,  2.8f, -0.0f);

//atas
	VerticesA[16] = vec3(-0.0f+10.0f,  2.8f,  1.4f);
	VerticesA[17] = vec3( 1.4f+10.0f,  2.8f,  1.4f);
	VerticesA[18] = vec3( 1.4f+10.0f,  2.8f, -0.0f);
	VerticesA[19] = vec3(-0.0f+10.0f,  2.8f, -0.0f);
//bawah
	VerticesA[20] = vec3(-0.0f+10.0f, -0.0f, -0.0f);
	VerticesA[21] = vec3( 1.4f+10.0f, -0.0f, -0.0f);
	VerticesA[22] = vec3( 1.4f+10.0f, -0.0f,  1.4f);
	VerticesA[23] = vec3(-0.0f+10.0f, -0.0f,  1.4f);

//--KAKI 2
	Vertices1A = new vec3[24];

//depan
	Vertices1A[0] = vec3( 1.6f+10.0f, -0.0f,  1.4f);
	Vertices1A[1] = vec3( 3.0f+10.0f, -0.0f,  1.4f);
	Vertices1A[2] = vec3( 3.0f+10.0f,  2.8f,  1.4f);
	Vertices1A[3] = vec3( 1.6f+10.0f,  2.8f,  1.4f);
//belakang
	Vertices1A[4] = vec3( 3.0f+10.0f, -0.0f, -0.0f);
	Vertices1A[5] = vec3( 1.6f+10.0f, -0.0f, -0.0f);
	Vertices1A[6] = vec3( 1.6f+10.0f,  2.8f, -0.0f);
	Vertices1A[7] = vec3( 3.0f+10.0f,  2.8f, -0.0f);

//kanan
	Vertices1A[8] = vec3( 3.0f+10.0f, -0.0f,  1.4f);
	Vertices1A[9] = vec3( 3.0f+10.0f, -0.0f, -0.0f);
	Vertices1A[10] = vec3( 3.0f+10.0f,  2.8f, -0.0f);
	Vertices1A[11] = vec3( 3.0f+10.0f,  2.8f,  1.4f);
//kiri
	Vertices1A[12] = vec3( 1.6f+10.0f, -0.0f, -0.0f);
	Vertices1A[13] = vec3( 1.6f+10.0f, -0.0f,  1.4f);
	Vertices1A[14] = vec3( 1.6f+10.0f,  2.8f,  1.4f);
	Vertices1A[15] = vec3( 1.6f+10.0f,  2.8f, -0.0f);

//atas
	Vertices1A[16] = vec3( 1.6f+10.0f,  2.8f,  1.4f);
	Vertices1A[17] = vec3( 3.0f+10.0f,  2.8f,  1.4f);
	Vertices1A[18] = vec3( 3.0f+10.0f,  2.8f, -0.0f);
	Vertices1A[19] = vec3( 1.6f+10.0f,  2.8f, -0.0f);
//bawah
	Vertices1A[20] = vec3( 1.6f+10.0f, -0.0f, -0.0f);
	Vertices1A[21] = vec3( 3.0f+10.0f, -0.0f, -0.0f);
	Vertices1A[22] = vec3( 3.0f+10.0f, -0.0f,  1.4f);
	Vertices1A[23] = vec3( 1.6f+10.0f, -0.0f,  1.4f);



//--Badan
	Vertices2A = new vec3[24];

//depan
	Vertices2A[0] = vec3(-0.2f+10.0f,  2.8f,  1.6f);
	Vertices2A[1] = vec3( 3.2f+10.0f,  2.8f,  1.6f);
	Vertices2A[2] = vec3( 3.2f+10.0f,  6.8f,  1.6f);
	Vertices2A[3] = vec3(-0.2f+10.0f,  6.8f,  1.6f);
//belakang
	Vertices2A[4] = vec3( 3.2f+10.0f,  2.8f, -0.2f);
	Vertices2A[5] = vec3(-0.2f+10.0f,  2.8f, -0.2f);
	Vertices2A[6] = vec3(-0.2f+10.0f,  6.8f, -0.2f);
	Vertices2A[7] = vec3( 3.2f+10.0f,  6.8f, -0.2f);

//kanan
	Vertices2A[8] = vec3( 3.2f+10.0f,  2.8f,  1.6f);
	Vertices2A[9] = vec3( 3.2f+10.0f,  2.8f, -0.2f);
	Vertices2A[10] = vec3( 3.2f+10.0f,  6.8f, -0.2f);
	Vertices2A[11] = vec3( 3.2f+10.0f,  6.8f,  1.6f);
//kiri
	Vertices2A[12] = vec3(-0.2f+10.0f,  2.8f, -0.2f);
	Vertices2A[13] = vec3(-0.2f+10.0f,  2.8f,  1.6f);
	Vertices2A[14] = vec3(-0.2f+10.0f,  6.8f,  1.6f);
	Vertices2A[15] = vec3(-0.2f+10.0f,  6.8f, -0.2f);

//atas
	Vertices2A[16] = vec3(-0.2f+10.0f,  6.8f,  1.6f);
	Vertices2A[17] = vec3( 3.2f+10.0f,  6.8f,  1.6f);
	Vertices2A[18] = vec3( 3.2f+10.0f,  6.8f, -0.2f);
	Vertices2A[19] = vec3(-0.2f+10.0f,  6.8f, -0.2f);
//bawah
	Vertices2A[20] = vec3(-0.2f+10.0f,  2.8f, -0.2f);
	Vertices2A[21] = vec3( 3.2f+10.0f,  2.8f, -0.2f);
	Vertices2A[22] = vec3( 3.2f+10.0f,  2.8f,  1.6f);
	Vertices2A[23] = vec3(-0.2f+10.0f,  2.8f,  1.6f);




//--KEPALA1
	Vertices3A = new vec3[24];
/*
//depan
	Vertices3[0] = vec3(-1.3f,  6.8f,  2.1f);
	Vertices3[1] = vec3( 4.3f,  6.8f,  2.1f);
	Vertices3[2] = vec3( 4.3f,  10.2f,  2.1f);
	Vertices3[3] = vec3(-1.3f,  10.2f,  2.1f);
	*/
//belakang
	Vertices3A[4] = vec3( 4.3f+10.0f,  6.8f, -0.7f);
	Vertices3A[5] = vec3(-1.3f+10.0f,  6.8f, -0.7f);
	Vertices3A[6] = vec3(-1.3f+10.0f,  10.2f, -0.7f);
	Vertices3A[7] = vec3( 4.3f+10.0f,  10.2f, -0.7f);

//kanan
	Vertices3A[8] = vec3( 4.3f+10.0f,  6.8f,  2.1f);
	Vertices3A[9] = vec3( 4.3f+10.0f,  6.8f, -0.7f);
	Vertices3A[10] = vec3( 4.3f+10.0f,  10.2f, -0.7f);
	Vertices3A[11] = vec3( 4.3f+10.0f,  10.2f,  2.1f);
//kiri
	Vertices3A[12] = vec3(-1.3f+10.0f,  6.8f, -0.7f);
	Vertices3A[13] = vec3(-1.3f+10.0f,  6.8f,  2.1f);
	Vertices3A[14] = vec3(-1.3f+10.0f,  10.2f,  2.1f);
	Vertices3A[15] = vec3(-1.3f+10.0f,  10.2f, -0.7f);

//atas
	Vertices3A[16] = vec3(-1.3f+10.0f,  10.2f,  2.1f);
	Vertices3A[17] = vec3( 4.3f+10.0f,  10.2f,  2.1f);
	Vertices3A[18] = vec3( 4.3f+10.0f,  10.2f, -0.7f);
	Vertices3A[19] = vec3(-1.3f+10.0f,  10.2f, -0.7f);
//bawah
	Vertices3A[20] = vec3(-1.3f+10.0f,  6.8f, -0.7f);
	Vertices3A[21] = vec3( 4.3f+10.0f,  6.8f, -0.7f);
	Vertices3A[22] = vec3( 4.3f+10.0f,  6.8f,  2.1f);
	Vertices3A[23] = vec3(-1.3f+10.0f,  6.8f,  2.1f);



//--KEPALA2
	Vertices4A = new vec3[24];

//depan
	Vertices4A[0] = vec3(-1.3f+10.0f,  6.8f,  2.1f);
	Vertices4A[1] = vec3( 4.3f+10.0f,  6.8f,  2.1f);
	Vertices4A[2] = vec3( 4.3f+10.0f,  10.2f,  2.1f);
	Vertices4A[3] = vec3(-1.3f+10.0f,  10.2f,  2.1f);




//--TANGAN KIRI
	Vertices5A = new vec3[24];

//depan
	Vertices5A[0] = vec3(-1.0f+10.0f,  2.3f,  1.3f);
	Vertices5A[1] = vec3(-0.2f+10.0f,  2.3f,  1.3f);
	Vertices5A[2] = vec3( -0.2f+10.0f,  6.3f,  1.3f);
	Vertices5A[3] = vec3(-1.0f+10.0f,  6.3f,  1.3f);
//belakang
	Vertices5A[4] = vec3( -0.2f+10.0f,  2.3f, 0.1f);
	Vertices5A[5] = vec3(-1.0f+10.0f,  2.3f, 0.1f);
	Vertices5A[6] = vec3(-1.0f+10.0f,  6.3f, 0.1f);
	Vertices5A[7] = vec3( -0.2f+10.0f,  6.3f, 0.1f);

//kanan
	Vertices5A[8] = vec3( -0.2f+10.0f,  2.3f,  1.3f);
	Vertices5A[9] = vec3( -0.2f+10.0f,  2.3f, 0.1f);
	Vertices5A[10] = vec3( -0.2f+10.0f,  6.3f, 0.1f);
	Vertices5A[11] = vec3( -0.2f+10.0f,  6.3f,  1.3f);
//kiri
	Vertices5A[12] = vec3(-1.0f+10.0f,  2.3f, 0.1f);
	Vertices5A[13] = vec3(-1.0f+10.0f,  2.3f,  1.3f);
	Vertices5A[14] = vec3(-1.0f+10.0f,  6.3f,  1.3f);
	Vertices5A[15] = vec3(-1.0f+10.0f,  6.3f, 0.1f);

//atas
	Vertices5A[16] = vec3(-1.0f+10.0f,  6.3f,  1.3f);
	Vertices5A[17] = vec3( -0.2f+10.0f,  6.3f,  1.3f);
	Vertices5A[18] = vec3( -0.2f+10.0f,  6.3f, 0.1f);
	Vertices5A[19] = vec3(-1.0f+10.0f,  6.3f,  0.1f);
//bawah
	Vertices5A[20] = vec3(-1.0f+10.0f,  2.3f, 0.1f);
	Vertices5A[21] = vec3( -0.2f+10.0f,  2.3f,0.1f);
	Vertices5A[22] = vec3( -0.2f+10.0f,  2.3f,  1.3f);
	Vertices5A[23] = vec3(-1.0f+10.0f,  2.3f,  1.3f);



//--TANGAN KANAN
	Vertices6A = new vec3[24];

//depan
	Vertices6A[0] = vec3(3.2f+10.0f,  2.3f,  1.3f);
	Vertices6A[1] = vec3(4.0f+10.0f,  2.3f,  1.3f);
	Vertices6A[2] = vec3( 4.0f+10.0f,  6.3f,  1.3f);
	Vertices6A[3] = vec3(3.2f+10.0f,  6.3f,  1.3f);
//belakang
	Vertices6A[4] = vec3( 4.0f+10.0f,  2.3f, 0.1f);
	Vertices6A[5] = vec3(3.2f+10.0f,  2.3f, 0.1f);
	Vertices6A[6] = vec3(3.2f+10.0f,  6.3f, 0.1f);
	Vertices6A[7] = vec3(4.0f+10.0f,  6.3f, 0.1f);

//kanan
	Vertices6A[8] = vec3( 4.0f+10.0f,  2.3f,  1.3f);
	Vertices6A[9] = vec3( 4.0f+10.0f,  2.3f, 0.1f);
	Vertices6A[10] = vec3( 4.0f+10.0f,  6.3f, 0.1f);
	Vertices6A[11] = vec3( 4.0f+10.0f,  6.3f,  1.3f);
//kiri
	Vertices6A[12] = vec3(3.2f+10.0f,  2.3f, 0.1f);
	Vertices6A[13] = vec3(3.2f+10.0f,  2.3f,  1.3f);
	Vertices6A[14] = vec3(3.2f+10.0f,  6.3f,  1.3f);
	Vertices6A[15] = vec3(3.2f+10.0f,  6.3f, 0.1f);

//atas
	Vertices6A[16] = vec3(3.2f+10.0f,  6.3f,  1.3f);
	Vertices6A[17] = vec3(4.0f+10.0f,  6.3f,  1.3f);
	Vertices6A[18] = vec3(4.0f+10.0f,  6.3f, 0.1f);
	Vertices6A[19] = vec3(3.2f+10.0f,  6.3f,  0.1f);
//bawah
	Vertices6A[20] = vec3(3.2f+10.0f,  2.3f, 0.1f);
	Vertices6A[21] = vec3(4.0f+10.0f,  2.3f,0.1f);
	Vertices6A[22] = vec3(4.0f+10.0f,  2.3f,  1.3f);
	Vertices6A[23] = vec3(3.2f+10.0f,  2.3f,  1.3f);
//==END DANBO2==========================================================================
 


//==DANBO3=============================================================================
	VerticesB = new vec3[24];

//depan
	VerticesB[0] = vec3((-0.0f-20.0f)/2, -0.0f,  1.4f/2);
	VerticesB[1] = vec3((1.4f-20.0f)/2, -0.0f,  1.4f/2);
	VerticesB[2] = vec3((1.4f-20.0f)/2,  2.8f/2,  1.4f/2);
	VerticesB[3] = vec3((-0.0f-20.0f)/2,  2.8f/2,  1.4f/2);
//belakang
	VerticesB[4] = vec3((1.4f-20.0f)/2, -0.0f, -0.0f);
	VerticesB[5] = vec3((-0.0f-20.0f)/2, -0.0f, -0.0f);
	VerticesB[6] = vec3((-0.0f-20.0f)/2,  2.8f/2, -0.0f);
	VerticesB[7] = vec3((1.4f-20.0f)/2,  2.8f/2, -0.0f);

//kanan
	VerticesB[8] = vec3((1.4f-20.0f)/2, -0.0f,  1.4f/2);
	VerticesB[9] = vec3((1.4f-20.0f)/2, -0.0f, -0.0f);
	VerticesB[10] = vec3((1.4f-20.0f)/2,  2.8f/2, -0.0f);
	VerticesB[11] = vec3((1.4f-20.0f)/2,  2.8f/2,  1.4f/2);
//kiri
	VerticesB[12] = vec3((-0.0f-20.0f)/2, -0.0f, -0.0f);
	VerticesB[13] = vec3((-0.0f-20.0f)/2, -0.0f,  1.4f/2);
	VerticesB[14] = vec3((-0.0f-20.0f)/2,  2.8f/2,  1.4f/2);
	VerticesB[15] = vec3((-0.0f-20.0f)/2,  2.8f/2, -0.0f);

//atas
	VerticesB[16] = vec3((-0.0f-20.0f)/2,  2.8f/2,  1.4f/2);
	VerticesB[17] = vec3((1.4f-20.0f)/2,  2.8f/2,  1.4f/2);
	VerticesB[18] = vec3((1.4f-20.0f)/2,  2.8f/2, -0.0f);
	VerticesB[19] = vec3((-0.0f-20.0f)/2,  2.8f/2, -0.0f);
//bawah
	VerticesB[20] = vec3((-0.0f-20.0f)/2, -0.0f, -0.0f);
	VerticesB[21] = vec3((1.4f-20.0f)/2, -0.0f, -0.0f);
	VerticesB[22] = vec3((1.4f-20.0f)/2, -0.0f,  1.4f/2);
	VerticesB[23] = vec3((-0.0f-20.0f)/2, -0.0f,  1.4f/2);

//--KAKI 2
	Vertices1B = new vec3[24];

//depan
	Vertices1B[0] = vec3((1.6f-20.0f)/2, -0.0f,  1.4f/2);
	Vertices1B[1] = vec3((3.0f-20.0f)/2, -0.0f,  1.4f/2);
	Vertices1B[2] = vec3((3.0f-20.0f)/2,  2.8f/2,  1.4f/2);
	Vertices1B[3] = vec3((1.6f-20.0f)/2,  2.8f/2,  1.4f/2);
//belakang
	Vertices1B[4] = vec3((3.0f-20.0f)/2, -0.0f, -0.0f);
	Vertices1B[5] = vec3( (1.6f-20.0f)/2, -0.0f, -0.0f);
	Vertices1B[6] = vec3( (1.6f-20.0f)/2,  2.8f/2, -0.0f);
	Vertices1B[7] = vec3( (3.0f-20.0f)/2,  2.8f/2, -0.0f);

//kanan
	Vertices1B[8] = vec3( (3.0f-20.0f)/2, -0.0f,  1.4f/2);
	Vertices1B[9] = vec3( (3.0f-20.0f)/2, -0.0f, -0.0f);
	Vertices1B[10] = vec3( (3.0f-20.0f)/2,  2.8f/2, -0.0f);
	Vertices1B[11] = vec3( (3.0f-20.0f)/2,  2.8f/2,  1.4f/2);
//kiri
	Vertices1B[12] = vec3( (1.6f-20.0f)/2, -0.0f, -0.0f);
	Vertices1B[13] = vec3( (1.6f-20.0f)/2, -0.0f,  1.4f/2);
	Vertices1B[14] = vec3( (1.6f-20.0f)/2,  2.8f/2,  1.4f/2);
	Vertices1B[15] = vec3( (1.6f-20.0f)/2,  2.8f/2, -0.0f);

//atas
	Vertices1B[16] = vec3( (1.6f-20.0f)/2,  2.8f/2,  1.4f/2);
	Vertices1B[17] = vec3( (3.0f-20.0f)/2,  2.8f/2,  1.4f/2);
	Vertices1B[18] = vec3( (3.0f-20.0f)/2,  2.8f/2, -0.0f);
	Vertices1B[19] = vec3( (1.6f-20.0f)/2,  2.8f/2, -0.0f);
//bawah
	Vertices1B[20] = vec3( (1.6f-20.0f)/2, -0.0f, -0.0f);
	Vertices1B[21] = vec3( (3.0f-20.0f)/2, -0.0f, -0.0f);
	Vertices1B[22] = vec3( (3.0f-20.0f)/2, -0.0f,  1.4f/2);
	Vertices1B[23] = vec3( (1.6f-20.0f)/2, -0.0f,  1.4f/2);


//--Badan
	Vertices2B = new vec3[24];

//depan
	Vertices2B[0] = vec3((-0.2f-20.0f)/2,  2.8f/2,  1.6f/2);
	Vertices2B[1] = vec3( (3.2f-20.0f)/2,  2.8f/2,  1.6f/2);
	Vertices2B[2] = vec3( (3.2f-20.0f)/2,  6.8f/2,  1.6f/2);
	Vertices2B[3] = vec3((-0.2f-20.0f)/2,  6.8f/2,  1.6f/2);
//belakang
	Vertices2B[4] = vec3( (3.2f-20.0f)/2,  2.8f/2, -0.2f/2);
	Vertices2B[5] = vec3((-0.2f-20.0f)/2,  2.8f/2, -0.2f/2);
	Vertices2B[6] = vec3((-0.2f-20.0f)/2,  6.8f/2, -0.2f/2);
	Vertices2B[7] = vec3( (3.2f-20.0f)/2,  6.8f/2, -0.2f/2);

//kanan
	Vertices2B[8] = vec3( (3.2f-20.0f)/2,  2.8f/2,  1.6f/2);
	Vertices2B[9] = vec3( (3.2f-20.0f)/2,  2.8f/2, -0.2f/2);
	Vertices2B[10] = vec3( (3.2f-20.0f)/2,  6.8f/2, -0.2f/2);
	Vertices2B[11] = vec3( (3.2f-20.0f)/2,  6.8f/2,  1.6f/2);
//kiri
	Vertices2B[12] = vec3((-0.2f-20.0f)/2,  2.8f/2, -0.2f/2);
	Vertices2B[13] = vec3((-0.2f-20.0f)/2,  2.8f/2,  1.6f/2);
	Vertices2B[14] = vec3((-0.2f-20.0f)/2,  6.8f/2,  1.6f/2);
	Vertices2B[15] = vec3((-0.2f-20.0f)/2,  6.8f/2, -0.2f/2);

//atas
	Vertices2B[16] = vec3((-0.2f-20.0f)/2,  6.8f/2,  1.6f/2);
	Vertices2B[17] = vec3( (3.2f-20.0f)/2,  6.8f/2,  1.6f/2);
	Vertices2B[18] = vec3( (3.2f-20.0f)/2,  6.8f/2, -0.2f/2);
	Vertices2B[19] = vec3((-0.2f-20.0f)/2,  6.8f/2, -0.2f/2);
//bawah
	Vertices2B[20] = vec3((-0.2f-20.0f)/2,  2.8f/2, -0.2f/2);
	Vertices2B[21] = vec3( (3.2f-20.0f)/2,  2.8f/2, -0.2f/2);
	Vertices2B[22] = vec3( (3.2f-20.0f)/2,  2.8f/2,  1.6f/2);
	Vertices2B[23] = vec3((-0.2f-20.0f)/2,  2.8f/2,  1.6f/2);




//--KEPALA1
	Vertices3B = new vec3[24];
//depan
	//Vertices3[0] = vec3(-1.3f,  6.8f,  2.1f);
	//Vertices3[1] = vec3( 4.3f,  6.8f,  2.1f);
	//Vertices3[2] = vec3( 4.3f,  10.2f,  2.1f);
	//Vertices3[3] = vec3(-1.3f,  10.2f,  2.1f);
	
//belakang
	Vertices3B[4] = vec3( (4.3f-20.0f)/2,  6.8f/2, -0.7f/2);
	Vertices3B[5] = vec3((-1.3f-20.0f)/2,  6.8f/2, -0.7f/2);
	Vertices3B[6] = vec3((-1.3f-20.0f)/2,  10.2f/2, -0.7f/2);
	Vertices3B[7] = vec3( (4.3f-20.0f)/2,  10.2f/2, -0.7f/2);

//kanan
	Vertices3B[8] = vec3( (4.3f-20.0f)/2,  6.8f/2,  2.1f/2);
	Vertices3B[9] = vec3( (4.3f-20.0f)/2,  6.8f/2, -0.7f/2);
	Vertices3B[10] = vec3( (4.3f-20.0f)/2,  10.2f/2, -0.7f/2);
	Vertices3B[11] = vec3( (4.3f-20.0f)/2,  10.2f/2,  2.1f/2);
//kiri
	Vertices3B[12] = vec3((-1.3f-20.0f)/2,  6.8f/2, -0.7f/2);
	Vertices3B[13] = vec3((-1.3f-20.0f)/2,  6.8f/2,  2.1f/2);
	Vertices3B[14] = vec3((-1.3f-20.0f)/2,  10.2f/2,  2.1f/2);
	Vertices3B[15] = vec3((-1.3f-20.0f)/2,  10.2f/2, -0.7f/2);

//atas
	Vertices3B[16] = vec3((-1.3f-20.0f)/2,  10.2f/2,  2.1f/2);
	Vertices3B[17] = vec3( (4.3f-20.0f)/2,  10.2f/2,  2.1f/2);
	Vertices3B[18] = vec3( (4.3f-20.0f)/2,  10.2f/2, -0.7f/2);
	Vertices3B[19] = vec3((-1.3f-20.0f)/2,  10.2f/2, -0.7f/2);
//bawah
	Vertices3B[20] = vec3((-1.3f-20.0f)/2,  6.8f/2, -0.7f/2);
	Vertices3B[21] = vec3( (4.3f-20.0f)/2,  6.8f/2, -0.7f/2);
	Vertices3B[22] = vec3( (4.3f-20.0f)/2,  6.8f/2,  2.1f/2);
	Vertices3B[23] = vec3((-1.3f-20.0f)/2,  6.8f/2,  2.1f/2);



//--KEPALA2
	Vertices4B = new vec3[24];

//depan
	Vertices4B[0] = vec3((-1.3f-20.0f)/2,  6.8f/2,  2.1f/2);
	Vertices4B[1] = vec3(( 4.3f-20.0f)/2,  6.8f/2,  2.1f/2);
	Vertices4B[2] = vec3(( 4.3f-20.0f)/2,  10.2f/2,  2.1f/2);
	Vertices4B[3] = vec3((-1.3f-20.0f)/2,  10.2f/2,  2.1f/2);




//--TANGAN KIRI
	Vertices5B = new vec3[24];

//depan
	Vertices5B[0] = vec3((-1.0f-20.0f)/2,  2.3f/2,  1.3f/2);
	Vertices5B[1] = vec3((-0.2f-20.0f)/2,  2.3f/2,  1.3f/2);
	Vertices5B[2] = vec3( (-0.2f-20.0f)/2,  6.3f/2,  1.3f/2);
	Vertices5B[3] = vec3((-1.0f-20.0f)/2,  6.3f/2,  1.3f/2);
//belakang
	Vertices5B[4] = vec3( (-0.2f-20.0f)/2,  2.3f/2, 0.1f/2);
	Vertices5B[5] = vec3((-1.0f-20.0f)/2,  2.3f/2, 0.1f/2);
	Vertices5B[6] = vec3((-1.0f-20.0f)/2,  6.3f/2, 0.1f/2);
	Vertices5B[7] = vec3( (-0.2f-20.0f)/2,  6.3f/2, 0.1f/2);

//kanan
	Vertices5B[8] = vec3( (-0.2f-20.0f)/2,  2.3f/2,  1.3f/2);
	Vertices5B[9] = vec3( (-0.2f-20.0f)/2,  2.3f/2, 0.1f/2);
	Vertices5B[10] = vec3( (-0.2f-20.0f)/2,  6.3f/2, 0.1f/2);
	Vertices5B[11] = vec3( (-0.2f-20.0f)/2,  6.3f/2,  1.3f/2);
//kiri
	Vertices5B[12] = vec3((-1.0f-20.0f)/2,  2.3f/2, 0.1f/2);
	Vertices5B[13] = vec3((-1.0f-20.0f)/2,  2.3f/2,  1.3f/2);
	Vertices5B[14] = vec3((-1.0f-20.0f)/2,  6.3f/2,  1.3f/2);
	Vertices5B[15] = vec3((-1.0f-20.0f)/2,  6.3f/2, 0.1f/2);

//atas
	Vertices5B[16] = vec3((-1.0f-20.0f)/2,  6.3f/2,  1.3f/2);
	Vertices5B[17] = vec3( (-0.2f-20.0f)/2,  6.3f/2,  1.3f/2);
	Vertices5B[18] = vec3( (-0.2f-20.0f)/2,  6.3f/2, 0.1f/2);
	Vertices5B[19] = vec3((-1.0f-20.0f)/2,  6.3f/2,  0.1f/2);
//bawah
	Vertices5B[20] = vec3((-1.0f-20.0f)/2,  2.3f/2, 0.1f/2);
	Vertices5B[21] = vec3( (-0.2f-20.0f)/2,  2.3f/2,0.1f/2);
	Vertices5B[22] = vec3( (-0.2f-20.0f)/2,  2.3f/2,  1.3f/2);
	Vertices5B[23] = vec3((-1.0f-20.0f)/2,  2.3f/2,  1.3f/2);


//--TANGAN KANAN
	Vertices6B = new vec3[24];

//depan
	Vertices6B[0] = vec3((3.2f-20.0f)/2,  2.3f/2,  1.3f/2);
	Vertices6B[1] = vec3((4.0f-20.0f)/2,  2.3f/2,  1.3f/2);
	Vertices6B[2] = vec3( (4.0f-20.0f)/2,  6.3f/2,  1.3f/2);
	Vertices6B[3] = vec3((3.2f-20.0f)/2,  6.3f/2,  1.3f/2);
//belakang
	Vertices6B[4] = vec3( (4.0f-20.0f)/2,  2.3f/2, 0.1f/2);
	Vertices6B[5] = vec3((3.2f-20.0f)/2,  2.3f/2, 0.1f/2);
	Vertices6B[6] = vec3((3.2f-20.0f)/2,  6.3f/2, 0.1f/2);
	Vertices6B[7] = vec3((4.0f-20.0f)/2,  6.3f/2, 0.1f/2);

//kanan
	Vertices6B[8] = vec3( (4.0f-20.0f)/2,  2.3f/2,  1.3f/2);
	Vertices6B[9] = vec3( (4.0f-20.0f)/2,  2.3f/2, 0.1f/2);
	Vertices6B[10] = vec3( (4.0f-20.0f)/2,  6.3f/2, 0.1f/2);
	Vertices6B[11] = vec3( (4.0f-20.0f)/2,  6.3f/2,  1.3f/2);
//kiri
	Vertices6B[12] = vec3((3.2f-20.0f)/2,  2.3f/2, 0.1f/2);
	Vertices6B[13] = vec3((3.2f-20.0f)/2,  2.3f/2,  1.3f/2);
	Vertices6B[14] = vec3((3.2f-20.0f)/2,  6.3f/2,  1.3f/2);
	Vertices6B[15] = vec3((3.2f-20.0f)/2,  6.3f/2, 0.1f/2);

//atas
	Vertices6B[16] = vec3((3.2f-20.0f)/2,  6.3f/2,  1.3f/2);
	Vertices6B[17] = vec3((4.0f-20.0f)/2,  6.3f/2,  1.3f/2);
	Vertices6B[18] = vec3((4.0f-20.0f)/2,  6.3f/2, 0.1f/2);
	Vertices6B[19] = vec3((3.2f-20.0f)/2,  6.3f/2,  0.1f/2);
//bawah
	Vertices6B[20] = vec3((3.2f-20.0f)/2,  2.3f/2, 0.1f/2);
	Vertices6B[21] = vec3((4.0f-20.0f)/2,  2.3f/2, 0.1f/2);
	Vertices6B[22] = vec3((4.0f-20.0f)/2,  2.3f/2,  1.3f/2);
	Vertices6B[23] = vec3((3.2f-20.0f)/2,  2.3f/2,  1.3f/2);
    
//==END DANBO3===============================================================================


//==DANBO3====================================================================================
	VerticesC = new vec3[24];
//depan
	VerticesC[0] = vec3(-0.0f-125.0f, -0.0f+20.0f,  1.4f);
	VerticesC[1] = vec3( 1.4f-125.0f, -0.0f+20.0f,  1.4f);
	VerticesC[2] = vec3( 1.4f-125.0f,  2.8f+20.0f,  1.4f);
	VerticesC[3] = vec3(-0.0f-125.0f,  2.8f+20.0f,  1.4f);
//belakang
	VerticesC[4] = vec3( 1.4f-125.0f, -0.0f+20.0f, -0.0f);
	VerticesC[5] = vec3(-0.0f-125.0f, -0.0f+20.0f, -0.0f);
	VerticesC[6] = vec3(-0.0f-125.0f,  2.8f+20.0f, -0.0f);
	VerticesC[7] = vec3( 1.4f-125.0f,  2.8f+20.0f, -0.0f);

//kanan
	VerticesC[8] = vec3( 1.4f-125.0f, -0.0f+20.0f,  1.4f);
	VerticesC[9] = vec3( 1.4f-125.0f, -0.0f+20.0f, -0.0f);
	VerticesC[10] = vec3( 1.4f-125.0f,  2.8f+20.0f, -0.0f);
	VerticesC[11] = vec3( 1.4f-125.0f,  2.8f+20.0f,  1.4f);
//kiri
	VerticesC[12] = vec3(-0.0f-125.0f, -0.0f+20.0f, -0.0f);
	VerticesC[13] = vec3(-0.0f-125.0f, -0.0f+20.0f,  1.4f);
	VerticesC[14] = vec3(-0.0f-125.0f,  2.8f+20.0f,  1.4f);
	VerticesC[15] = vec3(-0.0f-125.0f,  2.8f+20.0f, -0.0f);

//atas
	VerticesC[16] = vec3(-0.0f-125.0f,  2.8f+20.0f,  1.4f);
	VerticesC[17] = vec3( 1.4f-125.0f,  2.8f+20.0f,  1.4f);
	VerticesC[18] = vec3( 1.4f-125.0f,  2.8f+20.0f, -0.0f);
	VerticesC[19] = vec3(-0.0f-125.0f,  2.8f+20.0f, -0.0f);
//bawah
	VerticesC[20] = vec3(-0.0f-125.0f, -0.0f+20.0f, -0.0f);
	VerticesC[21] = vec3( 1.4f-125.0f, -0.0f+20.0f, -0.0f);
	VerticesC[22] = vec3( 1.4f-125.0f, -0.0f+20.0f,  1.4f);
	VerticesC[23] = vec3(-0.0f-125.0f, -0.0f+20.0f,  1.4f);

//--KAKI 2
	Vertices1C = new vec3[24];

//depan
	Vertices1C[0] = vec3( 1.6f-125.0f, -0.0f+20.0f,  1.4f);
	Vertices1C[1] = vec3( 3.0f-125.0f, -0.0f+20.0f,  1.4f);
	Vertices1C[2] = vec3( 3.0f-125.0f,  2.8f+20.0f,  1.4f);
	Vertices1C[3] = vec3( 1.6f-125.0f,  2.8f+20.0f,  1.4f);
//belakang
	Vertices1C[4] = vec3( 3.0f-125.0f, -0.0f+20.0f, -0.0f);
	Vertices1C[5] = vec3( 1.6f-125.0f, -0.0f+20.0f, -0.0f);
	Vertices1C[6] = vec3( 1.6f-125.0f,  2.8f+20.0f, -0.0f);
	Vertices1C[7] = vec3( 3.0f-125.0f,  2.8f+20.0f, -0.0f);

//kanan
	Vertices1C[8] = vec3( 3.0f-125.0f, -0.0f+20.0f,  1.4f);
	Vertices1C[9] = vec3( 3.0f-125.0f, -0.0f+20.0f, -0.0f);
	Vertices1C[10] = vec3( 3.0f-125.0f,  2.8f+20.0f, -0.0f);
	Vertices1C[11] = vec3( 3.0f-125.0f,  2.8f+20.0f,  1.4f);
//kiri
	Vertices1C[12] = vec3( 1.6f-125.0f, -0.0f+20.0f, -0.0f);
	Vertices1C[13] = vec3( 1.6f-125.0f, -0.0f+20.0f,  1.4f);
	Vertices1C[14] = vec3( 1.6f-125.0f,  2.8f+20.0f,  1.4f);
	Vertices1C[15] = vec3( 1.6f-125.0f,  2.8f+20.0f, -0.0f);

//atas
	Vertices1C[16] = vec3( 1.6f-125.0f,  2.8f+20.0f,  1.4f);
	Vertices1C[17] = vec3( 3.0f-125.0f,  2.8f+20.0f,  1.4f);
	Vertices1C[18] = vec3( 3.0f-125.0f,  2.8f+20.0f, -0.0f);
	Vertices1C[19] = vec3( 1.6f-125.0f,  2.8f+20.0f, -0.0f);
//bawaH
	Vertices1C[20] = vec3( 1.6f-125.0f, -0.0f+20.0f, -0.0f);
	Vertices1C[21] = vec3( 3.0f-125.0f, -0.0f+20.0f, -0.0f);
	Vertices1C[22] = vec3( 3.0f-125.0f, -0.0f+20.0f,  1.4f);
	Vertices1C[23] = vec3( 1.6f-125.0f, -0.0f+20.0f,  1.4f);



//--Badan
	Vertices2C = new vec3[24];

//depan
	Vertices2C[0] = vec3(-0.2f-125.0f,  2.8f+20.0f,  1.6f);
	Vertices2C[1] = vec3( 3.2f-125.0f,  2.8f+20.0f,  1.6f);
	Vertices2C[2] = vec3( 3.2f-125.0f,  6.8f+20.0f,  1.6f);
	Vertices2C[3] = vec3(-0.2f-125.0f,  6.8f+20.0f,  1.6f);
//belakanG
	Vertices2C[4] = vec3( 3.2f-125.0f,  2.8f+20.0f, -0.2f);
	Vertices2C[5] = vec3(-0.2f-125.0f,  2.8f+20.0f, -0.2f);
	Vertices2C[6] = vec3(-0.2f-125.0f,  6.8f+20.0f, -0.2f);
	Vertices2C[7] = vec3( 3.2f-125.0f,  6.8f+20.0f, -0.2f);

//kanan
	Vertices2C[8] = vec3( 3.2f-125.0f,  2.8f+20.0f,  1.6f);
	Vertices2C[9] = vec3( 3.2f-125.0f,  2.8f+20.0f, -0.2f);
	Vertices2C[10] = vec3( 3.2f-125.0f,  6.8f+20.0f, -0.2f);
	Vertices2C[11] = vec3( 3.2f-125.0f,  6.8f+20.0f,  1.6f);
//kiri
	Vertices2C[12] = vec3(-0.2f-125.0f,  2.8f+20.0f, -0.2f);
	Vertices2C[13] = vec3(-0.2f-125.0f,  2.8f+20.0f,  1.6f);
	Vertices2C[14] = vec3(-0.2f-125.0f,  6.8f+20.0f,  1.6f);
	Vertices2C[15] = vec3(-0.2f-125.0f,  6.8f+20.0f, -0.2f);

//atas
	Vertices2C[16] = vec3(-0.2f-125.0f,  6.8f+20.0f,  1.6f);
	Vertices2C[17] = vec3( 3.2f-125.0f,  6.8f+20.0f,  1.6f);
	Vertices2C[18] = vec3( 3.2f-125.0f,  6.8f+20.0f, -0.2f);
	Vertices2C[19] = vec3(-0.2f-125.0f,  6.8f+20.0f, -0.2f);
//bawaH
	Vertices2C[20] = vec3(-0.2f-125.0f,  2.8f+20.0f, -0.2f);
	Vertices2C[21] = vec3( 3.2f-125.0f,  2.8f+20.0f, -0.2f);
	Vertices2C[22] = vec3( 3.2f-125.0f,  2.8f+20.0f,  1.6f);
	Vertices2C[23] = vec3(-0.2f-125.0f,  2.8f+20.0f,  1.6f);




//--KEPALA1
	Vertices3C = new vec3[24];
/*
//depan
	Vertices3[0] = vec3(-1.3f,  6.8f,  2.1f);
	Vertices3[1] = vec3( 4.3f,  6.8f,  2.1f);
	Vertices3[2] = vec3( 4.3f,  10.2f,  2.1f);
	Vertices3[3] = vec3(-1.3f,  10.2f,  2.1f);
	*/
//belakang
	Vertices3C[4] = vec3( 4.3f-125.0f,  6.8f+20.0f, -0.7f);
	Vertices3C[5] = vec3(-1.3f-125.0f,  6.8f+20.0f, -0.7f);
	Vertices3C[6] = vec3(-1.3f-125.0f,  10.2f+20.0f, -0.7f);
	Vertices3C[7] = vec3( 4.3f-125.0f,  10.2f+20.0f, -0.7f);

//kanan
	Vertices3C[8] = vec3( 4.3f-125.0f,  6.8f+20.0f,  2.1f);
	Vertices3C[9] = vec3( 4.3f-125.0f,  6.8f+20.0f, -0.7f);
	Vertices3C[10] = vec3( 4.3f-125.0f,  10.2f+20.0f, -0.7f);
	Vertices3C[11] = vec3( 4.3f-125.0f,  10.2f+20.0f,  2.1f);
//kiri
	Vertices3C[12] = vec3(-1.3f-125.0f,  6.8f+20.0f, -0.7f);
	Vertices3C[13] = vec3(-1.3f-125.0f,  6.8f+20.0f,  2.1f);
	Vertices3C[14] = vec3(-1.3f-125.0f,  10.2f+20.0f,  2.1f);
	Vertices3C[15] = vec3(-1.3f-125.0f,  10.2f+20.0f, -0.7f);

//atas
	Vertices3C[16] = vec3(-1.3f-125.0f,  10.2f+20.0f,  2.1f);
	Vertices3C[17] = vec3( 4.3f-125.0f,  10.2f+20.0f,  2.1f);
	Vertices3C[18] = vec3( 4.3f-125.0f,  10.2f+20.0f, -0.7f);
	Vertices3C[19] = vec3(-1.3f-125.0f,  10.2f+20.0f, -0.7f);
//bawah
	Vertices3C[20] = vec3(-1.3f-125.0f,  6.8f+20.0f, -0.7f);
	Vertices3C[21] = vec3( 4.3f-125.0f,  6.8f+20.0f, -0.7f);
	Vertices3C[22] = vec3( 4.3f-125.0f,  6.8f+20.0f,  2.1f);
	Vertices3C[23] = vec3(-1.3f-125.0f,  6.8f+20.0f,  2.1f);



//--KEPALA2
	Vertices4C = new vec3[24];

//depan
	Vertices4C[0] = vec3(-1.3f-125.0f,  6.8f+20.0f,  2.1f);
	Vertices4C[1] = vec3( 4.3f-125.0f,  6.8f+20.0f,  2.1f);
	Vertices4C[2] = vec3( 4.3f-125.0f,  10.2f+20.0f,  2.1f);
	Vertices4C[3] = vec3(-1.3f-125.0f,  10.2f+20.0f,  2.1f);




//--TANGAN KIRI
	Vertices5C = new vec3[24];

//depan
	Vertices5C[0] = vec3(-1.0f-125.0f,  2.3f+20.0f,  1.3f);
	Vertices5C[1] = vec3(-0.2f-125.0f,  2.3f+20.0f,  1.3f);
	Vertices5C[2] = vec3( -0.2f-125.0f,  6.3f+20.0f,  1.3f);
	Vertices5C[3] = vec3(-1.0f-125.0f,  6.3f+20.0f,  1.3f);
//belakang
	Vertices5C[4] = vec3( -0.2f-125.0f,  2.3f+20.0f, 0.1f);
	Vertices5C[5] = vec3(-1.0f-125.0f,  2.3f+20.0f, 0.1f);
	Vertices5C[6] = vec3(-1.0f-125.0f,  6.3f+20.0f, 0.1f);
	Vertices5C[7] = vec3( -0.2f-125.0f,  6.3f+20.0f, 0.1f);

//kanan
	Vertices5C[8] = vec3( -0.2f-125.0f,  2.3f+20.0f,  1.3f);
	Vertices5C[9] = vec3( -0.2f-125.0f,  2.3f+20.0f, 0.1f);
	Vertices5C[10] = vec3( -0.2f-125.0f,  6.3f+20.0f, 0.1f);
	Vertices5C[11] = vec3( -0.2f-125.0f,  6.3f+20.0f,  1.3f);
//kiri
	Vertices5C[12] = vec3(-1.0f-125.0f,  2.3f+20.0f, 0.1f);
	Vertices5C[13] = vec3(-1.0f-125.0f,  2.3f+20.0f,  1.3f);
	Vertices5C[14] = vec3(-1.0f-125.0f,  6.3f+20.0f,  1.3f);
	Vertices5C[15] = vec3(-1.0f-125.0f,  6.3f+20.0f, 0.1f);

//atas
	Vertices5C[16] = vec3(-1.0f-125.0f,  6.3f+20.0f,  1.3f);
	Vertices5C[17] = vec3( -0.2f-125.0f,  6.3f+20.0f,  1.3f);
	Vertices5C[18] = vec3( -0.2f-125.0f,  6.3f+20.0f, 0.1f);
	Vertices5C[19] = vec3(-1.0f-125.0f,  6.3f+20.0f,  0.1f);
//bawah
	Vertices5C[20] = vec3(-1.0f-125.0f,  2.3f+20.0f, 0.1f);
	Vertices5C[21] = vec3( -0.2f-125.0f,  2.3f+20.0f,0.1f);
	Vertices5C[22] = vec3( -0.2f-125.0f,  2.3f+20.0f,  1.3f);
	Vertices5C[23] = vec3(-1.0f-125.0f,  2.3f+20.0f,  1.3f);



//--TANGAN KANAN
	Vertices6C = new vec3[24];

//depan
	Vertices6C[0] = vec3(3.2f-125.0f,  2.3f+20.0f,  1.3f);
	Vertices6C[1] = vec3(4.0f-125.0f,  2.3f+20.0f,  1.3f);
	Vertices6C[2] = vec3( 4.0f-125.0f,  6.3f+20.0f,  1.3f);
	Vertices6C[3] = vec3(3.2f-125.0f,  6.3f+20.0f,  1.3f);
//belakang
	Vertices6C[4] = vec3( 4.0f-125.0f,  2.3f+20.0f, 0.1f);
	Vertices6C[5] = vec3(3.2f-125.0f,  2.3f+20.0f, 0.1f);
	Vertices6C[6] = vec3(3.2f-125.0f,  6.3f+20.0f, 0.1f);
	Vertices6C[7] = vec3(4.0f-125.0f,  6.3f+20.0f, 0.1f);

//kanan
	Vertices6C[8] = vec3( 4.0f-125.0f,  2.3f+20.0f,  1.3f);
	Vertices6C[9] = vec3( 4.0f-125.0f,  2.3f+20.0f, 0.1f);
	Vertices6C[10] = vec3( 4.0f-125.0f,  6.3f+20.0f, 0.1f);
	Vertices6C[11] = vec3( 4.0f-125.0f,  6.3f+20.0f,  1.3f);
//kiri
	Vertices6C[12] = vec3(3.2f-125.0f,  2.3f+20.0f, 0.1f);
	Vertices6C[13] = vec3(3.2f-125.0f,  2.3f+20.0f,  1.3f);
	Vertices6C[14] = vec3(3.2f-125.0f,  6.3f+20.0f,  1.3f);
	Vertices6C[15] = vec3(3.2f-125.0f,  6.3f+20.0f, 0.1f);

//atas
	Vertices6C[16] = vec3(3.2f-125.0f,  6.3f+20.0f,  1.3f);
	Vertices6C[17] = vec3(4.0f-125.0f,  6.3f+20.0f,  1.3f);
	Vertices6C[18] = vec3(4.0f-125.0f,  6.3f+20.0f, 0.1f);
	Vertices6C[19] = vec3(3.2f-125.0f,  6.3f+20.0f,  0.1f);
//bawah
	Vertices6C[20] = vec3(3.2f-125.0f,  2.3f+20.0f, 0.1f);
	Vertices6C[21] = vec3(4.0f-125.0f,  2.3f+20.0f,0.1f);
	Vertices6C[22] = vec3(4.0f-125.0f,  2.3f+20.0f,  1.3f);
	Vertices6C[23] = vec3(3.2f-125.0f,  2.3f+20.0f,  1.3f);
//==END DANBO4==========================================================================
 



	VerticesE = new vec3[24];
//==DANBOE====================================================================================
//depan
	VerticesE[0] = vec3(-0.0f, -0.0f,  1.4f);
	VerticesE[1] = vec3( 1.4f, -0.0f,  1.4f);
	VerticesE[2] = vec3( 1.4f,  2.8f,  1.4f);
	VerticesE[3] = vec3(-0.0f,  2.8f,  1.4f);
//belakang
	VerticesE[4] = vec3( 1.4f, -0.0f, -0.0f);
	VerticesE[5] = vec3(-0.0f, -0.0f, -0.0f);
	VerticesE[6] = vec3(-0.0f,  2.8f, -0.0f);
	VerticesE[7] = vec3( 1.4f,  2.8f, -0.0f);

//kanan
	VerticesE[8] = vec3( 1.4f, -0.0f,  1.4f);
	VerticesE[9] = vec3( 1.4f, -0.0f, -0.0f);
	VerticesE[10] = vec3( 1.4f,  2.8f, -0.0f);
	VerticesE[11] = vec3( 1.4f,  2.8f,  1.4f);
//kiri
	VerticesE[12] = vec3(-0.0f, -0.0f, -0.0f);
	VerticesE[13] = vec3(-0.0f, -0.0f,  1.4f);
	VerticesE[14] = vec3(-0.0f,  2.8f,  1.4f);
	VerticesE[15] = vec3(-0.0f,  2.8f, -0.0f);

//atas
	VerticesE[16] = vec3(-0.0f,  2.8f,  1.4f);
	VerticesE[17] = vec3( 1.4f,  2.8f,  1.4f);
	VerticesE[18] = vec3( 1.4f,  2.8f, -0.0f);
	VerticesE[19] = vec3(-0.0f,  2.8f, -0.0f);
//bawah
	VerticesE[20] = vec3(-0.0f, -0.0f, -0.0f);
	VerticesE[21] = vec3( 1.4f, -0.0f, -0.0f);
	VerticesE[22] = vec3( 1.4f, -0.0f,  1.4f);
	VerticesE[23] = vec3(-0.0f, -0.0f,  1.4f);

//--KAKI 2
	Vertices1E = new vec3[24];

//depan
	Vertices1E[0] = vec3( 1.6f, -0.0f,  1.4f);
	Vertices1E[1] = vec3( 3.0f, -0.0f,  1.4f);
	Vertices1E[2] = vec3( 3.0f,  2.8f,  1.4f);
	Vertices1E[3] = vec3( 1.6f,  2.8f,  1.4f);
//belakang
	Vertices1E[4] = vec3( 3.0f, -0.0f, -0.0f);
	Vertices1E[5] = vec3( 1.6f, -0.0f, -0.0f);
	Vertices1E[6] = vec3( 1.6f,  2.8f, -0.0f);
	Vertices1E[7] = vec3( 3.0f,  2.8f, -0.0f);

//kanan
	Vertices1E[8] = vec3( 3.0f, -0.0f,  1.4f);
	Vertices1E[9] = vec3( 3.0f, -0.0f, -0.0f);
	Vertices1E[10] = vec3( 3.0f,  2.8f, -0.0f);
	Vertices1E[11] = vec3( 3.0f,  2.8f,  1.4f);
//kiri
	Vertices1E[12] = vec3( 1.6f, -0.0f, -0.0f);
	Vertices1E[13] = vec3( 1.6f, -0.0f,  1.4f);
	Vertices1E[14] = vec3( 1.6f,  2.8f,  1.4f);
	Vertices1E[15] = vec3( 1.6f,  2.8f, -0.0f);

//atas
	Vertices1E[16] = vec3( 1.6f,  2.8f,  1.4f);
	Vertices1E[17] = vec3( 3.0f,  2.8f,  1.4f);
	Vertices1E[18] = vec3( 3.0f,  2.8f, -0.0f);
	Vertices1E[19] = vec3( 1.6f,  2.8f, -0.0f);
//bawah
	Vertices1E[20] = vec3( 1.6f, -0.0f, -0.0f);
	Vertices1E[21] = vec3( 3.0f, -0.0f, -0.0f);
	Vertices1E[22] = vec3( 3.0f, -0.0f,  1.4f);
	Vertices1E[23] = vec3( 1.6f, -0.0f,  1.4f);



//--Badan
	Vertices2E = new vec3[24];

//depan
	Vertices2E[0] = vec3(-0.2f,  2.8f,  1.6f);
	Vertices2E[1] = vec3( 3.2f,  2.8f,  1.6f);
	Vertices2E[2] = vec3( 3.2f,  6.8f,  1.6f);
	Vertices2E[3] = vec3(-0.2f,  6.8f,  1.6f);
//belakang
	Vertices2E[4] = vec3( 3.2f,  2.8f, -0.2f);
	Vertices2E[5] = vec3(-0.2f,  2.8f, -0.2f);
	Vertices2E[6] = vec3(-0.2f,  6.8f, -0.2f);
	Vertices2E[7] = vec3( 3.2f,  6.8f, -0.2f);

//kanan
	Vertices2E[8] = vec3( 3.2f,  2.8f,  1.6f);
	Vertices2E[9] = vec3( 3.2f,  2.8f, -0.2f);
	Vertices2E[10] = vec3( 3.2f,  6.8f, -0.2f);
	Vertices2E[11] = vec3( 3.2f,  6.8f,  1.6f);
//kiri
	Vertices2E[12] = vec3(-0.2f,  2.8f, -0.2f);
	Vertices2E[13] = vec3(-0.2f,  2.8f,  1.6f);
	Vertices2E[14] = vec3(-0.2f,  6.8f,  1.6f);
	Vertices2E[15] = vec3(-0.2f,  6.8f, -0.2f);

//atas
	Vertices2E[16] = vec3(-0.2f,  6.8f,  1.6f);
	Vertices2E[17] = vec3( 3.2f,  6.8f,  1.6f);
	Vertices2E[18] = vec3( 3.2f,  6.8f, -0.2f);
	Vertices2E[19] = vec3(-0.2f,  6.8f, -0.2f);
//bawah
	Vertices2E[20] = vec3(-0.2f,  2.8f, -0.2f);
	Vertices2E[21] = vec3( 3.2f,  2.8f, -0.2f);
	Vertices2E[22] = vec3( 3.2f,  2.8f,  1.6f);
	Vertices2E[23] = vec3(-0.2f,  2.8f,  1.6f);




//--KEPALA1
	Vertices3E = new vec3[24];
/*
//depan
	Vertices3[0] = vec3(-1.3f,  6.8f,  2.1f);
	Vertices3[1] = vec3( 4.3f,  6.8f,  2.1f);
	Vertices3[2] = vec3( 4.3f,  10.2f,  2.1f);
	Vertices3[3] = vec3(-1.3f,  10.2f,  2.1f);
	*/
//belakang
	Vertices3E[4] = vec3( 4.3f,  6.8f, -0.7f);
	Vertices3E[5] = vec3(-1.3f,  6.8f, -0.7f);
	Vertices3E[6] = vec3(-1.3f,  10.2f, -0.7f);
	Vertices3E[7] = vec3( 4.3f,  10.2f, -0.7f);

//kanan
	Vertices3E[8] = vec3( 4.3f,  6.8f,  2.1f);
	Vertices3E[9] = vec3( 4.3f,  6.8f, -0.7f);
	Vertices3E[10] = vec3( 4.3f,  10.2f, -0.7f);
	Vertices3E[11] = vec3( 4.3f,  10.2f,  2.1f);
//kiri
	Vertices3E[12] = vec3(-1.3f,  6.8f, -0.7f);
	Vertices3E[13] = vec3(-1.3f,  6.8f,  2.1f);
	Vertices3E[14] = vec3(-1.3f,  10.2f,  2.1f);
	Vertices3E[15] = vec3(-1.3f,  10.2f, -0.7f);

//atas
	Vertices3E[16] = vec3(-1.3f,  10.2f,  2.1f);
	Vertices3E[17] = vec3( 4.3f,  10.2f,  2.1f);
	Vertices3E[18] = vec3( 4.3f,  10.2f, -0.7f);
	Vertices3E[19] = vec3(-1.3f,  10.2f, -0.7f);
//bawah
	Vertices3E[20] = vec3(-1.3f,  6.8f, -0.7f);
	Vertices3E[21] = vec3( 4.3f,  6.8f, -0.7f);
	Vertices3E[22] = vec3( 4.3f,  6.8f,  2.1f);
	Vertices3E[23] = vec3(-1.3f,  6.8f,  2.1f);



//--KEPALA2
	Vertices4E = new vec3[24];

//depan
	Vertices4E[0] = vec3(-1.3f,  6.8f,  2.1f);
	Vertices4E[1] = vec3( 4.3f,  6.8f,  2.1f);
	Vertices4E[2] = vec3( 4.3f,  10.2f,  2.1f);
	Vertices4E[3] = vec3(-1.3f,  10.2f,  2.1f);




//--TANGAN KIRI
	Vertices5E = new vec3[24];

//depan
	Vertices5E[0] = vec3(-1.0f,  2.3f,  1.3f);
	Vertices5E[1] = vec3(-0.2f,  2.3f,  1.3f);
	Vertices5E[2] = vec3( -0.2f,  6.3f,  1.3f);
	Vertices5E[3] = vec3(-1.0f,  6.3f,  1.3f);
//belakang
	Vertices5E[4] = vec3( -0.2f,  2.3f, 0.1f);
	Vertices5E[5] = vec3(-1.0f,  2.3f, 0.1f);
	Vertices5E[6] = vec3(-1.0f,  6.3f, 0.1f);
	Vertices5E[7] = vec3( -0.2f,  6.3f, 0.1f);

//kanan
	Vertices5E[8] = vec3( -0.2f,  2.3f,  1.3f);
	Vertices5E[9] = vec3( -0.2f,  2.3f, 0.1f);
	Vertices5E[10] = vec3( -0.2f,  6.3f, 0.1f);
	Vertices5E[11] = vec3( -0.2f,  6.3f,  1.3f);
//kiri
	Vertices5E[12] = vec3(-1.0f,  2.3f, 0.1f);
	Vertices5E[13] = vec3(-1.0f,  2.3f,  1.3f);
	Vertices5E[14] = vec3(-1.0f,  6.3f,  1.3f);
	Vertices5E[15] = vec3(-1.0f,  6.3f, 0.1f);

//atas
	Vertices5E[16] = vec3(-1.0f,  6.3f,  1.3f);
	Vertices5E[17] = vec3( -0.2f,  6.3f,  1.3f);
	Vertices5E[18] = vec3( -0.2f,  6.3f, 0.1f);
	Vertices5E[19] = vec3(-1.0f,  6.3f,  0.1f);
//bawah
	Vertices5E[20] = vec3(-1.0f,  2.3f, 0.1f);
	Vertices5E[21] = vec3( -0.2f,  2.3f,0.1f);
	Vertices5E[22] = vec3( -0.2f,  2.3f,  1.3f);
	Vertices5E[23] = vec3(-1.0f,  2.3f,  1.3f);



//--TANGAN KANAN
	Vertices6E = new vec3[24];

//depan
	Vertices6E[0] = vec3(3.2f,  2.3f,  1.3f);
	Vertices6E[1] = vec3(4.0f,  2.3f,  1.3f);
	Vertices6E[2] = vec3( 4.0f,  6.3f,  1.3f);
	Vertices6E[3] = vec3(3.2f,  6.3f,  1.3f);
//belakang
	Vertices6E[4] = vec3( 4.0f,  2.3f, 0.1f);
	Vertices6E[5] = vec3(3.2f,  2.3f, 0.1f);
	Vertices6E[6] = vec3(3.2f,  6.3f, 0.1f);
	Vertices6E[7] = vec3(4.0f,  6.3f, 0.1f);

//kanan
	Vertices6E[8] = vec3( 4.0f,  2.3f,  1.3f);
	Vertices6E[9] = vec3( 4.0f,  2.3f, 0.1f);
	Vertices6E[10] = vec3( 4.0f,  6.3f, 0.1f);
	Vertices6E[11] = vec3( 4.0f,  6.3f,  1.3f);
//kiri
	Vertices6E[12] = vec3(3.2f,  2.3f, 0.1f);
	Vertices6E[13] = vec3(3.2f,  2.3f,  1.3f);
	Vertices6E[14] = vec3(3.2f,  6.3f,  1.3f);
	Vertices6E[15] = vec3(3.2f,  6.3f, 0.1f);

//atas
	Vertices6E[16] = vec3(3.2f,  6.3f,  1.3f);
	Vertices6E[17] = vec3(4.0f,  6.3f,  1.3f);
	Vertices6E[18] = vec3(4.0f,  6.3f, 0.1f);
	Vertices6E[19] = vec3(3.2f,  6.3f,  0.1f);
//bawah
	Vertices6E[20] = vec3(3.2f,  2.3f, 0.1f);
	Vertices6E[21] = vec3(4.0f,  2.3f,0.1f);
	Vertices6E[22] = vec3(4.0f,  2.3f,  1.3f);
	Vertices6E[23] = vec3(3.2f,  2.3f,  1.3f);
//=END DANBOE===========================================================================




	VerticesD = new vec3[24];
//==DANBOD====================================================================================
//depan
	VerticesD[0] = vec3(-0.0f-80.0f, -0.0f,  1.4f);
	VerticesD[1] = vec3( 1.4f-80.0f, -0.0f,  1.4f);
	VerticesD[2] = vec3( 1.4f-80.0f,  2.8f,  1.4f);
	VerticesD[3] = vec3(-0.0f-80.0f,  2.8f,  1.4f);
//belakang
	VerticesD[4] = vec3( 1.4f-80.0f, -0.0f, -0.0f);
	VerticesD[5] = vec3(-0.0f-80.0f, -0.0f, -0.0f);
	VerticesD[6] = vec3(-0.0f-80.0f,  2.8f, -0.0f);
	VerticesD[7] = vec3( 1.4f-80.0f,  2.8f, -0.0f);

//kanan
	VerticesD[8] = vec3( 1.4f-80.0f, -0.0f,  1.4f);
	VerticesD[9] = vec3( 1.4f-80.0f, -0.0f, -0.0f);
	VerticesD[10] = vec3( 1.4f-80.0f,  2.8f, -0.0f);
	VerticesD[11] = vec3( 1.4f-80.0f,  2.8f,  1.4f);
//kiri
	VerticesD[12] = vec3(-0.0f-80.0f, -0.0f, -0.0f);
	VerticesD[13] = vec3(-0.0f-80.0f, -0.0f,  1.4f);
	VerticesD[14] = vec3(-0.0f-80.0f,  2.8f,  1.4f);
	VerticesD[15] = vec3(-0.0f-80.0f,  2.8f, -0.0f);

//atas
	VerticesD[16] = vec3(-0.0f-80.0f,  2.8f,  1.4f);
	VerticesD[17] = vec3( 1.4f-80.0f,  2.8f,  1.4f);
	VerticesD[18] = vec3( 1.4f-80.0f,  2.8f, -0.0f);
	VerticesD[19] = vec3(-0.0f-80.0f,  2.8f, -0.0f);
//bawah
	VerticesD[20] = vec3(-0.0f-80.0f, -0.0f, -0.0f);
	VerticesD[21] = vec3( 1.4f-80.0f, -0.0f, -0.0f);
	VerticesD[22] = vec3( 1.4f-80.0f, -0.0f,  1.4f);
	VerticesD[23] = vec3(-0.0f-80.0f, -0.0f,  1.4f);

//--KAKI 2
	Vertices1D = new vec3[24];

//depan
	Vertices1D[0] = vec3( 1.6f-80.0f, -0.0f,  1.4f);
	Vertices1D[1] = vec3( 3.0f-80.0f, -0.0f,  1.4f);
	Vertices1D[2] = vec3( 3.0f-80.0f,  2.8f,  1.4f);
	Vertices1D[3] = vec3( 1.6f-80.0f,  2.8f,  1.4f);
//belakang
	Vertices1D[4] = vec3( 3.0f-80.0f, -0.0f, -0.0f);
	Vertices1D[5] = vec3( 1.6f-80.0f, -0.0f, -0.0f);
	Vertices1D[6] = vec3( 1.6f-80.0f,  2.8f, -0.0f);
	Vertices1D[7] = vec3( 3.0f-80.0f,  2.8f, -0.0f);

//kanan
	Vertices1D[8] = vec3( 3.0f-80.0f, -0.0f,  1.4f);
	Vertices1D[9] = vec3( 3.0f-80.0f, -0.0f, -0.0f);
	Vertices1D[10] = vec3( 3.0f-80.0f,  2.8f, -0.0f);
	Vertices1D[11] = vec3( 3.0f-80.0f,  2.8f,  1.4f);
//kiri
	Vertices1D[12] = vec3( 1.6f-80.0f, -0.0f, -0.0f);
	Vertices1D[13] = vec3( 1.6f-80.0f, -0.0f,  1.4f);
	Vertices1D[14] = vec3( 1.6f-80.0f,  2.8f,  1.4f);
	Vertices1D[15] = vec3( 1.6f-80.0f,  2.8f, -0.0f);

//atas
	Vertices1D[16] = vec3( 1.6f-80.0f,  2.8f,  1.4f);
	Vertices1D[17] = vec3( 3.0f-80.0f,  2.8f,  1.4f);
	Vertices1D[18] = vec3( 3.0f-80.0f,  2.8f, -0.0f);
	Vertices1D[19] = vec3( 1.6f-80.0f,  2.8f, -0.0f);
//bawah
	Vertices1D[20] = vec3( 1.6f-80.0f, -0.0f, -0.0f);
	Vertices1D[21] = vec3( 3.0f-80.0f, -0.0f, -0.0f);
	Vertices1D[22] = vec3( 3.0f-80.0f, -0.0f,  1.4f);
	Vertices1D[23] = vec3( 1.6f-80.0f, -0.0f,  1.4f);



//--Badan
	Vertices2D = new vec3[24];

//depan
	Vertices2D[0] = vec3(-0.2f-80.0f,  2.8f,  1.6f);
	Vertices2D[1] = vec3( 3.2f-80.0f,  2.8f,  1.6f);
	Vertices2D[2] = vec3( 3.2f-80.0f,  6.8f,  1.6f);
	Vertices2D[3] = vec3(-0.2f-80.0f,  6.8f,  1.6f);
//belakang
	Vertices2D[4] = vec3( 3.2f-80.0f,  2.8f, -0.2f);
	Vertices2D[5] = vec3(-0.2f-80.0f,  2.8f, -0.2f);
	Vertices2D[6] = vec3(-0.2f-80.0f,  6.8f, -0.2f);
	Vertices2D[7] = vec3( 3.2f-80.0f,  6.8f, -0.2f);

//kanan
	Vertices2D[8] = vec3( 3.2f-80.0f,  2.8f,  1.6f);
	Vertices2D[9] = vec3( 3.2f-80.0f,  2.8f, -0.2f);
	Vertices2D[10] = vec3( 3.2f-80.0f,  6.8f, -0.2f);
	Vertices2D[11] = vec3( 3.2f-80.0f,  6.8f,  1.6f);
//kiri
	Vertices2D[12] = vec3(-0.2f-80.0f,  2.8f, -0.2f);
	Vertices2D[13] = vec3(-0.2f-80.0f,  2.8f,  1.6f);
	Vertices2D[14] = vec3(-0.2f-80.0f,  6.8f,  1.6f);
	Vertices2D[15] = vec3(-0.2f-80.0f,  6.8f, -0.2f);

//atas
	Vertices2D[16] = vec3(-0.2f-80.0f,  6.8f,  1.6f);
	Vertices2D[17] = vec3( 3.2f-80.0f,  6.8f,  1.6f);
	Vertices2D[18] = vec3( 3.2f-80.0f,  6.8f, -0.2f);
	Vertices2D[19] = vec3(-0.2f-80.0f,  6.8f, -0.2f);
//bawah
	Vertices2D[20] = vec3(-0.2f-80.0f,  2.8f, -0.2f);
	Vertices2D[21] = vec3( 3.2f-80.0f,  2.8f, -0.2f);
	Vertices2D[22] = vec3( 3.2f-80.0f,  2.8f,  1.6f);
	Vertices2D[23] = vec3(-0.2f-80.0f,  2.8f,  1.6f);




//--KEPALA1
	Vertices3D = new vec3[24];
/*
//depan
	Vertices3[0] = vec3(-1.3f,  6.8f,  2.1f);
	Vertices3[1] = vec3( 4.3f,  6.8f,  2.1f);
	Vertices3[2] = vec3( 4.3f,  10.2f,  2.1f);
	Vertices3[3] = vec3(-1.3f,  10.2f,  2.1f);
	*/
//belakang
	Vertices3D[4] = vec3( 4.3f-80.0f,  6.8f, -0.7f);
	Vertices3D[5] = vec3(-1.3f-80.0f,  6.8f, -0.7f);
	Vertices3D[6] = vec3(-1.3f-80.0f,  10.2f, -0.7f);
	Vertices3D[7] = vec3( 4.3f-80.0f,  10.2f, -0.7f);

//kanan
	Vertices3D[8] = vec3( 4.3f-80.0f,  6.8f,  2.1f);
	Vertices3D[9] = vec3( 4.3f-80.0f,  6.8f, -0.7f);
	Vertices3D[10] = vec3( 4.3f-80.0f,  10.2f, -0.7f);
	Vertices3D[11] = vec3( 4.3f-80.0f,  10.2f,  2.1f);
//kiri
	Vertices3D[12] = vec3(-1.3f-80.0f,  6.8f, -0.7f);
	Vertices3D[13] = vec3(-1.3f-80.0f,  6.8f,  2.1f);
	Vertices3D[14] = vec3(-1.3f-80.0f,  10.2f,  2.1f);
	Vertices3D[15] = vec3(-1.3f-80.0f,  10.2f, -0.7f);

//atas
	Vertices3D[16] = vec3(-1.3f-80.0f,  10.2f,  2.1f);
	Vertices3D[17] = vec3( 4.3f-80.0f,  10.2f,  2.1f);
	Vertices3D[18] = vec3( 4.3f-80.0f,  10.2f, -0.7f);
	Vertices3D[19] = vec3(-1.3f-80.0f,  10.2f, -0.7f);
//bawah
	Vertices3D[20] = vec3(-1.3f-80.0f,  6.8f, -0.7f);
	Vertices3D[21] = vec3( 4.3f-80.0f,  6.8f, -0.7f);
	Vertices3D[22] = vec3( 4.3f-80.0f,  6.8f,  2.1f);
	Vertices3D[23] = vec3(-1.3f-80.0f,  6.8f,  2.1f);



//--KEPALA2
	Vertices4D = new vec3[24];

//depan
	Vertices4D[0] = vec3(-1.3f-80.0f,  6.8f,  2.1f);
	Vertices4D[1] = vec3( 4.3f-80.0f,  6.8f,  2.1f);
	Vertices4D[2] = vec3( 4.3f-80.0f,  10.2f,  2.1f);
	Vertices4D[3] = vec3(-1.3f-80.0f,  10.2f,  2.1f);




//--TANGAN KIRI
	Vertices5D = new vec3[24];

//depan
	Vertices5D[0] = vec3(-1.0f-80.0f,  2.3f,  1.3f);
	Vertices5D[1] = vec3(-0.2f-80.0f,  2.3f,  1.3f);
	Vertices5D[2] = vec3( -0.2f-80.0f,  6.3f,  1.3f);
	Vertices5D[3] = vec3(-1.0f-80.0f,  6.3f,  1.3f);
//belakang
	Vertices5D[4] = vec3( -0.2f-80.0f,  2.3f, 0.1f);
	Vertices5D[5] = vec3(-1.0f-80.0f,  2.3f, 0.1f);
	Vertices5D[6] = vec3(-1.0f-80.0f,  6.3f, 0.1f);
	Vertices5D[7] = vec3( -0.2f-80.0f,  6.3f, 0.1f);

//kanan
	Vertices5D[8] = vec3( -0.2f-80.0f,  2.3f,  1.3f);
	Vertices5D[9] = vec3( -0.2f-80.0f,  2.3f, 0.1f);
	Vertices5D[10] = vec3( -0.2f-80.0f,  6.3f, 0.1f);
	Vertices5D[11] = vec3( -0.2f-80.0f,  6.3f,  1.3f);
//kiri
	Vertices5D[12] = vec3(-1.0f-80.0f,  2.3f, 0.1f);
	Vertices5D[13] = vec3(-1.0f-80.0f,  2.3f,  1.3f);
	Vertices5D[14] = vec3(-1.0f-80.0f,  6.3f,  1.3f);
	Vertices5D[15] = vec3(-1.0f-80.0f,  6.3f, 0.1f);

//atas
	Vertices5D[16] = vec3(-1.0f-80.0f,  6.3f,  1.3f);
	Vertices5D[17] = vec3( -0.2f-80.0f,  6.3f,  1.3f);
	Vertices5D[18] = vec3( -0.2f-80.0f,  6.3f, 0.1f);
	Vertices5D[19] = vec3(-1.0f-80.0f,  6.3f,  0.1f);
//bawah
	Vertices5D[20] = vec3(-1.0f-80.0f,  2.3f, 0.1f);
	Vertices5D[21] = vec3( -0.2f-80.0f,  2.3f,0.1f);
	Vertices5D[22] = vec3( -0.2f-80.0f,  2.3f,  1.3f);
	Vertices5D[23] = vec3(-1.0f-80.0f,  2.3f,  1.3f);



//--TANGAN KANAN
	Vertices6D = new vec3[24];

//depan
	Vertices6D[0] = vec3(3.2f-80.0f,  2.3f,  1.3f);
	Vertices6D[1] = vec3(4.0f-80.0f,  2.3f,  1.3f);
	Vertices6D[2] = vec3( 4.0f-80.0f,  6.3f,  1.3f);
	Vertices6D[3] = vec3(3.2f-80.0f,  6.3f,  1.3f);
//belakang
	Vertices6D[4] = vec3( 4.0f-80.0f,  2.3f, 0.1f);
	Vertices6D[5] = vec3(3.2f-80.0f,  2.3f, 0.1f);
	Vertices6D[6] = vec3(3.2f-80.0f,  6.3f, 0.1f);
	Vertices6D[7] = vec3(4.0f-80.0f,  6.3f, 0.1f);

//kanan
	Vertices6D[8] = vec3( 4.0f-80.0f,  2.3f,  1.3f);
	Vertices6D[9] = vec3( 4.0f-80.0f,  2.3f, 0.1f);
	Vertices6D[10] = vec3( 4.0f-80.0f,  6.3f, 0.1f);
	Vertices6D[11] = vec3( 4.0f-80.0f,  6.3f,  1.3f);
//kiri
	Vertices6D[12] = vec3(3.2f-80.0f,  2.3f, 0.1f);
	Vertices6D[13] = vec3(3.2f-80.0f,  2.3f,  1.3f);
	Vertices6D[14] = vec3(3.2f-80.0f,  6.3f,  1.3f);
	Vertices6D[15] = vec3(3.2f-80.0f,  6.3f, 0.1f);

//atas
	Vertices6D[16] = vec3(3.2f-80.0f,  6.3f,  1.3f);
	Vertices6D[17] = vec3(4.0f-80.0f,  6.3f,  1.3f);
	Vertices6D[18] = vec3(4.0f-80.0f,  6.3f, 0.1f);
	Vertices6D[19] = vec3(3.2f-80.0f,  6.3f,  0.1f);
//bawah
	Vertices6D[20] = vec3(3.2f-80.0f,  2.3f, 0.1f);
	Vertices6D[21] = vec3(4.0f-80.0f,  2.3f,0.1f);
	Vertices6D[22] = vec3(4.0f-80.0f,  2.3f,  1.3f);
	Vertices6D[23] = vec3(3.2f-80.0f,  2.3f,  1.3f);
//=END DANBOD===========================================================================


















//==RUMAH=======================================================================

//--
	VerRumah = new vec3[24];

//depan
	VerRumah[0] = vec3(-30.0f,  0.0f,  -51.3f);
	VerRumah[1] = vec3(-50.0f,  0.0f,  -51.3f);
	VerRumah[2] = vec3( -50.0f,  26.3f,  -51.3f);
	VerRumah[3] = vec3(-30.0f,  26.3f,  -51.3f);
//belakang
	VerRumah[4] = vec3( -50.0f,  0.0f, -20.1f);
	VerRumah[5] = vec3(-30.0f,  0.0f, -20.1f);
	VerRumah[6] = vec3(-30.0f,  26.3f, -20.1f);
	VerRumah[7] = vec3(-50.0f,  26.3f, -20.1f);

//kanan
	VerRumah[8] = vec3( -50.0f,  0.0f,  -51.3f);
	VerRumah[9] = vec3( -50.0f,  0.0f, -20.1f);
	VerRumah[10] = vec3( -50.0f,  26.3f,-20.1f);
	VerRumah[11] = vec3( -50.0f,  26.3f,  -51.3f);
//kiri
	VerRumah[12] = vec3(-30.0f,  0.0f, -20.1f);
	VerRumah[13] = vec3(-30.0f,  0.0f,  -51.3f);
	VerRumah[14] = vec3(-30.0f,  26.3f,  -51.3f);
	VerRumah[15] = vec3(-30.0f,  26.3f, -20.1f);

//atas
	VerRumah[16] = vec3(-30.0f,  26.3f,  -51.3f);
	VerRumah[17] = vec3(-50.0f,  26.3f,  -51.3f);
	VerRumah[18] = vec3(-50.0f,  26.3f, -20.1f);
	VerRumah[19] = vec3(-30.0f,  26.3f,  -20.1f);
//bawah
	VerRumah[20] = vec3(-30.0f,  0.0f, -20.1f);
	VerRumah[21] = vec3(-50.0f,  0.0f,-20.1f);
	VerRumah[22] = vec3(-50.0f,  0.0f,  -51.3f);
	VerRumah[23] = vec3(-30.0f,  0.0f,  -51.3f);
//======================================================================



//==LANTAI=======================================================================

//--
    TexCoordsRumput = new vec2[24];
	VerRumput = new vec3[24];
    
//atas
    for(i=0;i<24;i++){  
      TexCoordsRumput[i] = vec2(0.0f, 0.0f); 
      i++;
      TexCoordsRumput[i] = vec2(1.0f, 0.0f); 
      i++;
	  TexCoordsRumput[i] = vec2(1.0f, 1.0f); 
	  i++;
	  TexCoordsRumput[i] = vec2(0.0f, 1.0f); 
    }

//atas

	VerRumput[0] = vec3(-60.0f,  0.1f,  -50.0f);
	VerRumput[1] = vec3(-110.0f,  0.1f,  -50.0f);
	VerRumput[2] = vec3(-110.0f,  0.1f, -0.0f);
	VerRumput[3] = vec3(-60.0f,  0.1f, -0.0f);	
	
	VerRumput[4] = vec3(0.0f,  0.1f,  -50.0f);
	VerRumput[5] = vec3(-50.0f,  0.1f,  -50.0f);
	VerRumput[6] = vec3(-50.0f,  0.1f, 0.0f);
	VerRumput[7] = vec3(0.0f,  0.1f, 0.0f);
		
	VerRumput[8] = vec3(60.0f,  0.1f,  -50.0f);
	VerRumput[9] = vec3(10.0f,  0.1f,  -50.0f);
	VerRumput[10] = vec3(10.0f,  0.1f, -0.0f);
	VerRumput[11] = vec3(60.0f,  0.1f, -0.0f);	

	VerRumput[12] = vec3(120.0f,  0.1f,  -50.0f);
	VerRumput[13] = vec3(70.0f,  0.1f,  -50.0f);
	VerRumput[14] = vec3(70.0f,  0.1f, -0.0f);
	VerRumput[15] = vec3(120.0f,  0.1f, -0.0f);	

	VerRumput[16] = vec3(180.0f,  0.1f,  -50.0f);
	VerRumput[17] = vec3(130.0f,  0.1f,  -50.0f);
	VerRumput[18] = vec3(130.0f,  0.1f, -0.0f);
	VerRumput[19] = vec3(180.0f,  0.1f, -0.0f);	

	VerRumput[20] = vec3(240.0f,  0.1f,  -50.0f);
	VerRumput[21] = vec3(190.0f,  0.1f,  -50.0f);
	VerRumput[22] = vec3(190.0f,  0.1f, -0.0f);
	VerRumput[23] = vec3(240.0f,  0.1f, -0.0f);	
//======================================================================


//==lantai=======================================================================

//--
TexCoordsRumput2 = new vec2[24];
	VerRumput2 = new vec3[24];
    
//atas
    for(i=0;i<24;i++){  
	  TexCoordsRumput2[i] = vec2(0.0f, 0.0f); 
	  i++;
	  TexCoordsRumput2[i] = vec2(1.0f, 0.0f); 
	  i++;
	  TexCoordsRumput2[i] = vec2(1.0f, 1.0f); 
	  i++;
	  TexCoordsRumput2[i] = vec2(0.0f, 1.0f); 
    }

//atas

	VerRumput2[0] = vec3(-60.0f,  0.1f,  10.0f);
	VerRumput2[1] = vec3(-110.0f,  0.1f,  10.0f);
	VerRumput2[2] = vec3(-110.0f,  0.1f, 60.0f);
	VerRumput2[3] = vec3(-60.0f,  0.1f, 60.0f);	
	
	VerRumput2[4] = vec3(0.0f,  0.1f,  10.0f);
	VerRumput2[5] = vec3(-50.0f,  0.1f,  10.0f);
	VerRumput2[6] = vec3(-50.0f,  0.1f, 60.0f);
	VerRumput2[7] = vec3(0.0f,  0.1f, 60.0f);
	
	VerRumput2[8] = vec3(60.0f,  0.1f,  10.0f);
	VerRumput2[9] = vec3(10.0f,  0.1f,  10.0f);
	VerRumput2[10] = vec3(10.0f,  0.1f, 60.0f);
	VerRumput2[11] = vec3(60.0f,  0.1f, 60.0f);	

	VerRumput2[12] = vec3(120.0f,  0.1f,  10.0f);
	VerRumput2[13] = vec3(70.0f,  0.1f,  10.0f);
	VerRumput2[14] = vec3(70.0f,  0.1f, 60.0f);
	VerRumput2[15] = vec3(120.0f,  0.1f, 60.0f);	

	VerRumput2[16] = vec3(180.0f,  0.1f,  10.0f);
	VerRumput2[17] = vec3(130.0f,  0.1f,  10.0f);
	VerRumput2[18] = vec3(130.0f,  0.1f, 60.0f);
	VerRumput2[19] = vec3(180.0f,  0.1f, 60.0f);	

	VerRumput2[20] = vec3(240.0f,  0.1f,  10.0f);
	VerRumput2[21] = vec3(190.0f,  0.1f,  10.0f);
	VerRumput2[22] = vec3(190.0f,  0.1f, 60.0f);
	VerRumput2[23] = vec3(240.0f,  0.1f, 60.0f);	
//======================================================================

//==RUMPUT3=======================================================================

//--
    TexCoordsRumput3 = new vec2[24];
	VerRumput3 = new vec3[24];
    
//atas

    TexCoordsRumput3[0] = vec2(0.0f, 0.0f); 
	TexCoordsRumput3[1] = vec2(10.0f, 0.0f); 
	TexCoordsRumput3[2] = vec2(10.0f, 10.0f); 
	TexCoordsRumput3[3] = vec2(0.0f, 10.0f); 

	TexCoordsRumput3[4] = vec2(0.0f, 0.0f); 
	TexCoordsRumput3[5] = vec2(1.0f, 0.0f); 
	TexCoordsRumput3[6] = vec2(1.0f, 1.0f); 
	TexCoordsRumput3[7] = vec2(0.0f, 1.0f); 

	TexCoordsRumput3[8] = vec2(0.0f, 0.0f); 
	TexCoordsRumput3[9] = vec2(1.0f, 0.0f); 
	TexCoordsRumput3[10] = vec2(1.0f, 1.0f); 
	TexCoordsRumput3[11] = vec2(0.0f, 1.0f); 
	
	TexCoordsRumput3[12] = vec2(0.0f, 0.0f); 
	TexCoordsRumput3[13] = vec2(1.0f, 0.0f); 
	TexCoordsRumput3[14] = vec2(1.0f, 1.0f); 
	TexCoordsRumput3[15] = vec2(0.0f, 1.0f); 

	TexCoordsRumput3[16] = vec2(0.0f, 0.0f); 
	TexCoordsRumput3[17] = vec2(1.0f, 0.0f); 
	TexCoordsRumput3[18] = vec2(1.0f, 1.0f); 
	TexCoordsRumput3[19] = vec2(0.0f, 1.0f); 
	
	TexCoordsRumput3[20] = vec2(0.0f, 0.0f); 
	TexCoordsRumput3[21] = vec2(1.0f, 0.0f); 
	TexCoordsRumput3[22] = vec2(1.0f, 1.0f); 
	TexCoordsRumput3[23] = vec2(0.0f, 1.0f); 
//atas

	VerRumput3[0] = vec3(500.0f,  0.0f,  -500.0f);
	VerRumput3[1] = vec3(-500.0f,  0.0f,  -500.0f);
	VerRumput3[2] = vec3(-500.0f,  0.0f, 500.0f);
	VerRumput3[3] = vec3(500.0f,  0.0f, 500.0f);	

//==JALAN==========================================================================
    /*TexJalan = new vec2[24];
	VerJalan = new vec3[24];
    
//atas
    for(i=0;i<24;i++){  
	  TexJalan[i] = vec2(0.0f, 0.0f); 
      i++;
	  TexJalan[i] = vec2(1.0f, 0.0f); 
	  i++;
	  TexJalan[i] = vec2(1.0f, 25.0f); 
	  i++;
	  TexJalan[i] = vec2(0.0f, 25.0f); 
    }
//atas

	VerJalan[0] = vec3(460.0f,  0.2f,  -500.0f);
	VerJalan[1] = vec3(280.0f,  0.2f,  -500.0f);
	VerJalan[2] = vec3(280.0f,  0.2f, 500.0f);
	VerJalan[3] = vec3(460.0f,  0.2f, 500.0f);
*/
//=================================================================================


//==TANGGGA1=======================================================================

//--
    TexTangga1 = new vec2[24];
	VerTangga1 = new vec3[24];
    
//atas
    for(i=0;i<24;i++){  
	  TexTangga1[i] = vec2(0.0f, 0.0f); 
	  i++;
	  TexTangga1[i] = vec2(0.5f, 0.0f); 
	  i++;
	  TexTangga1[i] = vec2(0.5f, 0.5f); 
	  i++;
	  TexTangga1[i] = vec2(0.0f, 0.5f); 
    }


//kanan
	VerTangga1[0] = vec3( -120.0f,  0.0f,  70.0f);
	VerTangga1[1] = vec3( -120.0f,  0.0f, -70.0f);
	VerTangga1[2] = vec3( -120.0f,  20.0f,-70.0f);
	VerTangga1[3] = vec3( -120.0f,  20.0f,  70.0f);
	
//atas
	VerTangga1[4] = vec3(-120.0f,  20.0f,  -70.0f);
	VerTangga1[5] = vec3(-150.0f,  20.0f,  -70.0f);
	VerTangga1[6] = vec3(-150.0f,  20.0f, 70.0f);
	VerTangga1[7] = vec3(-120.0f,  20.0f,  70.0f);

//kanan
	VerTangga1[8] = vec3( -150.0f,  20.0f,  70.0f);
	VerTangga1[9] = vec3( -150.0f,  20.0f, -70.0f);
	VerTangga1[10] = vec3( -150.0f,  40.0f,-70.0f);
	VerTangga1[11] = vec3( -150.0f,  40.0f,  70.0f);

//atas
	VerTangga1[12] = vec3(-150.0f,  40.0f,  -70.0f);
	VerTangga1[13] = vec3(-180.0f,  40.0f,  -70.0f);
	VerTangga1[14] = vec3(-180.0f,  40.0f, 70.0f);
	VerTangga1[15] = vec3(-150.0f,  40.0f,  70.0f);
    	
//======================================================================

//==TANGGGA1=======================================================================

//--
TexTangga2 = new vec2[80];
	VerTangga2 = new vec3[80];
    
//atas
    for(i=0;i<80;i++){           
	  TexTangga2[i] = vec2(0.0f, 0.0f); 
	  i++;
	  TexTangga2[i] = vec2(2.5f, 0.0f); 
	  i++;
	  TexTangga2[i] = vec2(2.5f, 2.5f); 
	  i++;
	  TexTangga2[i] = vec2(0.0f, 2.5f); 
    }
    

//kanan
	VerTangga2[0] = vec3( -130.0f,  0.0f,  68.0f);
	VerTangga2[1] = vec3( -130.0f,  0.0f, 60.0f);
	VerTangga2[2] = vec3( -130.0f,  80.0f,60.0f);
	VerTangga2[3] = vec3( -130.0f,  80.0f,  68.0f);


//depan
	VerTangga2[16] = vec3(-130.0f,  0.0f,  60.0f);
	VerTangga2[17] = vec3(-138.0f,  0.0f,  60.0f);
	VerTangga2[18] = vec3( -138.0f,  80.0f, 60.0f);
	VerTangga2[19] = vec3(-130.0f,  80.0f,  60.0f);
	
//belakang
	VerTangga2[20] = vec3( -138.0f,  0.0f, 68.0f);
	VerTangga2[21] = vec3(-130.0f,  0.0f, 68.0f);
	VerTangga2[22] = vec3(-130.0f,  80.0f, 68.0f);
	VerTangga2[23] = vec3(-138.0f,  80.0f, 68.0f);
	
//kiri
	VerTangga2[24] = vec3(-130.0f,  0.0f, 68.0f);
	VerTangga2[25] = vec3(-130.0f,  0.0f,  60.0f);
	VerTangga2[26] = vec3(-130.0f,  80.0f,  60.0f);
	VerTangga2[27] = vec3(-130.0f,  80.0f, 68.0f);

//atas
	VerTangga2[28] = vec3(-130.0f,  80.0f, 60.0f);
	VerTangga2[29] = vec3(-138.0f,  80.0f, 60.0f);
	VerTangga2[30] = vec3(-138.0f,  80.0f, 68.0f);
	VerTangga2[31] = vec3(-130.0f,  80.0f,  68.0f);

//-------

//kanan
	VerTangga2[4] = vec3( -130.0f,  0.0f,  -60.0f);
	VerTangga2[5] = vec3( -130.0f,  0.0f, -68.0f);
	VerTangga2[6] = vec3( -130.0f,  80.0f,-68.0f);
	VerTangga2[7] = vec3( -130.0f,  80.0f,  -60.0f);

//depan
	VerTangga2[32] = vec3(-130.0f,  0.0f,  -68.0f);
	VerTangga2[33] = vec3(-138.0f,  0.0f,  -68.0f);
	VerTangga2[34] = vec3( -138.0f,  80.0f, -68.0f);
	VerTangga2[35] = vec3(-130.0f,  80.0f,  -68.0f);
	
//belakang
	VerTangga2[36] = vec3( -138.0f,  0.0f, -60.0f);
	VerTangga2[37] = vec3(-130.0f,  0.0f, -60.0f);
	VerTangga2[38] = vec3(-130.0f,  80.0f, -60.0f);
	VerTangga2[39] = vec3(-138.0f,  80.0f, -60.0f);
	
//kiri
	VerTangga2[40] = vec3(-130.0f,  0.0f, -60.0f);
	VerTangga2[41] = vec3(-130.0f,  0.0f,  -68.0f);
	VerTangga2[42] = vec3(-130.0f,  80.0f,  -68.0f);
	VerTangga2[43] = vec3(-130.0f,  80.0f, -60.0f);

//atas
	VerTangga2[44] = vec3(-130.0f,  80.0f, -68.0f);
	VerTangga2[45] = vec3(-138.0f,  80.0f, -68.0f);
	VerTangga2[46] = vec3(-138.0f,  80.0f, -60.0f);
	VerTangga2[47] = vec3(-130.0f,  80.0f,  -60.0f);
//-------

//kanan
	VerTangga2[8] = vec3( -160.0f,  0.0f,  68.0f);
	VerTangga2[9] = vec3( -160.0f,  0.0f, 60.0f);
	VerTangga2[10] = vec3( -160.0f,  100.0f,60.0f);
	VerTangga2[11] = vec3( -160.0f,  100.0f,  68.0f);

//depan
	VerTangga2[48] = vec3(-160.0f,  0.0f,  60.0f);
	VerTangga2[49] = vec3(-168.0f,  0.0f,  60.0f);
	VerTangga2[50] = vec3( -168.0f,  100.0f, 60.0f);
	VerTangga2[51] = vec3(-160.0f,  100.0f,  60.0f);
	
//belakang
	VerTangga2[52] = vec3( -168.0f,  0.0f, 68.0f);
	VerTangga2[53] = vec3(-160.0f,  0.0f,  68.0f);
	VerTangga2[54] = vec3(-160.0f,  100.0f, 68.0f);
	VerTangga2[55] = vec3(-168.0f,  100.0f, 68.0f);
	
//kiri
	VerTangga2[56] = vec3(-160.0f,  0.0f,  68.0f);
	VerTangga2[57] = vec3(-160.0f,  0.0f,  60.0f);
	VerTangga2[58] = vec3(-160.0f,  100.0f, 60.0f);
	VerTangga2[59] = vec3(-160.0f,  100.0f, 68.0f);

//atas
	VerTangga2[60] = vec3(-160.0f,  100.0f,  60.0f);
	VerTangga2[61] = vec3(-168.0f,  100.0f, 60.0f);
	VerTangga2[62] = vec3(-168.0f,  100.0f, 68.0f);
	VerTangga2[63] = vec3(-160.0f,  100.0f,  68.0f);
//-------

//kanan
	VerTangga2[12] = vec3( -160.0f,  0.0f,  -60.0f);
	VerTangga2[13] = vec3( -160.0f,  0.0f, -68.0f);
	VerTangga2[14] = vec3( -160.0f,  100.0f,-68.0f);
	VerTangga2[15] = vec3( -160.0f,  100.0f,  -60.0f);	

//depan
	VerTangga2[64] = vec3(-160.0f,  0.0f,  -68.0f);
	VerTangga2[65] = vec3(-168.0f,  0.0f,  -68.0f);
	VerTangga2[66] = vec3( -168.0f,  100.0f, -68.0f);
	VerTangga2[67] = vec3(-160.0f,  100.0f,  -68.0f);
	
//belakang
	VerTangga2[68] = vec3( -168.0f,  0.0f, -60.0f);
	VerTangga2[69] = vec3(-160.0f,  0.0f,  -60.0f);
	VerTangga2[70] = vec3(-160.0f,  100.0f, -60.0f);
	VerTangga2[71] = vec3(-168.0f,  100.0f, -60.0f);
	
//kiri
	VerTangga2[72] = vec3(-160.0f,  0.0f,  -60.0f);
	VerTangga2[73] = vec3(-160.0f,  0.0f,  -68.0f);
	VerTangga2[74] = vec3(-160.0f,  100.0f, -68.0f);
	VerTangga2[75] = vec3(-160.0f,  100.0f, -60.0f);

//atas
	VerTangga2[76] = vec3(-160.0f,  100.0f,  -68.0f);
	VerTangga2[77] = vec3(-168.0f,  100.0f, -68.0f);
	VerTangga2[78] = vec3(-168.0f,  100.0f, -60.0f);
	VerTangga2[79] = vec3(-160.0f,  100.0f, -60.0f);
//-------
	
//======================================================================
//--TANGGA3
//--
    //TexKusen1 = new vec2[24];
	VerTangga3 = new vec3[24];

//depan
	VerTangga3[0] = vec3(-180.0f,  110.0f,  71.0f);
	VerTangga3[1] = vec3(-115.0f,  65.0f,  71.0f);
	VerTangga3[2] = vec3(-115.0f,  70.0f,  71.0f);
	VerTangga3[3] = vec3(-180.0f,  115.0f,  71.0f);

//belakang
	VerTangga3[4] = vec3(-115.0f,  65.0f, 55.0f);
	VerTangga3[5] = vec3(-180.0f,  110.0f, 55.0f);
	VerTangga3[6] = vec3(-180.0f,  115.0f, 55.0f);
	VerTangga3[7] = vec3(-115.0f,  70.0f, 55.0f);

//kanan
	VerTangga3[8] = vec3( -115.0f,  65.0f,  71.0f);
	VerTangga3[9] = vec3( -115.0f,  65.0f, 55.0f);
	VerTangga3[10] = vec3(-115.0f,  70.0f, 55.0f);
	VerTangga3[11] = vec3(-115.0f,  70.0f,  71.0f);
//kiri
	VerTangga3[12] = vec3(-180.0f,  110.0f, 55.0f);
	VerTangga3[13] = vec3(-180.0f,  110.0f,  71.0f);
	VerTangga3[14] = vec3(-180.0f,  115.0f,  71.0f);
	VerTangga3[15] = vec3(-180.0f,  115.0f, 55.0f);

//atas
	VerTangga3[16] = vec3(-180.0f,  115.0f,  71.0f);
	VerTangga3[17] = vec3(-115.0f,  70.0f,  71.0f);
	VerTangga3[18] = vec3(-115.0f,  70.0f, 55.0f);
	VerTangga3[19] = vec3(-180.0f,  115.0f, 55.0f);
//bawah
	VerTangga3[20] = vec3(-180.0f,  110.0f, 55.0f);
	VerTangga3[21] = vec3(-115.0f,  65.0f, 55.0f);
	VerTangga3[22] = vec3(-115.0f,  65.0f,  71.0f);
	VerTangga3[23] = vec3(-180.0f,  110.0f,  71.0f);
	
//========================

//--TANGGA3
//--
    //TexKusen1 = new vec2[24];
	VerTangga4 = new vec3[24];

//depan
	VerTangga4[0] = vec3(-180.0f,  110.0f,  -55.0f);
	VerTangga4[1] = vec3(-115.0f,  65.0f,  -55.0f);
	VerTangga4[2] = vec3(-115.0f,  70.0f,  -55.0f);
	VerTangga4[3] = vec3(-180.0f,  115.0f,  -55.0f);

//belakang
	VerTangga4[4] = vec3(-115.0f,  65.0f, -71.0f);
	VerTangga4[5] = vec3(-180.0f,  110.0f, -71.0f);
	VerTangga4[6] = vec3(-180.0f,  115.0f, -71.0f);
	VerTangga4[7] = vec3(-115.0f,  70.0f, -71.0f);

//kanan
	VerTangga4[8] = vec3( -115.0f,  65.0f,  -55.0f);
	VerTangga4[9] = vec3( -115.0f,  65.0f, -71.0f);
	VerTangga4[10] = vec3(-115.0f,  70.0f, -71.0f);
	VerTangga4[11] = vec3(-115.0f,  70.0f,  -55.0f);
//kiri
	VerTangga4[12] = vec3(-180.0f,  110.0f, -71.0f);
	VerTangga4[13] = vec3(-180.0f,  110.0f,  -55.0f);
	VerTangga4[14] = vec3(-180.0f,  115.0f,  -55.0f);
	VerTangga4[15] = vec3(-180.0f,  115.0f, -71.0f);

//atas
	VerTangga4[16] = vec3(-180.0f,  115.0f,  -55.0f);
	VerTangga4[17] = vec3(-115.0f,  70.0f,  -55.0f);
	VerTangga4[18] = vec3(-115.0f,  70.0f, -71.0f);
	VerTangga4[19] = vec3(-180.0f,  115.0f, -71.0f);
//bawah
	VerTangga4[20] = vec3(-180.0f,  110.0f, -71.0f);
	VerTangga4[21] = vec3(-115.0f,  65.0f, -71.0f);
	VerTangga4[22] = vec3(-115.0f,  65.0f,  -55.0f);
	VerTangga4[23] = vec3(-180.0f,  110.0f,  -55.0f);
	
//========================



//==BATA=======================================================================

//--
    TexBata = new vec2[24];
	VerBata = new vec3[24];
    
//atas
	TexBata[0] = vec2(0.0f, 0.0f); 
	TexBata[1] = vec2(7.8f, 0.0f); 
	TexBata[2] = vec2(7.8f, 0.5f); 
	TexBata[3] = vec2(0.0f, 0.5f); 

	TexBata[4] = vec2(0.0f, 0.0f); 
	TexBata[5] = vec2(2.0f, 0.0f); 
	TexBata[6] = vec2(2.0f, 2.0f); 
	TexBata[7] = vec2(0.0f, 2.0f); 

	TexBata[8] = vec2(0.0f, 0.0f); 
	TexBata[9] = vec2(2.0f, 0.0f); 
	TexBata[10] = vec2(2.0f, 2.0f); 
	TexBata[11] = vec2(0.0f, 2.0f); 
	
	TexBata[12] = vec2(0.0f, 0.0f); 
	TexBata[13] = vec2(2.0f, 0.0f); 
	TexBata[14] = vec2(2.0f, 2.0f); 
	TexBata[15] = vec2(0.0f, 2.0f); 

	TexBata[16] = vec2(0.0f, 0.0f); 
	TexBata[17] = vec2(2.0f, 0.0f); 
	TexBata[18] = vec2(2.0f, 2.0f); 
	TexBata[19] = vec2(0.0f, 2.0f); 
	
	TexBata[20] = vec2(0.0f, 0.0f); 
	TexBata[21] = vec2(2.0f, 0.0f); 
	TexBata[22] = vec2(2.0f, 2.0f); 
	TexBata[23] = vec2(0.0f, 2.0f); 


//kanan
	VerBata[0] = vec3( -180.0f,  0.0f,  200.0f);
	VerBata[1] = vec3( -180.0f,  0.0f, -200.0f);
	VerBata[2] = vec3( -180.0f,  40.0f,-200.0f);
	VerBata[3] = vec3( -180.0f,  40.0f,  200.0f);

//atas
	VerBata[4] = vec3(-180.0f,  40.0f,  -200.0f);
	VerBata[5] = vec3(-460.0f,  40.0f,  -200.0f);
	VerBata[6] = vec3(-460.0f,  40.0f, 200.0f);
	VerBata[7] = vec3(-180.0f,  40.0f,  200.0f);    	
//======================================================================



//--TEMBOK
//--
    TexTembok = new vec2[24];
	VerTembok = new vec3[24];
    
//atas
    for(i=0;i<24;i++){  
	  TexTembok[i] = vec2(0.0f, 0.0f); 
      i++;
	  TexTembok[i] = vec2(7.8f, 0.0f); 
	  i++;
	  TexTembok[i] = vec2(7.8f, 2.0f); 
	  i++;
	  TexTembok[i] = vec2(0.0f, 2.0f); 
    }


//depan
	VerTembok[0] = vec3(-460.0f,  0.0f,  200.0f);
	VerTembok[1] = vec3( -180.0f,  0.0f,  200.0f);
	VerTembok[2] = vec3( -180.0f,  260.0f,  200.0f);
	VerTembok[3] = vec3(-460.0f,  260.0f,  200.0f);
//belakang
	VerTembok[4] = vec3( -180.0f,  0.0f, -200.0f);
	VerTembok[5] = vec3(-460.0f,  0.0f, -200.0f);
	VerTembok[6] = vec3(-460.0f,  260.0f, -200.0f);
	VerTembok[7] = vec3( -180.0f,  260.0f, -200.0f);

//kanan
	VerTembok[8] = vec3( -180.0f,  0.0f,  200.0f);
	VerTembok[9] = vec3( -180.0f,  0.0f, -200.0f);
	VerTembok[10] = vec3(-180.0f,  260.0f, -200.0f);
	VerTembok[11] = vec3(-180.0f,  260.0f,  200.0f);
//kiri
	VerTembok[12] = vec3(-460.0f,  0.0f, -200.0f);
	VerTembok[13] = vec3(-460.0f,  0.0f,  200.0f);
	VerTembok[14] = vec3(-460.0f,  260.0f,  200.0f);
	VerTembok[15] = vec3(-460.0f,  260.0f, -200.0f);

//atas
	VerTembok[16] = vec3(-460.0f,  260.0f,  200.0f);
	VerTembok[17] = vec3(-180.0f,  260.0f,  200.0f);
	VerTembok[18] = vec3( -180.0f, 260.0f, -200.0f);
	VerTembok[19] = vec3(-460.0f,  260.0f, -200.0f);
//bawah
	VerTembok[20] = vec3(-460.0f,  0.0f, -200.0f);
	VerTembok[21] = vec3(-180.0f,  0.0f, -200.0f);
	VerTembok[22] = vec3(-180.0f,  0.0f,  200.0f);
	VerTembok[23] = vec3(-460.0f,  0.0f,  200.0f);
//========================

//======================================================================



//--TEMBOK
//--
    TexAtap = new vec2[24];
	VerAtap = new vec3[24];
 	VerAtap2 = new vec3[24];   
//atas
    for(i=0;i<24;i++){  
	  TexAtap[i] = vec2(0.0f, 0.0f); 
      i++;
	  TexAtap[i] = vec2(25.0f, 0.0f); 
	  i++;
	  TexAtap[i] = vec2(5.0f, 20.0f); 
	  i++;
	  TexAtap[i] = vec2(0.0f, 20.0f); 
    }


//depan
	VerAtap[0] = vec3(-540.0f,  240.0f,  280.0f);
	VerAtap[1] = vec3( -100.0f,  240.0f,  280.0f);
	VerAtap[2] = vec3( -100.0f,  360.0f,  40.0f);
	VerAtap[3] = vec3(-540.0f,  360.0f,  40.0f);
//belakang
	VerAtap[4] = vec3( -100.0f,  240.0f, -280.0f);
	VerAtap[5] = vec3(-540.0f,  240.0f, -280.0f);
	VerAtap[6] = vec3(-540.0f,  360.0f, -40.0f);
	VerAtap[7] = vec3( -100.0f,  360.0f, -40.0f);

//kanan
	VerAtap[8] = vec3( -100.0f,  240.0f,  280.0f);
	VerAtap[9] = vec3( -100.0f,  240.0f, -280.0f);
	VerAtap[10] = vec3(-280.0f,  360.0f, -40.0f);
	VerAtap[11] = vec3(-280.0f,  360.0f,  40.0f);
//kiri
	VerAtap[12] = vec3(-540.0f,  240.0f, -280.0f);
	VerAtap[13] = vec3(-540.0f,  240.0f,  280.0f);
	VerAtap[14] = vec3(-360.0f,  360.0f,  40.0f);
	VerAtap[15] = vec3(-360.0f,  360.0f, -40.0f);

//atas
	VerAtap[16] = vec3(-360.0f,  360.0f,  40.0f);
	VerAtap[17] = vec3(-280.0f,  360.0f,  40.0f);
	VerAtap[18] = vec3( -280.0f, 360.0f, -40.0f);
	VerAtap[19] = vec3(-360.0f,  360.0f, -40.0f);
//bawah
	VerAtap[20] = vec3(-540.0f,  240.0f, -280.0f);
	VerAtap[21] = vec3(-100.0f,  240.0f, -280.0f);
	VerAtap[22] = vec3(-100.0f,  240.0f,  280.0f);
	VerAtap[23] = vec3(-540.0f,  240.0f,  280.0f);

/*
//depan
	VerAtap2[0] = vec3(-540.0f,  230.0f,  280.0f);
	VerAtap2[1] = vec3( -100.0f,  230.0f,  280.0f);
	VerAtap2[2] = vec3( -100.0f,  240.0f,  280.0f);
	VerAtap2[3] = vec3(-540.0f,  240.0f,  280.0f);
//belakang
	VerAtap2[4] = vec3( -100.0f,  230.0f, -280.0f);
	VerAtap2[5] = vec3(-540.0f,  230.0f, -280.0f);
	VerAtap2[6] = vec3(-540.0f,  240.0f, -280.0f);
	VerAtap2[7] = vec3( -100.0f,  240.0f, -280.0f);

//kanan
	VerAtap2[8] = vec3( -100.0f,  230.0f,  280.0f);
	VerAtap2[9] = vec3( -100.0f,  230.0f, -280.0f);
	VerAtap2[10] = vec3(-540.0f,  240.0f, -280.0f);
	VerAtap2[11] = vec3(-540.0f,  240.0f,  280.0f);
//kiri
	VerAtap2[12] = vec3(-540.0f,  230.0f, -280.0f);
	VerAtap2[13] = vec3(-540.0f,  230.0f,  280.0f);
	VerAtap2[14] = vec3(-100.0f,  240.0f,  280.0f);
	VerAtap2[15] = vec3(-100.0f,  240.0f, -280.0f);

//atas
	VerAtap2[16] = vec3(-360.0f,  240.0f,  280.0f);
	VerAtap2[17] = vec3(-100.0f,  240.0f,  280.0f);
	VerAtap2[18] = vec3( -100.0f, 240.0f, -280.0f);
	VerAtap2[19] = vec3(-360.0f,  240.0f, -280.0f);*/
//bawah
	VerAtap2[20] = vec3(-540.0f,  238.0f, -280.0f);
	VerAtap2[21] = vec3(-100.0f,  238.0f, -280.0f);
	VerAtap2[22] = vec3(-100.0f,  238.0f,  280.0f);
	VerAtap2[23] = vec3(-540.0f,  238.0f,  280.0f);	
//========================




//--PINTU
//--
    TexPintu = new vec2[24];
	VerPintu = new vec3[24];
    
//atas
    for(i=0;i<24;i++){  
	  TexPintu[i] = vec2(0.0f, 0.0f); 
      i++;
	  TexPintu[i] = vec2(1.0f, 0.0f); 
	  i++;
	  TexPintu[i] = vec2(1.0f, 1.0f); 
	  i++;
	  TexPintu[i] = vec2(0.0f, 1.0f); 
    }

//kanan
	VerPintu[0] = vec3( -175.0f,  40.0f,  30.0f);
	VerPintu[1] = vec3( -175.0f,  40.0f, -30.0f);
	VerPintu[2] = vec3(-175.0f,  200.0f, -30.0f);
	VerPintu[3] = vec3(-175.0f,  200.0f,  30.0f);
//========================
//--TEMBOK
//--
    TexKusen1 = new vec2[24];
	VerKusen1 = new vec3[24];

//depan
	VerKusen1[0] = vec3(-175.0f,  40.0f,  40.0f);
	VerKusen1[1] = vec3(-170.0f,  40.0f,  40.0f);
	VerKusen1[2] = vec3(-170.0f,  210.0f,  40.0f);
	VerKusen1[3] = vec3(-175.0f,  210.0f,  40.0f);
//belakang
	VerKusen1[4] = vec3(-170.0f,  40.0f, 30.0f);
	VerKusen1[5] = vec3(-175.0f,  40.0f, 30.0f);
	VerKusen1[6] = vec3(-175.0f,  210.0f, 30.0f);
	VerKusen1[7] = vec3(-170.0f,  210.0f, 30.0f);

//kanan
	VerKusen1[8] = vec3( -170.0f,  40.0f,  40.0f);
	VerKusen1[9] = vec3( -170.0f,  40.0f, 30.0f);
	VerKusen1[10] = vec3(-170.0f,  210.0f, 30.0f);
	VerKusen1[11] = vec3(-170.0f,  210.0f,  40.0f);
//kiri
	VerKusen1[12] = vec3(-175.0f,  40.0f, 30.0f);
	VerKusen1[13] = vec3(-175.0f,  40.0f,  40.0f);
	VerKusen1[14] = vec3(-175.0f,  210.0f,  40.0f);
	VerKusen1[15] = vec3(-175.0f,  210.0f, 30.0f);

//atas
	VerKusen1[16] = vec3(-175.0f,  210.0f,  40.0f);
	VerKusen1[17] = vec3(-170.0f,  210.0f,  40.0f);
	VerKusen1[18] = vec3(-170.0f,  210.0f, 30.0f);
	VerKusen1[19] = vec3(-175.0f,  210.0f, 30.0f);
//bawah
	VerKusen1[20] = vec3(-175.0f,  40.0f, 30.0f);
	VerKusen1[21] = vec3(-170.0f,  40.0f, 30.0f);
	VerKusen1[22] = vec3(-170.0f,  40.0f,  40.0f);
	VerKusen1[23] = vec3(-175.0f,  40.0f,  40.0f);
//========================

//--KUSEN
//--
    //TexJendela2 = new vec2[24];
	VerKusen2 = new vec3[24];
/*    
//atas
    for(i=0;i<24;i++){  
	  TexJendela2[i] = vec2(0.0f, 0.0f); 
      i++;
	  TexJendela2[i] = vec2(1.0f, 0.0f); 
	  i++;
	  TexJendela2[i] = vec2(1.0f, 1.0f); 
	  i++;
	  TexJendela2[i] = vec2(0.0f, 1.0f); 
    }
*/

//depan
	VerKusen2[0] = vec3(-175.0f,  40.0f,  -30.0f);
	VerKusen2[1] = vec3(-170.0f,  40.0f,  -30.0f);
	VerKusen2[2] = vec3(-170.0f,  210.0f,  -30.0f);
	VerKusen2[3] = vec3(-175.0f,  210.0f,  -30.0f);
//belakang
	VerKusen2[4] = vec3(-170.0f,  40.0f, -40.0f);
	VerKusen2[5] = vec3(-175.0f,  40.0f, -40.0f);
	VerKusen2[6] = vec3(-175.0f,  210.0f, -40.0f);
	VerKusen2[7] = vec3(-170.0f,  210.0f, -40.0f);

//kanan
	VerKusen2[8] = vec3( -170.0f,  40.0f,  -30.0f);
	VerKusen2[9] = vec3( -170.0f,  40.0f, -40.0f);
	VerKusen2[10] = vec3(-170.0f,  210.0f, -40.0f);
	VerKusen2[11] = vec3(-170.0f,  210.0f,  -30.0f);
//kiri
	VerKusen2[12] = vec3(-175.0f,  40.0f, -40.0f);
	VerKusen2[13] = vec3(-175.0f,  40.0f,  -30.0f);
	VerKusen2[14] = vec3(-175.0f,  210.0f,  -30.0f);
	VerKusen2[15] = vec3(-175.0f,  210.0f, -40.0f);

//atas
	VerKusen2[16] = vec3(-175.0f,  210.0f,  -30.0f);
	VerKusen2[17] = vec3(-170.0f,  210.0f,  -30.0f);
	VerKusen2[18] = vec3(-170.0f,  210.0f, -40.0f);
	VerKusen2[19] = vec3(-175.0f,  210.0f, -40.0f);
//bawah
	VerKusen2[20] = vec3(-175.0f,  40.0f, -40.0f);
	VerKusen2[21] = vec3(-170.0f,  40.0f, -40.0f);
	VerKusen2[22] = vec3(-170.0f,  40.0f,  -30.0f);
	VerKusen2[23] = vec3(-175.0f,  40.0f,  -30.0f);
//========================

//--KUSEN
//--
    //TexJendela3 = new vec2[24];
	VerKusen3 = new vec3[24];
    
//atas
    /*for(i=0;i<24;i++){  
	  TexJendela3[i] = vec2(0.0f, 0.0f); 
      i++;
	  TexJendela3[i] = vec2(1.0f, 0.0f); 
	  i++;
	  TexJendela3[i] = vec2(1.0f, 1.0f); 
	  i++;
	  TexJendela3[i] = vec2(0.0f, 1.0f); 
    }*/


//depan
	VerKusen3[0] = vec3(-175.0f,  200.0f,  30.0f);
	VerKusen3[1] = vec3(-170.0f,  200.0f,  30.0f);
	VerKusen3[2] = vec3(-170.0f,  210.0f,  30.0f);
	VerKusen3[3] = vec3(-175.0f,  210.0f,  30.0f);
//belakang
	VerKusen3[4] = vec3(-170.0f,  200.0f, -30.0f);
	VerKusen3[5] = vec3(-175.0f,  200.0f, -30.0f);
	VerKusen3[6] = vec3(-175.0f,  210.0f, -30.0f);
	VerKusen3[7] = vec3(-170.0f,  210.0f, -30.0f);

//kanan
	VerKusen3[8] = vec3( -170.0f,  200.0f,  30.0f);
	VerKusen3[9] = vec3( -170.0f,  200.0f, -30.0f);
	VerKusen3[10] = vec3(-170.0f,  210.0f, -30.0f);
	VerKusen3[11] = vec3(-170.0f,  210.0f,  30.0f);
//kiri
	VerKusen3[12] = vec3(-175.0f,  200.0f, -30.0f);
	VerKusen3[13] = vec3(-175.0f,  200.0f,  30.0f);
	VerKusen3[14] = vec3(-175.0f,  210.0f,  30.0f);
	VerKusen3[15] = vec3(-175.0f,  210.0f, -30.0f);

//atas
	VerKusen3[16] = vec3(-175.0f,  210.0f,  30.0f);
	VerKusen3[17] = vec3(-170.0f,  210.0f,  30.0f);
	VerKusen3[18] = vec3(-170.0f,  210.0f, -30.0f);
	VerKusen3[19] = vec3(-175.0f,  210.0f, -30.0f);
//bawah
	VerKusen3[20] = vec3(-175.0f,  200.0f, -30.0f);
	VerKusen3[21] = vec3(-170.0f,  200.0f, -30.0f);
	VerKusen3[22] = vec3(-170.0f,  200.0f,  30.0f);
	VerKusen3[23] = vec3(-175.0f,  200.0f,  30.0f);
//========================




//--TEMBOK
//--
    TexJendela1 = new vec2[24];
	VerJendela1 = new vec3[24];
    
//atas
    for(i=0;i<24;i++){  
	  TexJendela1[i] = vec2(0.0f, 0.0f); 
      i++;
	  TexJendela1[i] = vec2(1.0f, 0.0f); 
	  i++;
	  TexJendela1[i] = vec2(1.0f, 1.0f); 
	  i++;
	  TexJendela1[i] = vec2(0.0f, 1.0f); 
    }


//depan
	VerJendela1[0] = vec3(-175.0f,  90.0f,  150.0f);
	VerJendela1[1] = vec3(-170.0f,  90.0f,  150.0f);
	VerJendela1[2] = vec3(-170.0f,  200.0f,  150.0f);
	VerJendela1[3] = vec3(-175.0f,  200.0f,  150.0f);
//belakang
	VerJendela1[4] = vec3(-170.0f,  90.0f, 140.0f);
	VerJendela1[5] = vec3(-175.0f,  90.0f, 140.0f);
	VerJendela1[6] = vec3(-175.0f,  200.0f, 140.0f);
	VerJendela1[7] = vec3(-170.0f,  200.0f, 140.0f);

//kanan
	VerJendela1[8] = vec3( -170.0f,  90.0f,  150.0f);
	VerJendela1[9] = vec3( -170.0f,  90.0f, 140.0f);
	VerJendela1[10] = vec3(-170.0f,  200.0f, 140.0f);
	VerJendela1[11] = vec3(-170.0f,  200.0f,  150.0f);
//kiri
	VerJendela1[12] = vec3(-175.0f,  90.0f, 140.0f);
	VerJendela1[13] = vec3(-175.0f,  90.0f,  150.0f);
	VerJendela1[14] = vec3(-175.0f,  200.0f,  150.0f);
	VerJendela1[15] = vec3(-175.0f,  200.0f, 140.0f);

//atas
	VerJendela1[16] = vec3(-175.0f,  200.0f,  150.0f);
	VerJendela1[17] = vec3(-170.0f,  200.0f,  150.0f);
	VerJendela1[18] = vec3(-170.0f,  200.0f, 140.0f);
	VerJendela1[19] = vec3(-175.0f,  200.0f, 140.0f);
//bawah
	VerJendela1[20] = vec3(-175.0f,  90.0f, 140.0f);
	VerJendela1[21] = vec3(-170.0f,  90.0f, 140.0f);
	VerJendela1[22] = vec3(-170.0f,  90.0f,  150.0f);
	VerJendela1[23] = vec3(-175.0f,  90.0f,  150.0f);
//========================

//--TEMBOK
//--
    TexJendela2 = new vec2[24];
	VerJendela2 = new vec3[24];
    
//atas
    for(i=0;i<24;i++){  
	  TexJendela2[i] = vec2(0.0f, 0.0f); 
      i++;
	  TexJendela2[i] = vec2(1.0f, 0.0f); 
	  i++;
	  TexJendela2[i] = vec2(1.0f, 1.0f); 
	  i++;
	  TexJendela2[i] = vec2(0.0f, 1.0f); 
    }


//depan
	VerJendela2[0] = vec3(-175.0f,  90.0f,  100.0f);
	VerJendela2[1] = vec3(-170.0f,  90.0f,  100.0f);
	VerJendela2[2] = vec3(-170.0f,  200.0f,  100.0f);
	VerJendela2[3] = vec3(-175.0f,  200.0f,  100.0f);
//belakang
	VerJendela2[4] = vec3(-170.0f,  90.0f, 90.0f);
	VerJendela2[5] = vec3(-175.0f,  90.0f, 90.0f);
	VerJendela2[6] = vec3(-175.0f,  200.0f, 90.0f);
	VerJendela2[7] = vec3(-170.0f,  200.0f, 90.0f);

//kanan
	VerJendela2[8] = vec3( -170.0f,  90.0f,  100.0f);
	VerJendela2[9] = vec3( -170.0f,  90.0f, 90.0f);
	VerJendela2[10] = vec3(-170.0f,  200.0f, 90.0f);
	VerJendela2[11] = vec3(-170.0f,  200.0f,  100.0f);
//kiri
	VerJendela2[12] = vec3(-175.0f,  90.0f, 90.0f);
	VerJendela2[13] = vec3(-175.0f,  90.0f,  100.0f);
	VerJendela2[14] = vec3(-175.0f,  200.0f,  100.0f);
	VerJendela2[15] = vec3(-175.0f,  200.0f, 90.0f);

//atas
	VerJendela2[16] = vec3(-175.0f,  200.0f,  100.0f);
	VerJendela2[17] = vec3(-170.0f,  200.0f,  100.0f);
	VerJendela2[18] = vec3(-170.0f,  200.0f, 90.0f);
	VerJendela2[19] = vec3(-175.0f,  200.0f, 90.0f);
//bawah
	VerJendela2[20] = vec3(-175.0f,  90.0f, 90.0f);
	VerJendela2[21] = vec3(-170.0f,  90.0f, 90.0f);
	VerJendela2[22] = vec3(-170.0f,  90.0f,  100.0f);
	VerJendela2[23] = vec3(-175.0f,  90.0f,  100.0f);
//========================

//--TEMBOK
//--
    TexJendela3 = new vec2[24];
	VerJendela3 = new vec3[24];
    
//atas
    for(i=0;i<24;i++){  
	  TexJendela3[i] = vec2(0.0f, 0.0f); 
      i++;
	  TexJendela3[i] = vec2(1.0f, 0.0f); 
	  i++;
	  TexJendela3[i] = vec2(1.0f, 1.0f); 
	  i++;
	  TexJendela3[i] = vec2(0.0f, 1.0f); 
    }


//depan
	VerJendela3[0] = vec3(-175.0f,  90.0f,  140.0f);
	VerJendela3[1] = vec3(-170.0f,  90.0f,  140.0f);
	VerJendela3[2] = vec3(-170.0f,  100.0f,  140.0f);
	VerJendela3[3] = vec3(-175.0f,  100.0f,  140.0f);
//belakang
	VerJendela3[4] = vec3(-170.0f,  90.0f, 100.0f);
	VerJendela3[5] = vec3(-175.0f,  90.0f, 100.0f);
	VerJendela3[6] = vec3(-175.0f,  100.0f, 100.0f);
	VerJendela3[7] = vec3(-170.0f,  100.0f, 100.0f);

//kanan
	VerJendela3[8] = vec3( -170.0f,  90.0f,  140.0f);
	VerJendela3[9] = vec3( -170.0f,  90.0f, 100.0f);
	VerJendela3[10] = vec3(-170.0f,  100.0f, 100.0f);
	VerJendela3[11] = vec3(-170.0f,  100.0f,  140.0f);
//kiri
	VerJendela3[12] = vec3(-175.0f,  90.0f, 100.0f);
	VerJendela3[13] = vec3(-175.0f,  90.0f,  140.0f);
	VerJendela3[14] = vec3(-175.0f,  100.0f,  140.0f);
	VerJendela3[15] = vec3(-175.0f,  100.0f, 100.0f);

//atas
	VerJendela3[16] = vec3(-175.0f,  100.0f,  140.0f);
	VerJendela3[17] = vec3(-170.0f,  100.0f,  140.0f);
	VerJendela3[18] = vec3(-170.0f,  100.0f, 100.0f);
	VerJendela3[19] = vec3(-175.0f,  100.0f, 100.0f);
//bawah
	VerJendela3[20] = vec3(-175.0f,  90.0f, 100.0f);
	VerJendela3[21] = vec3(-170.0f,  90.0f, 100.0f);
	VerJendela3[22] = vec3(-170.0f,  90.0f,  140.0f);
	VerJendela3[23] = vec3(-175.0f,  90.0f,  140.0f);
//========================


//--TEMBOK
//--
    TexJendela4 = new vec2[24];
	VerJendela4 = new vec3[24];
    
//atas
    for(i=0;i<24;i++){  
	  TexJendela4[i] = vec2(0.0f, 0.0f); 
      i++;
	  TexJendela4[i] = vec2(1.0f, 0.0f); 
	  i++;
	  TexJendela4[i] = vec2(1.0f, 1.0f); 
	  i++;
	  TexJendela4[i] = vec2(0.0f, 1.0f); 
    }


//depan
	VerJendela4[0] = vec3(-175.0f,  190.0f,  140.0f);
	VerJendela4[1] = vec3(-170.0f,  190.0f,  140.0f);
	VerJendela4[2] = vec3(-170.0f,  200.0f,  140.0f);
	VerJendela4[3] = vec3(-175.0f,  200.0f,  140.0f);
//belakang
	VerJendela4[4] = vec3(-170.0f,  190.0f, 100.0f);
	VerJendela4[5] = vec3(-175.0f,  190.0f, 100.0f);
	VerJendela4[6] = vec3(-175.0f,  200.0f, 100.0f);
	VerJendela4[7] = vec3(-170.0f,  200.0f, 100.0f);

//kanan
	VerJendela4[8] = vec3( -170.0f,  190.0f,  140.0f);
	VerJendela4[9] = vec3( -170.0f,  190.0f, 100.0f);
	VerJendela4[10] = vec3(-170.0f,  200.0f, 100.0f);
	VerJendela4[11] = vec3(-170.0f,  200.0f,  140.0f);
//kiri
	VerJendela4[12] = vec3(-175.0f,  190.0f, 100.0f);
	VerJendela4[13] = vec3(-175.0f,  190.0f,  140.0f);
	VerJendela4[14] = vec3(-175.0f,  200.0f,  140.0f);
	VerJendela4[15] = vec3(-175.0f,  200.0f, 100.0f);

//atas
	VerJendela4[16] = vec3(-175.0f,  200.0f,  140.0f);
	VerJendela4[17] = vec3(-170.0f,  200.0f,  140.0f);
	VerJendela4[18] = vec3(-170.0f,  200.0f, 100.0f);
	VerJendela4[19] = vec3(-175.0f,  200.0f, 100.0f);
//bawah
	VerJendela4[20] = vec3(-175.0f,  190.0f, 100.0f);
	VerJendela4[21] = vec3(-170.0f,  190.0f, 100.0f);
	VerJendela4[22] = vec3(-170.0f,  190.0f,  140.0f);
	VerJendela4[23] = vec3(-175.0f,  190.0f,  140.0f);
//========================

//--TEMBOK
//--
    TexJendela5 = new vec2[24];
	VerJendela5 = new vec3[24];
    
//atas
    for(i=0;i<24;i++){  
	  TexJendela5[i] = vec2(0.0f, 0.0f); 
      i++;
	  TexJendela5[i] = vec2(1.0f, 0.0f); 
	  i++;
	  TexJendela5[i] = vec2(1.0f, 1.0f); 
	  i++;
	  TexJendela5[i] = vec2(0.0f, 1.0f); 
    }


//kanan
	VerJendela5[0] = vec3( -173.0f,  100.0f,  140.0f);
	VerJendela5[1] = vec3( -173.0f,  100.0f, 100.0f);
	VerJendela5[2] = vec3(-173.0f,  190.0f, 100.0f);
	VerJendela5[3] = vec3(-173.0f,  190.0f,  140.0f);
//========================




//--TEMBOK
//--
    TexJendela1a = new vec2[24];
	VerJendela1a = new vec3[24];
    
//atas
    for(i=0;i<24;i++){  
	  TexJendela1a[i] = vec2(0.0f, 0.0f); 
      i++;
	  TexJendela1a[i] = vec2(1.0f, 0.0f); 
	  i++;
	  TexJendela1a[i] = vec2(1.0f, 1.0f); 
	  i++;
	  TexJendela1a[i] = vec2(0.0f, 1.0f); 
    }


//depan
	VerJendela1a[0] = vec3(-175.0f,  90.0f,  -140.0f);
	VerJendela1a[1] = vec3(-170.0f,  90.0f,  -140.0f);
	VerJendela1a[2] = vec3(-170.0f,  200.0f,  -140.0f);
	VerJendela1a[3] = vec3(-175.0f,  200.0f,  -140.0f);
//belakang
	VerJendela1a[4] = vec3(-170.0f,  90.0f, -150.0f);
	VerJendela1a[5] = vec3(-175.0f,  90.0f, -150.0f);
	VerJendela1a[6] = vec3(-175.0f,  200.0f, -150.0f);
	VerJendela1a[7] = vec3(-170.0f,  200.0f, -150.0f);

//kanan
	VerJendela1a[8] = vec3( -170.0f,  90.0f,  -140.0f);
	VerJendela1a[9] = vec3( -170.0f,  90.0f, -150.0f);
	VerJendela1a[10] = vec3(-170.0f,  200.0f, -150.0f);
	VerJendela1a[11] = vec3(-170.0f,  200.0f,  -140.0f);
//kiri
	VerJendela1a[12] = vec3(-175.0f,  90.0f, -150.0f);
	VerJendela1a[13] = vec3(-175.0f,  90.0f,  -140.0f);
	VerJendela1a[14] = vec3(-175.0f,  200.0f,  -140.0f);
	VerJendela1a[15] = vec3(-175.0f,  200.0f, -150.0f);

//atas
	VerJendela1a[16] = vec3(-175.0f,  200.0f,  -140.0f);
	VerJendela1a[17] = vec3(-170.0f,  200.0f,  -140.0f);
	VerJendela1a[18] = vec3(-170.0f,  200.0f, -150.0f);
	VerJendela1a[19] = vec3(-175.0f,  200.0f, -150.0f);
//bawah
	VerJendela1a[20] = vec3(-175.0f,  90.0f, -150.0f);
	VerJendela1a[21] = vec3(-170.0f,  90.0f, -150.0f);
	VerJendela1a[22] = vec3(-170.0f,  90.0f,  -140.0f);
	VerJendela1a[23] = vec3(-175.0f,  90.0f,  -140.0f);
//========================

//--TEMBOK
//--
    TexJendela2a = new vec2[24];
	VerJendela2a = new vec3[24];
    
//atas
    for(i=0;i<24;i++){  
	  TexJendela2a[i] = vec2(0.0f, 0.0f); 
      i++;
	  TexJendela2a[i] = vec2(1.0f, 0.0f); 
	  i++;
	  TexJendela2a[i] = vec2(1.0f, 1.0f); 
	  i++;
	  TexJendela2a[i] = vec2(0.0f, 1.0f); 
    }


//depan
	VerJendela2a[0] = vec3(-175.0f,  90.0f,  -90.0f);
	VerJendela2a[1] = vec3(-170.0f,  90.0f,  -90.0f);
	VerJendela2a[2] = vec3(-170.0f,  200.0f,  -90.0f);
	VerJendela2a[3] = vec3(-175.0f,  200.0f,  -90.0f);
//belakang
	VerJendela2a[4] = vec3(-170.0f,  90.0f, -100.0f);
	VerJendela2a[5] = vec3(-175.0f,  90.0f, -100.0f);
	VerJendela2a[6] = vec3(-175.0f,  200.0f, -100.0f);
	VerJendela2a[7] = vec3(-170.0f,  200.0f, -100.0f);

//kanan
	VerJendela2a[8] = vec3( -170.0f,  90.0f,  -90.0f);
	VerJendela2a[9] = vec3( -170.0f,  90.0f, -100.0f);
	VerJendela2a[10] = vec3(-170.0f,  200.0f, -100.0f);
	VerJendela2a[11] = vec3(-170.0f,  200.0f,  -90.0f);
//kiri
	VerJendela2a[12] = vec3(-175.0f,  90.0f, -100.0f);
	VerJendela2a[13] = vec3(-175.0f,  90.0f,  -90.0f);
	VerJendela2a[14] = vec3(-175.0f,  200.0f,  -90.0f);
	VerJendela2a[15] = vec3(-175.0f,  200.0f, -100.0f);

//atas
	VerJendela2a[16] = vec3(-175.0f,  200.0f,  -90.0f);
	VerJendela2a[17] = vec3(-170.0f,  200.0f,  -90.0f);
	VerJendela2a[18] = vec3(-170.0f,  200.0f, -100.0f);
	VerJendela2a[19] = vec3(-175.0f,  200.0f, -100.0f);
//bawah
	VerJendela2a[20] = vec3(-175.0f,  90.0f, -100.0f);
	VerJendela2a[21] = vec3(-170.0f,  90.0f, -100.0f);
	VerJendela2a[22] = vec3(-170.0f,  90.0f,  -90.0f);
	VerJendela2a[23] = vec3(-175.0f,  90.0f,  -90.0f);
//========================

//--TEMBOK
//--
    TexJendela3a = new vec2[24];
	VerJendela3a = new vec3[24];
    
//atas
    for(i=0;i<24;i++){  
	  TexJendela3a[i] = vec2(0.0f, 0.0f); 
      i++;
	  TexJendela3a[i] = vec2(1.0f, 0.0f); 
	  i++;
	  TexJendela3a[i] = vec2(1.0f, 1.0f); 
	  i++;
	  TexJendela3a[i] = vec2(0.0f, 1.0f); 
    }


//depan
	VerJendela3a[0] = vec3(-175.0f,  90.0f,  -100.0f);
	VerJendela3a[1] = vec3(-170.0f,  90.0f,  -100.0f);
	VerJendela3a[2] = vec3(-170.0f,  100.0f,  -100.0f);
	VerJendela3a[3] = vec3(-175.0f,  100.0f,  -100.0f);
//belakang
	VerJendela3a[4] = vec3(-170.0f,  90.0f, -140.0f);
	VerJendela3a[5] = vec3(-175.0f,  90.0f, -140.0f);
	VerJendela3a[6] = vec3(-175.0f,  100.0f, -140.0f);
	VerJendela3a[7] = vec3(-170.0f,  100.0f, -140.0f);

//kanan
	VerJendela3a[8] = vec3( -170.0f,  90.0f,  -100.0f);
	VerJendela3a[9] = vec3( -170.0f,  90.0f, -140.0f);
	VerJendela3a[10] = vec3(-170.0f,  100.0f, -140.0f);
	VerJendela3a[11] = vec3(-170.0f,  100.0f,  -100.0f);
//kiri
	VerJendela3a[12] = vec3(-175.0f,  90.0f, -140.0f);
	VerJendela3a[13] = vec3(-175.0f,  90.0f,  -100.0f);
	VerJendela3a[14] = vec3(-175.0f,  100.0f,  -100.0f);
	VerJendela3a[15] = vec3(-175.0f,  100.0f, -140.0f);

//atas
	VerJendela3a[16] = vec3(-175.0f,  100.0f,  -100.0f);
	VerJendela3a[17] = vec3(-170.0f,  100.0f,  -100.0f);
	VerJendela3a[18] = vec3(-170.0f,  100.0f, -140.0f);
	VerJendela3a[19] = vec3(-175.0f,  100.0f, -140.0f);
//bawah
	VerJendela3a[20] = vec3(-175.0f,  90.0f, -140.0f);
	VerJendela3a[21] = vec3(-170.0f,  90.0f, -140.0f);
	VerJendela3a[22] = vec3(-170.0f,  90.0f,  -100.0f);
	VerJendela3a[23] = vec3(-175.0f,  90.0f,  -100.0f);
//========================


//--TEMBOK
//--
    TexJendela4a = new vec2[24];
	VerJendela4a = new vec3[24];
    
//atas
    for(i=0;i<24;i++){  
	  TexJendela4a[i] = vec2(0.0f, 0.0f); 
      i++;
	  TexJendela4a[i] = vec2(1.0f, 0.0f); 
	  i++;
	  TexJendela4a[i] = vec2(1.0f, 1.0f); 
	  i++;
	  TexJendela4a[i] = vec2(0.0f, 1.0f); 
    }


//depan
	VerJendela4a[0] = vec3(-175.0f,  190.0f,  -100.0f);
	VerJendela4a[1] = vec3(-170.0f,  190.0f,  -100.0f);
	VerJendela4a[2] = vec3(-170.0f,  200.0f,  -100.0f);
	VerJendela4a[3] = vec3(-175.0f,  200.0f,  -100.0f);
//belakang
	VerJendela4a[4] = vec3(-170.0f,  190.0f, -140.0f);
	VerJendela4a[5] = vec3(-175.0f,  190.0f, -140.0f);
	VerJendela4a[6] = vec3(-175.0f,  200.0f, -140.0f);
	VerJendela4a[7] = vec3(-170.0f,  200.0f, -140.0f);

//kanan
	VerJendela4a[8] = vec3( -170.0f,  190.0f,  -100.0f);
	VerJendela4a[9] = vec3( -170.0f,  190.0f, -140.0f);
	VerJendela4a[10] = vec3(-170.0f,  200.0f, -140.0f);
	VerJendela4a[11] = vec3(-170.0f,  200.0f,  -100.0f);
//kiri
	VerJendela4a[12] = vec3(-175.0f,  190.0f, -140.0f);
	VerJendela4a[13] = vec3(-175.0f,  190.0f,  -100.0f);
	VerJendela4a[14] = vec3(-175.0f,  200.0f,  -100.0f);
	VerJendela4a[15] = vec3(-175.0f,  200.0f, -140.0f);

//atas
	VerJendela4a[16] = vec3(-175.0f,  200.0f,  -100.0f);
	VerJendela4a[17] = vec3(-170.0f,  200.0f,  -100.0f);
	VerJendela4a[18] = vec3(-170.0f,  200.0f, -140.0f);
	VerJendela4a[19] = vec3(-175.0f,  200.0f, -140.0f);
//bawah
	VerJendela4a[20] = vec3(-175.0f,  190.0f, -140.0f);
	VerJendela4a[21] = vec3(-170.0f,  190.0f, -140.0f);
	VerJendela4a[22] = vec3(-170.0f,  190.0f,  -100.0f);
	VerJendela4a[23] = vec3(-175.0f,  190.0f,  -100.0f);
//========================

//--TEMBOK
//--
    TexJendela5a = new vec2[24];
	VerJendela5a = new vec3[24];
    
//atas
    for(i=0;i<24;i++){  
	  TexJendela5a[i] = vec2(0.0f, 0.0f); 
      i++;
	  TexJendela5a[i] = vec2(1.0f, 0.0f); 
	  i++;
	  TexJendela5a[i] = vec2(1.0f, 1.0f); 
	  i++;
	  TexJendela5a[i] = vec2(0.0f, 1.0f); 
    }


//kanan
	VerJendela5a[0] = vec3( -173.0f,  100.0f,  -100.0f);
	VerJendela5a[1] = vec3( -173.0f,  100.0f, -140.0f);
	VerJendela5a[2] = vec3(-173.0f,  190.0f, -140.0f);
	VerJendela5a[3] = vec3(-173.0f,  190.0f,  -100.0f);
//========================



//--KANDANG
//--
    TexKd = new vec2[29];
	VerKd = new vec3[29];
    
//atas
    for(i=0;i<29;i++){  
	  TexKd[i] = vec2(0.0f, 0.0f); 
      i++;
	  TexKd[i] = vec2(1.0f, 0.0f); 
	  i++;
	  TexKd[i] = vec2(1.0f, 1.0f); 
	  i++;
	  TexKd[i] = vec2(0.0f, 1.0f); 
    }

//==KANDANG lubang====================================================================================
	VerKd4 = new vec3[24];
//depan
	VerKd4[0] = vec3(8.0f, -0.0f,  51.1f);
	VerKd4[1] = vec3( 23.0f, -0.0f,  51.1f);
	VerKd4[2] = vec3( 23.0f,  22.0f,  51.1f);
	VerKd4[3] = vec3(8.0f,  22.0f,  51.1f);


//==KANDANG====================================================================================
//depan
	VerKd[0] = vec3(-0.0f, -0.0f,  51.0f);
	VerKd[1] = vec3( 31.0f, -0.0f,  51.0f);
	VerKd[2] = vec3( 31.0f,  32.0f,  51.0f);
	VerKd[3] = vec3(-0.0f,  32.0f,  51.0f);
//belakang
	VerKd[4] = vec3( 31.0f, -0.0f, -0.0f);
	VerKd[5] = vec3(-0.0f, -0.0f, -0.0f);
	VerKd[6] = vec3(-0.0f,  32.0f, -0.0f);
	VerKd[7] = vec3( 31.0f,  32.0f, -0.0f);

//kanan
	VerKd[8] = vec3( 31.0f, -0.0f,  51.0f);
	VerKd[9] = vec3( 31.0f, -0.0f, -0.0f);
	VerKd[10] = vec3( 31.0f,  32.0f, -0.0f);
	VerKd[11] = vec3( 31.0f,  32.0f,  51.0f);
	
	
//kiri
	VerKd[12] = vec3(-0.0f, -0.0f, -0.0f);
	VerKd[13] = vec3(-0.0f, -0.0f,  51.0f);
	VerKd[14] = vec3(-0.0f,  32.0f,  51.0f);
	VerKd[15] = vec3(-0.0f,  32.0f, -0.0f);

//atas
	VerKd[16] = vec3(-0.0f,  32.0f,  51.0f);
	VerKd[17] = vec3( 31.0f,  32.0f,  51.0f);
	VerKd[18] = vec3( 31.0f,  32.0f, -0.0f);
	VerKd[19] = vec3(-0.0f,  32.0f, -0.0f);
//bawah
	VerKd[20] = vec3(-0.0f, -0.0f, -0.0f);
	VerKd[21] = vec3( 31.0f, -0.0f, -0.0f);
	VerKd[22] = vec3( 31.0f, -0.0f,  51.0f);
	VerKd[23] = vec3(-0.0f, -0.0f,  51.0f);

//segitiga depan
	VerKd[24] = vec3(-0.0f, 32.0f,  51.0f);
	VerKd[25] = vec3( 31.0f, 32.0f,  51.0f);
	//VerKd[26] = vec3( 31.0f,  32.0f,  51.0f);
	VerKd[26] = vec3( 15.5f,  40.0f,  51.0f);
	//VerKd[28] = vec3(-0.0f,  32.0f,  51.0f);

//==atap KANDANG====================================================================================
	VerKd2 = new vec3[24];
/*
//depan
	VerKd[0] = vec3(-0.0f, -0.0f,  51.0f);
	VerKd[1] = vec3( 31.0f, -0.0f,  51.0f);
	VerKd[2] = vec3( 31.0f,  32.0f,  51.0f);
	VerKd[3] = vec3(-0.0f,  32.0f,  51.0f);
    */	
//depan
	VerKd2[0] = vec3(-12.0f, 27.0f,  55.0f);
	VerKd2[1] = vec3( 15.5f,  40.0f,  55.0f);
	VerKd2[2] = vec3( 15.5f,  42.0f,  55.0f);
	VerKd2[3] = vec3(-12.0f,  29.0f,  55.0f);
/*
//belakang
	VerKd2[4] = vec3( 31.0f, -0.0f, -0.0f);
	VerKd2[5] = vec3(-0.0f, -0.0f, -0.0f);
	VerKd2[6] = vec3(-0.0f,  32.0f, -0.0f);
	VerKd2[7] = vec3( 31.0f,  32.0f, -0.0f);
*/	
//belakang
	VerKd2[4] = vec3( 15.5f, 27.0f, -4.0f);
	VerKd2[5] = vec3(-12.0f, 40.0f, -4.0f);
	VerKd2[6] = vec3(-12.0f,  42.0f, -4.0f);
	VerKd2[7] = vec3( 15.5f,  29.0f, -4.0f);
	
/*
//kanan
	VerKd[8] = vec3( 31.0f, -0.0f,  51.0f);
	VerKd[9] = vec3( 31.0f, -0.0f, -0.0f);
	VerKd[10] = vec3( 31.0f,  32.0f, -0.0f);
	VerKd[11] = vec3( 31.0f,  32.0f,  51.0f);
		
//kiri
	VerKd[12] = vec3(-0.0f, -0.0f, -0.0f);
	VerKd[13] = vec3(-0.0f, -0.0f,  51.0f);
	VerKd[14] = vec3(-0.0f,  32.0f,  51.0f);
	VerKd[15] = vec3(-0.0f,  32.0f, -0.0f);
*/
//kiri
	VerKd2[12] = vec3(-12.0f, 27.0f, -4.0f);
	VerKd2[13] = vec3(-12.0f, 27.0f,  55.0f);
	VerKd2[14] = vec3(-12.0f,  29.0f,  55.0f);
	VerKd2[15] = vec3(-12.0f,  29.0f, -4.0f);
/*
//atas
	VerKd[16] = vec3(-0.0f,  32.0f,  51.0f);
	VerKd[17] = vec3( 31.0f,  32.0f,  51.0f);
	VerKd[18] = vec3( 31.0f,  32.0f, -0.0f);
	VerKd[19] = vec3(-0.0f,  32.0f, -0.0f);
*/
//atas
	VerKd2[16] = vec3(-12.0f,  29.0f,  55.0f);
	VerKd2[17] = vec3( 15.5f,  42.0f,  55.0f);
	VerKd2[18] = vec3( 15.5f,  42.0f, -4.0f);
	VerKd2[19] = vec3(-12.0f,  29.0f, -4.0f);
	
/*	
//bawah
	VerKd[20] = vec3(-0.0f, -0.0f, -0.0f);
	VerKd[21] = vec3( 31.0f, -0.0f, -0.0f);
	VerKd[22] = vec3( 31.0f, -0.0f,  51.0f);
	VerKd[23] = vec3(-0.0f, -0.0f,  51.0f);
*/
//bawah
	VerKd2[20] = vec3(-12.0f, 27.0f, -4.0f);
	VerKd2[21] = vec3( 15.5f, 40.0f, -4.0f);
	VerKd2[22] = vec3( 15.5f, 40.0f,  55.0f);
	VerKd2[23] = vec3(-12.0f, 27.0f,  55.0f);


//==atap KANDANG====================================================================================
	VerKd3 = new vec3[24];
/*
//depan
	VerKd[0] = vec3(-0.0f, -0.0f,  51.0f);
	VerKd[1] = vec3( 31.0f, -0.0f,  51.0f);
	VerKd[2] = vec3( 31.0f,  32.0f,  51.0f);
	VerKd[3] = vec3(-0.0f,  32.0f,  51.0f);
    */	
//depan
	VerKd3[0] = vec3(15.5f, 40.0f,  55.0f);
	VerKd3[1] = vec3( 43.0f,  27.0f,  55.0f);
	VerKd3[2] = vec3( 43.0f,  29.0f,  55.0f);
	VerKd3[3] = vec3(15.5f,  42.0f,  55.0f);
/*
//belakang
	VerKd2[4] = vec3( 31.0f, -0.0f, -0.0f);
	VerKd2[5] = vec3(-0.0f, -0.0f, -0.0f);
	VerKd2[6] = vec3(-0.0f,  32.0f, -0.0f);
	VerKd2[7] = vec3( 31.0f,  32.0f, -0.0f);
*/	
//belakang
	//VerKd3[4] = vec3( 43.0f, 40.0f, -4.0f);
	//VerKd3[5] = vec3(15.5f, 27.0f, -4.0f);
	//VerKd3[6] = vec3(15.5f,  29.0f, -4.0f);
	//VerKd3[7] = vec3( 43.0f,  42.0f, -4.0f);
	
/*
//kanan
	VerKd[8] = vec3( 31.0f, -0.0f,  51.0f);
	VerKd[9] = vec3( 31.0f, -0.0f, -0.0f);
	VerKd[10] = vec3( 31.0f,  32.0f, -0.0f);
	VerKd[11] = vec3( 31.0f,  32.0f,  51.0f);
		
//kiri
	VerKd[12] = vec3(-0.0f, -0.0f, -0.0f);
	VerKd[13] = vec3(-0.0f, -0.0f,  51.0f);
	VerKd[14] = vec3(-0.0f,  32.0f,  51.0f);
	VerKd[15] = vec3(-0.0f,  32.0f, -0.0f);
*/
//kiri
/*	VerKd3[12] = vec3(-12.0f, 27.0f, -4.0f);
	VerKd3[13] = vec3(-12.0f, 27.0f,  55.0f);
	VerKd3[14] = vec3(-12.0f,  29.0f,  55.0f);
	VerKd3[15] = vec3(-12.0f,  29.0f, -4.0f);
	*/

//atas
	//VerKd3[16] = vec3(-0.0f,  32.0f,  51.0f);
	//VerKd3[17] = vec3( 31.0f,  32.0f,  51.0f);
	//VerKd3[18] = vec3( 31.0f,  32.0f, -0.0f);
	//VerKd3[19] = vec3(-0.0f,  32.0f, -0.0f);

//atas
	VerKd3[16] = vec3(15.5f,  42.0f,  55.0f);
	VerKd3[17] = vec3( 43.0f,  29.0f,  55.0f);
	VerKd3[18] = vec3( 43.0f,  29.0f, -4.0f);
	VerKd3[19] = vec3(15.5f,  42.0f, -4.0f);
	
/*	
//bawah
	VerKd[20] = vec3(-0.0f, -0.0f, -0.0f);
	VerKd[21] = vec3( 31.0f, -0.0f, -0.0f);
	VerKd[22] = vec3( 31.0f, -0.0f,  51.0f);
	VerKd[23] = vec3(-0.0f, -0.0f,  51.0f);
*/
//bawah
	VerKd3[20] = vec3(15.5f, 40.0f, -4.0f);
	VerKd3[21] = vec3( 43.0f, 27.0f, -4.0f);
	VerKd3[22] = vec3( 43.0f, 27.0f,  55.0f);
	VerKd3[23] = vec3(15.5f, 40.0f,  55.0f);




//============================================
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);








	Camera.LookAt(vec3(0.0f, 0.0f, 0.0f), vec3(50.0f, 50.0f, 100.0f));

	// DisplayInfo("Information text ...");

	return true;
}

void COpenGLRenderer::Render(float FrameTime)
{

     
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf((GLfloat*)&View);



//+++++++++++++++++++++++++++

/*
glMatrixMode(GL_PROJECTION);
glPushMatrix();
glOrtho(0, 1, 0, 1, 0, 1);

glMatrixMode(GL_MODELVIEW);
glPushMatrix();
glLoadIdentity();
	
	// No depth buffer writes for background.
glDepthMask( false );

glBindTexture( GL_TEXTURE_2D, Texture[18] );
glBegin( GL_QUADS ); 
glTexCoord2f( 0.0f, 0.0f );
glVertex2f( 0, 0 );
glTexCoord2f( 0.0f, 1.0f );
glVertex2f( 0, 10.0f );
glTexCoord2f( 1.0f, 1.0f );
glVertex2f( 10.0f, 10.0f );
glTexCoord2f( 1.0f, 0.0f );
glVertex2f( 10.0f, 0 );
	glEnd();

glDepthMask( true );

glPopMatrix();
glMatrixMode(GL_PROJECTION);
glPopMatrix();
glMatrixMode(GL_MODELVIEW);*/
//+++++++++++++++++++++++++++


	if(ShowAxisGrid)
	{
		glLineWidth(2.0f);

		glBegin(GL_LINES);

		glColor4f(1.0f, 0.0f, 0.0f, 1.0f);

		glVertex3f(0.0f, 0.0f, 0.0f); glVertex3f(1.0f, 0.0f, 0.0f);
		glVertex3f(1.0f, 0.1f, 0.0f); glVertex3f(1.1f, -0.1f, 0.0f);
		glVertex3f(1.1f, 0.1f, 0.0f); glVertex3f(1.0f, -0.1f, 0.0f);

		glColor4f(0.0f, 1.0f, 0.0f, 1.0f);

		glVertex3f(0.0f, 0.0f, 0.0f); glVertex3f(0.0f, 1.0f, 0.0f);
		glVertex3f(-0.05f, 1.25f, 0.0f); glVertex3f(0.0f, 1.15f, 0.0f);
		glVertex3f(0.05f,1.25f, 0.0f); glVertex3f(0.0f, 1.15f, 0.0f);
		glVertex3f(0.0f,1.15f, 0.0f); glVertex3f(0.0f, 1.05f, 0.0f);

		glColor4f(0.0f, 0.0f, 1.0f, 1.0f);

		glVertex3f(0.0f,0.0f,0.0f); glVertex3f(0.0f, 0.0f, 1.0f);
		glVertex3f(-0.05f,0.1f,1.05f); glVertex3f(0.05f, 0.1f, 1.05f);
		glVertex3f(0.05f,0.1f,1.05f); glVertex3f(-0.05f, -0.1f, 1.05f);
		glVertex3f(-0.05f,-0.1f,1.05f); glVertex3f(0.05f, -0.1f, 1.05f);

		glEnd();

		glLineWidth(1.0f);

		glColor3f(1.0f, 1.0f, 1.0f);

		glBegin(GL_LINES);

		float d = 100.0f;

		for(float i = -d; i <= d; i += 1.0f)
		{
			glVertex3f(i, 0.0f, -d);
			glVertex3f(i, 0.0f, d);
			glVertex3f(-d, 0.0f, i);
			glVertex3f(d, 0.0f, i);
		}

		glEnd();
	}

	glMultMatrixf((GLfloat*)&Model);

	if(!Stop)
	{
		static float a = 0.0f;

		Model = rotate(mat4x4(), a, vec3(0.0f, 1.0f, 0.0f)) * rotate(mat4x4(), a, vec3(1.0f, 0.0f, 0.0f));

		a += 11.25f * FrameTime;
	}


     	/*glPushMatrix();
	glTranslatef(27.0f, -17.0f, -23.0f);
	glRotatef(-90,1.0f, 0.0f, 0.0f);
	glScalef(0.5f, 0.5f, 0.5f);
	divide_branch_third(5, 0.5, 7); //lets draw the tree
	glPopMatrix();
	*/
		// Save the matrix state and do the rotations
/*	glPushMatrix();
	glTranslatef(-80.0f, 0.0f, 120.0f);
	glRotatef(-90, 1.0f, 0.0f, 0.0f);
	//glRotatef(yRot, 0.0f, 1.0f, 0.0f);
	glScalef(8.5, 8.5, 8.5);
	//divide_branch_fourth(3, 0.3, 13, 0.0, 0.0, 0.0);   
    divide_branch_third(6, 0.5, 7, 0.0, 0.0, 0.0);
    //divide_branch_second(5, 1, 5, 0.0, 0.0, 0.0); 
    //divide_branch(9, 1, 5, 0.0, 0.0, 0.0);
    //divide_branch_fifth(3, 0.3, 1, 0.0, 0.0, 0.0);  
          	
	glPopMatrix();
	//glutSwapBuffers();// Display the results
      // Light values and coordinates
    GLfloat  ambientLight[] = { 0.3f, 0.3f, 0.3f, 1.0f };
    GLfloat  diffuseLight[] = { 0.7f, 0.7f, 0.7f, 1.0f };
    GLfloat  specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat  specref[] = { 1.0f, 1.0f, 1.0f, 1.0f };

    glEnable(GL_DEPTH_TEST);	// Hidden surface removal
    glFrontFace(GL_CCW);		// Counter clock-wise polygons face out
    glEnable(GL_CULL_FACE);		// Do not calculate inside of jet

    // Enable lighting
    glEnable(GL_LIGHTING);

    // Setup and enable light 0
    glLightfv(GL_LIGHT0,GL_AMBIENT,ambientLight);
    glLightfv(GL_LIGHT0,GL_DIFFUSE,diffuseLight);
    glLightfv(GL_LIGHT0,GL_SPECULAR, specular);
    glEnable(GL_LIGHT0);

    // Enable color tracking
    glEnable(GL_COLOR_MATERIAL);
	
    // Set Material properties to follow glColor values
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

    // All materials hereafter have full specular reflectivity
    // with a high shine
    glMaterialfv(GL_FRONT, GL_SPECULAR, specref);
    glMateriali(GL_FRONT, GL_SHININESS, 128);   
glEnable(GL_NORMALIZE);
*/






//DANBO1================
	glEnable(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, Texture[1]);

	if(gl_version >= 21)
	{
		glUseProgram(Shader);
	}

//--kaki 1
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, TexCoords);

	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, 0, Normals);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, Vertices);


	glDrawArrays(GL_QUADS, 0, 24);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

//--kaki2
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, TexCoords);

	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, 0, Normals);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, Vertices1);

	glDrawArrays(GL_QUADS, 0, 24);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

//--badan
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, TexCoords);

	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, 0, Normals);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, Vertices2);

	glDrawArrays(GL_QUADS, 0, 24);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

//--kepala1
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, TexCoords);

	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, 0, Normals);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, Vertices3);

	glDrawArrays(GL_QUADS, 0, 24);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);


//--tangan kiri
glPushMatrix();
glTranslatef(0.0f, 6.3f, -1.0f);
glRotatef(90.0, 1.0f, 0.0f, 0.0f);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, TexCoords);


	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, 0, Normals);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, Vertices5);

	glDrawArrays(GL_QUADS, 0, 24);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
glPopMatrix();	
	//--tangan kanan
glPushMatrix();
glTranslatef(0.0f, 5.3f, 1.0f);
glRotatef(90.0, 3.0f, 1.0f, 0.0f);	
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, TexCoords);

	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, 0, Normals);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, Vertices6);

	glDrawArrays(GL_QUADS, 0, 24);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
glPopMatrix();
	if(gl_version >= 21)
	{
		glUseProgram(0);
	}

	glBindTexture(GL_TEXTURE_2D, 0);

	glDisable(GL_TEXTURE_2D);
	
//--Texture2

	glEnable(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, Texture[2]);

	if(gl_version >= 21)
	{
		glUseProgram(Shader);
	}
//--kepala2
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, TexCoords);

	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, 0, Normals);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, Vertices4);

	glDrawArrays(GL_QUADS, 0, 24);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	if(gl_version >= 21)
	{
		glUseProgram(0);
	}

	glBindTexture(GL_TEXTURE_2D, 0);

	glDisable(GL_TEXTURE_2D);
//END DANBO1=====================


//DANBO2================
glPushMatrix();
glTranslatef(5.0f, 0.0f, 0.0f);
glRotatef(-60.0, 0.0f, 1.0f, 0.0f);
	glEnable(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, Texture[1]);

	if(gl_version >= 21)
	{
		glUseProgram(Shader);
	}
//--kaki 1
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, TexCoords);

	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, 0, Normals);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, VerticesA);


	glDrawArrays(GL_QUADS, 0, 24);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

//--kaki2
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, TexCoords);

	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, 0, Normals);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, Vertices1A);

	glDrawArrays(GL_QUADS, 0, 24);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

//--badan
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, TexCoords);

	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, 0, Normals);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, Vertices2A);

	glDrawArrays(GL_QUADS, 0, 24);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

//--kepala1
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, TexCoords);

	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, 0, Normals);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, Vertices3A);

	glDrawArrays(GL_QUADS, 0, 24);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);


//--tangan kiri
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, TexCoords);

	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, 0, Normals);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, Vertices5A);

	glDrawArrays(GL_QUADS, 0, 24);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);


	//--tangan kanan


	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, TexCoords);

	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, 0, Normals);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, Vertices6A);

	glDrawArrays(GL_QUADS, 0, 24);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	
	if(gl_version >= 21)
	{
		glUseProgram(0);
	}

	glBindTexture(GL_TEXTURE_2D, 0);

	glDisable(GL_TEXTURE_2D);
	
//--Texture2

	glEnable(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, Texture[2]);

	if(gl_version >= 21)
	{
		glUseProgram(Shader);
	}
//--kepala2
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, TexCoords);

	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, 0, Normals);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, Vertices4A);

	glDrawArrays(GL_QUADS, 0, 24);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	if(gl_version >= 21)
	{
		glUseProgram(0);
	}

	glBindTexture(GL_TEXTURE_2D, 0);

	glDisable(GL_TEXTURE_2D);
glPopMatrix();
//END DANBO2========================================


//DANBO3================
glPushMatrix();
glTranslatef(-5.0f, 0.0f, 0.0f);
glRotatef(90.0, 0.0f, 1.0f, 0.0f);
	glEnable(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, Texture[1]);

	if(gl_version >= 21)
	{
		glUseProgram(Shader);
	}

//--kaki 1
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, TexCoords);

	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, 0, Normals);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, VerticesB);


	glDrawArrays(GL_QUADS, 0, 24);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

//--kaki2
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, TexCoords);

	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, 0, Normals);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, Vertices1B);

	glDrawArrays(GL_QUADS, 0, 24);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

//--badan
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, TexCoords);

	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, 0, Normals);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, Vertices2B);

	glDrawArrays(GL_QUADS, 0, 24);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

//--kepala1
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, TexCoords);

	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, 0, Normals);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, Vertices3B);

	glDrawArrays(GL_QUADS, 0, 24);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);


//--tangan kiri
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, TexCoords);

	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, 0, Normals);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, Vertices5B);

	glDrawArrays(GL_QUADS, 0, 24);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	
	//--tangan kanan
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, TexCoords);

	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, 0, Normals);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, Vertices6B);

	glDrawArrays(GL_QUADS, 0, 24);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	if(gl_version >= 21)
	{
		glUseProgram(0);
	}

	glBindTexture(GL_TEXTURE_2D, 0);

	glDisable(GL_TEXTURE_2D);
	
//--Texture2

	glEnable(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, Texture[2]);

	if(gl_version >= 21)
	{
		glUseProgram(Shader);
	}
//--kepala2
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, TexCoords);

	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, 0, Normals);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, Vertices4B);

	glDrawArrays(GL_QUADS, 0, 24);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	if(gl_version >= 21)
	{
		glUseProgram(0);
	}

	glBindTexture(GL_TEXTURE_2D, 0);

	glDisable(GL_TEXTURE_2D);
glPopMatrix();
//END DANBO3=====================

//DANBO4================
glPushMatrix();
glTranslatef(-125.0f, 0.0f, -100.0f);
glRotatef(90.0, 0.0f, 1.0f, 0.0f);
//glTranslatef(.0f, 20.0f, 200.0f);
	glEnable(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, Texture[1]);

	if(gl_version >= 21)
	{
		glUseProgram(Shader);
	}

//--kaki 1
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, TexCoords);

	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, 0, Normals);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, VerticesC);


	glDrawArrays(GL_QUADS, 0, 24);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

//--kaki2
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, TexCoords);

	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, 0, Normals);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, Vertices1C);

	glDrawArrays(GL_QUADS, 0, 24);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

//--badan
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, TexCoords);

	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, 0, Normals);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, Vertices2C);

	glDrawArrays(GL_QUADS, 0, 24);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

//--kepala1
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, TexCoords);

	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, 0, Normals);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, Vertices3C);

	glDrawArrays(GL_QUADS, 0, 24);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);


//--tangan kiri
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, TexCoords);

	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, 0, Normals);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, Vertices5C);

	glDrawArrays(GL_QUADS, 0, 24);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	
	//--tangan kanan
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, TexCoords);

	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, 0, Normals);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, Vertices6C);

	glDrawArrays(GL_QUADS, 0, 24);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	if(gl_version >= 21)
	{
		glUseProgram(0);
	}

	glBindTexture(GL_TEXTURE_2D, 0);

	glDisable(GL_TEXTURE_2D);
	
//--Texture2

	glEnable(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, Texture[2]);

	if(gl_version >= 21)
	{
		glUseProgram(Shader);
	}
//--kepala2
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, TexCoords);

	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, 0, Normals);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, Vertices4C);

	glDrawArrays(GL_QUADS, 0, 24);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	if(gl_version >= 21)
	{
		glUseProgram(0);
	}

	glBindTexture(GL_TEXTURE_2D, 0);

	glDisable(GL_TEXTURE_2D);
glPopMatrix();
//END DANBO4=====================




//DANBOE================
glPushMatrix();
glTranslatef(0.0f, 0.0f, -80.0f);
glRotatef(-70.0, 0.0f, 1.0f, 0.0f);

	glEnable(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, Texture[1]);

	if(gl_version >= 21)
	{
		glUseProgram(Shader);
	}

//--kaki 1
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, TexCoords);

	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, 0, Normals);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, VerticesE);


	glDrawArrays(GL_QUADS, 0, 24);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

//--kaki2
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, TexCoords);

	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, 0, Normals);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, Vertices1E);

	glDrawArrays(GL_QUADS, 0, 24);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

//--badan
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, TexCoords);

	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, 0, Normals);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, Vertices2E);

	glDrawArrays(GL_QUADS, 0, 24);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

//--kepala1
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, TexCoords);

	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, 0, Normals);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, Vertices3E);

	glDrawArrays(GL_QUADS, 0, 24);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);


//--tangan kiri
glPushMatrix();
glTranslatef(0.0f, 6.3f, -1.0f);
glRotatef(90.0, 1.0f, 0.0f, 0.0f);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, TexCoords);


	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, 0, Normals);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, Vertices5E);

	glDrawArrays(GL_QUADS, 0, 24);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
glPopMatrix();	
	//--tangan kanan
/*glPushMatrix();
glTranslatef(0.0f, 5.3f, 1.0f);
glRotatef(90.0, 3.0f, 1.0f, 0.0f);	*/
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, TexCoords);

	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, 0, Normals);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, Vertices6E);

	glDrawArrays(GL_QUADS, 0, 24);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
//glPopMatrix();
	if(gl_version >= 21)
	{
		glUseProgram(0);
	}

	glBindTexture(GL_TEXTURE_2D, 0);

	glDisable(GL_TEXTURE_2D);
	
//--Texture2

	glEnable(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, Texture[2]);

	if(gl_version >= 21)
	{
		glUseProgram(Shader);
	}
//--kepala2
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, TexCoords);

	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, 0, Normals);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, Vertices4E);

	glDrawArrays(GL_QUADS, 0, 24);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	if(gl_version >= 21)
	{
		glUseProgram(0);
	}

	glBindTexture(GL_TEXTURE_2D, 0);

	glDisable(GL_TEXTURE_2D);
glPopMatrix();
//END DANBOE=====================


//DANBOD================
//glPushMatrix();
//glTranslatef(0.0f, 0.0f, -30.0f);

//glRotatef(-65.0, 0.0f, 1.0f, 0.0f);
//glTranslatef(0.0f, 0.0f, 80.0f);
	glEnable(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, Texture[1]);

	if(gl_version >= 21)
	{
		glUseProgram(Shader);
	}

//--kaki 1
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, TexCoords);

	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, 0, Normals);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, VerticesD);


	glDrawArrays(GL_QUADS, 0, 24);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

//--kaki2
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, TexCoords);

	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, 0, Normals);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, Vertices1D);

	glDrawArrays(GL_QUADS, 0, 24);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

//--badan
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, TexCoords);

	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, 0, Normals);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, Vertices2D);

	glDrawArrays(GL_QUADS, 0, 24);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

//--kepala1
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, TexCoords);

	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, 0, Normals);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, Vertices3D);

	glDrawArrays(GL_QUADS, 0, 24);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);


//--tangan kiri
//glPushMatrix();
//glTranslatef(0.0f, 6.3f, -1.0f);
//glRotatef(90.0, 1.0f, 0.0f, 0.0f);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, TexCoords);


	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, 0, Normals);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, Vertices5D);

	glDrawArrays(GL_QUADS, 0, 24);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
//glPopMatrix();	
	//--tangan kanan
//glPushMatrix();
//glTranslatef(0.0f, 5.3f, 1.0f);
//glRotatef(90.0, 2.0f, 1.0f, 0.0f);	
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, TexCoords);

	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, 0, Normals);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, Vertices6D);

	glDrawArrays(GL_QUADS, 0, 24);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
//glPopMatrix();
	if(gl_version >= 21)
	{
		glUseProgram(0);
	}

	glBindTexture(GL_TEXTURE_2D, 0);

	glDisable(GL_TEXTURE_2D);
	
//--Texture2

	glEnable(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, Texture[2]);

	if(gl_version >= 21)
	{
		glUseProgram(Shader);
	}
//--kepala2
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, TexCoords);

	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, 0, Normals);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, Vertices4D);

	glDrawArrays(GL_QUADS, 0, 24);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	if(gl_version >= 21)
	{
		glUseProgram(0);
	}

	glBindTexture(GL_TEXTURE_2D, 0);

	glDisable(GL_TEXTURE_2D);
//glPopMatrix();
//END DANBOD=====================









//--Texture RUMAH====================================

	/*glEnable(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, Texture[3]);

	if(gl_version >= 21)
	{
		glUseProgram(Shader);
	}
//--kepala2
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, TexCoords);

	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, 0, Normals);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, VerRumah);

	glDrawArrays(GL_QUADS, 0, 24);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	if(gl_version >= 21)
	{
		glUseProgram(0);
	}

	glBindTexture(GL_TEXTURE_2D, 0);

	glDisable(GL_TEXTURE_2D);
	                         
 */
//--Texture RUMPUT====================================

	glEnable(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, Texture[4]);

	if(gl_version >= 21)
	{
		glUseProgram(Shader);
	}
//--
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, TexCoordsRumput);

	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, 0, Normals);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, VerRumput);

	glDrawArrays(GL_QUADS, 0, 24);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	if(gl_version >= 21)
	{
		glUseProgram(0);
	}

	glBindTexture(GL_TEXTURE_2D, 0);

	glDisable(GL_TEXTURE_2D);
	
	
//--Texture RUMPUT2====================================

	glEnable(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, Texture[4]);

	if(gl_version >= 21)
	{
		glUseProgram(Shader);
	}
//--
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, TexCoordsRumput2);

	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, 0, Normals);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, VerRumput2);

	glDrawArrays(GL_QUADS, 0, 24);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	if(gl_version >= 21)
	{
		glUseProgram(0);
	}

	glBindTexture(GL_TEXTURE_2D, 0);

	glDisable(GL_TEXTURE_2D);
	
	
//--Texture TANGGA1====================================

	glEnable(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, Texture[6]);

	if(gl_version >= 21)
	{
		glUseProgram(Shader);
	}
//--
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, TexTangga1);

	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, 0, Normals);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, VerTangga1);

	glDrawArrays(GL_QUADS, 0, 24);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	if(gl_version >= 21)
	{
		glUseProgram(0);
	}

	glBindTexture(GL_TEXTURE_2D, 0);

	glDisable(GL_TEXTURE_2D);	
	
	
//--Texture TANGGA2====================================

	glEnable(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, Texture[8]);

	if(gl_version >= 21)
	{
		glUseProgram(Shader);
	}
//--
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, TexTangga2);

	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, 0, Normals);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, VerTangga2);

	glDrawArrays(GL_QUADS, 0, 80);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

//++
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, TexTangga2);

	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, 0, Normals);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, VerTangga3);

	glDrawArrays(GL_QUADS, 0, 24);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
//++
//++
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, TexTangga2);

	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, 0, Normals);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, VerTangga4);

	glDrawArrays(GL_QUADS, 0, 24);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
//++
	if(gl_version >= 21)
	{
		glUseProgram(0);
	}

	glBindTexture(GL_TEXTURE_2D, 0);

	glDisable(GL_TEXTURE_2D);		

//--Texture RUMPUT====================================

	glEnable(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, Texture[5]);

	if(gl_version >= 21)
	{
		glUseProgram(Shader);
	}
//--
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, TexCoordsRumput3);

	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, 0, Normals);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, VerRumput3);

	glDrawArrays(GL_QUADS, 0, 24);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	if(gl_version >= 21)
	{
		glUseProgram(0);
	}

	glBindTexture(GL_TEXTURE_2D, 0);

	glDisable(GL_TEXTURE_2D);


//--JALAN-------------------------------------------
//--Texture RUMPUT====================================
/*
	glEnable(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, Texture[19]);

	if(gl_version >= 21)
	{
		glUseProgram(Shader);
	}
//--
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, TexJalan);

	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, 0, Normals);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, VerJalan);

	glDrawArrays(GL_QUADS, 0, 24);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	if(gl_version >= 21)
	{
		glUseProgram(0);
	}

	glBindTexture(GL_TEXTURE_2D, 0);

	glDisable(GL_TEXTURE_2D);
	*/
//--Texture BATA====================================
/*
	glEnable(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, Texture[7]);

	if(gl_version >= 21)
	{
		glUseProgram(Shader);
	}
//--
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, TexBata);

	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, 0, Normals);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, VerBata);

	glDrawArrays(GL_QUADS, 0, 24);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	if(gl_version >= 21)
	{
		glUseProgram(0);
	}

	glBindTexture(GL_TEXTURE_2D, 0);

	glDisable(GL_TEXTURE_2D);
*/
//--Texture TEMBOK====================================

	glEnable(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, Texture[9]);

	if(gl_version >= 21)
	{
		glUseProgram(Shader);
	}
//--
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, TexTembok);

	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, 0, Normals);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, VerTembok);

	glDrawArrays(GL_QUADS, 0, 24);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	if(gl_version >= 21)
	{
		glUseProgram(0);
	}

	glBindTexture(GL_TEXTURE_2D, 0);

	glDisable(GL_TEXTURE_2D);
	
//--Texture PINTU====================================

	glEnable(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, Texture[10]);

	if(gl_version >= 21)
	{
		glUseProgram(Shader);
	}
//--
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, TexPintu);

	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, 0, Normals);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, VerPintu);

	glDrawArrays(GL_QUADS, 0, 24);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	if(gl_version >= 21)
	{
		glUseProgram(0);
	}

	glBindTexture(GL_TEXTURE_2D, 0);

	glDisable(GL_TEXTURE_2D);
	
	
//--Texture JENDELA1====================================

	glEnable(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, Texture[11]);

	if(gl_version >= 21)
	{
		glUseProgram(Shader);
	}
//--
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, TexJendela1);

	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, 0, Normals);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, VerJendela1);
	
	glDrawArrays(GL_QUADS, 0, 24);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
//++
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, TexJendela1a);

	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, 0, Normals);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, VerJendela1a);
	

	glDrawArrays(GL_QUADS, 0, 24);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
//++

//++
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, TexJendela1a);

	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, 0, Normals);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, VerKusen1);
	

	glDrawArrays(GL_QUADS, 0, 24);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
//++
	if(gl_version >= 21)
	{
		glUseProgram(0);
	}

	glBindTexture(GL_TEXTURE_2D, 0);

	glDisable(GL_TEXTURE_2D);
	
//--Texture JENDELA1====================================

	glEnable(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, Texture[11]);

	if(gl_version >= 21)
	{
		glUseProgram(Shader);
	}
//--
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, TexJendela2);

	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, 0, Normals);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, VerJendela2);

	glDrawArrays(GL_QUADS, 0, 24);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

//++
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, TexJendela2a);

	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, 0, Normals);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, VerJendela2a);
	

	glDrawArrays(GL_QUADS, 0, 24);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
//++

//++
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, TexJendela2a);

	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, 0, Normals);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, VerKusen2);
	

	glDrawArrays(GL_QUADS, 0, 24);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
//++
	if(gl_version >= 21)
	{
		glUseProgram(0);
	}

	glBindTexture(GL_TEXTURE_2D, 0);

	glDisable(GL_TEXTURE_2D);	
	
	//--Texture JENDELA1====================================

	glEnable(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, Texture[12]);

	if(gl_version >= 21)
	{
		glUseProgram(Shader);
	}
//--
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, TexJendela3);

	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, 0, Normals);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, VerJendela3);

	glDrawArrays(GL_QUADS, 0, 24);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

//++
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, TexJendela3a);

	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, 0, Normals);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, VerJendela3a);
	

	glDrawArrays(GL_QUADS, 0, 24);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
//++
//++
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, TexJendela3a);

	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, 0, Normals);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, VerKusen3);
	

	glDrawArrays(GL_QUADS, 0, 24);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
//++
	if(gl_version >= 21)
	{
		glUseProgram(0);
	}

	glBindTexture(GL_TEXTURE_2D, 0);

	glDisable(GL_TEXTURE_2D);
    
    //--Texture JENDELA1====================================

	glEnable(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, Texture[12]);

	if(gl_version >= 21)
	{
		glUseProgram(Shader);
	}
//--
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, TexJendela4);

	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, 0, Normals);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, VerJendela4);

	glDrawArrays(GL_QUADS, 0, 24);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);


//++
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, TexJendela4a);

	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, 0, Normals);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, VerJendela4a);
	

	glDrawArrays(GL_QUADS, 0, 24);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
//++
	if(gl_version >= 21)
	{
		glUseProgram(0);
	}

	glBindTexture(GL_TEXTURE_2D, 0);

	glDisable(GL_TEXTURE_2D);
    
    //--Texture JENDELA1====================================

	glEnable(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, Texture[13]);

	if(gl_version >= 21)
	{
		glUseProgram(Shader);
	}
//--
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, TexJendela5);

	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, 0, Normals);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, VerJendela5);

	glDrawArrays(GL_QUADS, 0, 24);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

//++

    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, TexJendela5a);

	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, 0, Normals);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, VerJendela5a);
	

	glDrawArrays(GL_QUADS, 0, 24);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
//++

	if(gl_version >= 21)
	{
		glUseProgram(0);
	}

	glBindTexture(GL_TEXTURE_2D, 0);

	glDisable(GL_TEXTURE_2D);	
    
    
    
    
//========================  
    glPushMatrix();  
	glEnable(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, Texture[10]);

	if(gl_version >= 21)
	{
		glUseProgram(Shader);
	}
	   glTranslatef(-30.0,5.0f,70);
	   
	//	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	//glTexCoordPointer(2, GL_FLOAT, 0, TexJendela5);

	//glEnableClientState(GL_NORMAL_ARRAY);
	//glNormalPointer(GL_FLOAT, 0, Normals);   
    // Set material properties
    //glRotatef(30,0.0,1.0,0.0);
    //glRotatef(30,0.0,0.0,1.0);
      /* glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, qaRed);

    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, qaRed);

     glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, qaRed);

     glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 20);
 */
    glutSolidCube(5);
    
    //glutSolidTorus(0.3f,0.8f,50,50);
    //glutSolidSphere(15,50,15);
    //glutSolidCone(0.3f,0.8f,50,50);
    //glDrawArrays(GL_QUADS, 0, 24);
	//glDisableClientState(GL_VERTEX_ARRAY);
	//glDisableClientState(GL_NORMAL_ARRAY);
	//glDisableClientState(GL_TEXTURE_COORD_ARRAY);
      //- See more at: http://www.codemiles.com/c-opengl-examples/add-spot-light-to-object-t9154.html#sthash.ObJtMYhU.dpuf
    
	if(gl_version >= 21)
	{
		glUseProgram(0);
	}

	glBindTexture(GL_TEXTURE_2D, 0);

	glDisable(GL_TEXTURE_2D);	
	
	
     glPopMatrix();
     
     
         glPushMatrix();  
	glEnable(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, Texture[10]);

	if(gl_version >= 21)
	{
		glUseProgram(Shader);
	}
	   glTranslatef(-30.0,5.0f,80);
glRotatef(30,0.0,1.0,0.0);
    glutSolidCube(5);

	if(gl_version >= 21)
	{
		glUseProgram(0);
	}

	glBindTexture(GL_TEXTURE_2D, 0);

	glDisable(GL_TEXTURE_2D);	
     glPopMatrix();
     
    glPushMatrix();  
	glEnable(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, Texture[10]);

	if(gl_version >= 21)
	{
		glUseProgram(Shader);
	}
    glTranslatef(-30.0,10.0f,75);
    glRotatef(60,0.0,1.0,0.0);
    glutSolidCube(5);

	if(gl_version >= 21)
	{
		glUseProgram(0);
	}

	glBindTexture(GL_TEXTURE_2D, 0);

	glDisable(GL_TEXTURE_2D);	
    glPopMatrix();
    
 //--
     glPushMatrix();  
	glEnable(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, Texture[10]);

	if(gl_version >= 21)
	{
		glUseProgram(Shader);
	}
    glTranslatef(-30.0,5.0f,63);
    glRotatef(-60,0.0,1.0,0.0);
    glutSolidCube(5);

	if(gl_version >= 21)
	{
		glUseProgram(0);
	}

	glBindTexture(GL_TEXTURE_2D, 0);

	glDisable(GL_TEXTURE_2D);	
    glPopMatrix();
    
  //--
     glPushMatrix();  
	glEnable(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, Texture[10]);

	if(gl_version >= 21)
	{
		glUseProgram(Shader);
	}
    glTranslatef(-30.0,10.0f,65);
    glRotatef(-20,0.0,1.0,0.0);
    glutSolidCube(5);

	if(gl_version >= 21)
	{
		glUseProgram(0);
	}

	glBindTexture(GL_TEXTURE_2D, 0);

	glDisable(GL_TEXTURE_2D);	
    glPopMatrix();
    
  //--
     glPushMatrix();  
	glEnable(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, Texture[10]);

	if(gl_version >= 21)
	{
		glUseProgram(Shader);
	}
    glTranslatef(-30.0,15.0f,70);
    //glRotatef(-20,0.0,1.0,0.0);
    glutSolidCube(5);

	if(gl_version >= 21)
	{
		glUseProgram(0);
	}

	glBindTexture(GL_TEXTURE_2D, 0);

	glDisable(GL_TEXTURE_2D);	
    glPopMatrix();
/*     
    glPushMatrix();  
    //glRotatef(90,0.0,500.0,90.0);
     
    glLightfv(GL_LIGHT0, GL_POSITION, qaLightPosition);
 

    glPopMatrix(); 


    

    glFlush();
    glutSwapBuffers();*/  
    
    

//--Texture KANDANG====================================
     glPushMatrix();  
     
    glTranslatef(-100.0,0.0f,-100);
    glRotatef(60,0.0,1.0,0.0);
	glEnable(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, Texture[16]);

	if(gl_version >= 21)
	{
		glUseProgram(Shader);
	}
//--
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, TexKd);

	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, 0, Normals);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, VerKd);

	glDrawArrays(GL_QUADS, 0, 28);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	if(gl_version >= 21)
	{
		glUseProgram(0);
	}

	glBindTexture(GL_TEXTURE_2D, 0);

	glDisable(GL_TEXTURE_2D);	    




	glEnable(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, Texture[15]);

	if(gl_version >= 21)
	{
		glUseProgram(Shader);
	}
//--
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, TexKd);

	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, 0, Normals);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, VerKd2);

	glDrawArrays(GL_QUADS, 0, 24);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

//--
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, TexKd);

	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, 0, Normals);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, VerKd3);

	glDrawArrays(GL_QUADS, 0, 24);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);


	if(gl_version >= 21)
	{
		glUseProgram(0);
	}

	glBindTexture(GL_TEXTURE_2D, 0);

	glDisable(GL_TEXTURE_2D);	    

//===
	glEnable(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, Texture[17]);

	if(gl_version >= 21)
	{
		glUseProgram(Shader);
	}
//--
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, TexCoords);

	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, 0, Normals);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, VerKd4);

	glDrawArrays(GL_QUADS, 0, 4);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	if(gl_version >= 21)
	{
		glUseProgram(0);
	}

	glBindTexture(GL_TEXTURE_2D, 0);

	glDisable(GL_TEXTURE_2D);	    


	
	 glPopMatrix();
	 
	 //========
/*

*/

quadratic=gluNewQuadric();          // Create A Pointer To The Quadric Object ( NEW )
gluQuadricNormals(quadratic, GLU_SMOOTH);   // Create Smooth Normals ( NEW )
gluQuadricTexture(quadratic, GL_TRUE);      // Create Texture Coords ( NEW )



	glEnable(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, Texture[21]);

	if(gl_version >= 21)
	{
		glUseProgram(Shader);
	}
	
glPushMatrix();	
	   glTranslatef(0.0f,0.0f,-60.0f);
	   glTranslatef(0.0f,7.0f,0.0f);
	   glTranslatef(-10.0f,0.0f,0.0f);
	   //glRotatef(90,1.0f,0.0f,0.0f);
//glutSolidDodecahedron();
//glutSolidCone(5,10,2,2);
    //glutWireTeapot(10);
    
//gluCylinder(quadratic,10.0f,10.0f,30.0f,32,32); 
gluSphere(quadratic,8.0f,10,20);
//gluDisk(quadratic,10.5f,11.5f,32,32); 
//gluCylinder(quadratic,11.0f,11.0f,13.0f,32,32);
//glDrawCube(); 

glPopMatrix();  

  glPushMatrix();	
	   glTranslatef(0.0f,0.0f,-50.0f);
	   glTranslatef(0.0f,7.0f,0.0f);
	   glTranslatef(-30.0f,0.0f,0.0f);
	   //glRotatef(90,1.0f,0.0f,0.0f);
//glutSolidDodecahedron();
//glutSolidCone(5,10,2,2);
    //glutWireTeapot(10);
    
//gluCylinder(quadratic,10.0f,10.0f,30.0f,32,32); 
gluSphere(quadratic,5.0f,6,20);
//gluDisk(quadratic,10.5f,11.5f,32,32); 
//gluCylinder(quadratic,11.0f,11.0f,13.0f,32,32);
//glDrawCube(); 

glPopMatrix();      
	if(gl_version >= 21)
	{
		glUseProgram(0);
	}

	glBindTexture(GL_TEXTURE_2D, 0);

	glDisable(GL_TEXTURE_2D);	
               //glTranslatef(-4.0f,-2.5f,-8.0f);
               //glutSolidIcosahedron(5);
               //glutSolidTeapot(10);


//glFlush();


//--ATAP

	glEnable(GL_TEXTURE_2D);

	//glBindTexture(GL_TEXTURE_2D, Texture[17]);

	if(gl_version >= 21)
	{
		glUseProgram(Shader);
	}
//--
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, TexAtap);

	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, 0, Normals);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, VerAtap);

	glDrawArrays(GL_QUADS, 0, 24);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	if(gl_version >= 21)
	{
		glUseProgram(0);
	}

	glBindTexture(GL_TEXTURE_2D, 0);

	glDisable(GL_TEXTURE_2D);
	
	
//--ATAP2

	glEnable(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, Texture[16]);

	if(gl_version >= 21)
	{
		glUseProgram(Shader);
	}
//--
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, TexAtap);

	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, 0, Normals);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, VerAtap2);

	glDrawArrays(GL_QUADS, 0, 24);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);






	



	if(gl_version >= 21)
	{
		glUseProgram(0);
	}

	glBindTexture(GL_TEXTURE_2D, 0);

	glDisable(GL_TEXTURE_2D);
	
	
}

void COpenGLRenderer::Resize(int Width, int Height)
{
	this->Width = Width;
	this->Height = Height;

	glViewport(0, 0, Width, Height);

	Projection = perspective(45.0f, (float)Width / (Height > 0 ? (float)Height : 1.0f), 0.125f, 512.0f);

	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf((GLfloat*)&Projection);
	
	
	
	
}

void COpenGLRenderer::Destroy()
{
	Texture[1].Delete();
	
	if(gl_version >= 21)
	{
		Shader.Delete();
	}

	delete [] TexCoords;
	delete [] Normals;
	delete [] Vertices;
	
	delete [] TexCoords1;
	delete [] Normals1;
	delete [] Vertices1;

	delete [] TexCoords2;
	delete [] Normals2;
	delete [] Vertices2;
	
	delete [] Vertices3;
	delete [] Vertices4;
	delete [] Vertices5;
	delete [] Vertices6;

	delete [] VerticesA;
	delete [] Vertices1A;	
	delete [] Vertices2A;
	delete [] Vertices3A;
	delete [] Vertices4A;
	delete [] Vertices5A;
	delete [] Vertices6A;
	
	delete [] VerticesB;
	delete [] Vertices1B;	
	delete [] Vertices2B;
	delete [] Vertices3B;
	delete [] Vertices4B;
	delete [] Vertices5B;
	delete [] Vertices6B;
	
		delete [] VerticesC;
	delete [] Vertices1C;	
	delete [] Vertices2C;
	delete [] Vertices3C;
	delete [] Vertices4C;
	delete [] Vertices5C;
	delete [] Vertices6C;
	
		delete [] VerticesD;
	delete [] Vertices1D;	
	delete [] Vertices2D;
	delete [] Vertices3D;
	delete [] Vertices4D;
	delete [] Vertices5D;
	delete [] Vertices6D;
	
	delete [] VerticesE;
	delete [] Vertices1E;	
	delete [] Vertices2E;
	delete [] Vertices3E;
	delete [] Vertices4E;
	delete [] Vertices5E;
	delete [] Vertices6E;
//==RUMAH
    delete [] VerRumah;
    	
//==RUMPUT
    delete [] VerRumput;
    delete [] TexCoordsRumput;

    delete [] VerRumput2;
    delete [] TexCoordsRumput2;
        delete [] VerRumput3;
    delete [] TexCoordsRumput3;
    
//==TANGGA
    delete [] VerTangga1;
    delete [] TexTangga1;
    delete [] VerTangga2;
    delete [] TexTangga2;
    delete [] VerTangga3;
    delete [] TexTangga3;
    delete [] VerTangga4;
    delete [] TexTangga4;
      
    
//==BATA
    delete [] VerBata;
    delete [] TexBata;    

//==BATA
    delete [] VerTembok;
    delete [] TexTembok;    

//==PINTU
    delete [] VerPintu;
    delete [] TexPintu;

//==JENDELA
    delete [] VerJendela1;
    delete [] TexJendela1;
    delete [] VerJendela2;
    delete [] TexJendela2;
    delete [] VerJendela3;
    delete [] TexJendela3;
    delete [] VerJendela4;
    delete [] TexJendela4;    
    delete [] VerJendela5;
    delete [] TexJendela5;    
   
    delete [] VerJendela1a;
    delete [] TexJendela1a;
    delete [] VerJendela2a;
    delete [] TexJendela2a;
    delete [] VerJendela3a;
    delete [] TexJendela3a;
    delete [] VerJendela4a;
    delete [] TexJendela4a;    
    delete [] VerJendela5a;
    delete [] TexJendela5a;  
    
//==KUSEN
    delete [] VerKusen1;
    delete [] VerKusen2;
    delete [] VerKusen3;

//==KANDANG
    delete [] VerKd;
    delete [] VerKd2;
    delete [] VerKd3;  
    delete [] VerKd4;
    delete [] VerJalan;  
    delete [] TexAtap;
    delete [] VerAtap;
    delete [] VerAtap2;
    
    gluDeleteQuadric(quadratic);
}

COpenGLRenderer OpenGLRenderer;

// ----------------------------------------------------------------------------------------------------------------------------

CWnd::CWnd()
{
	char *moduledirectory = new char[256];
	GetModuleFileName(GetModuleHandle(NULL), moduledirectory, 256);
	*(strrchr(moduledirectory, '\\') + 1) = 0;
	ModuleDirectory = moduledirectory;
	delete [] moduledirectory;

	DeFullScreened = false;
}

CWnd::~CWnd()
{
}

bool CWnd::Create(HINSTANCE hInstance, char *WindowName, int Width, int Height, bool FullScreen, int Samples, bool CreateForwardCompatibleContext, bool DisableVerticalSynchronization)
{
	WNDCLASSEX WndClassEx;

	memset(&WndClassEx, 0, sizeof(WNDCLASSEX));

	WndClassEx.cbSize = sizeof(WNDCLASSEX);
	WndClassEx.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	WndClassEx.lpfnWndProc = WndProc;
	WndClassEx.hInstance = hInstance;
	WndClassEx.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	WndClassEx.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	WndClassEx.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClassEx.lpszClassName = "Win32OpenGLWindowClass";

	if(!RegisterClassEx(&WndClassEx))
	{
		ErrorLog.Set("RegisterClassEx failed!");
		return false;
	}

	this->WindowName = WindowName;

    this->Width = Width;
    this->Height = Height;

	DWORD Style = (FullScreen ? WS_POPUP : WS_OVERLAPPEDWINDOW) | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

	if((hWnd = CreateWindowEx(WS_EX_APPWINDOW, WndClassEx.lpszClassName, WindowName, Style, 0, 0, Width, Height, NULL, NULL, hInstance, NULL)) == NULL)
	{
		ErrorLog.Set("CreateWindowEx failed!");
		return false;
	}

	this->FullScreen = FullScreen;

	if(FullScreen)
	{
		memset(&DevMode, 0, sizeof(DEVMODE));

		DevMode.dmSize = sizeof(DEVMODE);
		DevMode.dmPelsWidth = Width;
		DevMode.dmPelsHeight = Height;
		DevMode.dmBitsPerPel = 32;
		DevMode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL;

		this->FullScreen = ChangeDisplaySettings(&DevMode, CDS_FULLSCREEN) == DISP_CHANGE_SUCCESSFUL;
	}

	if((hDC = GetDC(hWnd)) == NULL)
	{
		ErrorLog.Set("GetDC failed!");
		return false;
	}

	PIXELFORMATDESCRIPTOR pfd;

	memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));

	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cDepthBits = 24;
	pfd.iLayerType = PFD_MAIN_PLANE;

	int PixelFormat;

	if((PixelFormat = ChoosePixelFormat(hDC, &pfd)) == 0)
	{
		ErrorLog.Set("ChoosePixelFormat failed!");
		return false;
	}

	static int MSAAPixelFormat = 0;

	if(SetPixelFormat(hDC, MSAAPixelFormat == 0 ? PixelFormat : MSAAPixelFormat, &pfd) == FALSE)
	{
		ErrorLog.Set("SetPixelFormat failed!");
		return false;
	}

	if((hGLRC = wglCreateContext(hDC)) == NULL)
	{
		ErrorLog.Set("wglCreateContext failed!");
		return false;
	}

	if(wglMakeCurrent(hDC, hGLRC) == FALSE)
	{
		ErrorLog.Set("wglMakeCurrent failed!");
		return false;
	}

	if(glewInit() != GLEW_OK)
	{
		ErrorLog.Set("glewInit failed!");
		return false;
	}

	if(MSAAPixelFormat == 0 && Samples > 0)
	{
		if(GLEW_ARB_multisample && WGLEW_ARB_pixel_format)
		{
			while(Samples > 0)
			{
				UINT NumFormats = 0;

				int iAttributes[] =
				{
					WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
					WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
					WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
					WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
					WGL_COLOR_BITS_ARB, 32,
					WGL_DEPTH_BITS_ARB, 24,
					WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
					WGL_SAMPLE_BUFFERS_ARB, GL_TRUE,
					WGL_SAMPLES_ARB, Samples,
					0
				};

				if(wglChoosePixelFormatARB(hDC, iAttributes, NULL, 1, &MSAAPixelFormat, &NumFormats) == TRUE && NumFormats > 0) break;
				
				Samples--;
			}

			wglDeleteContext(hGLRC);

			DestroyWindow(hWnd);

			UnregisterClass(WndClassEx.lpszClassName, hInstance);

			return Create(hInstance, WindowName, Width, Height, FullScreen, Samples, CreateForwardCompatibleContext, DisableVerticalSynchronization);
		}
		else
		{
			Samples = 0;
		}
	}

	this->Samples = Samples;

	int major, minor;

	sscanf((char*)glGetString(GL_VERSION), "%d.%d", &major, &minor);

	gl_version = major * 10 + minor;

	if(CreateForwardCompatibleContext && gl_version >= 30 && WGLEW_ARB_create_context)
	{
		wglDeleteContext(hGLRC);

		int GLFCRCAttribs[] =
		{
			WGL_CONTEXT_MAJOR_VERSION_ARB, major,
			WGL_CONTEXT_MINOR_VERSION_ARB, minor,
			WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
			0
		};

		if((hGLRC = wglCreateContextAttribsARB(hDC, 0, GLFCRCAttribs)) == NULL)
		{
			ErrorLog.Set("wglCreateContextAttribsARB failed!");
			return false;
		}

		if(wglMakeCurrent(hDC, hGLRC) == FALSE)
		{
			ErrorLog.Set("wglMakeCurrent failed!");
			return false;
		}

		wgl_context_forward_compatible = true;
	}
	else
	{
		wgl_context_forward_compatible = false;
	}

	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &gl_max_texture_size);

	if(GLEW_EXT_texture_filter_anisotropic)
	{
		glGetIntegerv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &gl_max_texture_max_anisotropy_ext);
	}

	if(DisableVerticalSynchronization  && WGLEW_EXT_swap_control)
	{
		wglSwapIntervalEXT(0);
	}

	return OpenGLRenderer.Init();
}

void CWnd::Show(bool MouseGameMode, bool Maximized)
{
	this->MouseGameMode = MouseGameMode;

	if(!FullScreen)
	{
		RECT dRect, wRect, cRect;

		GetWindowRect(GetDesktopWindow(), &dRect);
		GetWindowRect(hWnd, &wRect);
		GetClientRect(hWnd, &cRect);

		wRect.right += Width - cRect.right;
		wRect.bottom += Height - cRect.bottom;
		
		wRect.right -= wRect.left;
		wRect.bottom -= wRect.top;

		wRect.left = dRect.right / 2 - wRect.right / 2;
		wRect.top = dRect.bottom / 2 - wRect.bottom / 2;

		MoveWindow(hWnd, wRect.left, wRect.top, wRect.right, wRect.bottom, FALSE);
	}
	else
	{
		this->MouseGameMode = true;
	}

	OpenGLRenderer.Resize(Width, Height);

	StartFPSCounter();

	ShowWindow(hWnd, (!FullScreen && Maximized) ? SW_SHOWMAXIMIZED : SW_SHOWNORMAL);

	SetForegroundWindow(hWnd);

	KeyBoardFocus = MouseFocus = true;

	SetCurAccToMouseGameMode();
}

void CWnd::MsgLoop()
{
	MSG Msg;

	while(GetMessage(&Msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}
}

void CWnd::Destroy()
{
	OpenGLRenderer.Destroy();

	wglDeleteContext(hGLRC);

	DestroyWindow(hWnd);
}

void CWnd::GetCurPos(int *cx, int *cy)
{
	POINT Point;

	GetCursorPos(&Point);

	ScreenToClient(hWnd, &Point);

	*cx = Point.x;
	*cy = Point.y;
}

void CWnd::SetCurPos(int cx, int cy)
{
	POINT Point;

	Point.x = cx;
	Point.y = cy;

	ClientToScreen(hWnd, &Point);

	SetCursorPos(Point.x, Point.y);
}

void CWnd::SetCurAccToMouseGameMode()
{
	if(MouseGameMode)
	{
		SetCurPos(WidthD2, HeightD2);
		while(ShowCursor(FALSE) >= 0);
	}
	else
	{
		while(ShowCursor(TRUE) < 0);
	}
}

void CWnd::SetMouseFocus()
{
	SetCurAccToMouseGameMode();
	MouseFocus = true;
}

void CWnd::StartFPSCounter()
{
	Start = Begin = GetTickCount();
}

void CWnd::OnKeyDown(UINT nChar)
{
	switch(nChar)
	{
		case VK_ESCAPE:
			PostQuitMessage(0);
			break;

		case VK_F1:
			OpenGLRenderer.ShowAxisGrid = !OpenGLRenderer.ShowAxisGrid;
			break;

		case VK_F2:
			if(!FullScreen && MouseFocus)
			{
				MouseGameMode = !MouseGameMode;
				SetCurAccToMouseGameMode();
			}
			break;

		case VK_F3:
			if(!FullScreen)
			{
				WINDOWPLACEMENT WndPlcm;
				WndPlcm.length = sizeof(WINDOWPLACEMENT);
				GetWindowPlacement(hWnd, &WndPlcm);
				if(WndPlcm.showCmd == SW_SHOWNORMAL) ShowWindow(hWnd, SW_SHOWMAXIMIZED);
				if(WndPlcm.showCmd == SW_SHOWMAXIMIZED) ShowWindow(hWnd, SW_SHOWNORMAL);
			}
			break;

		case VK_SPACE:
			OpenGLRenderer.Stop = !OpenGLRenderer.Stop;
			break;
	}
}

void CWnd::OnKillFocus()
{
	if(MouseFocus && MouseGameMode)
	{
		while(ShowCursor(TRUE) < 0);
	}

	KeyBoardFocus = MouseFocus = false;

	if(FullScreen)
	{
		ShowWindow(hWnd, SW_SHOWMINIMIZED);
		ChangeDisplaySettings(NULL, 0);
		DeFullScreened = true;
	}
}

void CWnd::OnLButtonDown(int cx, int cy)
{
	SetMouseFocus();
}

void CWnd::OnMouseMove(int cx, int cy)
{
	if(MouseGameMode && MouseFocus)
	{
		if(cx != WidthD2 || cy != HeightD2)
		{
			Camera.OnMouseMove(WidthD2 - cx, HeightD2 - cy);
			SetCurPos(WidthD2, HeightD2);
		}
	}
	else if(GetKeyState(VK_RBUTTON) & 0x80)
	{
		Camera.OnMouseMove(LastCurPos.x - cx, LastCurPos.y - cy);

		LastCurPos.x = cx;
		LastCurPos.y = cy;
	}
}

void CWnd::OnMouseWheel(short zDelta)
{
	Camera.OnMouseWheel(zDelta);
}

void CWnd::OnPaint()
{
	PAINTSTRUCT ps;

	BeginPaint(hWnd, &ps);

	static int FPS = 0;

	DWORD End = GetTickCount();

	float FrameTime = (End - Begin) * 0.001f;
	Begin = End;

	if(End - Start > 1000)
	{
		CString Text = WindowName;

		Text.Append(" - %dx%d", Width, Height);
		Text.Append(", ATF %dx", gl_max_texture_max_anisotropy_ext);
		Text.Append(", MSAA %dx", Samples);
		Text.Append(", FPS: %d", FPS);
		Text.Append(" - OpenGL %d.%d", gl_version / 10, gl_version % 10);
		if(gl_version >= 30) if(wgl_context_forward_compatible) Text.Append(" Forward compatible"); else Text.Append(" Compatibility profile");
		Text.Append(" - %s", (char*)glGetString(GL_RENDERER));

		SetWindowText(hWnd, Text);

		FPS = 0;
		Start = End;
	}
	else
	{
		FPS++;
	}

	if(KeyBoardFocus)
	{
		BYTE Keys = 0x00;

		if(GetKeyState('W') & 0x80) Keys |= 0x01;
		if(GetKeyState('S') & 0x80) Keys |= 0x02;
		if(GetKeyState('A') & 0x80) Keys |= 0x04;
		if(GetKeyState('D') & 0x80) Keys |= 0x08;
		if(GetKeyState('R') & 0x80) Keys |= 0x10;
		if(GetKeyState('F') & 0x80) Keys |= 0x20;

		if(GetKeyState(VK_SHIFT) & 0x80) Keys |= 0x40;

		if(Keys & 0x3F)
		{
			vec3 Movement = Camera.OnKeys(Keys, FrameTime);
			Camera.Move(Movement);
		}
	}

	OpenGLRenderer.Render(FrameTime);

	SwapBuffers(hDC);

	EndPaint(hWnd, &ps);

	InvalidateRect(hWnd, NULL, FALSE);
}

void CWnd::OnRButtonDown(int cx, int cy)
{
	SetMouseFocus();

	if(!MouseGameMode)
	{
		LastCurPos.x = cx;
		LastCurPos.y = cy;
	}
}

void CWnd::OnSetFocus()
{
	KeyBoardFocus = true;

	if(DeFullScreened)
	{
		ChangeDisplaySettings(&DevMode, CDS_FULLSCREEN);
		MoveWindow(hWnd, 0, 0, DevMode.dmPelsWidth, DevMode.dmPelsHeight, FALSE);
		DeFullScreened = false;
	}

	int cx, cy;

	GetCurPos(&cx, &cy);

	if(cx >= 0 && cx < Width && cy >= 0 && cy < Height)
	{
		SetMouseFocus();
	}
}

void CWnd::OnSize(int sx, int sy)
{
	if(Width == 0 && Height == 0)
	{
		StartFPSCounter();
	}

	Width = sx;
	Height = sy;

	WidthD2 = Width / 2;
	HeightD2 = Height / 2;

	OpenGLRenderer.Resize(Width, Height);
}

CWnd Wnd;

// ----------------------------------------------------------------------------------------------------------------------------

LRESULT CALLBACK WndProc(HWND hWnd, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uiMsg)
	{
		case WM_CLOSE:
			PostQuitMessage(0);
			break;

		case WM_KEYDOWN:
			Wnd.OnKeyDown((UINT)wParam);
			break;

		case WM_KILLFOCUS:
			Wnd.OnKillFocus();
			break;

		case WM_LBUTTONDOWN:
			Wnd.OnLButtonDown(LOWORD(lParam), HIWORD(lParam));
			break;

		case WM_MOUSEMOVE:
			Wnd.OnMouseMove(LOWORD(lParam), HIWORD(lParam));
			break;

		case 0x020A: // WM_MOUSWHEEL
			Wnd.OnMouseWheel(HIWORD(wParam));
			break;

		case WM_PAINT:
			Wnd.OnPaint();
			break;

		case WM_RBUTTONDOWN:
			Wnd.OnRButtonDown(LOWORD(lParam), HIWORD(lParam));
			break;

		case WM_SETFOCUS:
			Wnd.OnSetFocus();
			break;

		case WM_SIZE:
			Wnd.OnSize(LOWORD(lParam), HIWORD(lParam));
			break;

		default:
			return DefWindowProc(hWnd, uiMsg, wParam, lParam);
	}

	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR sCmdLine, int iShow)
{
	if(Wnd.Create(hInstance, "WORLD of DANBO", 800, 600, DisplayQuestion("Would you like to run in fullscreen mode?")))
	{
		Wnd.Show();
		Wnd.MsgLoop();
	}
	else
	{
		DisplayError(ErrorLog);
	}

	Wnd.Destroy();

	return 0;
}
