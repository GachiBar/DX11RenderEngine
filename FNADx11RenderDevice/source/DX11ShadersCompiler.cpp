#include "DX11ShadersCompiler.h"

#include "GraphicsExceptions/GraphicsException.h"

using namespace Renderer;

DX11ShadersCompiler::DX11ShadersCompiler()
{
    infoManager = new DxgiInfoManager();
}

DX11ShadersCompiler::~DX11ShadersCompiler()
{
    delete infoManager;
}

D3D11PixelShader* DX11ShadersCompiler::CompilePixelShader(const ShaderCompileData& shaderData)
{
    D3D11PixelShader* result = new D3D11PixelShader();
    std::vector<D3D_SHADER_MACRO> d3ddefines(shaderData.definesSize + 1);
    for (size_t i = 0; i < shaderData.definesSize; i++)
    {
        d3ddefines[i].Definition = shaderData.defines[i].defenition;
        d3ddefines[i].Name = shaderData.defines[i].name;
    }
    d3ddefines[shaderData.definesSize].Definition = NULL;
    d3ddefines[shaderData.definesSize].Name = NULL;

    wrl::ComPtr<ID3D10Blob> pPSData;
    wrl::ComPtr<ID3D10Blob> psErrorBlob;
#if _DEBUG
    bool compiled = false;
    try
    {
        GFX_THROW_INFO(
            D3DCompile(shaderData.shaderData, shaderData.dataSize, NULL,
                d3ddefines.data(), (ID3DInclude*)shaderData.includes,
                shaderData.enteryPoint, shaderData.target,
                shaderData.flags, shaderData.flags << 8u, &pPSData, &psErrorBlob));
        result->dataSize = pPSData->GetBufferSize();
        result->data = new char[result->dataSize];
        memcpy(result->data, pPSData->GetBufferPointer(), result->dataSize);
        if (psErrorBlob != nullptr)
        {
            throw HrException{ __LINE__,__FILE__,0, {} };
        }
        compiled = true;
    }
    catch (HrException exe)
    {
        CompileException ce{
            __LINE__, __FILE__,(hr),infoManager->GetMessages(),(char*)psErrorBlob->GetBufferPointer(),
            shaderData.name,"PixelShader",d3ddefines
        };
        delete result;
        throw ce;
    }
    catch (InfoException exe)
    {
        CompileException ce{
            __LINE__, __FILE__,(hr),infoManager->GetMessages(),(char*)psErrorBlob->GetBufferPointer(),
            shaderData.name,"PixelShader",d3ddefines
        };
        delete result;
        throw ce;
    }
#else

    GFX_THROW_INFO(
        D3DCompile(shaderData.shaderData, shaderData.dataSize, NULL,
            d3ddefines.data(), (ID3DInclude*)shaderData.includes,
            shaderData.enteryPoint, shaderData.target,
            shaderData.flags, shaderData.flags << 8u, &pPSData, &psErrorBlob));

    result->dataSize = pPSData->GetBufferSize();
    result->data = new char[result->dataSize];
    memcpy(result->data, pPSData->GetBufferPointer(), result->dataSize);

#endif
    return result;
}


