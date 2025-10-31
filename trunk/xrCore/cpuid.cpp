#include "stdafx.h"
#include "cpuid.h"

XRCORE_API CProcessorID cpuID;

void CProcessorID::InitInfoCPU()
{
    int cpuidInstruction[4]{};

    char vendorTempName[sizeof(int) * 3 + 1]{};
    __cpuid(cpuidInstruction, 0);

    *reinterpret_cast<int*>(vendorTempName) = cpuidInstruction[1];
    *reinterpret_cast<int*>(vendorTempName + 4) = cpuidInstruction[3];
    *reinterpret_cast<int*>(vendorTempName + 8) = cpuidInstruction[2];

    vendorName = vendorTempName;

    __cpuid(cpuidInstruction, 1);
    eax1ECX = cpuidInstruction[2];
    eax1EDX = cpuidInstruction[3];
    
    __cpuid(cpuidInstruction, 0x8000'000'0);

    if (cpuidInstruction[0] < 0x8000'000'2)
        return;

    for (int i = 0x8'000'000'2; i <= 0x8'000'000'4; i++)
    {
        __cpuid(cpuidInstruction, i);
        std::string_view cpuTempName(reinterpret_cast<char*>(cpuidInstruction), sizeof cpuidInstruction);
        cpuName += cpuTempName;
    }
}

void CProcessorID::MessageInfoCPU()
{
    xr_string supportInstruction{};

    if (SupportSSE())
        supportInstruction += "SSE";

    if (SupportSSE2())
        supportInstruction += ", SSE2";

    if (SupportSSE3())
        supportInstruction += ", SSE3";

    if (SupportSSE41())
        supportInstruction += ", SSE 4.1";

    if (SupportSSE42())
        supportInstruction += ", SSE 4.2";

    Msg("* CPU Info: Vendor: [%s], CPU Name: [%s], CPU Instructions: [%s]", vendorName.c_str(), cpuName.c_str(), supportInstruction.c_str());
}