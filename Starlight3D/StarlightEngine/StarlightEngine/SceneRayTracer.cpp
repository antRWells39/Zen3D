#include "pch.h"
#include "Application.h"
#include "SceneRayTracer.h"
#include <math.h>
#include "VString.h"
#include "SceneGraph.h"
#include "Mesh3D.h"
#include "SmartDraw.h"


SceneRayTracer::SceneRayTracer(SceneGraph* graph) {


    TextureDesc RTDesc = {};
    RTDesc.Name = "RTColor buffer";
    RTDesc.Type = RESOURCE_DIM_TEX_2D;
    RTDesc.Width = Application::GetApp()->GetWidth();
    RTDesc.Height = Application::GetApp()->GetHeight();
    RTDesc.BindFlags = BIND_UNORDERED_ACCESS | BIND_SHADER_RESOURCE;
    RTDesc.ClearValue.Format = m_ColorBufferFormat;
    RTDesc.Format = m_ColorBufferFormat;
    mGraph = graph;
    Application::GetApp()->GetDevice()->CreateTexture(RTDesc, nullptr, &m_pColorRT);

    mColorTex = new Texture2D(m_pColorRT);

    mDraw = new SmartDraw(Application::GetApp());
    
    Initialize();

    mNumTextures = mGraph->GetRTTextureList().size();
        
	CreatePSO();
 
    MapTextures();
    CreateBindingTables();
}

void SceneRayTracer::ProcessScene() {



}

void SceneRayTracer::Initialize() {

    // Create a buffer with shared constants.
    BufferDesc BuffDesc;
    BuffDesc.Name = "Constant buffer";
    BuffDesc.Size = sizeof(m_Constants);
    BuffDesc.Usage = USAGE_DEFAULT;
    BuffDesc.BindFlags = BIND_UNIFORM_BUFFER;

    Application::GetApp()->GetDevice()->CreateBuffer(BuffDesc, nullptr, &m_ConstantsCB);
    VERIFY_EXPR(m_ConstantsCB != nullptr);

}

