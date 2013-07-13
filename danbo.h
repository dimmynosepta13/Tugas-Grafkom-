#include <windows.h>

#include "string.h"

#include <gl/glew.h> // http://glew.sourceforge.net/
#include <gl/wglew.h>

#include <FreeImage.h> // http://freeimage.sourceforge.net/

#include <glm/glm.hpp> // http://glm.g-truc.net/
#include <glm/gtx/rotate_vector.hpp>

using namespace glm;

// check path to glew32.dll in Project -> Project Options -> Parameters -> Linker

// ----------------------------------------------------------------------------------------------------------------------------

void DisplayError(char *ErrorText);
void DisplayInfo(char *InfoText);
bool DisplayQuestion(char *QuestionText);

// ----------------------------------------------------------------------------------------------------------------------------

class CTexture
{
protected:
	GLuint TextureID;

public:
	CTexture();
	~CTexture();

	operator GLuint ();

	void Delete();
	bool LoadTexture2D(char *Texture2DFileName);
};

// ----------------------------------------------------------------------------------------------------------------------------

class CShaderProgram
{
public:
	GLuint *UniformLocations;

protected:
	GLuint VertexShader, FragmentShader, Program;

public:
	CShaderProgram();
	~CShaderProgram();

	operator GLuint ();

	void Delete();
	bool Load(char *VertexShaderFileName, char *FragmentShaderFileName);

protected:
	GLuint LoadShader(GLenum Type, char *ShaderFileName);
	void SetDefaults();
};

// ----------------------------------------------------------------------------------------------------------------------------

class CCamera
{
protected:
	mat4x4 *View;

public:
	vec3 X, Y, Z, Reference, Position;

	CCamera();
	~CCamera();

	void CalculateViewMatrix();
	void LookAt(vec3 Reference, vec3 Position, bool RotateAroundReference = false);
	void Move(vec3 Movement);
	vec3 OnKeys(BYTE Keys, float FrameTime);
	void OnMouseMove(int dx, int dy);
	void OnMouseWheel(short zDelta);
	void SetViewMatrixPointer(mat4x4 *View);
};

// ----------------------------------------------------------------------------------------------------------------------------

class COpenGLRenderer
{
protected:
	int Width, Height;
	mat4x4 Model, View, Projection;

	CTexture Texture[100];
	CShaderProgram Shader;

	vec2 *TexCoords,*TexCoords1,*TexCoords2,*TexCoords3,
         *TexCoordsRumput,*TexCoordsRumput2,*TexCoordsRumput3,
         *TexTangga1,*TexTangga2,*TexTangga3,*TexTangga4,
         *TexBata,
         *TexTembok,*TexAtap,
         *TexKusen1,*TexKusen2,*TexKusen3,*TexKusen4,*TexKusen5,
         *TexPintu,
         *TexJendela1,*TexJendela2,*TexJendela3,*TexJendela4,*TexJendela5,
         *TexJendela1a,*TexJendela2a,*TexJendela3a,*TexJendela4a,*TexJendela5a,
         *TexKd,
         *TexJalan,
         *TexKertas;
	vec3 *Normals,*Normals1,*Normals2, 
         *Vertices,*Vertices1,*Vertices2,*Vertices3,*Vertices4,*Vertices5,*Vertices6,
         *VerticesA,*Vertices1A,*Vertices2A,*Vertices3A,*Vertices4A,*Vertices5A,*Vertices6A,
         *VerticesB,*Vertices1B,*Vertices2B,*Vertices3B,*Vertices4B,*Vertices5B,*Vertices6B,
         *VerticesC,*Vertices1C,*Vertices2C,*Vertices3C,*Vertices4C,*Vertices5C,*Vertices6C,
         *VerticesD,*Vertices1D,*Vertices2D,*Vertices3D,*Vertices4D,*Vertices5D,*Vertices6D,
         *VerticesE,*Vertices1E,*Vertices2E,*Vertices3E,*Vertices4E,*Vertices5E,*Vertices6E,
         *VerRumah,*VerAtap,*VerAtap2,
         *VerRumput,*VerRumput2,*VerRumput3,
         *VerTangga1,*VerTangga2,*VerTangga3,*VerTangga4,
         *VerBata,
         *VerTembok,
         *VerKusen1,*VerKusen2,*VerKusen3,*VerKusen4,*VerKusen5,
         *VerPintu,
         *VerJendela1,*VerJendela2,*VerJendela3,*VerJendela4,*VerJendela5,
         *VerJendela1a,*VerJendela2a,*VerJendela3a,*VerJendela4a,*VerJendela5a,
         *VerKd,*VerKd2,*VerKd3,*VerKd4,
         *VerJalan,
         *VerKertas;


//int part1;                      // Start Of Disc    ( NEW )
//int part2;                      // End Of Disc      ( NEW )
//int p1=0;                       // Increase 1       ( NEW )
//int p2=1;                       // Increase 2       ( NEW )
GLUquadricObj *quadratic;
public:
	bool ShowAxisGrid, Stop;

public:
	COpenGLRenderer();
	~COpenGLRenderer();

	bool Init();
	void Render(float FrameTime);
	void Resize(int Width, int Height);
	void Destroy();
};

// ----------------------------------------------------------------------------------------------------------------------------

class CWnd
{
protected:
	char *WindowName;
	bool FullScreen, DeFullScreened;
	DEVMODE DevMode;
	HWND hWnd;
	HDC hDC;
	int Samples;
	HGLRC hGLRC;
	int Width, Height, WidthD2, HeightD2;
	DWORD Start, Begin;
	POINT LastCurPos;
	bool MouseGameMode, KeyBoardFocus, MouseFocus;

public:
	CWnd();
	~CWnd();

	bool Create(HINSTANCE hInstance, char *WindowName, int Width, int Height, bool FullScreen = false, int Samples = 4, bool CreateForwardCompatibleContext = false, bool DisableVerticalSynchronization = true);
	void Show(bool MouseGameMode = false, bool Maximized = false);
	void MsgLoop();
	void Destroy();

protected:
	void GetCurPos(int *cx, int *cy);
	void SetCurPos(int cx, int cy);
	void SetCurAccToMouseGameMode();
	void SetMouseFocus();
	void StartFPSCounter();

public:
	void OnKeyDown(UINT nChar);
	void OnKillFocus();
	void OnLButtonDown(int cx, int cy);
	void OnMouseMove(int cx, int cy);
	void OnMouseWheel(short zDelta);
	void OnPaint();
	void OnRButtonDown(int cx, int cy);
	void OnSetFocus();
	void OnSize(int sx, int sy);
};

// ----------------------------------------------------------------------------------------------------------------------------

LRESULT CALLBACK WndProc(HWND hWnd, UINT uiMsg, WPARAM wParam, LPARAM lParam);
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR sCmdLine, int iShow);
