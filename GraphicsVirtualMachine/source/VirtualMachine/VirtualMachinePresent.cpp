#include "VirtualMachine.h"
#include <iostream>
#include <unordered_set>

using namespace GVM;


void VirtualMachine::ExecuteSetupPipeline(Compressed::PipelineSnapshot* ps)
{
    static PipelineDescription pipelineDescription;
    static IRenderDevice::RENDERTARGETVIEWHANDLE renderTargets[MAX_RENDERTARGET_ATTACHMENTS];
    static IRenderDevice::UATARGETVIEWHANDLE uaTargets[MAX_RENDERTARGET_ATTACHMENTS];
    static IRenderDevice::VERTEXBUFFERVIEWHANDLE vertexBuffers[32];
    static IRenderDevice::RESOURCEVIEWHANDLE textures[128];
    static IRenderDevice::CONSTBUFFERVIEWHANDLE constBuffers[32];
    static IRenderDevice::RESOURCEVIEWHANDLE texturesNulls[128];
    static IRenderDevice::CONSTBUFFERVIEWHANDLE constBuffersNulls[32];

    pipelinesQueueShift += ps->SnapshotByteSize;

    auto Textures = (ResourceView**)(ps->Data + ps->TexturesShift);
    auto ConstBuffers = (ConstBufferView**)(ps->Data + ps->ConstBuffersShift);

    for (int i = 0; i < ps->texturesNum; i++)
    {
        textures[i] = (IRenderDevice::RESOURCEVIEWHANDLE)resourcesManager.GetRealResourceView(Textures[i]);
    }
    for (int i = 0; i < ps->constBuffersNum; i++)
    {
        constBuffers[i] = (IRenderDevice::CONSTBUFFERVIEWHANDLE)resourcesManager.GetRealResourceView(ConstBuffers[i]);
    }
    RenderDevice->SetupTextures(texturesNulls, 128);
    RenderDevice->SetupRenderTargets(
        nullptr, 8,
        nullptr);
        RenderDevice->SetupUATargets(nullptr, 16);

    pipelineDescription.CS = resourcesManager.GetRealShader(ps->CS);
    pipelineDescription.PS = resourcesManager.GetRealShader(ps->PS);
    pipelineDescription.VS = resourcesManager.GetRealShader(ps->VS);
    pipelineDescription.DS = resourcesManager.GetRealShader(ps->DS);
    pipelineDescription.HS = resourcesManager.GetRealShader(ps->HS);
    pipelineDescription.GS = resourcesManager.GetRealShader(ps->GS);

    pipelineDescription.samplers = (Compressed::SamplerStateDesc*)(ps->Data + ps->SamplersShift);
    pipelineDescription.samplersNum = ps->samplersNum;
    
    if (ps->VS != nullptr && ps->PS != nullptr)
    {
        pipelineDescription.isCS = false;
        auto RenderTargets = (Compressed::RenderTargetDesc*)(ps->Data + ps->RenderTargetsShift);
        for (int i = 0; i < ps->renderTargetsNum; i++)
        {
            pipelineDescription.blendState.BlendStates[i] = RenderTargets[i].BlendState;
            renderTargets[i] = (IRenderDevice::RENDERTARGETVIEWHANDLE)resourcesManager.
                GetRealResourceView(RenderTargets[i].rtv);
        }


        for (int i = ps->renderTargetsNum; i < MAX_RENDERTARGET_ATTACHMENTS; i++)
        {
            pipelineDescription.blendState.BlendStates[i] = Compressed::BlendStateDesc();
            renderTargets[i] = nullptr;
        }
        auto VertexBuffers = (VertexBufferView**)(ps->Data + ps->VertexBuffersShift);
        for (int i = 0; i < ps->vertexBuffersNum; i++)
        {
            vertexBuffers[i] = (IRenderDevice::VERTEXBUFFERVIEWHANDLE)resourcesManager.GetRealResourceView(
                VertexBuffers[i]);
        }

        pipelineDescription.blendState.desc = ps->blendDesc;
        pipelineDescription.viewports = (Compressed::ViewportDesc*)(ps->Data + ps->ViewportsShift);
        pipelineDescription.viewportsNum = ps->viewportsNum;
        pipelineDescription.depthStencilState = ps->depthStencilState;
        pipelineDescription.rasterizerState = ps->rasterizerState;
        pipelineDescription.topology = ps->primitiveTopology;
        pipelineDescription.layout = resourcesManager.GetRealInputLayout(ps->vertexDeclaration);

        RenderDevice->SetupPipeline(pipelineDescription);

        RenderDevice->SetupVertexBuffers((const IRenderDevice::VERTEXBUFFERVIEWHANDLE*)vertexBuffers,
                                         ps->vertexBuffersNum);
        RenderDevice->SetupIndexBuffers(
            (IRenderDevice::INDEXBUFFERVIEWHANDLE)resourcesManager.GetRealResourceView(ps->indexBuffer));

        RenderDevice->SetupRenderTargets(
            (const IRenderDevice::RENDERTARGETVIEWHANDLE*)renderTargets, ps->renderTargetsNum,
            (IRenderDevice::DEPTHSTENCILVIEWHANDLE)resourcesManager.GetRealResourceView(ps->DepthBuffer));
    }

    if (ps->CS)
    {
        pipelineDescription.isCS = true;
        RenderDevice->SetupPipeline(pipelineDescription);

        auto UATargets = (UATargetView**)(ps->Data + ps->UATargetsShift);

        for (int i = 0; i < ps->uaTargetsNum; i++)
        {
            uaTargets[i] = (IRenderDevice::UATARGETVIEWHANDLE)resourcesManager.GetRealResourceView(UATargets[i]);
        }
        for (int i = ps->uaTargetsNum; i < MAX_RENDERTARGET_ATTACHMENTS; i++)
        {
            uaTargets[i] = nullptr;
        }

        RenderDevice->SetupUATargets((IRenderDevice::UATARGETVIEWHANDLE*)uaTargets, ps->renderTargetsNum);
    }


    RenderDevice->SetupConstBuffers(constBuffers, ps->constBuffersNum);
    RenderDevice->SetupTextures(textures, ps->texturesNum);
}

