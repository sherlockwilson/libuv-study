#include "url_cmd_manager.h"

#include "cmd.h"
#include "uv_http_util.h"

URLCmdManager* URLCmdManager::Instance() {
    static URLCmdManager ins;
    return &ins;
}

bool URLCmdManager::Init() {
    map_["/test"] = std::bind(&TestCmd::Print,"123");
    return true;
}

void URLCmdManager::UnInit() {

}

std::string URLCmdManager::HttpResponse(
    const HttpSessionSptr& in_sptr_session) {
    std::function<std::string()> cmd_fun;
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


    return HttpServerUtil::HttpResponse(200, context, cookie, cmd_fun().c_str());
}