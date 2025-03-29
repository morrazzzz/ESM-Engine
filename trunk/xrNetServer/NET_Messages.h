#pragma once
#pragma pack(push,1)

#define	DPNSEND_IMMEDIATELLY				0x0100

IC u32	net_flags	(BOOL bReliable=FALSE, BOOL bSequental=TRUE, BOOL bHighPriority=FALSE, 
					 BOOL bSendImmediatelly = FALSE)
{
	return 
		(bReliable?DPNSEND_GUARANTEED:DPNSEND_NOCOMPLETE) | 
		(bSequental?0:DPNSEND_NONSEQUENTIAL) | 
		(bHighPriority?DPNSEND_PRIORITY_HIGH:0) |
		(bSendImmediatelly?DPNSEND_IMMEDIATELLY:0)
		;
}

#pragma pack(pop)