D3D11ComputeShader* DX11ShadersCompiler::CompileComputeShader(const ShaderCompileData& shaderData)
{
    D3D11ComputeShader* result = new D3D11ComputeShader();
    std::vector<D3D_SHADER_MACRO> d3ddefines(shaderData.definesSize + 1);
    for (size_t i = 0; i < shaderData.definesSize; i++)
    {
        d3ddefines[i].Definition = shaderData.defines[i].defenition;
        d3ddefines[i].Name = shaderData.defines[i].name;
    }
    d3ddefines[shaderData.definesSize].Definition = NULL;
    d3ddefines[shaderData.definesSize].Name = NULL;

    wrl::ComPtr<ID3D10Blob> pPSData;
    wrl::ComPtr<ID3D10Blob> psErrorBlob;
#if _DEBUG
    try
    {
        GFX_THROW_INFO(
            D3DCompile(shaderData.shaderData, shaderData.dataSize, NULL,
                d3ddefines.data(), (ID3DInclude*)shaderData.includes, shaderData.enteryPoint,
                shaderData.target, shaderData.flags, shaderData.flags << 8u, &pPSData, &psErrorBlob));
        result->dataSize = pPSData->GetBufferSize();
        result->data = new char[result->dataSize];
        memcpy(result->data, pPSData->GetBufferPointer(), result->dataSize);
        if (psErrorBlob != nullptr)
        {
            throw HrException{ __LINE__,__FILE__,0, {} };
        }
    }
    catch (HrException exe)
    {
        CompileException ce{
            __LINE__, __FILE__,(hr),infoManager->GetMessages(),
            (char*)psErrorBlob->GetBufferPointer(),
            shaderData.name,"Compute Shader",d3ddefines
        };
        delete result;
        throw ce;
    }
    catch (InfoException exe)
    {
        CompileException ce{
            __LINE__, __FILE__,(hr),infoManager->GetMessages(),
            (char*)psErrorBlob->GetBufferPointer(),
            shaderData.name,"ComputeShader",d3ddefines
        };
        delete result;
        throw ce;
    }
#else
    GFX_THROW_INFO(
        D3DCompile(shaderData.shaderData, shaderData.dataSize, NULL,
            d3ddefines.data(), (ID3DInclude*)shaderData.includes,
            shaderData.enteryPoint,shaderData.target, shaderData.flags,
            shaderData.flags << 8u, &pPSData, &psErrorBlob));
    result->dataSize = pPSData->GetBufferSize();
    result->data = new char[result->dataSize];
    memcpy(result->data, pPSData->GetBufferPointer(), result->dataSize);

#endif
    return result;
}

D3D11GeometryShader* DX11ShadersCompiler::CompileGeometryShader(const ShaderCompileData& shaderData)
{
    D3D11GeometryShader* result = new D3D11GeometryShader();
    std::vector<D3D_SHADER_MACRO> d3ddefines(shaderData.definesSize + 1);
    for (size_t i = 0; i < shaderData.definesSize; i++)
    {
        d3ddefines[i].Definition = shaderData.defines[i].defenition;
        d3ddefines[i].Name = shaderData.defines[i].name;
    }
    d3ddefines[shaderData.definesSize].Definition = NULL;
    d3ddefines[shaderData.definesSize].Name = NULL;

    wrl::ComPtr<ID3D10Blob> pPSData;
    wrl::ComPtr<ID3D10Blob> psErrorBlob;
#if _DEBUG
    try
    {
        GFX_THROW_INFO(
            D3DCompile(shaderData.shaderData, shaderData.dataSize,
                NULL, d3ddefines.data(), (ID3DInclude*)shaderData.includes,
                shaderData.enteryPoint, shaderData.target, shaderData.flags,
                shaderData.flags << 8u, &pPSData, &psErrorBlob));
        result->dataSize = pPSData->GetBufferSize();
        result->data = new char[result->dataSize];
        memcpy(result->data, pPSData->GetBufferPointer(), result->dataSize);
        if (psErrorBlob != nullptr)
        {
            throw HrException{ __LINE__,__FILE__,0, {} };
        }
    }
    catch (HrException exe)
    {
        CompileException ce{
            __LINE__, __FILE__,(hr),infoManager->GetMessages(),(char*)psErrorBlob->GetBufferPointer(),
            shaderData.name
        };
        delete result;
        throw ce;
    }
    catch (InfoException exe)
    {
        CompileException ce{
            __LINE__, __FILE__,(hr),infoManager->GetMessages(),(char*)psErrorBlob->GetBufferPointer(),
            shaderData.name
        };
        delete result;
        throw ce;
    }
#else
    GFX_THROW_INFO(
        D3DCompile(shaderData.shaderData, shaderData.dataSize,
            NULL, d3ddefines.data(), (ID3DInclude*)shaderData.includes,
            shaderData.enteryPoint, shaderData.target, shaderData.flags,
            shaderData.flags << 8u, &pPSData, &psErrorBlob));
    result->dataSize = pPSData->GetBufferSize();
    result->data = new char[result->dataSize];
    memcpy(result->data, pPSData->GetBufferPointer(), result->dataSize);
#endif
    return result;
}



