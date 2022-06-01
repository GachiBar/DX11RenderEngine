﻿#pragma once
#include <cstdint>
#include <type_traits>
namespace GVM {

template<typename E>
constexpr auto to_underlying(E e) noexcept
{
    return static_cast<std::underlying_type_t<E>>(e);
}

template<typename E>
constexpr E to_enum(uint8_t e) noexcept
{
    return static_cast<E>(e);
}

enum class EShaderType : uint8_t  {
    UNKNOWN,
    VERTEX_SHADER,
    PIXEL_SHADER,
    GEOMETRY_SHADER,
    HULL_SHADER,
    COMPUTE_SHADER,
    DOMAIN_SHADER,
};

enum class EBlendOperator : uint8_t {
    BLEND_UNKNOWN,
    BLEND_OP_ADD,
    BLEND_OP_SUBTRACT,
    BLEND_OP_REV_SUBTRACT,
    BLEND_OP_MIN,
    BLEND_OP_MAX
};

enum class ELogicOperator : uint8_t {
    LOGIC_OP_UNDEFINED = 0,
    LOGIC_OP_CLEAR,
    LOGIC_OP_SET,
    LOGIC_OP_COPY,
    LOGIC_OP_COPY_INVERTED,
    LOGIC_OP_NOOP,
    LOGIC_OP_INVERT,
    LOGIC_OP_AND,
    LOGIC_OP_NAND,
    LOGIC_OP_OR,
    LOGIC_OP_NOR,
    LOGIC_OP_XOR,
    LOGIC_OP_EQUIV,
    LOGIC_OP_AND_REVERSE,
    LOGIC_OP_AND_INVERTED,
    LOGIC_OP_OR_REVERSE,
    LOGIC_OP_OR_INVERTED
};

enum class EBlendType : uint8_t {
    BLEND_UNKNOWN,
    BLEND_ZERO,
    BLEND_ONE,
    BLEND_SRC_COLOR,
    BLEND_INV_SRC_COLOR,
    BLEND_SRC_ALPHA,
    BLEND_INV_SRC_ALPHA,
    BLEND_DEST_ALPHA,
    BLEND_INV_DEST_ALPHA,
    BLEND_DEST_COLOR,
    BLEND_INV_DEST_COLOR,
    BLEND_BLEND_FACTOR,
    BLEND_INV_BLEND_FACTOR,
    BLEND_SRC_ALPHA_SAT,
};


enum class EDepthWriteMask : uint8_t {
    DEPTH_WRITE_MASK_ZERO,
    DEPTH_WRITE_MASK_ALL
};

enum class EComparisonFunc : uint8_t {
    COMPARISON_UNKNOWN,
    COMPARISON_NEVER,
    COMPARISON_LESS,
    COMPARISON_EQUAL,
    COMPARISON_LESS_EQUAL,
    COMPARISON_GREATER,
    COMPARISON_NOT_EQUAL,
    COMPARISON_GREATER_EQUAL,
    COMPARISON_ALWAYS
};

enum class EStencilOp : uint8_t {
    STENCIL_OP_KEEP,
    STENCIL_OP_ZERO,
    STENCIL_OP_REPLACE,
    STENCIL_OP_INCR_SAT,
    STENCIL_OP_DECR_SAT,
    STENCIL_OP_INVERT,
    STENCIL_OP_INCR,
    STENCIL_OP_DECR
};

enum class EFillMode: uint8_t {
    FILL_UNKNOWN,
    FILL_WIREFRAME,
    FILL_SOLID
};

enum class ECullMode : uint8_t {
    CULL_UNKNOWN,
    CULL_NONE,
    CULL_FRONT,
    CULL_BACK
};

enum class EInputClassification {
    INPUT_PER_VERTEX_DATA,
    INPUT_PER_INSTANCE_DATA
};//Can be deleted

enum class EPresentInterval {
    PRESENT_INTERVAL_DEFAULT,
    PRESENT_INTERVAL_ONE,
    PRESENT_INTERVAL_TWO,
    PRESENT_INTERVAL_IMMEDIATE
};

enum class EFormat : uint8_t {
    FORMAT_UNKNOWN = 0,
    
    //4 chanel
    FORMAT_R32G32B32A32_TYPELESS,
    FORMAT_R32G32B32A32_FLOAT,
    FORMAT_R32G32B32A32_UINT,
    FORMAT_R32G32B32A32_SINT,
    
