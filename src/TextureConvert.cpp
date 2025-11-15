#include "TextureConvert.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "RootWindow.hpp"

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

	memset(m_InputName, 0x00, 1024 + 1);
	memset(m_OutputName, 0x00, 1024 + 1);

	m_pPixelData = nullptr;
	m_bAvoidFree = true;
	m_Width = 0;
	m_Height = 0;
	m_TextureID = -1;
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
	memset(m_InputName, 0x00, 1024 + 1);
	memset(m_OutputName, 0x00, 1024 + 1);
	LoadTextureFromFile(filename);
	m_bAvoidFree = true;
}

TextureConvert::~TextureConvert() {
	if(m_pPixelData && !m_bAvoidFree) {
		stbi_image_free(m_pPixelData);
		m_pPixelData = nullptr;
	}
}

bool TextureConvert::Move() {

	bool is_open = true;
	ImGui::Begin(m_InternalName.c_str(), &is_open, ImGuiWindowFlags_NoSavedSettings);
	// Test some groupings
	ImGui::BeginGroup();
	if(ImGui::Button("Load texture")) {
#ifdef WIN32
		memset(m_InputName, 0x00, 1024 + 1);
		OpenDialogWindows(m_InputName, 1024);
#endif
		printf("%s\n", m_InputName);
		if(nullptr != m_pPixelData) {
			stbi_image_free(m_pPixelData);
			glDeleteTextures(1, &m_TextureID);
		}
		LoadTextureFromFile(m_InputName);
		
	}
	ImGui::SameLine();
	ImGui::Text(m_InputName);
	ImGui::InputText("##output_image", m_OutputName, 1024);
	ImGui::SameLine();
	if(ImGui::Button("Save file")){
		SaveFile(RootWindow::GetMaterialPath());
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
		bool created = m_VTFFile.Create(m_Width, m_Height, m_pPixelData, m_CreateOptions);
		if(created) {
			printf("Converted to VTF\n");
			std::filesystem::path path = base_path / std::string(m_OutputName);
			printf("Path is: %s\n", path.string().c_str());
			printf("Parent directory is %s\n", path.parent_path().string().c_str());
			printf("Filename is %s\n", path.filename().string().c_str());
			if(!std::filesystem::exists(path.parent_path())){
				printf("Path %s doesn't exist, creating it\n", path.parent_path().string().c_str());
				std::filesystem::create_directory(path.parent_path());
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

bool TextureConvert::LoadTextureFromFile(const char* filename) {
	int channel = 0;
	int w = 0, h = 0;
	uint8_t* pixels = nullptr;
	// printf("Loading texture \"%s\"\n", filename);
	if(!(pixels = stbi_load(filename, &w, &h, &channel, 0))) {
		printf("Failed loading texture \"%s\"\n", filename);
		return false;
	}

	printf("Loaded texture \"%s\" (w: %d h: %d)\n", filename, w, h);
	
	GLuint tex = 0;
	glCreateTextures(GL_TEXTURE_2D, 1, &tex);
	glTextureStorage2D(tex, 1, GL_RGBA32F, w, h);
	glTextureSubImage2D(tex, 0, 0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

	m_pPixelData = pixels;
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
	for(int i = size - 1; i > start; i--) {
		if(filename[i] == '.') {
		memcpy(m_OutputName, filename + start, i - start);
		if(size < 1024 - 4) {
			m_OutputName[i - start] = '.';
			m_OutputName[i - start + 1] = 'v';
			m_OutputName[i - start + 2] = 't';
			m_OutputName[i - start + 3] = 'f';
		}
		break;
		}
	}
	memcpy(m_InputName, filename, size);
	return true;
}

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Shobjidl.h>
#include <iostream>

void TextureConvert::OpenDialogWindows(char* buffer, size_t size) {
    	std::wstring strw;
	std::string stra = "";
	//File Dialog
	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED |
		COINIT_DISABLE_OLE1DDE);
	COMDLG_FILTERSPEC filter;
	filter.pszName = L"Image files";
	filter.pszSpec = L"*.png; *.jpg; *.tga; *.bmp";
	if (SUCCEEDED(hr))
	{
		IFileOpenDialog* pFileOpen;

		// Create the FileOpenDialog object.
		hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
			IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));
		hr = pFileOpen->SetFileTypes(1, &filter);
		if (SUCCEEDED(hr))
		{
			// Show the Open dialog box.
			hr = pFileOpen->Show(NULL);

			// Get the file name from the dialog box.
			if (SUCCEEDED(hr))
			{
				IShellItem* pItem;
				hr = pFileOpen->GetResult(&pItem);
				printf("Trying to get result\n");
				if (SUCCEEDED(hr))
				{
					PWSTR pszFilePath;
					hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
					printf("Trying to get DisplayName\n");
					// Display the file name to the user.
					if (SUCCEEDED(hr))
					{
						//MessageBoxW(NULL, pszFilePath, L"File Path", MB_OK);
						strw = pszFilePath;
						printf("Selection: %ls\n", pszFilePath);
						size_t sz = 0;
						wcstombs_s(&sz, buffer, size, pszFilePath, wcslen(pszFilePath));
						CoTaskMemFree(pszFilePath);
					}
					pItem->Release();
				}else {
					printf("HRESULT: %x", hr);
				}
			}
			pFileOpen->Release();
		}
		CoUninitialize();
	}  
}

#endif