void VirtualMachine::GetPipelineResourceDependencies(Compressed::PipelineSnapshot* ps)
{
    LastPsReadDep.clear();
    LastPsWriteDep.clear();
    std::unordered_set<Resource*> LastPsReadDepMap, LastPsWriteDepMap;

    auto RenderTargets = (Compressed::RenderTargetDesc*)(ps->Data + ps->RenderTargetsShift);
    auto UATargets = (UATargetView**)(ps->Data + ps->UATargetsShift);


    for (int i = 0; i < ps->renderTargetsNum; i++)
    {
        if (RenderTargets[i].rtv != nullptr)
            LastPsWriteDepMap.insert(resourcesManager.
                                     GetResourceView(RenderTargets[i].rtv).resource);
    }

    for (int i = 0; i < ps->uaTargetsNum; i++)
    {
        if (UATargets[i] != nullptr)
            LastPsWriteDepMap.insert(
                resourcesManager.GetResourceView(UATargets[i]).resource);
    }


    auto VertexBuffers = (VertexBufferView**)(ps->Data + ps->VertexBuffersShift);
    auto Textures = (ResourceView**)(ps->Data + ps->TexturesShift);
    auto ConstBuffers = (ConstBufferView**)(ps->Data + ps->ConstBuffersShift);

    for (int i = 0; i < ps->vertexBuffersNum; i++)
    {
        if (VertexBuffers[i] != nullptr)
            LastPsReadDepMap.insert(
                resourcesManager.GetResourceView(VertexBuffers[i]).resource);
    }
    for (int i = 0; i < ps->texturesNum; i++)
    {
        if (Textures[i] != nullptr)
            LastPsReadDepMap.insert(
                resourcesManager.GetResourceView(Textures[i]).resource);
    }
    for (int i = 0; i < ps->constBuffersNum; i++)
    {
        if (ConstBuffers[i] != nullptr)
            LastPsReadDepMap.insert(
                resourcesManager.GetResourceView(ConstBuffers[i]).resource);
    }

    if (ps->indexBuffer != nullptr)
        LastPsReadDepMap.insert(
            resourcesManager.GetResourceView(ps->indexBuffer).resource);

    for (auto& readDep : LastPsReadDepMap)
    {
        LastPsReadDep.push_back(readDep);
    }
    for (auto& wrightDep : LastPsWriteDepMap)
    {
        LastPsWriteDep.push_back(wrightDep);
    }
}