    FORMAT_R16G16B16A16_TYPELESS,
    FORMAT_R16G16B16A16_FLOAT,
    FORMAT_R16G16B16A16_UNORM,
    FORMAT_R16G16B16A16_UINT,
    FORMAT_R16G16B16A16_SNORM,
    FORMAT_R16G16B16A16_SINT,
    
    FORMAT_R10G10B10A2_TYPELESS,
    FORMAT_R10G10B10A2_UNORM,
    FORMAT_R10G10B10A2_UINT,

    FORMAT_R8G8B8A8_TYPELESS,
    FORMAT_R8G8B8A8_UNORM,
    FORMAT_B8G8R8A8_UNORM,
    FORMAT_R8G8B8A8_UNORM_SRGB,
    FORMAT_R8G8B8A8_UINT,
    FORMAT_R8G8B8A8_SNORM,
    FORMAT_R8G8B8A8_SINT,
    
    //3 chanel
    FORMAT_R32G32B32_TYPELESS,
    FORMAT_R32G32B32_FLOAT,
    FORMAT_R32G32B32_UINT,
    FORMAT_R32G32B32_SINT,
    
    //2 chanel
    FORMAT_R32G32_TYPELESS,
    FORMAT_R32G32_FLOAT,
    FORMAT_R32G32_UINT,
    FORMAT_R32G32_SINT,
    
    FORMAT_R16G16_TYPELESS,
    FORMAT_R16G16_FLOAT,
    FORMAT_R16G16_UNORM,
    FORMAT_R16G16_UINT,
    FORMAT_R16G16_SNORM,
    FORMAT_R16G16_SINT,
    
    FORMAT_R8G8_TYPELESS,
    FORMAT_R8G8_UNORM,
    FORMAT_R8G8_UINT,
    FORMAT_R8G8_SNORM,
    FORMAT_R8G8_SINT,
    
    //1 chanel
    FORMAT_R32_TYPELESS,
    FORMAT_R32_FLOAT,
    FORMAT_R32_UINT,
    FORMAT_R32_SINT,
    
    FORMAT_R16_TYPELESS,
    FORMAT_R16_FLOAT,
    FORMAT_R16_UNORM,
    FORMAT_R16_UINT,
    FORMAT_R16_SNORM,
    FORMAT_R16_SINT,
    
