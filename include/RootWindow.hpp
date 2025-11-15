#ifndef ROOTWINDOW_INCLUDED
#define ROOTWINDOW_INCLUDED

#include "TextureConvert.hpp"
#include <vector>
#include <filesystem>

class RootWindow {
public:
	RootWindow(float width, float height);
	~RootWindow();

	void Move();
	static const std::filesystem::path GetMaterialPath();
private:
	void MoveBaseVars();
	void MoveTexConvert();
#ifdef WIN32
	void OpenTextureDialogWindows();
#endif

private:
	bool m_bOpenFirstTime;
	float m_Width;
	float m_Height;
	int m_TexConvID;
	std::vector<TextureConvert> m_CvtInstances;
	static std::string m_MaterialPath;
};

#endif
