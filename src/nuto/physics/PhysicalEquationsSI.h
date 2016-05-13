#pragma once

#include "nuto/base/Exception.h"

namespace NuTo
{
namespace SI
{


    //! @brief ... Density of liquid water at standard atmospheric pressure depending on the temperature. Values linear interpolated from tables.
    //! @param rTemperature ... temperature in [K]
    //! @return density of liquid water
    //! SOURCE: https://en.wikipedia.org/wiki/Density
    constexpr double DensityLiquidWater(double rTemperature)
    {
                return (rTemperature<  243.15 ) ? (throw std::logic_error("Value not defined for temperatures < 243.15")) :
                       (rTemperature<= 253.15 ) ? 983.854  + (rTemperature - 243.15) * (993.547  - 983.854 ) / (10.0)  :
                       (rTemperature<= 263.15 ) ? 993.547  + (rTemperature - 253.15) * (998.117  - 993.547 ) / (10.0)  :
                       (rTemperature<= 273.15 ) ? 998.117  + (rTemperature - 263.15) * (999.8395 - 998.117 ) / (10.0)  :
                       (rTemperature<= 277.15 ) ? 999.8395 + (rTemperature - 273.15) * (999.9720 - 999.8395) / ( 4.0)  :
                       (rTemperature<= 283.15 ) ? 999.9720 + (rTemperature - 277.15) * (999.7026 - 999.9720) / ( 6.0)  :
                       (rTemperature<= 288.15 ) ? 999.7026 + (rTemperature - 283.15) * (999.1026 - 999.7026) / ( 5.0)  :
                       (rTemperature<= 293.15 ) ? 999.1026 + (rTemperature - 288.15) * (998.2071 - 999.1026) / ( 5.0)  :
                       (rTemperature<= 295.15 ) ? 998.2071 + (rTemperature - 293.15) * (997.7735 - 998.2071) / ( 2.0)  :
                       (rTemperature<= 298.15 ) ? 997.7735 + (rTemperature - 295.15) * (997.0479 - 997.7735) / ( 3.0)  :
                       (rTemperature<= 303.15 ) ? 997.0479 + (rTemperature - 298.15) * (995.6502 - 997.0479) / ( 5.0)  :
                       (rTemperature<= 313.15 ) ? 995.6502 + (rTemperature - 303.15) * (992.2    - 995.6502) / (10.0)  :
                       (rTemperature<= 333.15 ) ? 992.2    + (rTemperature - 313.15) * (983.2    - 992.2   ) / (20.0)  :
                       (rTemperature<= 353.15 ) ? 983.2    + (rTemperature - 333.15) * (971.8    - 983.2   ) / (20.0)  :
                       (rTemperature>  373.15 ) ? (throw std::logic_error("Value not defined for temperatures > 373.15")) :
                                                  971.8    + (rTemperature - 353.15) * (958.4    - 971.8   ) / (20.0)  ;

    }
} //namespace SI
} //namespace NuTo
