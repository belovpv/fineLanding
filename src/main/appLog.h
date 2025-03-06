#include <sstream>

namespace fineLanding
{
    class AppLog
    {
    public:
        static std::string timestamp()
        {
            std::time_t timet = std::time(nullptr);
            tm *ptmTime = localtime(&timet);
            std::ostringstream s;
            s << ptmTime->tm_hour << ":" << ptmTime->tm_min << ":" << ptmTime->tm_sec;
            return std::string(s.str());
        };

        static void log(const char *pstr)
        {
            std::cout << pstr << std::endl;
        }
    };
};
