#include "message_loop.h"

#include <string.h>

#include "cmd_manager.h"
#include "http_util.h"
#include "http_server.h"

namespace top
{
void MessageLoopForGet::Exec(const HttpSessionSptr session) {
    char* postfix = strrchr(const_cast<char*>(session->path_info.c_str()), '.');
    const char* url = HttpServer::Instance()->url().c_str();
    if (postfix) {
        postfix++;
        if (url) {
            char file[1024] = { 0 };
            snprintf(file, sizeof(file), "%s%s", url, session->path_info.c_str());
            HttpServerUtil::SendFile(
                session->client,
                HttpServerUtil::HandleContentType(postfix).c_str(),
                file, session->path_info.c_str());
        }
        else {
            std::string respone = HttpServerUtil::HttpErrorPage(
                403, session->path_info.c_str());
            HttpServerUtil::WriteUvData(
                session->client,
                respone.c_str(), -1, 0, 1);
        }
    }
    else {
        std::string str_respose = CmdManager::Instance()->HttpResponse(
            session);
        char* respone = const_cast<char*>(
            str_respose.c_str());
        if (*respone == ' ') {
            if (url) {
                char* file_path = respone;
                file_path++;
                postfix = strrchr(file_path, '.');
                postfix++;
                char file[1024] = { 0 };
                snprintf(file, sizeof(file), "%s%s", url, file_path);
                HttpServerUtil::SendFile(
                    session->client,
                    HttpServerUtil::HandleContentType(postfix).c_str(),
                    file,
                    session->path_info.c_str());
            }
            else {
                std::string respone =
                    HttpServerUtil::HttpErrorPage(
                        403, session->path_info.c_str());
                HttpServerUtil::WriteUvData(
                    session->client,
                    respone.c_str(), -1, 0, 1);
            }
        }
        else {
            HttpServerUtil::WriteUvData(
                session->client,
                respone, -1, 0, 1);
        }
    }
}
}  //  namespace top
