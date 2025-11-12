#ifndef TEXTURECONVERT_INCLUDED
#define TEXTURECONVERT_INCLUDED

#include "VTFLib.h"
#include "imgui.h"
#include "GL/glew.h"
#include <stdint.h>

// Texture convert window and its contents
class TextureConvert {
public:
	TextureConvert();
	~TextureConvert();

	bool Move();
	bool ConvertToVTF(VTFImageFormat format);

private:
	bool LoadTextureFromFile();

private:
	char m_InputName[1024 + 1];
	char m_OutputName[1024 + 1];
	uint8_t* m_pPixelData;
	uint32_t m_Width;
	uint32_t m_Height;
	GLuint m_TextureID;
	VTFLib::CVTFFile m_VTFFile;
	SVTFCreateOptions m_CreateOptions;
};

#endif
