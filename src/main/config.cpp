#include "config.h"

namespace fineLanding
{
    bool Config::init(libconfig::Config &cfg)
    {
        /*char currentDir[PATH_MAX];
        if (getcwd(currentDir, sizeof(currentDir)) == NULL)
        {
            std::cout << "error: getcwd\r\n";
            return false;
        }
        std::string sFile = std::string(currentDir) + "/config.txt";
        */
        try
        {
            cfg.readFile("finelanding.cfg");
        }
        catch (const libconfig::FileIOException &ex)
        {
            std::cerr << "I/O error while reading file config.txt" << std::endl;
            return false;
        }
        catch (const libconfig::ParseException &pex)
        {
            std::cerr << "Parse error at " << pex.getFile() << ":" << pex.getLine()
                      << " - " << pex.getError() << std::endl;
            return false;
        }

        return true;
    }

    std::string Config::getString(const char *name, const char *sDefault)
    {
        libconfig::Config cfg;
        if (init(cfg))
        {
            try
            {
                return cfg.lookup(name);
            }
            catch (libconfig::SettingNotFoundException ex)
            {
            }
        }
        return sDefault;
    }

    double Config::getDouble(const char *name, double fDefault)
    {
        libconfig::Config cfg;
        double fRet = fDefault;
        if (init(cfg))
        {
            try
            {
                fRet = cfg.lookup(name);
            }
            catch (libconfig::SettingNotFoundException ex)
            {
            }
        }
        return fRet;
    }
}