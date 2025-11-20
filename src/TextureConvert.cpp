#include "TextureConvert.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "RootWindow.hpp"
#include "imgui_stdlib.h"
#include "Helper.hpp"

VTFImageFormat ID2Format(int id) {
	switch(id) {
		case 0: return IMAGE_FORMAT_RGBA8888;
		case 1: return IMAGE_FORMAT_ABGR8888;
		case 2: return IMAGE_FORMAT_RGB888;
		case 3: return IMAGE_FORMAT_BGR888;
		case 4: return IMAGE_FORMAT_RGB565;
		case 5: return IMAGE_FORMAT_ARGB8888;
		case 6: return IMAGE_FORMAT_BGRA8888;
		case 7: return IMAGE_FORMAT_DXT1;
		case 8: return IMAGE_FORMAT_DXT3;
		default: case 9: return IMAGE_FORMAT_DXT5;
	}
}

const char* ID2Str(int id) {
	switch(id) {
		case IMAGE_FORMAT_RGBA8888: return "RGBA 8888";
		case IMAGE_FORMAT_ABGR8888: return "ABGR 8888";
		case IMAGE_FORMAT_RGB888: return "RGB 888";
		case IMAGE_FORMAT_BGR888: return "BGR 888";
		case IMAGE_FORMAT_RGB565: return "RGB 565";
		case IMAGE_FORMAT_ARGB8888: return "ARGB 8888";
		case IMAGE_FORMAT_BGRA8888: return "BGRA 8888";
		case IMAGE_FORMAT_DXT1: return "DXT1";
		case IMAGE_FORMAT_DXT3: return "DXT3";
		case IMAGE_FORMAT_DXT5: return "DXT5";
		default: return "Unknown";
	}
}

TextureConvert::TextureConvert(int id) {
	m_InternalName = "texture " + std::to_string(id); 
	// Set STB to flip image
	stbi_set_flip_vertically_on_load(false);
	memset(&m_CreateOptions, 0x00, sizeof(SVTFCreateOptions));
	m_CreateOptions.uiVersion[0] = 7;
	m_CreateOptions.uiVersion[1] = 2;
	m_CreateOptions.ImageFormat = IMAGE_FORMAT_DXT5;
	m_CreateOptions.uiFlags = 0;
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

	m_pPixelData = nullptr;
	m_bAvoidFree = true;
	m_Width = 0;
	m_Height = 0;
	m_TextureID = -1;
	m_TextureFormat = 8; 
}

TextureConvert::TextureConvert(int id, const char* filename) {
	m_InternalName = "texture " + std::to_string(id); 
	// Set STB to flip image
	stbi_set_flip_vertically_on_load(false);
	memset(&m_CreateOptions, 0x00, sizeof(SVTFCreateOptions));
	m_CreateOptions.uiVersion[0] = 7;
	m_CreateOptions.uiVersion[1] = 2;
	m_CreateOptions.ImageFormat = IMAGE_FORMAT_DXT5;
	m_CreateOptions.uiFlags = 0;
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
	
	printf("Load texture on constructor received: %s\n", filename);
	LoadTextureFromFile(filename);
	m_bAvoidFree = true;
	m_TextureFormat = 9; 
}

TextureConvert::~TextureConvert() {
	if(m_pPixelData && !m_bAvoidFree) {
		free(m_pPixelData);
		m_pPixelData = nullptr;
	}
}

bool TextureConvert::Move() {

	bool is_open = true;
	ImGui::Begin(m_InternalName.c_str(), &is_open, ImGuiWindowFlags_NoSavedSettings);
	// Test some groupings
	ImGui::BeginGroup();
	if(ImGui::Button("Load texture")) {
		bool on_success = false;
#ifdef WIN32
		COMDLG_FILTERSPEC filter;
		filter.pszName = L"Image files";
		filter.pszSpec = L"*.png; *.jpg; *.tga; *.bmp";

		on_success = CreateSingleSelectDialogWindows(&filter, 1, &m_InputName);
#endif
		if(on_success) {
			printf("Loading %s\n", m_InputName.c_str());
			if(nullptr != m_pPixelData) {
				free(m_pPixelData);
				glDeleteTextures(1, &m_TextureID);
			}
			LoadTextureFromFile(m_InputName.c_str());
		}
		
	}
	ImGui::SameLine();
	ImGui::Text(m_InputName.c_str());
	ImGui::InputText("##output_image", &m_OutputName);
	ImGui::SameLine();
	if(ImGui::Button("Save file")){
		SaveFile(RootWindow::GetBasePath() / RootWindow::GetMaterialPath());
	}
	ImGui::EndGroup();

	ImGui::Image(m_TextureID, {256.0f, 256.0f});
	ImGui::SameLine();
	ImGui::BeginGroup();

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
	ImGui::EndGroup();
	ImGui::SameLine();
		
	ImGui::BeginGroup();
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
	
	// For texture formats	
	const char* format_names[] = {
	       	"RGBA 8888", "ABGR 8888", "RGB 888", "BGR 888", "RGB 565",
	       	"ARGB 8888", "BGRA 8888", "DXT1", "DXT3", "DXT5"
       	};
	//bool in_combo = ImGui::BeginCombo("Format", ID2Str(m_TextureFormat));
	ImGui::SetNextItemWidth(96.0f);
	ImGui::Combo("Format", &m_TextureFormat, format_names, 10);
	/*for(int i = 0; i < 10; i++){
		ImGui::BeginChild(ID2Str(i));
		ImGui::EndChild();
	}*/
	
	//if(in_combo) ImGui::EndCombo();

	ImGui::EndGroup();
	
	ImGui::End();
	if(!is_open) {
		printf("closed window\n");
	}
	return is_open;
}

