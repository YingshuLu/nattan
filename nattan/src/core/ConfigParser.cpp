/** Copyright (C) Nattan, Inc - All Rights Reserved
 ** Unauthorized copying of this file, via any medium is strictly prohibited
 ** Proprietary and confidential
 ** Written by Kam Lu <yingshulu@gmail.com>, July 2019
 **/

#include <iostream>
#include <string>
#include <fstream>
#include <utility>
#include <unistd.h>
#include <string.h>
#include "core/ConfigParser.h"

namespace nattan {

void ConfigParser::load(const std::string& file)
{

    if (access(file.c_str(), F_OK) == -1)
    {
        isFileExisted = false;
        return;

    }

    filename = file;
    configMap.clear();
    loadConfig2Map();
}

int ConfigParser::reload() {
    
    configMap.clear();
    loadConfig2Map();
    return 0;
}

/* decide the line type
    return: line type
    content used for filled "section/keyValue" string
*/
int ConfigParser::decideLineType(const std::string& line, std::string& content)
{
    int idx = 0;
    int len = line.length();
    bool emptyLine = false;

    int bidx = idx;
    while (true)
    {
        if (line[idx] != ' ' || idx >= len)
        {
            if (idx >= len)
            {
                emptyLine = true;
            }
            break;
        }
        idx++;
    }

    idx = 0;
    char lst = line[idx];
    switch (lst)
    {
    case SECTB:

        bidx = idx;
        while (true)
        {
            if (line[idx] == SECTB || idx >= len)
            {
                if (idx >= len && line[idx] != SECTB)
                {
                    content = std::string();
                    return ERRLINE;
                }
                content = line.substr(bidx + 1, len-2);
                return SECTION;
            }
            idx++;
        }

        break;

    case COMTB:

        content = std::string();
        return COMMENT;
        break;

    default:

        if (emptyLine)
        {
            content = std::string();
            return ERRLINE;
        }

        while (true)
        {
            if (line[idx] == KEYB || idx >= len)
            {
                if (idx >= len && line[idx] != KEYB)
                {
                    content = std::string();
                    return ERRLINE;
                }
                content = line;
                return KEYVAL;
            }
            idx++;
        }

        content = std::string();
        return ERRLINE;
    }

    return ERRLINE;
}

int ConfigParser::loadConfig2Map()
{
    std::ifstream fin(filename.c_str());
    int ret;

    int idx;
    std::string line;
    std::string content, section, key, value;

    bool isSameSec = true;
    bool is1stSec = true;

    std::map<std::string, std::string> kvMap;
    while ( getline(fin, line) )
    {
        ret = decideLineType(line, content);
        switch (ret)
        {

        case SECTION:

            if (is1stSec)
            {
                is1stSec = false;
            }
            else{

                isSameSec = false;
                configMap.insert(std::pair<std::string, std::map<std::string, std::string> >(section, kvMap));
                kvMap.clear();
            }
            section = content;
            break;

        case KEYVAL:

            idx = 0;

            while (true)
            {
                if (content[idx] == '=')
                {
                    key = content.substr(0, idx);
                    value = content.substr(idx + 1);
                    break;
                }
                idx++;
            }
            //Insert in map
            kvMap.insert(std::pair<std::string, std::string>(key, value));
            break;

        default:
            continue;
        }

    }

    configMap.insert(std::pair<std::string, std::map<std::string, std::string> >(section, kvMap));
    fin.close();
    return 0;

}

std::string ConfigParser::getValueByKey(const std::string& section, const std::string& key)
{
    std::map<std::string, std::map<std::string, std::string> >::iterator cIter = configMap.find(section);

    if (cIter != configMap.end())
    {
        std::map<std::string, std::string>::iterator it = cIter->second.find(key);
        if (it != cIter->second.end())
        {
            return it->second;
        }
    }
    return "";
}

void ConfigParser::getAllKey()
{
    std::map<std::string, std::map<std::string, std::string> >::iterator cIter = configMap.begin();

    while (cIter != configMap.end())
    {
        std::cout <<"[ "<< cIter->first <<" ]"<<std::endl;

        std::map<std::string, std::string>::iterator it = cIter->second.begin();
        while(it != cIter->second.end())
        {
            std::cout<<it->first<<"="<<it->second<<std::endl;
            it++;
        }
        cIter++;
    }
    return;
}

}//namespace nattan
