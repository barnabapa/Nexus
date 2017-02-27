/*******************************************************************************************
 
			(c) Hash(BEGIN(Satoshi[2010]), END(Sunny[2012])) == Videlicet[2017] ++
			
			(c) Copyright Nexus Developers 2014 - 2017
			
			http://www.opensource.org/licenses/mit-license.php
  
*******************************************************************************************/

#ifndef NEXUS_CORE_INCLUDE_SUPPLY_H
#define NEXUS_CORE_INCLUDE_SUPPLY_H

#include <stdint.h>

#if defined(MAC_OSX) || defined(WIN32)
typedef int64_t int64;
typedef uint64_t uint64;
#else
typedef long long  int64;
typedef unsigned long long  uint64;
#endif

namespace Core
{
	class CBlockIndex;
	
	
	/* Get the Total Amount to be Released at a given Minute since the NETWORK_TIMELOCK. */
	int64 GetSubsidy(int nMinutes, int nType);
	
	
	/* Calculate the Compounded amount of NXS to be released over the (nInterval) minutes. */
	int64 CompoundSubsidy(int nMinutes, int nInterval);
	
	
	/* Calculate the Compounded amount of NXS that should "ideally" have been created to this minute. */
	int64 CompoundSubsidy(int nMinutes);
	
	
	/* Get the total supply of NXS in the chain from the Index Objects that is calculated as chain is built. */
	int64 GetMoneySupply(CBlockIndex* pindex);
	
	
	/* Get the age of the Nexus Blockchain in a figure of Seconds. */
	int64 GetChainAge(int64 nTime);
	
	
	/* Get the Fractional Reward basing the total amount on a number of minutes vs a total reward. */
	int64 GetFractionalSubsidy(int nMinutes, int nType, double nFraction);
	
	
	/* Get the Coinbase Rewards based on the Reserve Balances to keep the Coinbase rewards under the Reserve Production Rates. */
	int64 GetCoinbaseReward(const CBlockIndex* pindex, int nChannel, int nType);
	
	
	/* Release a certain amount of Nexus into the Reserve System at a given Minute of time. */
	int64 ReleaseRewards(int nTimespan, int nStart, int nType);
	
	
	/* Get the total amount released into this given reserve by this point in time in the Block Index Object. */
	int64 GetReleasedReserve(const CBlockIndex* pindex, int nChannel, int nType);
	
	
	/* Check if there is any Nexus that will be released on the Next block in case the reserve values have been severely depleted. */
	bool  ReleaseAvailable(const CBlockIndex* pindex, int nChannel);
	
}

#endif