void SceneRayTracer::CreatePSO() {


    m_MaxRecursionDepth = std::min(m_MaxRecursionDepth, Application::GetApp()->GetDevice()->GetAdapterInfo().RayTracing.MaxRecursionDepth);

    // Prepare ray tracing pipeline description.
    RayTracingPipelineStateCreateInfoX PSOCreateInfo;

    PSOCreateInfo.PSODesc.Name = "Ray tracing PSO";
    PSOCreateInfo.PSODesc.PipelineType = PIPELINE_TYPE_RAY_TRACING;

    // Define shader macros
    ShaderMacroHelper Macros;
    Macros.AddShaderMacro("NUM_TEXTURES", mNumTextures);

    ShaderCreateInfo ShaderCI;
    // We will not be using combined texture samplers as they
    // are only required for compatibility with OpenGL, and ray
    // tracing is not supported in OpenGL backend.
    ShaderCI.UseCombinedTextureSamplers = false;

    ShaderCI.Macros = Macros;

    // Only new DXC compiler can compile HLSL ray tracing shaders.
    ShaderCI.ShaderCompiler = SHADER_COMPILER_DXC;

    // Shader model 6.3 is required for DXR 1.0, shader model 6.5 is required for DXR 1.1 and enables additional features.
    // Use 6.3 for compatibility with DXR 1.0 and VK_NV_ray_tracing.
    ShaderCI.HLSLVersion = { 6, 3 };
    ShaderCI.SourceLanguage = SHADER_SOURCE_LANGUAGE_HLSL;

    // Create a shader source stream factory to load shaders from files.
    RefCntAutoPtr<IShaderSourceInputStreamFactory> pShaderSourceFactory;
    Application::GetApp()->GetFactory()->CreateDefaultShaderSourceStreamFactory(nullptr, &pShaderSourceFactory);
    ShaderCI.pShaderSourceStreamFactory = pShaderSourceFactory;

    // Create ray generation shader.
    RefCntAutoPtr<IShader> pRayGen;
    {
        ShaderCI.Desc.ShaderType = SHADER_TYPE_RAY_GEN;
        ShaderCI.Desc.Name = "Ray tracing RG";
        ShaderCI.FilePath = "data/rt/RayTrace.rgen";
        ShaderCI.EntryPoint = "main";
        Application::GetApp()->GetDevice()->CreateShader(ShaderCI, &pRayGen);
        VERIFY_EXPR(pRayGen != nullptr);
    }

    // Create miss shaders.
    RefCntAutoPtr<IShader> pPrimaryMiss, pShadowMiss;
    {
        ShaderCI.Desc.ShaderType = SHADER_TYPE_RAY_MISS;
        ShaderCI.Desc.Name = "Primary ray miss shader";
        ShaderCI.FilePath = "data/rt/PrimaryMiss.rmiss";
        ShaderCI.EntryPoint = "main";
        Application::GetApp()->GetDevice()->CreateShader(ShaderCI, &pPrimaryMiss);
        VERIFY_EXPR(pPrimaryMiss != nullptr);

        ShaderCI.Desc.Name = "Shadow ray miss shader";
        ShaderCI.FilePath = "data/rt/ShadowMiss.rmiss";
        ShaderCI.EntryPoint = "main";
        Application::GetApp()->GetDevice()->CreateShader(ShaderCI, &pShadowMiss);
        VERIFY_EXPR(pShadowMiss != nullptr);
    }

    // Create closest hit shaders.
    RefCntAutoPtr<IShader> pCubePrimaryHit, pGroundHit, pGlassPrimaryHit, pSpherePrimaryHit;
    {
        ShaderCI.Desc.ShaderType = SHADER_TYPE_RAY_CLOSEST_HIT;
        ShaderCI.Desc.Name = "Cube primary ray closest hit shader";
        ShaderCI.FilePath = "data/rt/CubePrimaryHit.rchit";
        ShaderCI.EntryPoint = "main";
        Application::GetApp()->GetDevice()->CreateShader(ShaderCI, &pCubePrimaryHit);
        VERIFY_EXPR(pCubePrimaryHit != nullptr);

        ShaderCI.Desc.Name = "Ground primary ray closest hit shader";
        ShaderCI.FilePath = "data/rt/Ground.rchit";
        ShaderCI.EntryPoint = "main";
        Application::GetApp()->GetDevice()->CreateShader(ShaderCI, &pGroundHit);
        VERIFY_EXPR(pGroundHit != nullptr);

        ShaderCI.Desc.Name = "Glass primary ray closest hit shader";
        ShaderCI.FilePath = "data/rt/GlassPrimaryHit.rchit";
        ShaderCI.EntryPoint = "main";
        Application::GetApp()->GetDevice()->CreateShader(ShaderCI, &pGlassPrimaryHit);
        VERIFY_EXPR(pGlassPrimaryHit != nullptr);

        ShaderCI.Desc.Name = "Sphere primary ray closest hit shader";
        ShaderCI.FilePath = "data/rt/SpherePrimaryHit.rchit";
        ShaderCI.EntryPoint = "main";
        Application::GetApp()->GetDevice()->CreateShader(ShaderCI, &pSpherePrimaryHit);
        VERIFY_EXPR(pSpherePrimaryHit != nullptr);
    }

    // Create intersection shader for a procedural sphere.
    RefCntAutoPtr<IShader> pSphereIntersection;
    {
        ShaderCI.Desc.ShaderType = SHADER_TYPE_RAY_INTERSECTION;
        ShaderCI.Desc.Name = "Sphere intersection shader";
        ShaderCI.FilePath = "data/rt/SphereIntersection.rint";
        ShaderCI.EntryPoint = "main";
        Application::GetApp()->GetDevice()->CreateShader(ShaderCI, &pSphereIntersection);
        VERIFY_EXPR(pSphereIntersection != nullptr);
    }

    // Setup shader groups

    // Ray generation shader is an entry point for a ray tracing pipeline.
    PSOCreateInfo.AddGeneralShader("Main", pRayGen);
    // Primary ray miss shader.
    PSOCreateInfo.AddGeneralShader("PrimaryMiss", pPrimaryMiss);
    // Shadow ray miss shader.
    PSOCreateInfo.AddGeneralShader("ShadowMiss", pShadowMiss);

    // Primary ray hit group for the textured cube.
    PSOCreateInfo.AddTriangleHitShader("CubePrimaryHit", pCubePrimaryHit);
    // Primary ray hit group for the ground.
//    PSOCreateInfo.AddTriangleHitShader("GroundHit", pGroundHit);
    // Primary ray hit group for the glass cube.
 //   PSOCreateInfo.AddTriangleHitShader("GlassPrimaryHit", pGlassPrimaryHit);

    // Intersection and closest hit shaders for the procedural sphere.
 //   PSOCreateInfo.AddProceduralHitShader("SpherePrimaryHit", pSphereIntersection, pSpherePrimaryHit);
    // Only intersection shader is needed for shadows.
  //  PSOCreateInfo.AddProceduralHitShader("SphereShadowHit", pSphereIntersection);

    // Specify the maximum ray recursion depth.
    // WARNING: the driver does not track the recursion depth and it is the
    //          application's responsibility to not exceed the specified limit.
    //          The value is used to reserve the necessary stack size and
    //          exceeding it will likely result in driver crash.
    PSOCreateInfo.RayTracingPipeline.MaxRecursionDepth = static_cast<Uint8>(m_MaxRecursionDepth);

    // Per-shader data is not used.
    PSOCreateInfo.RayTracingPipeline.ShaderRecordSize = 0;

    // DirectX 12 only: set attribute and payload size. Values should be as small as possible to minimize the memory usage.
    PSOCreateInfo.MaxAttributeSize = std::max<Uint32>(sizeof(/*BuiltInTriangleIntersectionAttributes*/ float2), sizeof(HLSL::ProceduralGeomIntersectionAttribs));
    PSOCreateInfo.MaxPayloadSize = std::max<Uint32>(sizeof(HLSL::PrimaryRayPayload), sizeof(HLSL::ShadowRayPayload));

    // Define immutable sampler for g_Texture and g_GroundTexture. Immutable samplers should be used whenever possible
    SamplerDesc SamLinearWrapDesc{
        FILTER_TYPE_LINEAR, FILTER_TYPE_LINEAR, FILTER_TYPE_LINEAR,
        TEXTURE_ADDRESS_WRAP, TEXTURE_ADDRESS_WRAP, TEXTURE_ADDRESS_WRAP //
    };

    PipelineResourceLayoutDescX ResourceLayout;
    ResourceLayout.DefaultVariableType = SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE;
    ResourceLayout.AddImmutableSampler(SHADER_TYPE_RAY_CLOSEST_HIT, "g_SamLinearWrap", SamLinearWrapDesc);
    ResourceLayout
        .AddVariable(SHADER_TYPE_RAY_GEN | SHADER_TYPE_RAY_MISS | SHADER_TYPE_RAY_CLOSEST_HIT, "g_ConstantsCB", SHADER_RESOURCE_VARIABLE_TYPE_STATIC)
        .AddVariable(SHADER_TYPE_RAY_GEN, "g_ColorBuffer", SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC);

    PSOCreateInfo.PSODesc.ResourceLayout = ResourceLayout;

    Application::GetApp()->GetDevice()->CreateRayTracingPipelineState(PSOCreateInfo, &m_pRayTracingPSO);
    VERIFY_EXPR(m_pRayTracingPSO != nullptr);

    m_pRayTracingPSO->GetStaticVariableByName(SHADER_TYPE_RAY_GEN, "g_ConstantsCB")->Set(m_ConstantsCB);
    m_pRayTracingPSO->GetStaticVariableByName(SHADER_TYPE_RAY_MISS, "g_ConstantsCB")->Set(m_ConstantsCB);
    m_pRayTracingPSO->GetStaticVariableByName(SHADER_TYPE_RAY_CLOSEST_HIT, "g_ConstantsCB")->Set(m_ConstantsCB);

    m_pRayTracingPSO->CreateShaderResourceBinding(&m_pRayTracingSRB, true);
    VERIFY_EXPR(m_pRayTracingSRB != nullptr);

    int aaa = 5;


}

