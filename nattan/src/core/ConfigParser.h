/** Copyright (C) Nattan, Inc - All Rights Reserved
 ** Unauthorized copying of this file, via any medium is strictly prohibited
 ** Proprietary and confidential
 ** Written by Kam Lu <yingshulu@gmail.com>, July 2019
 **/


#ifndef NATTAN_CORE_CONFIGPARSER_H
#define NATTAN_CORE_CONFIGPARSER_H

/*Common */
#include <iostream>
#include <string>
#include <map>
#include <memory>
#include "base/Uncopyable.h"

namespace nattan {

class ConfigParser: public Uncopyable
{

#define SECTB  '['
#define SECTE  ']'
#define COMTB  '#'
#define KEYB   '='
#define SECTION 0
#define COMMENT 1
#define KEYVAL  2
#define ERRLINE -1

public:
    ConfigParser() :isFileExisted(false), isRefreshed(false){}

    void load(const std::string&);

    std::string getValueByKey(const std::string& section, const std::string& key);

    void getAllKey();

    int reload();

    ~ConfigParser() {}

private:

    std::string filename;

    bool isFileExisted;

    bool isRefreshed;

    //map <section map<key, value>>
    std::map<std::string, std::map<std::string, std::string> > configMap;

    int loadConfig2Map();

    bool isCommented(const std::string& line);

    bool isVaildSetting(const std::string& line);

    bool isVaildSection(const std::string& line);

    int decideLineType(const std::string& line, std::string& content);

};

typedef std::shared_ptr<ConfigParser> ConfigParserPtr;

}//namespace nattan

#endif