void VirtualMachine::PushCommand(EMachineCommands command)
{
    commandQueue.push_back(command);

    switch (command)
    {
    case EMachineCommands::CREATE_RESOURCE:
        {
            auto& resource = resourcesManager.GetResource((Resource*)PullPointer());
            renderGraph.AddCommand(
                {
                    EMachineCommands::CREATE_RESOURCE,
                    (void*)resource.id
                },
                {},
                {resource.id}
            );
            break;
        }

    case EMachineCommands::UPDATE_RESOURCE:
        {
            auto& resource = resourcesManager.GetResource((Resource*)PullPointer());
            renderGraph.AddCommand(
                {
                    EMachineCommands::UPDATE_RESOURCE,
                    (void*)resource.id
                },
                {},
                {resource.id}
            );
            break;
        }

    case EMachineCommands::CREATE_RESOURCE_VIEW:
        {
            auto& resourceView = resourcesManager.GetResourceView((ResourceView*)PullPointer());
            renderGraph.AddCommand(
                {
                    EMachineCommands::CREATE_RESOURCE_VIEW,
                    (void*)resourceView.id
                },
                {},
                {resourceView.resource}
            );
            break;
        }

    case EMachineCommands::SET_RESOURCE_DATA:
        {
            // auto& resource = resourcesManager.GetResource((Resource*)PullPointer());
            auto& description = PullData<SetResourceDataDesc>();
            PullPointer(description.params.dataSize);
            renderGraph.AddCommand(
                {
                    EMachineCommands::SET_RESOURCE_DATA,
                    (void*)
                    ((uint8_t*)&description - dataQueue.data())
                },
                {},
                {description.resource}
            );
            break;
        }

    case EMachineCommands::SETUP_PIPELINE:
        {
            auto* ps = (Compressed::PipelineSnapshot*)((pipelinesQueue.data()) + pipelinesQueueShift);

            GetPipelineResourceDependencies(ps);

            renderGraph.AddCommand(
                {
                    EMachineCommands::SETUP_PIPELINE,
                    (void*)pipelinesQueueShift
                },
                {LastPsReadDep},
                {}
            );

            pipelinesQueueShift += ps->SnapshotByteSize;
            break;
        }

    case EMachineCommands::DRAW:
        {
            renderGraph.AddCommand(
                {
                    EMachineCommands::DRAW,
                    (void*)drawCallsQueueShift
                },
                LastPsReadDep,
                LastPsWriteDep
            );
            drawCallsQueueShift++;
            break;
        }
    case EMachineCommands::CLEAR_PIPELINE:
        {
            //RenderDevice->ClearState();
            break;
        }
    case EMachineCommands::CLEAR_RT:
        {
            auto& desc = PullData<ClearRenderTargetDesc>();

            renderGraph.AddCommand(
                {
                    EMachineCommands::CLEAR_RT,
                    (void*)
                    ((uint8_t*)&desc - dataQueue.data())
                },
                {},
                {resourcesManager.GetResourceView(desc.resource).resource}
            );
            break;
        }
    case EMachineCommands::CLEAR_DS:
        {
            auto& desc = PullData<ClearDepthStencilDesc>();
            renderGraph.AddCommand(
                {
                    EMachineCommands::CLEAR_DS,
                    (void*)
                    ((uint8_t*)&desc - dataQueue.data())
                },
                {},
                {resourcesManager.GetResourceView(desc.resource).resource}
            );
            break;
        }
    case EMachineCommands::BEGIN_EVENT:
        {
            const char* name = (const char*)dataQueue.data() + queueShift;
            //std::cout << name << std::endl;
            PullPointer(std::strlen(name) + 1);

            renderGraph.AddCommand(
                {
                    EMachineCommands::BEGIN_EVENT,
                    (void*)
                    ((uint8_t*)name - dataQueue.data())
                },
                {},
                {}
            );
            break;
        }
    case EMachineCommands::END_EVENT:
        {
            renderGraph.AddCommand(
                {
                    EMachineCommands::END_EVENT,
                    nullptr
                },
                {},
                {}
            );

            break;
        }
    }
}

void VirtualMachine::SyncResourcesRead(const std::vector<Resource*>& resorces)
{
    static std::vector<IRenderDevice::RESOURCEHANDLE> ReadDep;
    ReadDep.clear();
    for (auto* r : resorces)
    {
        auto& res = resourcesManager.GetResource(r);
        if (res.currentState != GpuResource::ResourceState::Read)
        {
            ReadDep.push_back(resourcesManager.GetRealResource(r));
            res.currentState = GpuResource::ResourceState::Read;
        }
    }
    if (ReadDep.size() > 2)
        RenderDevice->SyncResourcesRead(ReadDep.data(), ReadDep.size());
}

void VirtualMachine::SyncResourcesWrite(const std::vector<Resource*>& resorces)
{
    static std::vector<IRenderDevice::RESOURCEHANDLE> WriteDep;
    //ReadDep.clear();
    for (auto* r : resorces)
    {
        auto& res = resourcesManager.GetResource(r);
        if (res.currentState != GpuResource::ResourceState::Write)
        {
            WriteDep.push_back(resourcesManager.GetRealResource(r));
            res.currentState = GpuResource::ResourceState::Write;
        }
    }
    if (WriteDep.size() > 2)
        RenderDevice->SyncResourcesWrite(WriteDep.data(), WriteDep.size());
}


