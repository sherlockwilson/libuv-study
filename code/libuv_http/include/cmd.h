#pragma once

#include <string>

namespace top {

struct Cmd {
    static std::string Print(
        const std::string& in_str_print);
    static std::string Help();
    static std::string PrintRoutingTable();
    static std::string PrintRoutingTableAll();
    static std::string RelayTestAll();
    static std::string GetAllNodesFromBootNode();
    static std::string RemoveTestNode(
        const std::string& params);
    static std::string SaveAllNodeInfo();
    static std::string SaveAllNodeInfo2();
    static std::string RelayTest(
        const std::string& params);
    static std::string RRT(
        const std::string& params);
    static std::string TopNodeJoin(
        const std::string& params);
    static std::string FindClosestNodes();
    static std::string GetGroup(
        const std::string& params);
    static std::string SmartObjectStore(
        const std::string& params);
    static std::string SmartObjectFindValue(
        const std::string& params);
    static std::string SmartObjectStoreList(
        const std::string& params);
    static std::string SmartObjectFindValueList(
        const std::string& params);
};

}  //  namespace top