    FORMAT_R8_TYPELESS,
    FORMAT_R8_UNORM,
    FORMAT_R8_UINT,
    FORMAT_R8_SNORM,
    FORMAT_R8_SINT,
    FORMAT_A8_UNORM,
    
};

    
constexpr uint32_t FormatByteSize[] = {
    1, /*FORMAT_UNKNOWN*/

    //4 chanel
    4 * 4, /*FORMAT_R32G32B32A32_TYPELESS,*/
    4 * 4, /*FORMAT_R32G32B32A32_FLOAT,*/
    4 * 4, /*FORMAT_R32G32B32A32_UINT,*/
    4 * 4, /*FORMAT_R32G32B32A32_SINT,*/

    4 * 2, /*FORMAT_R16G16B16A16_TYPELESS,*/
    4 * 2, /*FORMAT_R16G16B16A16_FLOAT,*/
    4 * 2, /*FORMAT_R16G16B16A16_UNORM,*/
    4 * 2, /*FORMAT_R16G16B16A16_UINT,*/
    4 * 2, /*FORMAT_R16G16B16A16_SNORM,*/
    4 * 2, /*FORMAT_R16G16B16A16_SINT,*/

    4, /*FORMAT_R10G10B10A2_TYPELESS,*/
    4, /*FORMAT_R10G10B10A2_UNORM,*/
    4, /*FORMAT_R10G10B10A2_UINT,*/

    4, /*FORMAT_R8G8B8A8_TYPELESS,*/
    4, /*FORMAT_R8G8B8A8_UNORM,*/
    4, /*FORMAT_B8G8R8A8_UNORM,*/
    4, /*FORMAT_R8G8B8A8_UNORM_SRGB,*/
    4, /*FORMAT_R8G8B8A8_UINT,*/
    4, /*FORMAT_R8G8B8A8_SNORM,*/
    4, /*FORMAT_R8G8B8A8_SINT,*/

    
    //3 chanel*/
    3 * 4, /*FORMAT_R32G32B32_TYPELESS,*/
    3 * 4, /*FORMAT_R32G32B32_FLOAT,*/
    3 * 4, /*FORMAT_R32G32B32_UINT,*/
    3 * 4, /*FORMAT_R32G32B32_SINT,*/
    

    //2 chanel*/
    2 * 4, /*FORMAT_R32G32_TYPELESS,*/
    2 * 4, /*FORMAT_R32G32_FLOAT,*/
    2 * 4, /*FORMAT_R32G32_UINT,*/
    2 * 4, /*FORMAT_R32G32_SINT,*/

    2 * 2, /*FORMAT_R16G16_TYPELESS,*/
    2 * 2, /*FORMAT_R16G16_FLOAT,*/
    2 * 2, /*FORMAT_R16G16_UNORM,*/
    2 * 2, /*FORMAT_R16G16_UINT,*/
    2 * 2, /*FORMAT_R16G16_SNORM,*/
    2 * 2, /*FORMAT_R16G16_SINT,*/

    2, /*FORMAT_R8G8_TYPELESS,*/
    2, /*FORMAT_R8G8_UNORM,*/
    2, /*FORMAT_R8G8_UINT,*/
    2, /*FORMAT_R8G8_SNORM,*/
    2, /*FORMAT_R8G8_SINT,*/

    //1 chanel*/
    4, /*FORMAT_R32_TYPELESS,*/
    4, /*FORMAT_R32_FLOAT,*/
    4, /*FORMAT_R32_UINT,*/
    4, /*FORMAT_R32_SINT,*/

    2, /*FORMAT_R16_TYPELESS,*/
    2, /*FORMAT_R16_FLOAT,*/
    2, /*FORMAT_R16_UNORM,*/
    2, /*FORMAT_R16_UINT,*/
    2, /*FORMAT_R16_SNORM,*/
    2, /*FORMAT_R16_SINT,*/

    1, /*FORMAT_R8_TYPELESS,*/
    1, /*FORMAT_R8_UNORM,*/
    1, /*FORMAT_R8_UINT,*/
    1, /*FORMAT_R8_SNORM,*/
    1, /*FORMAT_R8_SINT,*/
    1 /*FORMAT_A8_UNORM,*/
};

    

enum class EDepthFormat : uint8_t {

