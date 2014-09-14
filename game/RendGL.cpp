#include "tr_local.h"

RendGL::RendGL() {
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, gl_major->Integer());
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, gl_minor->Integer());
};

RendGL::~RendGL() {

}

void RendGL::LoadShaderText(const string& shaderFile, string& text) {
	File* ptFile = File::Open(shaderFile, "rb+");
	text = ptFile->ReadPlaintext();
	ptFile->Close();
}

#define MAX_GL_SHADER_SIZE 1024 // 1kb sounds good
bool RendGL::Start(SDL_Window* ptWindow) {
	// TODO: check all contexts for valid GL mode
	gl = SDL_GL_CreateContext(ptWindow);

	glGenVertexArrays = (void (APIENTRY *)(GLsizei, GLuint*))SDL_GL_GetProcAddress("glGenVertexArrays");
	glBindVertexArray = (void (APIENTRY *)(GLuint))SDL_GL_GetProcAddress("glBindVertexArray");
	glGenBuffers = (void (APIENTRY*)(GLsizei, GLuint*))SDL_GL_GetProcAddress("glGenBuffers");
	glBindBuffer = (void(APIENTRY*)(GLenum, GLuint))SDL_GL_GetProcAddress("glBindBuffer");
	glBufferData = (void(APIENTRY*)(GLenum, GLsizeiptr, const GLvoid*, GLenum))SDL_GL_GetProcAddress("glBufferData");
	glActiveTexture = (void(APIENTRY*)(GLenum))SDL_GL_GetProcAddress("glActiveTexture");

	glCreateShader = (GLuint(APIENTRY*)(GLenum))SDL_GL_GetProcAddress("glCreateShader");
	glCreateProgram = (GLuint(APIENTRY*)(void))SDL_GL_GetProcAddress("glCreateProgram");
	glShaderSource = (void(APIENTRY*)(GLuint, GLsizei, const GLchar**, const GLint*))SDL_GL_GetProcAddress("glShaderSource");
	glCompileShader = (void(APIENTRY*)(GLuint))SDL_GL_GetProcAddress("glCompileShader");
	glAttachShader = (void(APIENTRY*)(GLuint, GLuint))SDL_GL_GetProcAddress("glAttachShader");
	glBindFragDataLocation = (void(APIENTRY*)(GLuint, GLuint, const char*))SDL_GL_GetProcAddress("glBindFragDataLocation");
	glLinkProgram = (void(APIENTRY*)(GLuint))SDL_GL_GetProcAddress("glLinkProgram");
	glUseProgram = (void(APIENTRY*)(GLuint))SDL_GL_GetProcAddress("glUseProgram");
	glGetAttribLocation = (GLint(APIENTRY*)(GLuint, const GLchar*))SDL_GL_GetProcAddress("glGetAttribLocation");
	glEnableVertexAttribArray = (void(APIENTRY*)(GLuint))SDL_GL_GetProcAddress("glEnableVertexAttribArray");
	glVertexAttribPointer = (void(APIENTRY*)(GLuint, GLint, GLenum, GLboolean, GLsizei, const GLvoid*))SDL_GL_GetProcAddress("glVertexAttribPointer");

	glGenVertexArrays(1, &screenVAO);
	glBindVertexArray(screenVAO);
	glGenBuffers(1, &screenVBO);
	glBindBuffer(GL_ARRAY_BUFFER, screenVBO);

	GLfloat screenVerts[] = {
		//  Position   Color             Texcoords
		-1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, // Top-left
		1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, // Top-right
		1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, // Bottom-right
		-1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f  // Bottom-left
	};

	glBufferData(GL_ARRAY_BUFFER, sizeof(screenVerts), screenVerts, GL_STATIC_DRAW);

	// Element Array
	GLuint ebo;
	glGenBuffers(1, &ebo);

	GLuint screenElements[] = {
		0, 1, 2,
		2, 3, 0
	};

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(screenElements), screenElements, GL_STATIC_DRAW);


	// Scene Shader
	// Create the vertex shader
	GLuint ptVertexShader = glCreateShader(GL_VERTEX_SHADER);
	string vertexShaderText = "";
	LoadShaderText("glshaders/scene.vertex", vertexShaderText);
	const char* ptVertexShaderText = vertexShaderText.c_str();
	glShaderSource(ptVertexShader, 1, (const GLchar**)&ptVertexShaderText, NULL);
	glCompileShader(ptVertexShader);

	// Create the fragment shader
	GLuint ptFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	string fragmentShaderText = "";
	LoadShaderText("glshaders/scene.frag", fragmentShaderText);
	const char* ptFragmentShaderText = fragmentShaderText.c_str();
	glShaderSource(ptFragmentShader, 1, (const GLchar**)&ptFragmentShaderText, NULL);
	glCompileShader(ptFragmentShader);

	// Link the vertex and fragment shaders
	GLuint ptShaderProgram;
	mShaderPrograms["scene"] = ptShaderProgram = glCreateProgram();
	glAttachShader(ptShaderProgram, ptVertexShader);
	glAttachShader(ptShaderProgram, ptFragmentShader);
	glBindFragDataLocation(ptShaderProgram, 0, "outColor");
	glLinkProgram(ptShaderProgram);
	glUseProgram(ptShaderProgram);

	// Set the args up correctly
	GLint attrPos = glGetAttribLocation(ptShaderProgram, "position");
	glEnableVertexAttribArray(attrPos);
	glVertexAttribPointer(attrPos, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), 0);
	
	GLint attrColor = glGetAttribLocation(ptShaderProgram, "color");
	glEnableVertexAttribArray(attrColor);
	glVertexAttribPointer(attrColor, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));

	GLint attrTex = glGetAttribLocation(ptShaderProgram, "texcoord");
	glEnableVertexAttribArray(attrTex);
	glVertexAttribPointer(attrTex, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)(5 * sizeof(GLfloat)));


	// Blend Shader
	// Create the vertex shader
	ptVertexShader = glCreateShader(GL_VERTEX_SHADER);
	vertexShaderText = "";
	LoadShaderText("glshaders/blend.vertex", vertexShaderText);
	ptVertexShaderText = vertexShaderText.c_str();
	glShaderSource(ptVertexShader, 1, (const GLchar**)ptVertexShaderText, NULL);
	glCompileShader(ptVertexShader);

	// Create the fragment shader
	ptFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	fragmentShaderText = "";
	LoadShaderText("glshaders/scene.frag", fragmentShaderText);
	ptFragmentShaderText = fragmentShaderText.c_str();
	glShaderSource(ptFragmentShader, 1, (const GLchar**)ptFragmentShaderText, NULL);
	glCompileShader(ptFragmentShader);

	// Link the vertex and fragment shaders
	ptShaderProgram;
	mShaderPrograms["blend"] = ptShaderProgram = glCreateProgram();
	glAttachShader(ptShaderProgram, ptVertexShader);
	glAttachShader(ptShaderProgram, ptFragmentShader);
	glBindFragDataLocation(ptShaderProgram, 0, "outColor");
	glLinkProgram(ptShaderProgram);
	glUseProgram(ptShaderProgram);

	// Set the args up correctly
	attrPos = glGetAttribLocation(ptShaderProgram, "position");
	glEnableVertexAttribArray(attrPos);
	glVertexAttribPointer(attrPos, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), 0);

	attrColor = glGetAttribLocation(ptShaderProgram, "color");
	glEnableVertexAttribArray(attrColor);
	glVertexAttribPointer(attrColor, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));

	attrTex = glGetAttribLocation(ptShaderProgram, "texcoord");
	glEnableVertexAttribArray(attrTex);
	glVertexAttribPointer(attrTex, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)(5 * sizeof(GLfloat)));
	return true;
}

