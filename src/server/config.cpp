#include "config.hpp"

#include <fstream>
#include <sstream>

// 读取配置文件中的配置项
// 逐行解析：分号开头是注释，[section] 是段落头，其余按 key=value 处理
string getConfigValue(const string &section, const string &key, const string &defaultValue)
{
    ifstream in("conf/chat.conf");
    // 配置文件不存在时，使用默认值
    if (!in.is_open())
    {
        return defaultValue;
    }

    string line;
    string curSection;
    while (getline(in, line))
    {
        // 去掉行首尾空白字符
        size_t start = line.find_first_not_of(" \t\r\n");
        if (start == string::npos)
        {
            continue; // 空行
        }
        line = line.substr(start);
        size_t end = line.find_last_not_of(" \t\r\n");
        if (end != string::npos)
        {
            line = line.substr(0, end + 1);
        }

        // 分号开头是注释
        if (line[0] == ';')
        {
            continue;
        }

        // 段落头，如 [mysql]
        if (line[0] == '[')
        {
            size_t pos = line.find(']');
            if (pos != string::npos)
            {
                curSection = line.substr(1, pos - 1);
            }
            continue;
        }

        // key=value
        size_t pos = line.find('=');
        if (pos == string::npos)
        {
            continue;
        }

        string k = line.substr(0, pos);
        string v = line.substr(pos + 1);
        // 去掉key和value两侧空白字符
        size_t kStart = k.find_first_not_of(" \t");
        size_t vStart = v.find_first_not_of(" \t");
        if (kStart != string::npos)
        {
            k = k.substr(kStart);
        }
        if (vStart != string::npos)
        {
            v = v.substr(vStart);
        }
        size_t vEnd = v.find_last_not_of(" \t\r\n");
        if (vEnd != string::npos)
        {
            v = v.substr(0, vEnd + 1);
        }

        // 当前段落匹配且key匹配，返回value
        if (curSection == section && k == key)
        {
            return v;
        }
    }

    return defaultValue;
}
