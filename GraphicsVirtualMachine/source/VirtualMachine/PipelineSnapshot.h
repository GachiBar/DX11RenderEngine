#pragma once
#include "GVMBaseStructures.h"
#include "VirtualMachine/ResourcesManager.h"

namespace GVM {

    
    
namespace Compressed {
    struct PipelineSnapshot;
}


struct PipelineSnapshot {

    using CompressedType = Compressed::PipelineSnapshot;
    Shader*                     VS = nullptr;
    Shader*                     PS = nullptr;
    Shader*                     CS = nullptr;
    Shader*                     GS = nullptr;
    Shader*                     HS = nullptr;
    Shader*                     DS = nullptr;
    
    uint32_t                    DrawCallsNum = 0;

    EPrimitiveTopology          primitiveTopology = EPrimitiveTopology::PRIMITIVE_TOPOLOGY_UNKNOWN;

    RasterizerStateDesc         rasterizerState = RasterizerStateDesc::Default;
    DepthStencilStateDesc       depthStencilState = DepthStencilStateDesc::Default;
    InputLayout*                InputDeclaration = nullptr;
    DepthStencilView*           DepthStencilBuffer = nullptr;

    Mesh                        mesh = {};
    
    CoreBlendDesc               blendDesc = CoreBlendDesc();
    
    uint8_t                     renderTargetsNum = 0;
    RenderTargetDesc            RenderTargets[8] = {};
    
    uint8_t                     uaTargetsNum = 0;
    UATargetView*               UATargets[8] = {};
        
    uint8_t                     samplersNum = 0;
    SamplerStateDesc            Samplers[16] = {};
        
    uint8_t                     viewportsNum = 0;
    ViewportDesc                Viewports[16] = {};
        
        
    uint8_t                     constBuffersNum = 0;
    ConstBufferView*            ConstBuffers[15] = {};
        
    uint8_t                     texturesNum = 0;
    ResourceView*               Textures[128] = {};

    uint32_t GetSize() const;

    struct CompressArgs
    {
        CompressedType* cps;
        ResourcesManager& resourceManager;
    };
    void Compress(const CompressArgs& args) const;
    
};

namespace Compressed {
    
    struct PipelineSnapshot {
        
        uint32_t                    SnapshotByteSize;
        uint32_t                    DrawCallsNum;


        RasterizerStateDesc         rasterizerState;   
        DepthStencilStateDesc       depthStencilState;   

        EPrimitiveTopology          primitiveTopology;
        
        GVM::CoreBlendDesc               blendDesc;
    
        uint8_t                     renderTargetsNum;
        //RenderTargetDesc*           RenderTargets;
        
        uint8_t                     uaTargetsNum;
        //RenderTargetDesc*           RenderTargets;
        
        uint8_t                     samplersNum;
        //SamplerStateDesc            Samplers[16];
        
        uint8_t                     viewportsNum;
        //ViewportDesc                Viewports[16];

        
        uint8_t                     vertexBuffersNum = 0;
        //IVertexBufferView* vertexBuffers [32];
        
        uint8_t                     constBuffersNum;
        //IConstBuffer*               ConstBuffers[15];
        
        uint8_t                     texturesNum;
        //IResourceView*              Textures[128];


        Shader*                     VS;
        Shader*                     PS;
        Shader*                     CS;
        Shader*                     GS;
        Shader*                     HS;
        Shader*                     DS;
        
        InputLayout*                vertexDeclaration;
        DepthStencilView*           DepthBuffer;
        IndexBufferView*            indexBuffer;

        //RenderTargetDesc*           RenderTargets = {};
        //SamplerStateDesc*           Samplers = {};
        //ViewportDesc*               Viewports = {};
        //ConstBufferView**           ConstBuffers = {};
        //VertexBufferView**          VertexBuffers = {};
        //ResourceView**              Textures = {};

        uint16_t          RenderTargetsShift = {};
        uint16_t          UATargetsShift = {};
        uint16_t          SamplersShift = {};
        uint16_t          ViewportsShift = {};
        uint16_t          ConstBuffersShift = {};
        uint16_t          VertexBuffersShift = {};
        uint16_t          TexturesShift = {};
        
        uint8_t Data[];

        
        
    
    };

}

enum class EMachineCommands : uint8_t {
    UNKNOWN,

    SETUP_PIPELINE,
    CLEAR_PIPELINE,
    CLEAR_RT,
    CLEAR_DS,
    GET_DATA,

    DRAW,
    BEGIN_EVENT,
    END_EVENT,

    CREATE_RESOURCE,
    CREATE_RESOURCE_VIEW,
    CREATE_SHADER,
    UPDATE_RESOURCE,
    SET_RESOURCE_DATA,
    COPY_RESOURCE_DATA,
};

struct SetResourceDataDesc{
    Resource* resource;
    struct ResourceUpdateData {
        UBox rect = {};
        uint16_t dstSubresource = 0;
        int32_t srcRowPitch = 0;
        int32_t srcDepthPitch = 0;
        uint32_t dataSize = 0;
    } params;
    uint32_t shift;
};

struct ClearRenderTargetDesc{
    ResourceView* resource;
    FColor color;
};

struct ClearDepthStencilDesc{
    ResourceView* resource;
    float depth;
    int8_t stencil;
};

struct BeginEventDesc{
    const char* name;
};

struct GetDataDesc{
    ShaderResourceViewDesc resource;
    std::vector<std::vector<uint32_t>>& dst;
};


}
