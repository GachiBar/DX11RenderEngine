#pragma once
#include "Utils/DrawData.h"
#include "Utils/ModelMesh.h"




struct DiffuseData {
    bool isTextured = false;
    DiffuseData():isTextured(false) {};
    DiffuseData(TextureData::Color color):isTextured(false), color(color) {};
    DiffuseData(uint64_t color):isTextured(true), textureId(color) {};
    DiffuseData(const DiffuseData& data):
        isTextured(data.isTextured),
        dummy1(data.dummy1),
        dummy2(data.dummy2)
        {};
    
    DiffuseData& operator=(const DiffuseData& other)
    {
        if (this != &other)
        {
            this->isTextured = other.isTextured;
            this->color = other.color;
        }
        return *this;
    }
    
    union {
        struct {
            uint64_t dummy1;
            uint64_t dummy2;
        };
        uint64_t textureId;
        TextureData::Color color = TextureData::Color(0,255,0,0);
    };
};

struct NormalData {
    bool isTextured = false;
    NormalData(Float3Data::Color normal):isTextured(false), normal(normal) {};
    NormalData(const NormalData& data):
    isTextured(data.isTextured),
    dummy1(data.dummy1),
    dummy2(data.dummy2)
    {}
    union {
        struct {
            uint64_t dummy1;
            uint64_t dummy2;
        };
        size_t textureId;
        Float3Data::Color normal = {1,1,1};
    };
};

struct RoughnessData {
    bool isTextured = false;;
    RoughnessData(FloatData::Color roughness):isTextured(false), roughness(roughness) {};
    RoughnessData(const RoughnessData& data):
    isTextured(data.isTextured),
    dummy1(data.dummy1)
    {}
    union {
        struct {
            uint64_t dummy1;
        };
        size_t textureId;
        FloatData::Color roughness = {1};
    };
};

struct MetallicData {
    bool isTextured = false;
    MetallicData(FloatData::Color metallic):isTextured(false), metallic(metallic) {};
    MetallicData(const MetallicData& data):
    isTextured(data.isTextured),
    dummy1(data.dummy1)
    {}
    union {
        struct {
            uint64_t dummy1;
        };
        size_t textureId;
        FloatData::Color metallic = {0};
    };
};

struct MaterialData {
    DiffuseData   diffuseData;
    NormalData    normalData;
    RoughnessData roughnessData;
    MetallicData  metallicData;

    MaterialData():
        diffuseData(TextureData::Color(255,255,255,255)),
        normalData(Float3Data::Color(1,0,0)),
        roughnessData(0),
        metallicData(0)
    {      
    }
    
    MaterialData& operator=(const MaterialData& other)
    {
        if (this != &other)
        {
            diffuseData = other.diffuseData;
            normalData = other.normalData;
            roughnessData = other.roughnessData;
            metallicData = other.metallicData;
        }
        return *this;
    }
};

struct OpaqueFlags {
    ModelsFlags flags;
};

struct OpaqueModelDrawData {
    OpaqueMeshId modelId;

    Transform world;
    Transform oldWorld;
    
    float MetallicMult = 1;
    float RoughnessMult = 1;
    
    
    MaterialData materialData;
     uint32_t objectId;
    OpaqueFlags flags;
};
