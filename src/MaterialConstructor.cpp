#include "MaterialConstructor.hpp"
#include "RootWindow.hpp"
#include "imgui_stdlib.h"
#include "Helper.hpp"
#include <fstream>
#include "json.hpp"
#include <iostream>
#include <assert.h>

MaterialConstructor::MaterialConstructor(int id) {
	m_InternalName = "material " + std::to_string(id);
	m_MaterialName = "Material " + std::to_string(id);
	m_ShaderType = "VertexLitGeneric";
}

MaterialConstructor::MaterialConstructor(int id, const char* json_template) {
	m_InternalName = "material " + std::to_string(id);
	m_MaterialName = "Material " + std::to_string(id);
	m_ShaderType = "VertexLitGeneric";
	LoadFromFile(json_template);
}

MaterialConstructor::~MaterialConstructor() {

}

bool MaterialConstructor::Move() {
	bool is_open = true;
	ImGui::Begin(m_InternalName.c_str(), &is_open, ImGuiWindowFlags_NoSavedSettings);

	// Set material name (Also displayed on outputs window)
	ImGui::InputText("Name", &m_MaterialName);
	if(ImGui::Button("Save template")) {
		// For now I use the name of the material template itself
		SaveTemplate();
	}
	DrawAddButtons();
	DrawNodeValues();
	ImGui::End();
	if(!is_open) {
		printf("Closing Material Constructor window\n");
	}
	return is_open;
}

void MaterialConstructor::CreateMaterial(const std::string& texture_name, const std::filesystem::path& name) {
	std::string ret;
	try {
		nlohmann::json src;
		nlohmann::json js;
		AsJSON(&js);
		src["material"].emplace_back(std::move(js));
		std::cout << src.dump(4) << "\n";
	}
	catch(const std::exception& e) {
		printf("JSON exception: %s\n", e.what());
	}
	printf("Texture name: %s\n", texture_name.c_str());
	ret = "\"" + m_ShaderType + "\" {\n";
	std::filesystem::path tex_path = RootWindow::GetMaterialPath() / std::filesystem::path(texture_name);
	if(texture_name != "<null>") {
		ret += "\t$basetexture \"" + tex_path.string() + "\"\n";
	}
	for(auto& node : m_Nodes){
		NormalizeString(&node.name);
		if(node.name == "") continue;
		ret += "\t$" + node.name + " \"";
		switch(node.type){
			case NODE_INTEGER: ret += std::to_string(node.integer); break;
			case NODE_FLOAT: ret += std::format("{}", node.single); break;
			case NODE_FLOAT2: ret += std::format("[{} {}]", node.float4[0], node.float4[1]); break;
			case NODE_FLOAT3: ret += std::format("[{} {} {}]", node.float4[0], node.float4[1], node.float4[2]); break;
			case NODE_FLOAT4: ret += std::format("[{} {} {} {}]", node.float4[0], node.float4[1], node.float4[2], node.float4[3]); break;
			case NODE_COLOR: ret += std::format("[{} {} {} {}]", node.float4[0], node.float4[1], node.float4[2], node.float4[3]); break;
			case NODE_STRING: ret += node.string;  break;
		}
		ret += "\"\n";
	}
	ret += "}";
	printf("Material generated:\n%s\n", ret.c_str());

	printf("Creating material %s\n", name.string().c_str());
	FILE* fp = fopen(name.string().c_str(), "wb");
	if(fp){
		fwrite(ret.data(), 1, ret.length(), fp);
		fclose(fp);
		printf("Saved VMT file\n");
	} else {
		printf("Couldn't save VMT file\n");
	}
	return;
}

const std::string MaterialConstructor::GetMaterialName() const {
	return m_MaterialName;	
}

