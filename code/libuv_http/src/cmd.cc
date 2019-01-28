#include "cmd.h"

#include "http_util.h"

#include <stdio.h>

std::string TestCmd::Print(
    const std::string& in_str_print) {
    const char* context = "text/html";
    const char* cookie = "";
    printf(in_str_print.c_str());
    return  HttpServerUtil::HttpResponse(200, context, cookie, in_str_print.c_str());
}