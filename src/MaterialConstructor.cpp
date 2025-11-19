#include "MaterialConstructor.hpp"
#include "RootWindow.hpp"
#include "imgui_stdlib.h"
#include "Helper.hpp"
#include <fstream>
#include "json.hpp"

MaterialConstructor::MaterialConstructor(int id) {
	m_InternalName = "material " + std::to_string(id);
	m_MaterialName = "Material " + std::to_string(id);
	m_pNodes = nullptr;
}

MaterialConstructor::MaterialConstructor(int id, const char* json_template) {
	m_InternalName = "material " + std::to_string(id);
	m_MaterialName = "Material " + std::to_string(id);
	LoadFromJSON(json_template);
}

MaterialConstructor::~MaterialConstructor() {

}

void MaterialConstructor::CreateMaterial(const std::string& texture_name, const std::filesystem::path& name) {
	std::string ret;
	printf("Texture name: %s\n", texture_name.c_str());
	ret = "\"VertexLitGeneric\" {\n";
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

bool MaterialConstructor::Move() {
	bool is_open = true;
	ImGui::Begin(m_InternalName.c_str(), &is_open, ImGuiWindowFlags_NoSavedSettings);

	ImGui::InputText("Name", &m_MaterialName);
	DrawAddButtons();
	DrawNodeValues();
	ImGui::End();
	if(!is_open) {
		printf("Closing Material Constructor window\n");
	}
	return is_open;
}

const std::string MaterialConstructor::GetMaterialName() const {
	return m_MaterialName;	
}

void MaterialConstructor::DrawAddButtons() {
	ImGui::BeginGroup();
	// To add values of different kinds
	if(ImGui::Button("Add integer")) {
		m_Nodes.emplace_back(NODE_INTEGER);
	}
	ImGui::SameLine();
	if(ImGui::Button("Add float")) {
		m_Nodes.emplace_back(NODE_FLOAT);
	}
	ImGui::SameLine();
	if(ImGui::Button("Add float 2")) {
		m_Nodes.emplace_back(NODE_FLOAT2);
	}
	ImGui::SameLine();
	if(ImGui::Button("Add float 3")) {
		m_Nodes.emplace_back(NODE_FLOAT3);
	}
	//ImGui::SameLine();
	if(ImGui::Button("Add float 4")) {
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
#include <iostream>
void MaterialConstructor::LoadFromJSON(const char* filename) {
	// Material JSON file
	std::ifstream material_src;
	
	// Open file and do checks
	material_src.open(filename);
	if(material_src.is_open()){
		nlohmann::json js = nlohmann::json::parse(material_src);
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
		}
	}
	
}
