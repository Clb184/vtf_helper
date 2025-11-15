#ifndef MATERIALCONSTRUCTOR_INCLUDED
#define MATERIALCONSTRUCTOR_INCLUDED

#include "VTFLib.h"
#include "imgui.h"
#include <stdint.h>
#include <string>
#include <vector>
#include <filesystem>

enum NODE_TYPE {
	NODE_INTEGER,
	NODE_FLOAT,
	NODE_FLOAT2,
	NODE_FLOAT3,
	NODE_FLOAT4,
	NODE_COLOR,
	NODE_STRING,
};

struct value_node_t {
	std::string name;
	NODE_TYPE type;
	union {
		int integer;
		float single;
		float float2[2];
		float float3[3];
		float float4[4];
	};
	std::string string;
	value_node_t(NODE_TYPE ty) { 
		type = ty;
	       	memset(float4, 0, sizeof(float4));
		if(ty == NODE_COLOR) float4[3] = 1.0f;
       	}
};

class MaterialConstructor {
public:
	MaterialConstructor(int id);
	~MaterialConstructor();

	bool Move();
private:
	void DrawAddButtons();
	void DrawNodeValues();

private:
	std::string m_InternalName;
	std::string m_ShaderType;
	VTFLib::CVMTFile m_VMTFile;
	VTFLib::Nodes::CVMTGroupNode* m_pNodes;
	std::vector<value_node_t> m_Nodes;
};

#endif
