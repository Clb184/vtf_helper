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
		// Open a dialog to select files
		OpenTextureDialog();
	}
	if(ImGui::MenuItem("New Material")) {
		// Create empty material template
		CreateMaterialConstructor();
	}
	if(ImGui::MenuItem("Load Material Preset")) {
		// Do some material preset loading
		OpenMaterialTemplateDialog();
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

	if(ImGui::Button("Convert textures")) {
		NormalizeString(&m_BasePath);
		NormalizeString(&m_MaterialPath);
		for(auto& cvt : m_CvtInstances){
			cvt.SaveFile(std::filesystem::path(m_BasePath) / std::filesystem::path(m_MaterialPath));
		}
	}
	ImGui::SameLine();
	if(ImGui::Button("Create materials")){
		NormalizeString(&m_BasePath);
		NormalizeString(&m_MaterialPath);
		if(m_MatCInstances.size() > 0) {
			printf("Creating all materials\n");
			CheckCreateMissingPath();
			for(auto& out : m_OutputsList){
				SaveMaterial(out);
			}
		}
	}
	ImGui::SameLine();
	if(ImGui::Button("Save all")) {
		NormalizeString(&m_BasePath);
		NormalizeString(&m_MaterialPath);
		CheckCreateMissingPath();
		for(auto& cvt : m_CvtInstances){
			cvt.SaveFile(std::filesystem::path(m_BasePath) / std::filesystem::path(m_MaterialPath));
		}
		if(m_MatCInstances.size() > 0) {
			printf("Creating all materials\n");
			for(auto& out : m_OutputsList){
				SaveMaterial(out);
			}
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
			CheckCreateMissingPath();
			SaveMaterial(m_OutputsList[i]);
		}
		ImGui::EndGroup();
	}
	ImGui::EndGroup();
	ImGui::End();
}

void RootWindow::OpenTextureDialog() {
	bool on_success = false;
	std::vector<std::string> tex_names;
#ifdef WIN32
	COMDLG_FILTERSPEC filter;
	filter.pszName = L"Image files";
	filter.pszSpec = L"*.png; *.jpg; *.tga; *.bmp";

	on_success = CreateMultiSelectDialogWindows(&filter, 1, &tex_names);
	printf("Received %d texture names\n", tex_names.size());
#endif
	if(false == on_success) return;

	for(auto& tex : tex_names) {
		printf("Load texture\n");
		m_CvtInstances.emplace_back(m_TexConvID, tex.c_str());
		printf("Added texconvert instance at %d with ID %d\n",
			m_CvtInstances.size() -1,
			m_TexConvID
			);
		m_TexConvID++;
	}
}

void RootWindow::OpenMaterialTemplateDialog() {
	bool on_success = false;
	std::vector<std::string> mat_names;
#ifdef WIN32
	COMDLG_FILTERSPEC filter;
	filter.pszName = L"Material Template (JSON)";
	filter.pszSpec = L"*.json";

	on_success = CreateMultiSelectDialogWindows(&filter, 1, &mat_names);
#endif
	if(false == on_success) return;
	
	for(auto& mat : mat_names){
		printf("Load material template\n");
		m_MatCInstances.emplace_back(m_MatConstID, mat.c_str());
		printf("Added materialconst instance at %d with ID %d\n",
		  	m_MatCInstances.size() -1,
			m_MatConstID
			);
		m_MatConstID++;
	}
}


void RootWindow::CreateMaterialConstructor(){
	printf("Create Material Constructor\n");
	m_MatCInstances.emplace_back(m_MatConstID);
	printf("Created MATC instance with ID %d\n", m_MatConstID);
	m_MatConstID++;
}

bool RootWindow::SaveMaterial(const output_vmt_t& output) {
	printf("Saving material\n");
	printf("texid: %d, matid: %d\n", output.base_texture, output.template_material);
	std::string texture = (output.base_texture > 0) ? m_CvtInstances[output.base_texture - 1].GetTextureName() : "<null>";
	printf("Texture name for material is %s\n", texture.c_str());
	if(m_MatCInstances.size() > 0){
		std::filesystem::path out_p = std::filesystem::path(m_BasePath) /
	      		std::filesystem::path(m_MaterialPath) /
		       	std::filesystem::path(output.name + ".vmt");
		m_MatCInstances[output.template_material].CreateMaterial(texture, out_p);
		return true;
	}
}

void RootWindow::CheckCreateMissingPath() {
	std::filesystem::path target = std::filesystem::path(m_BasePath) / std::filesystem::path(m_MaterialPath);
	if(target.string() == "") {
		printf("Path not specified, skipping\n");
		return;
	}
	printf("Looking for path %s\n", target.string().c_str());
	if(!std::filesystem::exists(target)) {
		printf("Path %s doesn't exist, creating it\n", target.string().c_str());
		std::filesystem::create_directories(target);
	}else {
		printf("Path %s already exists\n", target.string().c_str());
	}
}

void RootWindow::LoadMaterialPreset() {
	
}

void RootWindow::RemoveTextureFromOutputs(int id) {
	for(auto& o : m_OutputsList) {
		o.base_texture -= (o.base_texture >= id);
		if(o.base_texture < 0) o.base_texture = 0;
	}
}

void RootWindow::RemoveMaterialFromOutputs(int id) {
	for(auto& o : m_OutputsList) {
		o.template_material -= (o.template_material >= id);
		if(o.template_material < 0) o.template_material = 0;
	}
}
