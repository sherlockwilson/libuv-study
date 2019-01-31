#include "cmd.h"

#include <stdio.h>

#include "http_util.h"

namespace top {

static const char* g_context = "text/html";
static const char* g_cookie = "";

std::string Cmd::Print(
    const std::string& in_str_print) {
    printf(in_str_print.c_str());
    return  HttpServerUtil::HttpResponse(200, g_context, g_cookie, "Print OK");
}

std::string Cmd::Help() {
    return HttpServerUtil::HttpResponse(200, g_context, g_cookie, "Help OK");
}

std::string Cmd::PrintRoutingTable() {
    return HttpServerUtil::HttpResponse(200, g_context, g_cookie, "PrintRoutingTable OK");
}

std::string Cmd::PrintRoutingTableAll() {
    return HttpServerUtil::HttpResponse(200, g_context, g_cookie, "PrintRoutingTableAll OK");
}

std::string Cmd::RelayTestAll() {
    return HttpServerUtil::HttpResponse(200, g_context, g_cookie, "RelayTestAll OK");
}

std::string Cmd::GetAllNodesFromBootNode() {
    return HttpServerUtil::HttpResponse(200, g_context, g_cookie, "GetAllNodesFromBootNode OK");
}

std::string Cmd::RemoveTestNode(
    const std::string& params) {
    return HttpServerUtil::HttpResponse(200, g_context, g_cookie, "RemoveTestNode OK");
}

std::string Cmd::SaveAllNodeInfo() {
    return HttpServerUtil::HttpResponse(200, g_context, g_cookie, "SaveAllNodeInfo OK");
}

std::string Cmd::SaveAllNodeInfo2() {
    return HttpServerUtil::HttpResponse(200, g_context, g_cookie, "SaveAllNodeInfo2 OK");
}

std::string Cmd::RelayTest(
    const std::string& params) {
    return HttpServerUtil::HttpResponse(200, g_context, g_cookie, "RelayTest OK");
}

std::string Cmd::RRT(
    const std::string& params) {
    return HttpServerUtil::HttpResponse(200, g_context, g_cookie, "RRT OK");
}

std::string Cmd::TopNodeJoin(
    const std::string& params) {
    return HttpServerUtil::HttpResponse(200, g_context, g_cookie, "TopNodeJoin OK");
}

std::string Cmd::FindClosestNodes() {
    return HttpServerUtil::HttpResponse(200, g_context, g_cookie, "FindClosestNodes OK");
}

std::string Cmd::GetGroup(
    const std::string& params) {
    return HttpServerUtil::HttpResponse(200, g_context, g_cookie, "GetGroup OK");
}

std::string Cmd::SmartObjectStore(
    const std::string& params) {
    return HttpServerUtil::HttpResponse(200, g_context, g_cookie, "SmartObjectStore OK");
}

std::string Cmd::SmartObjectFindValue(
    const std::string& params) {
    return HttpServerUtil::HttpResponse(200, g_context, g_cookie, "SmartObjectFindValue OK");
}

std::string Cmd::SmartObjectStoreList(
    const std::string& params) {
    return HttpServerUtil::HttpResponse(200, g_context, g_cookie, "SmartObjectStoreList OK");
}

std::string Cmd::SmartObjectFindValueList(
    const std::string& params) {
    return HttpServerUtil::HttpResponse(200, g_context, g_cookie, "SmartObjectFindValueList OK");
}

} //  namespace top