#pragma once

#include <string>
#include "nuto/base/Logger.h"

#ifdef SHOW_TIME
#include <iostream>
#include <sstream>
#include <chrono>
#include <algorithm>
#include <iomanip>

#ifdef _OPENMP
#include <ctime>
#endif // _OPENMP

#endif // SHOW_TIME

namespace NuTo
{

//! @brief prints the lifetime of a Timer object on destruction
class Timer
{
public:
    //! @brief ctor, saves the current time
    //! @param rMsg ... msg to print on destruction
    //! @param rShowTime ... false: no output
    Timer(std::string rMsg, bool rShowTime = true)
#ifdef SHOW_TIME
    : mMsg(rMsg),
      mShowTime(rShowTime),
      mLogger(nullptr),
#ifdef _OPENMP
      mCPUTimeInit(clock()),
#endif // _OPENMP
      mWallTimeInit(std::chrono::system_clock::now())
#endif // SHOW_TIME
    {}

    //! @brief ctor, saves the current time
    //! @param rMsg ... msg to print on destruction
    //! @param rShowTime ... false: no output
    Timer(std::string rMsg, bool rShowTime, Logger& rLogger)
#ifdef SHOW_TIME
    : mMsg(rMsg),
      mShowTime(rShowTime),
      mLogger(&rLogger),
#ifdef _OPENMP
      mCPUTimeInit(clock()),
#endif // _OPENMP
      mWallTimeInit(std::chrono::system_clock::now())
#endif // SHOW_TIME
    {}
#ifdef SHOW_TIME
    //! @brief dtor, prints the msg and the lifetime
    ~Timer()
    {
        Reset();
    }
#endif

    void Reset()
    {
#ifdef SHOW_TIME
        if (mShowTime)
        {
#ifdef _OPENMP
            double cpuTimeDifference = GetCPUTimeDifference();
#endif // _OPENMP
            double wallTimeDifference = GetTimeDifference();


            int numAdditionalBlanks = std::max(0, mMinMsgLength - (int)mMsg.length());
            const std::string& additionalBlanks = std::string(numAdditionalBlanks, '.');

            std::ostringstream out;
            out << "[" << mMsg << "] " << additionalBlanks << "W:" << std::scientific << std::setprecision(2) << wallTimeDifference << "s";


#ifdef _OPENMP
            out << "  C:" << cpuTimeDifference << "s";
            out << "  S:" << std::fixed << cpuTimeDifference / wallTimeDifference;
#endif // _OPENMP
            out << "\n";

            if (mLogger == nullptr)
                std::cout << out.str();
            else
                *mLogger << out.str();

        }
        mWallTimeInit = std::chrono::system_clock::now();
#ifdef _OPENMP
        mCPUTimeInit = clock();
#endif // _OPENMP
#endif
    }

    void Reset(std::string rMsg)
    {
#ifdef SHOW_TIME
        Reset();
        mMsg = rMsg;
#endif
    }

    //! @brief returns the time from ctor to now in seconds
    double GetTimeDifference() const
    {
#ifdef SHOW_TIME
        const auto timeDiff = std::chrono::system_clock::now() - mWallTimeInit;
        return std::chrono::duration_cast<std::chrono::duration<double>>(timeDiff).count();
#else
        return 0.;
#endif // SHOW_TIME
    }

#ifdef _OPENMP
    //! @brief returns the time from ctor to now in seconds
    double GetCPUTimeDifference() const
    {
#ifdef SHOW_TIME
        return difftime(clock(), mCPUTimeInit) / CLOCKS_PER_SEC;
#else
        return 0.;
#endif // SHOW_TIME
    }
#endif // _OPENMP

#ifdef SHOW_TIME
    std::string mMsg;
    bool mShowTime;
    Logger* mLogger;

#ifdef _OPENMP
    double mCPUTimeInit;
#endif // _OPENMP

    std::chrono::time_point<std::chrono::system_clock> mWallTimeInit;

    const int mMinMsgLength = 75;

#endif // SHOW_TIME
};

}