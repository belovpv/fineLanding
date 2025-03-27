#include "math.h"

#include <cmath>

namespace fineLanding
{
    // Calculate direction angle in degrees from point1 to point2
    double Math::getDirectionAngle(double lat1, double lon1, double lat2, double lon2)
    {
        double φ1 = lat1 * M_PI / 180;
        double φ2 = lat2 * M_PI / 180;
        double dLon = (lon2 - lon1) * M_PI / 180;
        double y = std::sin(dLon) * std::cos(φ2);
        double x = std::cos(φ1) * std::sin(φ2) -
                   std::sin(φ1) * std::cos(φ2) * std::cos(dLon);
        double ret = std::atan2(y, x);
        if (ret < 0) {
            ret = M_PI * 2 + ret;
        }
        return ret * 180 / M_PI;
    }

    double Math::getDistance(double lat1, double lon1, double lat2, double lon2)
    {
        double r = 6371000; // meters
        double pd = M_PI / 180;

        double a = 0.5 - std::cos((lat2 - lat1) * pd) / 2 + std::cos(lat1 * pd) * std::cos(lat2 * pd) * (1 - std::cos((lon2 - lon1) * pd)) / 2;

        return 2 * r * std::asin(std::sqrt(a));
    }
} // namespace fineLanding