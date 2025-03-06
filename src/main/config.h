#include <iostream>
#include <libconfig.h++>

namespace fineLanding
{
    class Config
    {
    public:
        std::string getString(const char *, const char *);
        double getDouble(const char *, double fDefault);
        template <typename T>
        T get(const char *name, T vDefault)
        {
            libconfig::Config cfg;
            T ret = vDefault;
            if (init(cfg))
            {
                try
                {
                    ret = cfg.lookup(name);
                }
                catch (libconfig::SettingNotFoundException ex)
                {
                }
            }
            return ret;
        };

    private:
        bool init(libconfig::Config &cfg);
    };
};