void VirtualMachine::RunVM()
{
    int comandIndex = 0;

    for (auto& block : renderGraph.Blocks)
    {

        SyncResourcesRead(block.ReadDependencies);
        SyncResourcesWrite(block.WrightDependencies);

        for (auto& [command, description] : block.Nodes)
        {
            comandIndex++;
            switch (command)
            {
            case EMachineCommands::CREATE_RESOURCE:
                {
                    auto& resource = resourcesManager.GetResource((Resource*)(description));
                    resource.resource = RenderDevice->CreateResource(resource); //todo
                    break;
                }

            case EMachineCommands::UPDATE_RESOURCE:
                {
                    auto& resource = resourcesManager.GetResource((Resource*)description);
                    RenderDevice->DestroyResource(resource.resource);
                    resource.resource = RenderDevice->CreateResource(resource);
                    break;
                }

            case EMachineCommands::CREATE_RESOURCE_VIEW:
                {
                    auto& resourceView = resourcesManager.GetResourceView((ResourceView*)description);
                    resourceView.view = RenderDevice->CreateResourceView(resourceView,
                                                                         resourcesManager.GetResource(
                                                                             resourceView.resource));
                    break;
                }

            case EMachineCommands::SET_RESOURCE_DATA:
                {
                    // auto& resource = resourcesManager.GetResource((Resource*)PullPointer());
                    auto& desc = *(SetResourceDataDesc*)(dataQueue.data() + (uint32_t)description);
                    RenderDevice->SetResourceData(resourcesManager.GetResource(desc.resource),
                                                  desc.params.dstSubresource,
                                                  desc.params.rect,
                                                  dataQueue.data() + desc.shift,
                                                  desc.params.srcRowPitch,
                                                  desc.params.srcDepthPitch);
                    break;
                }

            case EMachineCommands::SETUP_PIPELINE:
                {
                    auto* ps = (Compressed::PipelineSnapshot*)((pipelinesQueue.data()) + (uint32_t)description);
                    ExecuteSetupPipeline(ps);
                    break;
                }

            case EMachineCommands::DRAW:
                {
                    //auto drawData = *(DrawCall*)description;
                    RenderDevice->Draw(*(DrawCall*)(drawCallsQueue.data() + (uint32_t)description));
                    drawCallsQueueShift++;
                    break;
                }
            case EMachineCommands::CLEAR_PIPELINE:
                {
                    RenderDevice->ClearState();
                    break;
                }
            case EMachineCommands::CLEAR_RT:
                {
                    auto& desc = *(ClearRenderTargetDesc*)(dataQueue.data() + (uint32_t)description);
                    RenderDevice->ClearRenderTarget(
                        (IRenderDevice::RENDERTARGETVIEWHANDLE)resourcesManager.GetRealResourceView(desc.resource),
                        desc.color);
                    break;
                }
            case EMachineCommands::CLEAR_DS:
                {
                    auto& desc = *(ClearDepthStencilDesc*)(dataQueue.data() + (uint32_t)description);
                    //auto& resource = PullData<DepthStencilView*>();
                    //auto& depth = PullData<float>();
                    //auto& stencil = PullData<uint8_t>();
                    RenderDevice->ClearDepthStencil(
                        (IRenderDevice::DEPTHSTENCILVIEWHANDLE)resourcesManager.GetRealResourceView(desc.resource),
                        desc.depth,
                        desc.stencil
                    );
                    break;
                }
            case EMachineCommands::BEGIN_EVENT:
                {
                    const char* name = (char*)(dataQueue.data() + (uint32_t)description);
                    // std::cout << name << std::endl;
                    // PullPointer(std::strlen(name) + 1);
                    RenderDevice->BeginEvent(name);
                    break;
                }
            case EMachineCommands::END_EVENT:
                {
                    RenderDevice->EndEvent();

                    break;
                }
            }
        }
        std::cout << std::flush;
    }

    static IRenderDevice::RENDERTARGETVIEWHANDLE renderTargets[1] = {nullptr};
    RenderDevice->SetupRenderTargets((const IRenderDevice::RENDERTARGETVIEWHANDLE*)renderTargets, 1, nullptr);

    renderGraph.Clear();
    queueShift = 0;
    pipelinesQueueShift = 0;
    drawCallsQueueShift = 0;
    pipelinesQueue.clear();
    commandQueue.clear();
    dataQueue.clear();
    drawCallsQueue.clear();
}


void VirtualMachine::Present()
{
    RenderDevice->Present();
}


void VirtualMachine::ResizeBackbuffer(int32_t width, int32_t height)
{
    RenderDevice->ResizeBackbuffer(width, height);
}
