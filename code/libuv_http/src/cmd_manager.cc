#include "cmd_manager.h"

#include "cmd.h"
#include "http_util.h"

namespace top {

CmdManager* CmdManager::Instance() {
    static CmdManager ins;
    return &ins;
}

bool CmdManager::Init() {
    map_["/test"] = std::bind(&Cmd::Print, std::placeholders::_1);
    map_["/command/routingtable/print"] = std::bind(&Cmd::PrintRoutingTable);
    map_["/command/routingtable/print/all"] = std::bind(&Cmd::PrintRoutingTableAll);
    map_["/command/routingtable/print/relaytest/all"] = std::bind(&Cmd::RelayTestAll);
    map_["/command/routingtable/print/node/all"] = std::bind(&Cmd::GetAllNodesFromBootNode);
    map_["/command/routingtable/print/relay/id"] = std::bind(&Cmd::RelayTest, std::placeholders::_1);
    map_["/command/routingtable/print/rrt/id"] = std::bind(&Cmd::RRT, std::placeholders::_1);
    map_["/command/routingtable/remove/node/id"] = std::bind(&Cmd::RemoveTestNode, std::placeholders::_1);
    map_["/command/routingtable/join/node"] = std::bind(&Cmd::TopNodeJoin, std::placeholders::_1);
    map_["/command/routingtable/save/node/id"] = std::bind(&Cmd::SaveAllNodeInfo);
    map_["/command/routingtable/save/node/all"] = std::bind(&Cmd::SaveAllNodeInfo2);
    map_["/command/routingtable/find/local/closets/nodes"] = std::bind(&Cmd::FindClosestNodes);
    map_["/command/routingtable/find/global/closets/nodes"] = std::bind(&Cmd::GetGroup, std::placeholders::_1);
    map_["/command/smartobject/store/value"] = std::bind(&Cmd::SmartObjectStore, std::placeholders::_1);   
    map_["/command/smartobject/find/value"] = std::bind(&Cmd::SmartObjectFindValue, std::placeholders::_1); 
    map_["/command/smartobject/store/list"] = std::bind(&Cmd::SmartObjectStoreList, std::placeholders::_1);
    map_["/command/smartobject/find/list"] = std::bind(&Cmd::SmartObjectFindValueList, std::placeholders::_1);
    return true;
}

void CmdManager::UnInit() {

}

std::string CmdManager::HttpResponse(
    const HttpSessionSptr& in_sptr_session) {
    std::function<std::string(const std::string&)> cmd_fun;
    const char* context = "text/html";
    const char* cookie = "";

    if (NULL == in_sptr_session) {
        return HttpServerUtil::HttpResponse(200, context, cookie, "");
    }
    if (!FindData(
        in_sptr_session->path_info,
        cmd_fun)) {
        return HttpServerUtil::HttpResponse(200, context, cookie, "");
    }


    return HttpServerUtil::HttpResponse(200, context, cookie, 
        cmd_fun(in_sptr_session->query).c_str());
}

}  //  namespace top