#pragma once

#include <functional>

#include "manager_template.h"
#include "http_util_def.h"

namespace top {

class CmdManager :
    public ManagerTemplate<
    std::string,
    std::function<
    std::string(const std::string&)>> {
public:
    static CmdManager* Instance();
    bool Init();
    void UnInit();
    std::string HttpResponse(
        const HttpSessionSptr& in_sptr_session);
private:
    CmdManager() {}
    ~CmdManager() {}
};

}  //  namespace top