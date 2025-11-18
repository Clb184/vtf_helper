#include "RootWindow.hpp"
#include "imgui_stdlib.h"
#include "Helper.hpp"

std::string RootWindow::m_BasePath;
std::string RootWindow::m_MaterialPath;

RootWindow::RootWindow(float width, float height) 
	: m_Width(width), m_Height(height) 
{
	m_TexConvID = 0;
	m_MatConstID = 0;
	//memset(m_MaterialPath, 0, 1024 + 4);
	m_bOpenFirstTime = false;
}

RootWindow::~RootWindow() {
	printf("Destroying Root Window contents\n");

	printf("Destroying %d Texture Convert instances\n", m_CvtInstances.size());
	for(auto& texconv : m_CvtInstances) {
		texconv.SetDelete();
	}

	printf("Destroying %d Material Constructor instances\n", m_MatCInstances.size());
	for(auto& matc : m_MatCInstances) {

	}
}

void RootWindow::Move() {
	// Window itself
	/*ImGui::SetNextWindowPos({0.0f, 0.0f});
	ImGui::SetNextWindowSize({m_Width, m_Height});
	ImGui::Begin("vtftool_root", nullptr,
	       	ImGuiWindowFlags_NoCollapse |
	       	ImGuiWindowFlags_NoResize |
	       	ImGuiWindowFlags_NoMove |
	       	ImGuiWindowFlags_MenuBar |
	       	ImGuiWindowFlags_NoTitleBar |
	       	ImGuiWindowFlags_NoBringToFrontOnFocus |
		ImGuiWindowFlags_NoSavedSettings
		);*/

	// The menu bar where some useful stuff is
	ImGui::BeginMainMenuBar();
	if(ImGui::MenuItem("Load Texture")) {
#ifdef WIN32
		// Implementation for Windows
		OpenTextureDialogWindows();
#endif
	}
	if(ImGui::MenuItem("Create Material")) {
		// Create empty material template
		CreateMaterialConstructor();
	}
	if(ImGui::MenuItem("Load Material Preset")) {
		// Do some material preset loading
	}
	ImGui::EndMainMenuBar();
	
	// Move base paths window
	if(!m_bOpenFirstTime) {
		ImGui::SetNextWindowPos({64.0f, 80.0f});
	}
	MoveBaseVars();

	// Window with the outputs at first init
	if(!m_bOpenFirstTime) { 
		ImGui::SetNextWindowSize({500.0f, 100.0f}); 
		ImGui::SetNextWindowPos({64.0f, 192.0f});
		m_bOpenFirstTime = true;
	}

	// Material outputs and that
	MoveMaterialOutputs();

	// Texture convert
	MoveTexConvert();

	// Move the material constructor
	MoveMaterialConstructors();

	// End main window
	//ImGui::End();

}

const std::filesystem::path RootWindow::GetBasePath() {
	NormalizeString(&m_BasePath);
	return m_BasePath;
}

const std::filesystem::path RootWindow::GetMaterialPath() {
	NormalizeString(&m_BasePath);
	return m_MaterialPath;
}

void RootWindow::MoveBaseVars() {
	ImGui::Begin("Paths", nullptr,
		       	ImGuiWindowFlags_NoSavedSettings |
		       	ImGuiWindowFlags_NoCollapse |
			ImGuiWindowFlags_NoResize
		    );

	ImGui::BeginGroup();
	ImGui::Text("Base path    ");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(300.0f);
	ImGui::InputText("##base_path", &m_BasePath);
	ImGui::EndGroup();

	ImGui::BeginGroup();
	ImGui::Text("Material path");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(300.0f);
	ImGui::InputText("##material_path", &m_MaterialPath);
	ImGui::EndGroup();

	if(ImGui::Button("Convert materials")) {
		NormalizeString(&m_BasePath);
		NormalizeString(&m_MaterialPath);
		for(auto& cvt : m_CvtInstances){
			cvt.SaveFile(std::filesystem::path(m_BasePath) / std::filesystem::path(m_MaterialPath));
		}
	}
	ImGui::End();
}

