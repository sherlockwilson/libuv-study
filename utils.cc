#include "utils.h"

#include <ctype.h>
#include <string.h>
#include <stdarg.h>
#include <array>
#include <cstdint>
#include <algorithm>
#include <limits>

#include "log.h"

namespace top {

namespace storage {

int get_log4cpp_level(int level) {
    switch (level) {
    case maidsafe::log::kVerbose:
        return log4cpp::Priority::DEBUG;
        break;
    case maidsafe::log::kInfo:
    case maidsafe::log::kSuccess:
        return log4cpp::Priority::INFO;
        break;
    case maidsafe::log::kWarning:
        return log4cpp::Priority::WARN;
        break;
    case maidsafe::log::kError:
    case maidsafe::log::kAlways:
        return log4cpp::Priority::ERROR;
    default:
        return level;
    }
}

const static std::string CHUNK_KEY_PREFIX = "ck_";
const static std::string GROUP_KEY_PREFIX = "gr_";
std::atomic<uint32_t> TopParameters::message_id(1);

std::string create_chunk_key(const std::string& chunk_id) {
    return CHUNK_KEY_PREFIX + chunk_id;
}

std::string create_chunk_key(const char* chunk_id, int len) {
    return CHUNK_KEY_PREFIX + std::string(chunk_id, len);
}

std::string create_group_key(const std::string& group_id) {
    return GROUP_KEY_PREFIX + group_id;

}
std::string create_group_key(const char* group_id, int len) {
    return GROUP_KEY_PREFIX + std::string(group_id, len);

}

//skip last chr
int skip_chr(const char* str, int len, const char* skip) {
    bool match;
    char* p, *s;

    if (len <= 0) {
        len = (int)strlen(str);
        if (len == 0) return 0;
    }
    if (!skip) return len;

    p = (char*)str + len;
    while (p > str) {
        if (*p != 0) {
            match = 0;
            s = (char*)skip;
            while (*s) {
                if (*p == *s) {
                    match = 1;
                    break;
                }
                s++;
            }
            if (match) *p = 0;
            else break;
        }
        p--;
    }
    return (int)(p - str);
}

#ifdef _WIN32
int popen_fill(char* data, int len, const char* cmd, ...) {
    return 0;
}
#else
//get command line
int popen_fill(char* data, int len, const char* cmd, ...) {
    FILE* output;
    va_list vlist;
    char line[2048];
    int line_len, line_count, total_len, id;

    if (!data || len <= 0 || !cmd)
        return -1;

    va_start(vlist, cmd);
    line_len = vsnprintf(line, 2047, cmd, vlist);
    if (line_len < 0) line_len = 0;
    va_end(vlist);
    line[line_len] = 0;

    line_count = id = total_len = 0;
    if ((output = popen(line, "r"))) {
        while (!feof(output) && fgets(line, sizeof(line) - 1, output)) {
            line_len = (int)strlen(line);
            if (len - total_len - 1 < line_len) {
                break;
            }
            //			if (!matchline || match_line(id, line, line_len-1, matchline)){
            memcpy(data + total_len, line, line_len);
            total_len += line_len;
            line_count++;
            //			}
            id++;
        }
        pclose(output);
    }
    data[total_len] = 0;
    if (line_count == 1 && total_len > 0) {
        skip_chr(data, 0, "\n");
    }
    return total_len;
}
#endif // _WIN32

}  // namespace storage

}  // namespace top