    FORMAT_D32_FLOAT_S8X24_UINT,
    FORMAT_D32_UNORM,
    FORMAT_D24_UNORM_S8_UINT,
    FORMAT_D16_UNORM,
};

enum class EDsvFlags : uint8_t {
    DSV_FLAG_NONE,
    DSV_FLAG_READ_ONLY_DEPTH,
    DSV_FLAG_READ_ONLY_STENCIL 
};

    
enum class EPrimitiveTopology : uint8_t {
    PRIMITIVE_TOPOLOGY_UNDEFINED                   ,
    PRIMITIVE_TOPOLOGY_POINTLIST                   ,
    PRIMITIVE_TOPOLOGY_LINELIST                    ,
    PRIMITIVE_TOPOLOGY_LINESTRIP                   ,
    PRIMITIVE_TOPOLOGY_TRIANGLE_FAN                ,
    PRIMITIVE_TOPOLOGY_TRIANGLELIST                ,
    PRIMITIVE_TOPOLOGY_TRIANGLESTRIP               ,
    PRIMITIVE_TOPOLOGY_LINELIST_ADJ                ,
    PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ               ,
    PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ            ,
    PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ           ,
    PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST   ,
    PRIMITIVE_TOPOLOGY_2_CONTROL_POINT_PATCHLIST   ,
    PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST   ,
    PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST   ,
};

enum class EResourceDimension
{
    RESOURCE_DIMENSION_UNKNOWN,
    RESOURCE_DIMENSION_BUFFER,
    RESOURCE_DIMENSION_TEXTURE1D,
    RESOURCE_DIMENSION_TEXTURE2D,
    RESOURCE_DIMENSION_TEXTURE3D,
};
    
enum class EShaderViewDimension {
    DIMENSION_UNKNOWN = 0,
    DIMENSION_TEXTURE1D,
    DIMENSION_TEXTURE1D_ARRAY,
    DIMENSION_TEXTURE2D,
    DIMENSION_TEXTURE2D_ARRAY,
    DIMENSION_TEXTURE3D,
    DIMENSION_TEXTURECUBE ,
    DIMENSION_TEXTURECUBE_ARRAY,
};
    
enum class ERTViewDimension {
    DIMENSION_UNKNOWN = 0,
    //DIMENSION_BUFFER, TODO unsupported
    DIMENSION_TEXTURE1D,
    DIMENSION_TEXTURE1D_ARRAY,
    DIMENSION_TEXTURE2D,
    DIMENSION_TEXTURE2D_ARRAY,
    DIMENSION_TEXTURE3D,
};

enum class EBindFlags : uint32_t {
    BIND_VERTEX_BUFFER          = 0x1L,
    BIND_INDEX_BUFFER           = 0x2L,
    BIND_CONSTANT_BUFFER        = 0x4L,
    BIND_SHADER_RESOURCE        = 0x8L,
    BIND_STREAM_OUTPUT          = 0x10L,
    BIND_RENDER_TARGET          = 0x20L,
    BIND_DEPTH_STENCIL          = 0x40L,
    BIND_UNORDERED_ACCESS       = 0x80L,
    //BIND_DECODER                = 0x200L,
    //BIND_VIDEO_ENCODER          = 0x400L
};
    
enum class ESamplerFilter : uint8_t {
    FILTER_UNKNOWN,
    FILTER_MIN_MAG_MIP_POINT,
    FILTER_MIN_MAG_POINT_MIP_LINEAR,
    FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT,
    FILTER_MIN_POINT_MAG_MIP_LINEAR,
    FILTER_MIN_LINEAR_MAG_MIP_POINT,
    FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR,
    FILTER_MIN_MAG_LINEAR_MIP_POINT,
    FILTER_MIN_MAG_MIP_LINEAR,
    FILTER_ANISOTROPIC,
    FILTER_COMPARISON_MIN_MAG_MIP_POINT,
    FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR,
    FILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT,
    FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR,
    FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT,
    FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR,
    FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT,
    FILTER_COMPARISON_MIN_MAG_MIP_LINEAR,
    FILTER_COMPARISON_ANISOTROPIC,
    FILTER_MINIMUM_MIN_MAG_MIP_POINT,
    FILTER_MINIMUM_MIN_MAG_POINT_MIP_LINEAR,
    FILTER_MINIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT,
    FILTER_MINIMUM_MIN_POINT_MAG_MIP_LINEAR,
    FILTER_MINIMUM_MIN_LINEAR_MAG_MIP_POINT,
    FILTER_MINIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR,
    FILTER_MINIMUM_MIN_MAG_LINEAR_MIP_POINT,
    FILTER_MINIMUM_MIN_MAG_MIP_LINEAR,
    FILTER_MINIMUM_ANISOTROPIC,
    FILTER_MAXIMUM_MIN_MAG_MIP_POINT,
    FILTER_MAXIMUM_MIN_MAG_POINT_MIP_LINEAR,
    FILTER_MAXIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT,
    FILTER_MAXIMUM_MIN_POINT_MAG_MIP_LINEAR,
    FILTER_MAXIMUM_MIN_LINEAR_MAG_MIP_POINT,
    FILTER_MAXIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR,
    FILTER_MAXIMUM_MIN_MAG_LINEAR_MIP_POINT,
    FILTER_MAXIMUM_MIN_MAG_MIP_LINEAR,
    FILTER_MAXIMUM_ANISOTROPIC
};

enum class ETextureAddressMode : uint8_t {
    TEXTURE_ADDRESS_UNKNOWN,
    TEXTURE_ADDRESS_WRAP,
    TEXTURE_ADDRESS_MIRROR,
    TEXTURE_ADDRESS_CLAMP,
    TEXTURE_ADDRESS_BORDER,
    TEXTURE_ADDRESS_MIRROR_ONCE
};

enum class EDrawCallType : uint8_t {
    DISPATCH,
    DISPATCH_INDIRECT,
    DRAW,
    DRAW_AUTO,
    DRAW_INDEXED,
    DRAW_INDEXED_INSTANCED,
    DRAW_INDEXED_INSTANCED_INDIRECT,
    DRAW_INSTANCED,
    DRAW_INSTANCED_INDIRECT
};

}