void SceneRayTracer::CreateBindingTables() {

    ShaderBindingTableDesc SBTDesc;
    SBTDesc.Name = "SBT";
    SBTDesc.pPSO = m_pRayTracingPSO;

    Application::GetApp()->GetDevice()->CreateSBT(SBTDesc, &m_pSBT);
    VERIFY_EXPR(m_pSBT != nullptr);

    m_pSBT->BindRayGenShader("Main");

    m_pSBT->BindMissShader("PrimaryMiss", PRIMARY_RAY_INDEX);
    m_pSBT->BindMissShader("ShadowMiss", SHADOW_RAY_INDEX);

    auto m_pTLAS = mGraph->GetTLAS();




    int num_inst = mGraph->RTInstanceCount();
    
    
    //mInstanceGeos.push_back()


    for (int i = 0;i < num_inst;i++)
    {

        mInstanceGeos.push_back(mGraph->GetRTInstance(i));

        VString name = VString("Instance ");

        name.Add(VString(i));
        m_pSBT->BindHitGroupForInstance(m_pTLAS,name.GetConst(), PRIMARY_RAY_INDEX, "CubePrimaryHit");
    }

    //m_pSBT->BindHitGroupForInstance(m_pTLAS, "Cube Instance 2", PRIMARY_RAY_INDEX, "CubePrimaryHit");
    //m_pSBT->BindHitGroupForInstance(m_pTLAS, "Cube Instance 3", PRIMARY_RAY_INDEX, "CubePrimaryHit");
    //m_pSBT->BindHitGroupForInstance(m_pTLAS, "Cube Instance 4", PRIMARY_RAY_INDEX, "CubePrimaryHit");
    //m_pSBT->BindHitGroupForInstance(m_pTLAS, "Ground Instance", PRIMARY_RAY_INDEX, "GroundHit");
    //m_pSBT->BindHitGroupForInstance(m_pTLAS, "Glass Instance", PRIMARY_RAY_INDEX, "GlassPrimaryHit");
    //m_pSBT->BindHitGroupForInstance(m_pTLAS, "Sphere Instance", PRIMARY_RAY_INDEX, "SpherePrimaryHit");

    // clang-format on

    // Hit groups for shadow ray.
    // null means no shaders are bound and hit shader invocation will be skipped.
    m_pSBT->BindHitGroupForTLAS(m_pTLAS, SHADOW_RAY_INDEX, nullptr);

    // We must specify the intersection shader for procedural geometry.
    //m_pSBT->BindHitGroupForInstance(m_pTLAS, "Sphere Instance", SHADOW_RAY_INDEX, "SphereShadowHit");

    // Update SBT with the shader groups we bound
    Application::GetApp()->GetContext()->UpdateSBT(m_pSBT);

}