D3D11VertexShader* DX11ShadersCompiler::CompileVertexShader(const ShaderCompileData& shaderData)
{
    D3D11VertexShader* result = new D3D11VertexShader();

    std::vector<D3D_SHADER_MACRO> d3ddefines(shaderData.definesSize + 1);
    for (size_t i = 0; i < shaderData.definesSize; i++)
    {
        d3ddefines[i].Definition = shaderData.defines[i].defenition;
        d3ddefines[i].Name = shaderData.defines[i].name;
    }
    d3ddefines[shaderData.definesSize].Definition = NULL;
    d3ddefines[shaderData.definesSize].Name = NULL;

    wrl::ComPtr<ID3D10Blob> pVSData;
    wrl::ComPtr<ID3D10Blob> psErrorBlob;

    //{
    //		{"Position",  0, DXGI_FORMAT_R32G32_FLOAT,  0,                           0, D3D11_INPUT_PER_VERTEX_DATA, 0},
    //		{"TEXCOORD",  0, DXGI_FORMAT_R32G32_FLOAT,  0,D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    //};
#if _DEBUG
    try
    {
        GFX_THROW_INFO(
            D3DCompile(shaderData.shaderData, shaderData.dataSize, NULL,
                d3ddefines.data(), (ID3DInclude*)shaderData.includes,
                shaderData.enteryPoint, shaderData.target,
                shaderData.flags, shaderData.flags << 8u, &pVSData, &psErrorBlob));

        result->dataSize = pVSData->GetBufferSize();
        result->data = new char[result->dataSize];
        memcpy(result->data, pVSData->GetBufferPointer(), result->dataSize);
        //todo InputLayout
        if (psErrorBlob != nullptr)
        {
            throw HrException{ __LINE__,__FILE__,0, {} };
        }
    }
    catch (HrException exe)
    {
        if (psErrorBlob != nullptr)
        {
            CompileException ce{
                __LINE__, __FILE__,(hr),infoManager->GetMessages(),
                (char*)psErrorBlob->GetBufferPointer(),
                shaderData.name,"VertexShader",d3ddefines
            };
            delete result;
            throw ce;
        }
        throw;
    }
    catch (InfoException exe)
    {
        if (psErrorBlob != nullptr)
        {
            CompileException ce{
                __LINE__, __FILE__,(hr),infoManager->GetMessages(),
                (char*)psErrorBlob->GetBufferPointer(),
                shaderData.name,"VertexShader",d3ddefines
            };
            delete result;
            throw ce;
        }
        delete result;
        throw;
    }
#else

    GFX_THROW_INFO(
        D3DCompile(shaderData.shaderData, shaderData.dataSize, NULL,
            d3ddefines.data(), (ID3DInclude*)shaderData.includes,
            shaderData.enteryPoint, shaderData.target,
            shaderData.flags, shaderData.flags << 8u, &pVSData, &psErrorBlob));

    result->dataSize = pVSData->GetBufferSize();
    result->data = new char[result->dataSize];
    memcpy(result->data, pVSData->GetBufferPointer(), result->dataSize);
#endif
    return result;
}


GVM::InputAssemblerDeclarationDesc DX11ShadersCompiler::ToGVM(const void* inputLayoutt, uint32_t inputLayoutSize)
{
    const std::unordered_map<DXGI_FORMAT, GVM::EFormat> formatMap = {
        {DXGI_FORMAT_R32_FLOAT,GVM::EFormat::FORMAT_R32_FLOAT},
        {DXGI_FORMAT_R32G32_FLOAT,GVM::EFormat::FORMAT_R32G32_FLOAT},
        {DXGI_FORMAT_R32G32B32_FLOAT,GVM::EFormat::FORMAT_R32G32B32_FLOAT},
        {DXGI_FORMAT_R32G32B32A32_FLOAT,GVM::EFormat::FORMAT_R32G32B32A32_FLOAT},
    };

    const D3D11_INPUT_ELEMENT_DESC* inputLayout = (const D3D11_INPUT_ELEMENT_DESC*)inputLayoutt;

    GVM::InputAssemblerDeclarationDesc result;
    for (int i = 0; i < inputLayoutSize; i++)
    {
        result.PushBack({
            inputLayout[i].SemanticName,
            formatMap.at(inputLayout[i].Format),
            uint8_t(inputLayout[i].InputSlot),
            uint8_t(inputLayout[i].SemanticIndex),
            inputLayout[i].AlignedByteOffset,
            GVM::EInputClassification::INPUT_PER_VERTEX_DATA,
            0
        });
    }
    return result;
}