void TextureConvert::SetDelete() {
	m_bAvoidFree = false;
}

void TextureConvert::SaveFile(const std::filesystem::path& base_path) {
	if(nullptr != m_pPixelData) {
		NormalizeString(&m_OutputName);
		if(m_OutputName == "") return;
		m_CreateOptions.ImageFormat = ID2Format(m_TextureFormat);
		bool created = m_VTFFile.Create(m_Width, m_Height, m_pPixelData, m_CreateOptions);
		if(created) {
			printf("Converted to VTF (Format: %d)\n", ID2Format(m_TextureFormat));
			std::filesystem::path path = base_path / (m_OutputName + ".vtf");
			std::filesystem::path parent = path.parent_path();
			printf("Path is: %s\n", path.string().c_str());
			printf("Parent directory is %s\n", parent.string().c_str());
			printf("Filename is %s\n", path.filename().string().c_str());
			if(parent.string() != ""){
				if(!std::filesystem::exists(parent)){
					printf("Path %s doesn't exist, creating it\n", path.parent_path().string().c_str());
					std::filesystem::create_directories(path.parent_path());
				}
			}
			if(m_VTFFile.Save(path.string().c_str())) {
				printf("Saving converted VTF file\n");
			} else {
				printf("Error converting VTF file\n");
			}
		} else {
			printf("Error converting file: %s\n", vlGetLastError());
		}
	} else {
		printf("Attempted to save null data\n");
	}
}

const std::string TextureConvert::GetTextureName() const {
	return m_OutputName;
}

bool TextureConvert::LoadTextureFromFile(const char* filename) {
	int channel = 0;
	int w = 0, h = 0;
	uint8_t* pixels = nullptr;
	// printf("Loading texture \"%s\"\n", filename);
	if(!(pixels = stbi_load(filename, &w, &h, &channel, 0))) {
		printf("Failed loading texture \"%s\"\n", filename);
		m_Width = 256;
		m_Height = 256;
		m_pPixelData = (uint8_t*)calloc(256 * 256, 4);
		return false;
	}

	printf("Loaded texture \"%s\" (w: %d h: %d channels: %d)\n", filename, w, h, channel);
	
	// Fine, I'll do it myself (Manual forced alpha or other channels)
	uint8_t* ndata = (uint8_t*)calloc(w * h, 4);

	switch(channel) {
	case 1:
		// All channels with same value
		for(int j = 0; j < h; j++) {
			for(int i = 0; i < w; i++){
				int idx = i + w * j;
				ndata[idx * 4] = pixels[idx];
				ndata[idx * 4 + 1] = pixels[idx];
				ndata[idx * 4 + 2] = pixels[idx];
				ndata[idx * 4 + 3] = pixels[idx];
			}
		}
		break;
	case 2:
		// I've never seen textures with two channels only	
		for(int j = 0; j < h; j++) {
			for(int i = 0; i < w; i++){
				int idx = i + w * j;
				ndata[idx * 4] = pixels[idx * 2];
				ndata[idx * 4 + 1] = pixels[idx * 2 + 1];
				ndata[idx * 4 + 2] = 255;
				ndata[idx * 4 + 3] = 255;
			}
		}
		break;
	case 3:
		// Alpha guaranteed to be 255
		for(int j = 0; j < h; j++) {
			for(int i = 0; i < w; i++){
				int idx = i + w * j;
				ndata[idx * 4] = pixels[idx * 3];
				ndata[idx * 4 + 1] = pixels[idx * 3 + 1];
				ndata[idx * 4 + 2] = pixels[idx * 3 + 1];
				ndata[idx * 4 + 3] = 255;
			}
		}
		break;
	case 4:
		// Just copy
		memcpy(ndata, pixels, w * h * 4);
		break;
	}

	GLuint tex = 0;
	GLenum format = channel == 4 ? GL_RGBA : (channel == 3) ? GL_RGB : (channel == 2) ? GL_RG : GL_RED;
	glCreateTextures(GL_TEXTURE_2D, 1, &tex);
	glTextureStorage2D(tex, 1, GL_RGBA32F, w, h);
	glTextureSubImage2D(tex, 0, 0, 0, w, h, format, GL_UNSIGNED_BYTE, pixels);
	stbi_image_free(pixels);

	m_pPixelData = ndata;
	m_Width = w;	
	m_Height = h;
	m_TextureID = tex;
	
	size_t size = strlen(filename);
	int start = 0;
	for(size_t i = 0; i < size; i++){
		if(filename[i] == '\\' || filename[i] == '/'){
			start = i + 1;
		}
	}
	m_OutputName = "";
	for(int i = size - 1; i > start; i--) {
		if(filename[i] == '.') {
			for(int c = start; c < i; c++){
				m_OutputName.push_back(filename[c]);
			}
			break;
		}
	}
	m_InputName = filename;
	return true;
}