void SceneRayTracer::UpdateAttribs() {

    mUpdateAttribs = false;
    HLSL::CubeAttribs Attribs;
    BufferDesc BuffDesc;
    BuffDesc.Name = "Cube Attribs";
    BuffDesc.Usage = USAGE_IMMUTABLE;
    BuffDesc.BindFlags = BIND_UNIFORM_BUFFER;
    BuffDesc.Size = sizeof(Attribs);

    auto def_mesh = mInstanceGeos[0];

    std::vector<Vertex> vertices = def_mesh->GetVertices();

    for (int i = 0;i <vertices.size();i++) {

        Vertex cur_vert = vertices[i];

        Attribs.UVs[i].x = cur_vert.texture_coord.x;
        Attribs.UVs[i].y = cur_vert.texture_coord.y;
        Attribs.UVs[i].z = cur_vert.texture_coord.z;

        Attribs.Normals[i].x = cur_vert.normal.x;
        Attribs.Normals[i].y = cur_vert.normal.y;
        Attribs.Normals[i].z = cur_vert.normal.z;

    }

    for (int i = 0;i < def_mesh->NumTris();i++) {

        auto cur_tri = def_mesh->GetTri(i);

        Attribs.Primitives[i].x = cur_tri.v0;
        Attribs.Primitives[i].y = cur_tri.v1;
        Attribs.Primitives[i].z = cur_tri.v2;

    }

    BufferData BufData = { &Attribs, BuffDesc.Size };

    Application::GetApp()->GetDevice()->CreateBuffer(BuffDesc, &BufData, &m_CubeAttribsCB);
    int a = 5;
}

