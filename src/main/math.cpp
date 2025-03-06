#include "math.h"

#include <cmath>

namespace fineLanding
{
    // Calculate direction angle from point1 to point2
    double Math::getDirectionAngle(double lat1, double lon1, double lat2, double lon2)
    {
        double φ1 = lat1 * M_PI / 180;
        double φ2 = lat2 * M_PI / 180;
        double dLon = (lon2 - lon1) * M_PI / 180;
        double y = std::sin(dLon) * std::cos(lat2);
        double x = std::cos(lat1) * std::sin(lat2) -
                   std::sin(lat1) * std::cos(lat2) * std::cos(dLon);
        return std::atan2(y, x);
    }

    double Math::getDistance(double lat1, double lon1, double lat2, double lon2)
    {
        double r = 6371000; // meters
        double pd = M_PI / 180;

        double a = 0.5 - std::cos((lat2 - lat1) * pd) / 2 + std::cos(lat1 * pd) * std::cos(lat2 * pd) * (1 - std::cos((lon2 - lon1) * pd)) / 2;

        return 2 * r * std::asin(std::sqrt(a));
    }
} // namespace fineLanding