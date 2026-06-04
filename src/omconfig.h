
#ifndef OM_CONFIGURATOR_H
#define OM_CONFIGURATOR_H

#include <string>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <algorithm>

class OmConfigurator
{
  public:

    explicit OmConfigurator( const std::string& confFile = "../conf/omicro.conf")
    {
        load(confFile);
    }

    std::string get( const std::string& key, const std::string& defval= "") const
    {
        auto itr = map_.find(key);

        if (itr == map_.end()) {
            return defval;
        }

        return itr->second;
    }

    bool exists( const std::string& key) const
    {
        return map_.find(key) != map_.end();
    }


  private:
    std::unordered_map< std::string, std::string > map_;

    static std::string trim( const std::string& s)
    {
        size_t b = s.find_first_not_of( " \t\r\n");

        if (b == std::string::npos) { 
            return "";
        }

        size_t e = s.find_last_not_of( " \t\r\n");
        return s.substr( b, e - b + 1);
    }

    void load ( const std::string& confFile)
    {
        std::ifstream fin( confFile.c_str());

        if (!fin) { return;
        }

        std::string line;

        while (std::getline(fin, line)) {
        
            line = trim(line);

            if (line.empty()) {
                continue;
            }

            ////////////////////////////////////////////////////
            // comments:
            // ###
            // #
            ////////////////////////////////////////////////////

            if (line[0] == '#') {
                continue;
            }

            size_t pos = line.find('=');

            if (pos == std::string::npos) {
                continue;
            }

            std::string key = trim( line.substr( 0, pos));
            std::string value = trim( line.substr( pos + 1));
            map_[key] = value;
        }
    }
};

#endif

