#include "TextureConvert.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

TextureConvert::TextureConvert() {
	// Set STB to flip image
	stbi_set_flip_vertically_on_load(false);
	memset(&m_CreateOptions, 0x00, sizeof(SVTFCreateOptions));
	m_CreateOptions.uiVersion[0] = 7;
	m_CreateOptions.uiVersion[1] = 2;
	m_CreateOptions.ImageFormat = IMAGE_FORMAT_DXT5;
	m_CreateOptions.uiFlags = TEXTUREFLAGS_TRILINEAR;
	m_CreateOptions.uiStartFrame = 0;
	m_CreateOptions.sBumpScale = 1.0f;
	m_CreateOptions.bMipmaps = true;
	m_CreateOptions.MipmapFilter = MIPMAP_FILTER_BOX;
	m_CreateOptions.MipmapSharpenFilter = SHARPEN_FILTER_SHARPENSOFT;
	m_CreateOptions.bThumbnail = true;
	m_CreateOptions.bReflectivity = true;
	m_CreateOptions.bResize = true;
	m_CreateOptions.ResizeMethod = RESIZE_NEAREST_POWER2;
	m_CreateOptions.ResizeFilter = MIPMAP_FILTER_TRIANGLE;
	m_CreateOptions.ResizeSharpenFilter = SHARPEN_FILTER_NONE;
	m_CreateOptions.bResizeClamp = true;
	m_CreateOptions.uiResizeClampWidth = 8192;
	m_CreateOptions.uiResizeClampHeight = 8192;

	memset(m_InputName, 0x00, 1024 + 1);
	memset(m_OutputName, 0x00, 1024 + 1);

	m_pPixelData = nullptr;
	m_Width = 0;
	m_Height = 0;
}

TextureConvert::~TextureConvert() {
	if(m_pPixelData) {
		stbi_image_free(m_pPixelData);
		m_pPixelData = nullptr;
	}
}