void RootWindow::MoveTexConvert() {
	for (int i = 0; i < m_CvtInstances.size(); i++) {
		// Delete from list if closes
		if(!m_CvtInstances[i].Move()) {
			printf("Erasing CVT instance at %d\n", i);
			m_CvtInstances[i].SetDelete();
			m_CvtInstances.erase(m_CvtInstances.begin() + i);
			RemoveTextureFromOutputs(i + 1);
			i--;
		}
	}
}

void RootWindow::MoveMaterialConstructors() {
	for (int i = 0; i < m_MatCInstances.size(); i++) {
		// Delete from list if closes
		if(!m_MatCInstances[i].Move()) {
			printf("Erasing MATC instance at %d\n", i);
			//m_MatcInstances[i].SetDelete();
			m_MatCInstances.erase(m_MatCInstances.begin() + i);
			RemoveMaterialFromOutputs(i);
			i--;
		}
	}	
}

void RootWindow::MoveMaterialOutputs() {
	ImGui::Begin("Material Outputs", nullptr, 
			ImGuiWindowFlags_NoSavedSettings |
			ImGuiWindowFlags_NoCollapse
			);
	if(ImGui::Button("Add output")) {
		m_OutputsList.emplace_back();
	}
	ImGui::BeginGroup();
	for(int i = 0; i < m_OutputsList.size(); i++) {
		ImGui::BeginGroup();
		char buff[64] = "";
		std::string id = "##out_name" + std::to_string(i);
		std::string del_this = "Remove##del_" + std::to_string(i);
		std::string save_this = "Save##sav_" + std::to_string(i);
		ImGui::SetNextItemWidth(100.0f);
		ImGui::InputText(id.c_str(), &m_OutputsList[i].name);
		ImGui::SameLine();

		// Texture select
		ImGui::SetNextItemWidth(96.0f);
		std::string texture = m_OutputsList[i].base_texture > 0 ? m_CvtInstances[m_OutputsList[i].base_texture - 1].GetTextureName() : "<null>";
		sprintf(buff, "Tex##%d", i);
		if(ImGui::BeginCombo(buff, texture.c_str())) {
			bool selected = m_OutputsList[i].base_texture == 0;
			sprintf(buff, "<null>#%d", i);
			if(ImGui::Selectable("<null>##", selected)) {
				m_OutputsList[i].base_texture == 0;
			}
			for(int j = 0; j < m_CvtInstances.size(); j++){
				selected = (m_OutputsList[i].base_texture - 1) == j;
				if(ImGui::Selectable(std::string(m_CvtInstances[j].GetTextureName() + "##" + std::to_string(i)).c_str(), selected)){
					m_OutputsList[i].base_texture = j + 1;
				}
			}
			ImGui::EndCombo();
		}
		//ImGui::Text("Texture id");
		ImGui::SameLine();

		// Material template select
		ImGui::SetNextItemWidth(96.0f);
		sprintf(buff, "Mat##%d", i);
		if(ImGui::BeginCombo(buff, m_MatCInstances.size() > 0 ? m_MatCInstances[m_OutputsList[i].template_material].GetMaterialName().c_str() : "<null>")) {
			for(int j = 0; j < m_MatCInstances.size(); j++){
				bool selected = (m_OutputsList[i].template_material) == j;
				if(ImGui::Selectable(std::string(m_MatCInstances[j].GetMaterialName() + "##" + std::to_string(i)).c_str(), selected)){
					m_OutputsList[i].template_material = j;
				}
			}
			ImGui::EndCombo();
		}
		//ImGui::Text("Material temp");
		ImGui::SameLine();
		// Remove material from list
		if(ImGui::Button(del_this.c_str())) {
			m_OutputsList.erase(m_OutputsList.begin() + i);
			i--;
			ImGui::EndGroup();
			continue;
		}
		ImGui::SameLine();
		// Save	material
		if(ImGui::Button(save_this.c_str())){
			printf("Saving material\n");
			if(m_MatCInstances.size() > 0){
				m_MatCInstances[m_OutputsList[i].template_material].CreateMaterial(texture);
			}
		}
		ImGui::EndGroup();
	}
	ImGui::EndGroup();
	ImGui::End();
}

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Shobjidl.h>

