#pragma once
#include "TransformUtils.h"
#include "TextureData.h"
#include "FloatData.h"
#include "Float3Data.h"
#include "ModelMesh.h"
#include "Lights.h"



struct UIDrawData {

    int x; int y;
    int width; int height; 
    int top; int left; int texW; int texH;
    float4 color = { 0,0,0,0 };
    uint64_t flag;
};

union ModelsFlags{
    ModelsFlags() 
    {
        isRed = 0;
        isVisible = 1;
        isColored = 0;
        isWireframe = 0;
    }
    uint32_t flags;
    struct {
        uint8_t isRed : 1 ;  
        uint8_t isVisible : 1;  
        uint8_t isColored : 1;  
        uint8_t isWireframe : 1;
    } ;
};

union DebugPassFlags{
    DebugPassFlags() {}
    uint32_t flags;
    struct {
        uint8_t isRed : 1;  
        uint8_t is2D : 1;  
        uint8_t is3D : 1;  
    } ;
};

struct DebugDraw3DData {
    MeshData<DebugVertex3D> mesh;
    Transform world;
    float3 color;
    DebugPassFlags flags;
};

struct DebugDraw2DData {
    MeshData<DebugVertex2D> mesh;
    float3 color;
    DebugPassFlags flags;
};

struct ModelDrawData {
    ModelDrawData() = default;
    
    ModelDrawData(size_t modelId, size_t textureId, Transform position):
    modelId(modelId),
    textureId(textureId),
    oldPosition(position),
    newPosition(position)
    {};
    
    ModelDrawData(size_t modelId, Transform position):
    modelId(modelId),
    textureId(0),
    oldPosition(position),
    newPosition(position)
    {
        flags.isRed = 1;
    };
    
    ModelDrawData(size_t modelId, float4 color, Transform position):
    modelId(modelId),
    textureId(0),
    oldPosition(position),
    newPosition(position)
    {
        flags.isColored = 1;
    };
    
    size_t modelId = 0;
    size_t textureId = 0;
    
    Transform oldPosition;
    Transform newPosition;
    
    float4 color;
    ModelsFlags flags;
};

struct MeshHashData {
    union {
        uint64_t id = 0;
        struct {
            uint32_t vertexOffset : 22;
            uint32_t indexOffset : 22;
            uint32_t numElem : 16;
            uint8_t pt : 4;
        };
    };
};


struct UPDrawData {
    //UPModelData model;
    Transform position;
    float2 texOffset;
    float4 light;

    bool dynamic;

    bool dynamicLight = false;
    ImageUpdate* lightUpdate = nullptr;

    uint64_t flags;
};



union LightFlags{
    LightFlags() {isVisible = 1;}
    uint32_t flags;
    struct {
        uint8_t isRed : 1 ;  
        uint8_t isVisible : 1;  
        uint8_t isDebug : 1;  
    } ;
};


struct LightDrawData {
    //UPModelData model;
    Transform world;

    UniversalLight Light;
    
    LightFlags flags;
};


struct ParticlesDrawData {
    uint64_t flags;
};

typedef size_t StaticMesh;
