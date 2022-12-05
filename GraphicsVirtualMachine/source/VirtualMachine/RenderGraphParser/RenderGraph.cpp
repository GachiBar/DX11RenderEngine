#include "RenderGraph.h"

#include <cassert>

GVM::RenderGraphNode::RenderGraphNode(EMachineCommands Command, void* Description):
    tuple<GVM::EMachineCommands, void*>(Command, Description)
{}



bool GVM::SyncThreadBlock::TryAdd(const RenderGraphNode& Node,
    const std::vector<Resource*>& readDependencies,
    const std::vector<Resource*>& wrightDependencies
)
{
    static std::vector<Resource*> NewReadDep;
    static std::vector<Resource*> NewWrightDep;
    NewReadDep.clear();
    NewWrightDep.clear();

    bool result = true;

    for (auto& readDep : readDependencies)
    {
        if (std::find(WrightDependencies.rbegin(), WrightDependencies.rend(), readDep) != WrightDependencies.rend())
        {
            return false;
        }
        if (std::find(ReadDependencies.rbegin(), ReadDependencies.rend(), readDep) != ReadDependencies.rend())
        {
            continue;
        }
        NewReadDep.push_back(readDep);
        //NewWrightDep.push_back(readDep);
    }
    for (auto& wrightDep : wrightDependencies)
    {
        if (std::find(ReadDependencies.rbegin(), ReadDependencies.rend(), wrightDep) != ReadDependencies.rend())
        {
            return false;
        }
        if (std::find(WrightDependencies.rbegin(), WrightDependencies.rend(), wrightDep) != WrightDependencies.rend())
        {
            continue;
        }
        NewWrightDep.push_back(wrightDep);
    }
    
    for (auto& readDep : NewReadDep)
    {
        ReadDependencies.push_back(readDep);
    }
    for (auto& wrightDep : NewWrightDep)
    {
        WrightDependencies.push_back(wrightDep);
    }

    Nodes.push_back(Node);
    return true;
}


void GVM::RenderGraph::AddCommand(RenderGraphNode Node,
    const std::vector<Resource*>& ReadDependencies,
    const std::vector<Resource*>& WrightDependencies)
{
    //switch (Node.Command())
    //{
    //case EMachineCommands::BEGIN_EVENT:
    //case EMachineCommands::END_EVENT:
    //case EMachineCommands::CLEAR_PIPELINE:
    //{
    //    if (lasrLockedBlock == Blocks.size() - 1)
    //    {
    //        assert(Blocks[lasrLockedBlock].TryAdd(Node, ReadDependencies, WrightDependencies), "Draw could not be added.");
    //        return;
    //    }
    //    lasrLockedBlock = Blocks.size();
    //    Blocks.push_back({});
    //    assert(Blocks[lasrLockedBlock].TryAdd(Node, ReadDependencies, WrightDependencies), "Draw could not be added.");
    //    return;
    //}
    //}


    if (!Blocks.rbegin()->TryAdd(Node, ReadDependencies, WrightDependencies))
    {
        Blocks.push_back({});
        bool succses = (Blocks[Blocks.size() - 1].TryAdd(Node, ReadDependencies, WrightDependencies));
        assert(succses);
    }
}


void GVM::RenderGraph::Clear()
{
    Blocks.clear();
    Blocks.push_back({});
}
