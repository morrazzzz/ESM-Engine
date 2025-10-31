#pragma once

#include <bitset>

class CProcessorID
{
    xr_string vendorName;
    xr_string cpuName;
    std::bitset<32> eax1ECX;
    std::bitset<32> eax1EDX;
public:
    CProcessorID() = default;
    ~CProcessorID() = default;

    void InitInfoCPU();
    void MessageInfoCPU();

    inline bool SupportSSE() const { return eax1EDX[25]; }
    inline bool SupportSSE2() const { return eax1EDX[26]; }
    inline bool SupportSSE3() const { return eax1ECX[0]; }
    inline bool SupportSSE41() const { return eax1ECX[19]; }
    inline bool SupportSSE42() const { return eax1ECX[20]; }
};

extern XRCORE_API CProcessorID cpuID;