void MaterialConstructor::AsJSON(nlohmann::json* out) {
	try {
		// Create a json blob to hold the material information
		nlohmann::json js;
		js["name"] = m_MaterialName;
		js["type"] = m_ShaderType;
	
		for(auto& node : m_Nodes){
			NormalizeString(&node.name);
			if(node.name == "") continue;
			
			auto& type = js["body"][node.name]["type"];
			auto& value = js["body"][node.name]["value"];

			switch(node.type){
				case NODE_INTEGER: type = "integer"; break;
				case NODE_FLOAT: type = "float"; break;
				case NODE_FLOAT2: type = "float2"; break;
				case NODE_FLOAT3: type = "float3"; break;
				case NODE_FLOAT4: type = "float4"; break;
				case NODE_COLOR: type = "color"; break;
				case NODE_STRING: type = "string";  break;
			}
			switch(node.type){
				case NODE_INTEGER: value = node.integer; break;
				case NODE_FLOAT: value = node.single; break;
				case NODE_FLOAT2: value = node.float2; break;
				case NODE_FLOAT3: value = node.float3; break;
				case NODE_FLOAT4: value = node.float4; break;
				case NODE_COLOR: value = node.float4; break;
				case NODE_STRING: value = node.string;  break;
			}
		}
		std::cout << js.dump(4) << "\n";
		*out = std::move(js);
	}
	catch(const std::exception& e) {
		printf("JSON error: %s\n", e.what());
	}
}

void MaterialConstructor::DrawAddButtons() {
	ImGui::BeginGroup();
	// Display how many components there are
	ImGui::Text("Component count: %d", m_Nodes.size());

	// To add values of different kinds
	if(ImGui::Button("Add integer")) {
		m_Nodes.emplace_back(NODE_INTEGER);
	}
	ImGui::SameLine();
	if(ImGui::Button("Add float")) {
		m_Nodes.emplace_back(NODE_FLOAT);
	}
	ImGui::SameLine();
	if(ImGui::Button("Add float2")) {
		m_Nodes.emplace_back(NODE_FLOAT2);
	}
	ImGui::SameLine();
	if(ImGui::Button("Add float3")) {
		m_Nodes.emplace_back(NODE_FLOAT3);
	}
	//ImGui::SameLine();
	if(ImGui::Button("Add float4")) {
		m_Nodes.emplace_back(NODE_FLOAT4);
	}
	ImGui::SameLine();
	if(ImGui::Button("Add string")) {
		m_Nodes.emplace_back(NODE_STRING);
	}
	ImGui::SameLine();
	if(ImGui::Button("Add color")) {
		m_Nodes.emplace_back(NODE_COLOR);
	}
	ImGui::EndGroup();
	//ImGui::SameLine();
}

void MaterialConstructor::DrawNodeValues() {
	ImGui::BeginGroup();
	for(int i = 0; i < m_Nodes.size(); i++){
		std::string name = "##name" + std::to_string(i);
		std::string value = "##value" + std::to_string(i);
		ImGui::SetNextItemWidth(96.0f);
		ImGui::InputText(name.c_str(), &m_Nodes[i].name);
		ImGui::SameLine();
		switch(m_Nodes[i].type){
			case NODE_INTEGER: ImGui::InputInt(value.c_str(), &m_Nodes[i].integer); break;
			case NODE_FLOAT: ImGui::InputFloat(value.c_str(), &m_Nodes[i].single); break;
			case NODE_FLOAT2: ImGui::InputFloat2(value.c_str(), m_Nodes[i].float2); break;
			case NODE_FLOAT3: ImGui::InputFloat3(value.c_str(), m_Nodes[i].float3); break;
			case NODE_FLOAT4: ImGui::InputFloat4(value.c_str(), m_Nodes[i].float4); break;
			case NODE_COLOR: ImGui::ColorEdit4(value.c_str(), m_Nodes[i].float4); break;
			case NODE_STRING: ImGui::InputText(value.c_str(), &m_Nodes[i].string); break;
		}		
	}
	ImGui::EndGroup();
}

