#ifndef CONFIG_H
#define CONFIG_H

#include <string>
using namespace std;

// 从配置文件 conf/chat.conf 中读取配置项
// 配置格式：INI风格，[section] 分段，key=value，分号开头为注释
// 配置文件不存在或键不存在时，返回默认值 defaultValue
string getConfigValue(const string &section, const string &key, const string &defaultValue);

#endif // CONFIG_H