void RootWindow::OpenTextureDialogWindows() {
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
		FILEOPENDIALOGOPTIONS options;
		pFileOpen->GetOptions(&options);
		pFileOpen->SetOptions(options | FOS_ALLOWMULTISELECT);

		if (SUCCEEDED(hr))
		{
			// Show the Open dialog box.
			hr = pFileOpen->Show(NULL);

			// Get the file name from the dialog box.
			if (SUCCEEDED(hr))
			{
				IShellItemArray* items;
				hr = pFileOpen->GetResults(&items);
				printf("Trying to get result\n");
				if (SUCCEEDED(hr))
				{
					DWORD cnt = 0;
					items->GetCount(&cnt);
					for(int i = 0; i < cnt; i++) {
						IShellItem* item;
						hr = items->GetItemAt(i, &item);
						if(SUCCEEDED(hr)) {
							PWSTR filepath;
							hr = item->GetDisplayName(SIGDN_FILESYSPATH, &filepath);
							printf("Trying to get DisplayName\n");
							// Display the file name to the user.
							if (SUCCEEDED(hr)) {
								size_t nbs = wcslen(filepath) * sizeof(wchar_t);
								char* filepathc = (char*)calloc(nbs, 1);
								size_t sz = 0;
								printf("Selection: %ls\n", filepath);
								wcstombs_s(&sz, filepathc, nbs, filepath, wcslen(filepath));

								printf("Load texture\n");
								m_CvtInstances.emplace_back(m_TexConvID, filepathc);
								printf("Added texconvert instance at %d with ID %d\n",
				      				 	m_CvtInstances.size() -1,
									m_TexConvID
			     					);
								m_TexConvID++;
								CoTaskMemFree(filepath);
							}
						}
						item->Release();
					}
					items->Release();
				} else {
					printf("HRESULT: %x", hr);
				}
			}
			pFileOpen->Release();
		}
		CoUninitialize();
	}  
}

void RootWindow::OpenMaterialTemplateDialogWindows() {
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
		FILEOPENDIALOGOPTIONS options;
		pFileOpen->GetOptions(&options);
		pFileOpen->SetOptions(options | FOS_ALLOWMULTISELECT);

		if (SUCCEEDED(hr))
		{
			// Show the Open dialog box.
			hr = pFileOpen->Show(NULL);

			// Get the file name from the dialog box.
			if (SUCCEEDED(hr))
			{
				IShellItemArray* items;
				hr = pFileOpen->GetResults(&items);
				printf("Trying to get result\n");
				if (SUCCEEDED(hr))
				{
					DWORD cnt = 0;
					items->GetCount(&cnt);
					for(int i = 0; i < cnt; i++) {
						IShellItem* item;
						hr = items->GetItemAt(i, &item);
						if(SUCCEEDED(hr)) {
							PWSTR filepath;
							hr = item->GetDisplayName(SIGDN_FILESYSPATH, &filepath);
							printf("Trying to get DisplayName\n");
							// Display the file name to the user.
							if (SUCCEEDED(hr)) {
								size_t nbs = wcslen(filepath) * sizeof(wchar_t);
								char* filepathc = (char*)calloc(nbs, 1);
								size_t sz = 0;
								printf("Selection: %ls\n", filepath);
								wcstombs_s(&sz, filepathc, nbs, filepath, wcslen(filepath));

								printf("Load texture\n");
								m_CvtInstances.emplace_back(m_TexConvID, filepathc);
								printf("Added texconvert instance at %d with ID %d\n",
				      				 	m_CvtInstances.size() -1,
									m_TexConvID
			     					);
								m_TexConvID++;
								CoTaskMemFree(filepath);
							}
						}
						item->Release();
					}
					items->Release();
				} else {
					printf("HRESULT: %x", hr);
				}
			}
			pFileOpen->Release();
		}
		CoUninitialize();
	}  
}

#endif

void RootWindow::CreateMaterialConstructor(){
	printf("Create Material Constructor\n");
	m_MatCInstances.emplace_back(m_MatConstID);
	printf("Created MATC instance with ID %d\n", m_MatConstID);
	m_MatConstID++;
}

void RootWindow::LoadMaterialPreset() {
	
}

void RootWindow::RemoveTextureFromOutputs(int id) {
	for(auto& o : m_OutputsList) {
		o.base_texture -= (o.base_texture >= id);
	}
}

void RootWindow::RemoveMaterialFromOutputs(int id) {
	for(auto& o : m_OutputsList) {
		o.template_material -= (o.template_material >= id);
	}
}
