#ifndef ROOTWINDOW_INCLUDED
#define ROOTWINDOW_INCLUDED

#include "TextureConvert.hpp"
#include "MaterialConstructor.hpp"

#include <vector>
#include <filesystem>

struct output_vmt_t {
	std::string name;
	int base_texture;
	int template_material;
};

class RootWindow {
public:
	RootWindow(float width, float height);
	~RootWindow();

	void Move();
	static const std::filesystem::path GetBasePath();
	static const std::filesystem::path GetMaterialPath();
private:
	void MoveBaseVars();
	void MoveTexConvert();
	void MoveMaterialConstructors();
	void MoveMaterialOutputs();

#ifdef WIN32
	void OpenTextureDialogWindows();
#endif

	void CreateMaterialConstructor();
	void LoadMaterialPreset();


private:
	bool m_bOpenFirstTime;
	float m_Width;
	float m_Height;
	int m_TexConvID;
	int m_MatConstID;
	std::vector<TextureConvert> m_CvtInstances;
	std::vector<MaterialConstructor> m_MatCInstances;

	// Combine material templates and base textures
	std::vector<output_vmt_t> m_OutputsList;

	// Paths
	static std::string m_BasePath;
	static std::string m_MaterialPath;
};

#endif
