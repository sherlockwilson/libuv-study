#pragma once

#include <functional>

#include "manager_template.h"
#include "uv_http_util_def.h"

class URLCmdManager :
    public ManagerTemplate<
    std::string,
    std::function<std::string()>> {
public:
    static URLCmdManager* Instance();
    bool Init();
    void UnInit();
    std::string HttpResponse(
        const HttpSessionSptr& in_sptr_session);
private:
    URLCmdManager() {}
    ~URLCmdManager() {}
};