bool TextureConvert::Move() {

	bool is_open = true;
	ImGui::Begin("texture", &is_open, ImGuiWindowFlags_NoSavedSettings);
		// Test some groupings
		ImGui::BeginGroup();
		//ImGui::Text("texture");
	       	//ImGui::SameLine();
		ImGui::InputText("##input_image", m_InputName, 1024);
		ImGui::SameLine();
		if(ImGui::Button("Load texture")) {
			printf("%s\n", m_InputName);
			if(nullptr != m_pPixelData) stbi_image_free(m_pPixelData);
			LoadTextureFromFile();
			size_t size = strlen(m_InputName);
			for(int i = size - 1; i > 0; i--){
				if(m_InputName[i] == '.') {
				memcpy(m_OutputName, m_InputName, i);
				if(size < 1024 - 4) {
					m_OutputName[i] = '.';
					m_OutputName[i + 1] = 'v';
					m_OutputName[i + 2] = 't';
					m_OutputName[i + 3] = 'f';
				}
				break;
				}
			}
		}
		ImGui::InputText("##output_image", m_OutputName, 1024);
		ImGui::SameLine();
		if(ImGui::Button("Save file")){
			if(nullptr != m_pPixelData) {
				bool created = m_VTFFile.Create(m_Width, m_Height, m_pPixelData, m_CreateOptions);
				if(created) {
					printf("Converted to VTF\n");
					m_VTFFile.Save(m_OutputName);
					printf("Saving converted VTF file\n");
				} else {
					printf("Error converting file: %s\n", vlGetLastError());
				}
			} else {
				printf("Attempted to save null data\n");
			}
		}
		ImGui::EndGroup();
		ImGui::Image(m_TextureID, {512.0f, 512.0f});
		ImGui::SameLine();

		ImGui::BeginGroup();
		ImGui::CheckboxFlags("Point Sample", &m_CreateOptions.uiFlags, TEXTUREFLAGS_POINTSAMPLE);
		ImGui::CheckboxFlags("Trilinear", &m_CreateOptions.uiFlags, TEXTUREFLAGS_TRILINEAR);
		ImGui::CheckboxFlags("Clamp S", &m_CreateOptions.uiFlags, TEXTUREFLAGS_CLAMPS);
		ImGui::CheckboxFlags("Clamp T", &m_CreateOptions.uiFlags, TEXTUREFLAGS_CLAMPT);
		ImGui::CheckboxFlags("Anisotropic", &m_CreateOptions.uiFlags, TEXTUREFLAGS_ANISOTROPIC);
		ImGui::CheckboxFlags("Hint DXT5", &m_CreateOptions.uiFlags, TEXTUREFLAGS_HINT_DXT5);
		ImGui::CheckboxFlags("SRGB", &m_CreateOptions.uiFlags, TEXTUREFLAGS_SRGB);
		ImGui::CheckboxFlags("Normal", &m_CreateOptions.uiFlags, TEXTUREFLAGS_NORMAL);
		ImGui::CheckboxFlags("No MipMap", &m_CreateOptions.uiFlags, TEXTUREFLAGS_NOMIP);
		ImGui::CheckboxFlags("No LoD", &m_CreateOptions.uiFlags, TEXTUREFLAGS_NOLOD);
		ImGui::CheckboxFlags("Min MipMap", &m_CreateOptions.uiFlags, TEXTUREFLAGS_MINMIP);
		ImGui::CheckboxFlags("Procedural", &m_CreateOptions.uiFlags, TEXTUREFLAGS_PROCEDURAL);
		//ImGui::CheckboxFlags("One bit Alpha", &m_CreateOptions.uiFlags, TEXTUREFLAGS_ONEBITALPHA);
		//ImGui::CheckboxFlags("Eight bit Alpha", &m_CreateOptions.uiFlags, TEXTUREFLAGS_EIGHTBITALPHA);
		//ImGui::CheckboxFlags("Environemnt map", &m_CreateOptions.uiFlags, TEXTUREFLAGS_ENVMAP);
		ImGui::CheckboxFlags("Render Target", &m_CreateOptions.uiFlags, TEXTUREFLAGS_RENDERTARGET);
		ImGui::CheckboxFlags("Depth Render Target", &m_CreateOptions.uiFlags, TEXTUREFLAGS_DEPTHRENDERTARGET);
		ImGui::CheckboxFlags("Single Copy", &m_CreateOptions.uiFlags, TEXTUREFLAGS_SINGLECOPY);
		ImGui::CheckboxFlags("No Depth Buffer", &m_CreateOptions.uiFlags, TEXTUREFLAGS_NODEPTHBUFFER);
		ImGui::CheckboxFlags("Clamp U", &m_CreateOptions.uiFlags, TEXTUREFLAGS_CLAMPU);
		ImGui::CheckboxFlags("Vertex texture", &m_CreateOptions.uiFlags, TEXTUREFLAGS_VERTEXTEXTURE);
		ImGui::CheckboxFlags("SS Bump", &m_CreateOptions.uiFlags, TEXTUREFLAGS_SSBUMP);
		ImGui::CheckboxFlags("Border", &m_CreateOptions.uiFlags, TEXTUREFLAGS_BORDER);
		ImGui::EndGroup();
		
		ImGui::End();
		if(!is_open) {
			printf("closed window\n");
		}
		return is_open;
}

bool TextureConvert::LoadTextureFromFile() {
	int channel = 0;
	int w = 0, h = 0;
	uint8_t* pixels = nullptr;
	// printf("Loading texture \"%s\"\n", m_InputName);
	if(!(pixels = stbi_load(m_InputName, &w, &h, &channel, 0))) {
		printf("Failed loading texture \"%s\"\n", m_InputName);
		return false;
	}

	printf("Loaded texture \"%s\" (w: %d h: %d)\n", m_InputName, w, h);
	
	GLuint tex = 0;
	glCreateTextures(GL_TEXTURE_2D, 1, &tex);
	glTextureStorage2D(tex, 1, GL_RGBA32F, w, h);
	glTextureSubImage2D(tex, 0, 0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

	m_pPixelData = pixels;
	m_Width = w;	
	m_Height = h;
	m_TextureID = tex;

	return true;
}