void RendGL::Restart() {

}

void RendGL::StartFrame() {
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
}

void RendGL::EndFrame() {
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

// Texture manipulation

RenderTexture* RendGL::CreateTexture(Uint32 dwWidth, Uint32 dwHeight) {
	RenderTexture* tex = Zone::New<RenderTexture>(Zone::TAG_IMAGES); // FIXME
	tex->width = dwWidth;
	tex->height = dwHeight;
	tex->pixels = nullptr;

	glGenTextures(1, &tex->data.glTexture);
	return tex;
}

RenderTexture* RendGL::TextureFromPixels(void* pixels, int width, int height, uint32_t extra) {
	RenderTexture* tex = Zone::New<RenderTexture>(Zone::TAG_IMAGES);
	bool alpha = (extra > 0);
	tex->width = width;
	tex->height = height;
	tex->pixels = (GLuint*)pixels;

	glGenTextures(1, &tex->data.glTexture);
	glBindTexture(GL_TEXTURE_2D, tex->data.glTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, alpha ? GL_RGBA : GL_RGB, width, height, 0, alpha ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, pixels);

	return tex;
}

void RendGL::DeleteTexture(RenderTexture* ptvTexture) {
	glDeleteTextures(1, &ptvTexture->data.glTexture);
	if(ptvTexture->pixels != nullptr) {
		R_Message(PRIORITY_WARNING, "WARNING: Tried to delete a locked texture!\n");
		delete[] ptvTexture->pixels;
	}
	Zone::FastFree(ptvTexture, "images");
}

void RendGL::LockTexture(RenderTexture* ptvTexture, void** pixels, int* span, bool bWriteOnly) {
	*span = 4 * ptvTexture->width; // Always this large on OpenGL
	if(bWriteOnly) {
		*span = 4 * ptvTexture->width;

		size_t size = ptvTexture->width * ptvTexture->height;
		ptvTexture->pixels = new GLuint[size];
		*pixels = ptvTexture->pixels;
		return;
	}
	if(ptvTexture->data.glTexture != 0) {
		GLuint size = ptvTexture->width * ptvTexture->height;
		ptvTexture->pixels = new GLuint[size];

		glBindTexture(GL_TEXTURE_2D, ptvTexture->data.glTexture);
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, ptvTexture->pixels);
	} else {
		R_Error("GL: could not lock texture (not created)\n");
	}
}

void RendGL::UnlockTexture(RenderTexture* ptvTexture) {
	glBindTexture(GL_TEXTURE_2D, ptvTexture->data.glTexture);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, ptvTexture->width, ptvTexture->height, GL_RGBA, GL_UNSIGNED_BYTE, ptvTexture->pixels);

	delete[] ptvTexture->pixels;
	ptvTexture->pixels = nullptr;
}

void RendGL::BlendTexture(RenderTexture* ptvTexture) {
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, ptvTexture->data.glTexture);

	
}

// Screenshot

void RendGL::CreateScreenshot(const string& sFileName) {

}