void MaterialConstructor::SaveJSON(const char* filename) {
	try {
		// The output JSON file
		std::ofstream output(filename, std::ios::out);
		printf("Saving material template %s as JSON (%s)\n", m_MaterialName.c_str(), filename);
		if(output.is_open()) {
			nlohmann::json js;
			AsJSON(&js);
			output << js.dump(4);
			printf("Material template saved");
		} else {
			printf("Failed saving material template\n");
		}
	}
	catch(const std::exception& e) {
		printf("JSON exception: %s\n", e.what());
	}
}

bool MaterialConstructor::LoadFromJSON(nlohmann::json& js) {
	try {
		std::string name = js["name"];
		std::string type = js["type"];
		printf("Name: %s, Type: %s\n", name.c_str(), type.c_str());

		m_MaterialName = name;
		m_ShaderType = type;
			
		auto& body = js["body"];
		for(nlohmann::json::iterator it = js["body"].begin(); it != js["body"].end(); it++){
			printf("Key: %s\n", it.key().c_str());
			std::string node_type = body[it.key()]["type"];
			printf("Node type: %s\n", node_type.c_str());
			if (node_type == "integer"){
				int value = body[it.key()]["value"];
				value_node_t node(NODE_INTEGER);
				node.name = it.key();
				node.integer = value;
				m_Nodes.emplace_back(node);
			}
			else if (node_type == "float"){
				float value = body[it.key()]["value"];
				value_node_t node(NODE_FLOAT);
				node.name = it.key();
				node.single = value;
				m_Nodes.emplace_back(node);
			}
			else if (node_type == "float2"){
				std::array<float, 2> value = body[it.key()]["value"];
				value_node_t node(NODE_FLOAT2);
				node.name = it.key();
				memcpy(node.float2, value.data(), sizeof(float) * 2);
				m_Nodes.emplace_back(node);
			}
			else if (node_type == "float3"){
				std::array<float, 3> value = body[it.key()]["value"];
				value_node_t node(NODE_FLOAT3);
				node.name = it.key();
				memcpy(node.float4, value.data(), sizeof(float) * 3);
				m_Nodes.emplace_back(node);
			}
			else if (node_type == "float4"){
				std::array<float, 4> value = body[it.key()]["value"];
				value_node_t node(NODE_FLOAT4);
				node.name = it.key();
				memcpy(node.float4, value.data(), sizeof(float) * 4);
				m_Nodes.emplace_back(node);
			}
			else if (node_type == "color"){
				std::array<float, 4> value = body[it.key()]["value"];
				value_node_t node(NODE_COLOR);
				node.name = it.key();
				memcpy(node.float4, value.data(), sizeof(float) * 4);
				m_Nodes.emplace_back(node);
			}
			else if (node_type == "string"){
				value_node_t node(NODE_STRING);
				node.name = it.key();
				node.string = body[it.key()]["value"];
				m_Nodes.emplace_back(node);
			}
		}
	}
	catch(const std::exception& e){
		printf("Exception reached: %s\n", e.what());
		return false;
	}
	return true;
}

void MaterialConstructor::SaveTemplate() {
	std::string name;
#ifdef WIN32
	COMDLG_FILTERSPEC filter;
	filter.pszName = L"JSON template";
	filter.pszSpec = L"*.json";
	printf("Saving template (Windows save dialog)\n");
	if(CreateSaveDialogWindows(&filter, 1, &name, L"json")) {
		printf("Got %s as file name\n", name.c_str());
		SaveJSON((name).c_str());
	}
#endif
}

void MaterialConstructor::LoadFromFile(const char* filename) {
	// Material JSON file
	std::ifstream material_src;
	
	// Open file and do checks
	material_src.open(filename);
	if(material_src.is_open()){
		nlohmann::json js = nlohmann::json::parse(material_src);
		LoadFromJSON(js);
	}
}