void SceneRayTracer::Render() {

    UpdateConstants();

    auto cam = mGraph->GetCamera();

    float3 CameraWorldPos = cam->GetPosition();//  float3::MakeVector(m_Camera.GetWorldMatrix()[3]);
    auto   CameraViewProj = cam->GetWorldMatrix() * cam->GetProjectionMatrix();

    m_Constants.CameraPos = float4{ CameraWorldPos, 1.0f };
    m_Constants.InvViewProj = CameraViewProj.Inverse().Transpose();

    auto context = Application::GetApp()->GetContext();

    context->UpdateBuffer(m_ConstantsCB, 0, sizeof(m_Constants), &m_Constants, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    
    if (mUpdateAttribs) {

        UpdateAttribs();

    }
    m_pRayTracingSRB->GetVariableByName(SHADER_TYPE_RAY_CLOSEST_HIT, "g_CubeAttribsCB")->Set(m_CubeAttribsCB);

    m_pRayTracingSRB->GetVariableByName(SHADER_TYPE_RAY_GEN, "g_ColorBuffer")->Set(m_pColorRT->GetDefaultView(TEXTURE_VIEW_UNORDERED_ACCESS));

    m_pRayTracingSRB->GetVariableByName(SHADER_TYPE_RAY_GEN, "g_TLAS")->Set(mGraph->GetTLAS());
    m_pRayTracingSRB->GetVariableByName(SHADER_TYPE_RAY_CLOSEST_HIT, "g_TLAS")->Set(mGraph->GetTLAS());

    context->SetPipelineState(m_pRayTracingPSO);
    context->CommitShaderResources(m_pRayTracingSRB, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

    TraceRaysAttribs Attribs;
    Attribs.DimensionX = m_pColorRT->GetDesc().Width;
    Attribs.DimensionY = m_pColorRT->GetDesc().Height;
    Attribs.pSBT = m_pSBT;

    context->TraceRays(Attribs);

    context->Flush();

    mDraw->Begin();

    mDraw->DrawTexture(0, 0, Application::GetApp()->GetWidth(), Application::GetApp()->GetHeight(), mColorTex, 1, 1, 1, 1);

    mDraw->End();

}


void SceneRayTracer::MapTextures()
{

    auto tex_list= mGraph->GetRTTextureList();

    mNumTextures = tex_list.size();

    std::vector<IDeviceObject*> pTexSRV;
    std::vector<ITexture> pTex;

    for (int i = 0;i < mNumTextures;i++) {

        auto tex = tex_list[i];

        auto* pSRV = tex->GetViewPTR();

        pTexSRV.push_back(pSRV);


    }

    m_pRayTracingSRB->GetVariableByName(SHADER_TYPE_RAY_CLOSEST_HIT, "g_CubeTextures")->SetArray(pTexSRV.data(), 0, mNumTextures);


    //IDeviceObject* pTexSRVs[mNumTextures] = {};
    //RefCntAutoPtr<ITexture> pTex[NumTextures];


}

void SceneRayTracer::UpdateConstants() {

    {
        m_Constants.ClipPlanes = float2{ 0.1f, 100.0f };
        m_Constants.ShadowPCF = 1;
        m_Constants.MaxRecursion = std::min(Uint32{ 6 }, m_MaxRecursionDepth);

        // Sphere constants.
        m_Constants.SphereReflectionColorMask = { 0.81f, 1.0f, 0.45f };
        m_Constants.SphereReflectionBlur = 1;

        // Glass cube constants.
        m_Constants.GlassReflectionColorMask = { 0.22f, 0.83f, 0.93f };
        m_Constants.GlassAbsorption = 0.5f;
        m_Constants.GlassMaterialColor = { 0.33f, 0.93f, 0.29f };
        m_Constants.GlassIndexOfRefraction = { 1.5f, 1.02f };
        m_Constants.GlassEnableDispersion = 0;

        // Wavelength to RGB and index of refraction interpolation factor.
        m_Constants.DispersionSamples[0] = { 0.140000f, 0.000000f, 0.266667f, 0.53f };
        m_Constants.DispersionSamples[1] = { 0.130031f, 0.037556f, 0.612267f, 0.25f };
        m_Constants.DispersionSamples[2] = { 0.100123f, 0.213556f, 0.785067f, 0.16f };
        m_Constants.DispersionSamples[3] = { 0.050277f, 0.533556f, 0.785067f, 0.00f };
        m_Constants.DispersionSamples[4] = { 0.000000f, 0.843297f, 0.619682f, 0.13f };
        m_Constants.DispersionSamples[5] = { 0.000000f, 0.927410f, 0.431834f, 0.38f };
        m_Constants.DispersionSamples[6] = { 0.000000f, 0.972325f, 0.270893f, 0.27f };
        m_Constants.DispersionSamples[7] = { 0.000000f, 0.978042f, 0.136858f, 0.19f };
        m_Constants.DispersionSamples[8] = { 0.324000f, 0.944560f, 0.029730f, 0.47f };
        m_Constants.DispersionSamples[9] = { 0.777600f, 0.871879f, 0.000000f, 0.64f };
        m_Constants.DispersionSamples[10] = { 0.972000f, 0.762222f, 0.000000f, 0.77f };
        m_Constants.DispersionSamples[11] = { 0.971835f, 0.482222f, 0.000000f, 0.62f };
        m_Constants.DispersionSamples[12] = { 0.886744f, 0.202222f, 0.000000f, 0.73f };
        m_Constants.DispersionSamples[13] = { 0.715967f, 0.000000f, 0.000000f, 0.68f };
        m_Constants.DispersionSamples[14] = { 0.459920f, 0.000000f, 0.000000f, 0.91f };
        m_Constants.DispersionSamples[15] = { 0.218000f, 0.000000f, 0.000000f, 0.99f };
        m_Constants.DispersionSampleCount = 4;

        m_Constants.AmbientColor = float4(1.f, 1.f, 1.f, 0.f) * 0.015f;
        m_Constants.LightPos[0] = { 8.00f, +8.0f, +0.00f, 0.f };
        m_Constants.LightColor[0] = { 1.00f, +0.8f, +0.80f, 0.f };
        m_Constants.LightPos[1] = { 0.00f, +4.0f, -5.00f, 0.f };
        m_Constants.LightColor[1] = { 0.85f, +1.0f, +0.85f, 0.f };

        // Random points on disc.
        m_Constants.DiscPoints[0] = { +0.0f, +0.0f, +0.9f, -0.9f };
        m_Constants.DiscPoints[1] = { -0.8f, +1.0f, -1.1f, -0.8f };
        m_Constants.DiscPoints[2] = { +1.5f, +1.2f, -2.1f, +0.7f };
        m_Constants.DiscPoints[3] = { +0.1f, -2.2f, -0.2f, +2.4f };
        m_Constants.DiscPoints[4] = { +2.4f, -0.3f, -3.0f, +2.8f };
        m_Constants.DiscPoints[5] = { +2.0f, -2.6f, +0.7f, +3.5f };
        m_Constants.DiscPoints[6] = { -3.2f, -1.6f, +3.4f, +2.2f };
        m_Constants.DiscPoints[7] = { -1.8f, -3.2f, -1.1f, +3.6f };
    }
    static_assert(sizeof(HLSL::Constants) % 16 == 0, "must be aligned by 16 bytes");

}