#include"load_conf.h"
#include<iostream>

namespace top {
namespace storage {

Config* Config::Instance(std::string path) {
    static int flag = 0;
    static Config _cf;
    if(flag == 0) {
        _cf.load(path);
        flag ++;
    }
    return &_cf;
}

std::string Config::GetCountryCode() {
    return this->_countryCode;
}

std::string Config::GetIp() {
    return this->_ip;
}

uint16_t Config::GetTcpPort() {
    return this->_tcpPort;
}

uint16_t Config::GetUdpPort() {
    return this->_udpPort;
}

std::string Config::GetDbPath() {
    return this->_db;
}


char * Config::rtrim(char *str) {
    if (str == NULL || *str == '\0') {
        return str;
    }
    int len = strlen(str);
    char *p = str + len - 1;
    while (p >= str && isspace(*p)) {
        *p = '\0';
        --p;
    }
    return str;
}

char * Config::ltrim(char *str) {
    if (str == NULL || *str == '\0') {
        return str;
    }
    int len = 0;
    char *p = str;
    while (*p != '\0' && isspace(*p)) {
        ++p;
        ++len;
    }
    memmove(str, p, strlen(str) - len + 1);
    return str;
}

char * Config::trim(char *str) {
    str = rtrim(str);
    str = ltrim(str);
    return str;
}


/*
 * 字符串中寻找以等号分开的键值对
 * @param  src  源字符串 [输入参数]
 * @param  key     键    [输出参数]
 * @param value    值    [输出参数]
 */
int Config::strkv(char *src, char *key, char *value) {
    char *p, *q;
    int len;

    if (*src == '#') return 0; // # 号开头为注视，直接忽略

    p = strchr(src, '=');   // p找到等号
    q = strchr(src, '\n');   // q找到换行

    // 如果有等号有换行
    if (p != NULL && q != NULL) {
        *q = '\0'; // 将换行设置为字符串结尾
        strncpy(key, src, p - src); // 将等号前的内容拷贝到 key 中
        char * tk = trim(key);
        memmove(key, tk, strlen(tk));

        strcpy(value, p + 1); // 将等号后的内容拷贝到 value 中
        char * tv = trim(value);
        if (strlen(tv) == 0) {
            //如果去除左右两边空格后得到一个空串，那么直接返回 0
            return 0;
        }
        memmove(value, tv, strlen(tv));
        return 1;
    } else {
        return 0;
    }
}

/*
 * 配置函数
 * @param configFilePath 配置文件路径 [输入参数]
 * @param   configVar    存储配置的值 [输出参数]
 * @param   configNum    需配置的个数 [输入参数]
 */
void Config::load(std::string configFilePath) {
    int i;
    FILE *fd;
    char buf[256] = "";  // 缓冲字符串
    char key[64] = "";  // 配置变量名
    char value[128] = ""; // 配置变量值

    // 打开配置文件
    fd = fopen(configFilePath.c_str(), "r");

    if (fd == NULL) {
        printf("配置文件打开失败！\n");
        exit(-1);
    }

    try {
        // 依次读取文件的每一行
        while (fgets(buf, sizeof(buf), fd)) {
            // 读取键值对
            if (strkv(buf, key, value)) {
                // 读取成功则循环与配置数组比较
                // 名称相等则拷贝
                if (strcmp(key, "countrycode") == 0) {
                    _countryCode = value;
                } else if(strcmp(key, "ip") == 0) {
                    _ip = value;
                } else if(strcmp(key, "tcp_port") == 0) {
                    std::string t_str(value, strlen(value));
                    _tcpPort = std::stoi(t_str);
                } else if(strcmp(key, "udp_port") == 0) {
                    std::string u_str(value, strlen(value));
                    _udpPort = std::stoi(u_str);
                } else if(strcmp(key, "db") == 0) {
                    _db = value;
                }
                // 清空 读取出来的 key
                memset(key, 0, strlen(key));
                memset(value, 0, strlen(value));
            }
        }

    } catch (const std::exception& e) {
        std::cout << "catch error:" << e.what() << std::endl;
        return;
    }

    fclose(fd);
}


} // end namespace storage
} // end namespace top
