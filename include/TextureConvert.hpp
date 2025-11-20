#ifndef TEXTURECONVERT_INCLUDED
#define TEXTURECONVERT_INCLUDED

#include "VTFLib.h"
#include "imgui.h"
#include "GL/glew.h"
#include <stdint.h>
#include <string>
#include <filesystem>
#include "json.hpp"

// Texture convert window and its contents
class TextureConvert {
public:
	TextureConvert(int id);
	TextureConvert(int id, const char* filename);
	~TextureConvert();

	bool Move();
	void SetDelete();
	void SaveFile(const std::filesystem::path& base_path);

	void SetTextureFlags(int flags);
	void SetTextureFormat(int format);

	const std::string GetTextureSource() const;
	const std::string GetTextureName() const;
	int GetTextureFlags();
	int GetTextureFormat();
	
	void AsJSON(nlohmann::json* out);
private:
	bool LoadTextureFromFile(const char* filename);

private:
	std::string m_InternalName;
	std::string m_InputName;
	std::string m_OutputName;
	bool m_bAvoidFree;
	uint8_t* m_pPixelData;
	uint32_t m_Width;
	uint32_t m_Height;
	int m_TextureFormat;
	GLuint m_TextureID;
	VTFLib::CVTFFile m_VTFFile;
	SVTFCreateOptions m_CreateOptions;
};

#endif
