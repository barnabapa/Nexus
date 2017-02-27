/*******************************************************************************************
 
			(c) Hash(BEGIN(Satoshi[2010]), END(Sunny[2012])) == Videlicet[2017] ++
			
			(c) Copyright Nexus Developers 2014 - 2017
			
			http://www.opensource.org/licenses/mit-license.php
  
*******************************************************************************************/

#ifndef NEXUS_CORE_INCLUDE_TRANSACTION_H
#define NEXUS_CORE_INCLUDE_TRANSACTION_H

#include <map>

class uint256;
class uint512;
class uint576;
class uint1024;

namespace Wallet
{ 
	class CScript;
}

namespace LLD 
{
	class CIndexDB;
}

namespace Core
{
	class CTxIndex;
	class CTransaction;
	
	
	enum GetMinFee_mode
	{
		GMF_BLOCK,
		GMF_RELAY,
		GMF_SEND,
	};
	
	
	typedef std::map<uint512, std::pair<CTxIndex, CTransaction> > MapPrevTx;

	
	/** An inpoint - a combination of a transaction and an index n into its vin */
	class CInPoint
	{
	public:
		CTransaction* ptx;
		unsigned int n;

		CInPoint() { SetNull(); }
		CInPoint(CTransaction* ptxIn, unsigned int nIn) { ptx = ptxIn; n = nIn; }
		void SetNull() { ptx = NULL; n = -1; }
		bool IsNull() const { return (ptx == NULL && n == -1); }
	};


	/** An outpoint - a combination of a transaction hash and an index n into its vout */
	class COutPoint
	{
	public:
		uint512 hash;
		unsigned int n;

		COutPoint() { SetNull(); }
		COutPoint(uint512 hashIn, unsigned int nIn) { hash = hashIn; n = nIn; }
		IMPLEMENT_SERIALIZE( READWRITE(FLATDATA(*this)); )
		void SetNull() { hash = 0; n = -1; }
		bool IsNull() const { return (hash == 0 && n == -1); }

		friend bool operator<(const COutPoint& a, const COutPoint& b)
		{
			return (a.hash < b.hash || (a.hash == b.hash && a.n < b.n));
		}

		friend bool operator==(const COutPoint& a, const COutPoint& b)
		{
			return (a.hash == b.hash && a.n == b.n);
		}

		friend bool operator!=(const COutPoint& a, const COutPoint& b)
		{
			return !(a == b);
		}

		std::string ToString() const
		{
			return strprintf("COutPoint(%s, %d)", hash.ToString().substr(0,10).c_str(), n);
		}

		void print() const
		{
			printf("%s\n", ToString().c_str());
		}
	};



	/** An input of a transaction.  It contains the location of the previous
	 * transaction's output that it claims and a signature that matches the
	 * output's public key.
	 */
	class CTxIn
	{
	public:
		COutPoint prevout;
		Wallet::CScript scriptSig;
		unsigned int nSequence;

		CTxIn()
		{
			nSequence = std::numeric_limits<unsigned int>::max();
		}

		explicit CTxIn(COutPoint prevoutIn, Wallet::CScript scriptSigIn=Wallet::CScript(), unsigned int nSequenceIn=std::numeric_limits<unsigned int>::max())
		{
			prevout = prevoutIn;
			scriptSig = scriptSigIn;
			nSequence = nSequenceIn;
		}

		CTxIn(uint512 hashPrevTx, unsigned int nOut, Wallet::CScript scriptSigIn=Wallet::CScript(), unsigned int nSequenceIn=std::numeric_limits<unsigned int>::max())
		{
			prevout = COutPoint(hashPrevTx, nOut);
			scriptSig = scriptSigIn;
			nSequence = nSequenceIn;
		}

		IMPLEMENT_SERIALIZE
		(
			READWRITE(prevout);
			READWRITE(scriptSig);
			READWRITE(nSequence);
		)

		bool IsFinal() const
		{
			return (nSequence == std::numeric_limits<unsigned int>::max());
		}
		
		bool IsStakeSig() const
		{
			if( scriptSig.size() != 8)
				return false;
				
			if( scriptSig[0] != 1 || scriptSig[1] != 2 || scriptSig[2] != 3 || scriptSig[3] != 5 || 
				scriptSig[4] != 8 || scriptSig[5] != 13 || scriptSig[6] != 21 || scriptSig[7] != 34)
				return false;
			
			return true;
		}

		friend bool operator==(const CTxIn& a, const CTxIn& b)
		{
			return (a.prevout   == b.prevout &&
					a.scriptSig == b.scriptSig &&
					a.nSequence == b.nSequence);
		}

		friend bool operator!=(const CTxIn& a, const CTxIn& b)
		{
			return !(a == b);
		}

		std::string ToStringShort() const
		{
			return strprintf(" %s %d", prevout.hash.ToString().c_str(), prevout.n);
		}

		std::string ToString() const
		{
			std::string str;
			str += "CTxIn(";
			str += prevout.ToString();
			if (prevout.IsNull())
			{	
				if(IsStakeSig())
					str += strprintf(", genesis %s", HexStr(scriptSig).c_str());
				else
					str += strprintf(", coinbase %s", HexStr(scriptSig).c_str());
			}
			else if(IsStakeSig())
				str += strprintf(", trust %s", HexStr(scriptSig).c_str());
			else
				str += strprintf(", scriptSig=%s", scriptSig.ToString().substr(0,24).c_str());
			if (nSequence != std::numeric_limits<unsigned int>::max())
				str += strprintf(", nSequence=%u", nSequence);
			str += ")";
			return str;
		}

		void print() const
		{
			printf("%s\n", ToString().c_str());
		}
	};



	/** An output of a transaction.  It contains the public key that the next input
	 * must be able to sign with to claim it.
	 */
	class CTxOut
	{
	public:
		int64 nValue;
		Wallet::CScript scriptPubKey;

		CTxOut()
		{
			SetNull();
		}

		CTxOut(int64 nValueIn, Wallet::CScript scriptPubKeyIn)
		{
			nValue = nValueIn;
			scriptPubKey = scriptPubKeyIn;
		}

		IMPLEMENT_SERIALIZE
		(
			READWRITE(nValue);
			READWRITE(scriptPubKey);
		)

		void SetNull()
		{
			nValue = -1;
			scriptPubKey.clear();
		}

		bool IsNull()
		{
			return (nValue == -1);
		}

		void SetEmpty()
		{
			nValue = 0;
			scriptPubKey.clear();
		}

		bool IsEmpty() const
		{
			return (nValue == 0 && scriptPubKey.empty());
		}

		uint512 GetHash() const
		{
			return SerializeHash(*this);
		}

		friend bool operator==(const CTxOut& a, const CTxOut& b)
		{
			return (a.nValue       == b.nValue &&
					a.scriptPubKey == b.scriptPubKey);
		}

		friend bool operator!=(const CTxOut& a, const CTxOut& b)
		{
			return !(a == b);
		}

		std::string ToStringShort() const
		{
			return strprintf(" out %s %s", FormatMoney(nValue).c_str(), scriptPubKey.ToString(true).c_str());
		}

		std::string ToString() const
		{
			if (IsEmpty()) return "CTxOut(empty)";
			if (scriptPubKey.size() < 6)
				return "CTxOut(error)";
			return strprintf("CTxOut(nValue=%s, scriptPubKey=%s)", FormatMoney(nValue).c_str(), scriptPubKey.ToString().c_str());
		}

		void print() const
		{
			printf("%s\n", ToString().c_str());
		}
	};
	
	
	/** The basic transaction that is broadcasted on the network and contained in
	 * blocks.  A transaction can contain multiple inputs and outputs.
	 */
	class CTransaction
	{
	public:
		int nVersion;
		unsigned int nTime;
		std::vector<CTxIn> vin;
		std::vector<CTxOut> vout;
		unsigned int nLockTime;

		CTransaction()
		{
			SetNull();
		}

		IMPLEMENT_SERIALIZE
		(
			READWRITE(this->nVersion);
			nVersion = this->nVersion;
			READWRITE(nTime);
			READWRITE(vin);
			READWRITE(vout);
			READWRITE(nLockTime);
		)

		void SetNull()
		{
			nVersion = 1;
			nTime = GetUnifiedTimestamp();
			vin.clear();
			vout.clear();
			nLockTime = 0;
			nDoS = 0;  // Denial-of-service prevention
		}

		bool IsNull() const
		{
			return (vin.empty() && vout.empty());
		}

		uint512 GetHash() const
		{
			return SerializeHash(*this);
		}

		bool IsFinal(int nBlockHeight=0, int64 nBlockTime=0) const
		{
			// Time based nLockTime implemented in 0.1.6
			if (nLockTime == 0)
				return true;
			if (nBlockHeight == 0)
				nBlockHeight = nBestHeight;
			if (nBlockTime == 0)
				nBlockTime = GetUnifiedTimestamp();
			if ((int64)nLockTime < ((int64)nLockTime < LOCKTIME_THRESHOLD ? (int64)nBlockHeight : nBlockTime))
				return true;
			BOOST_FOREACH(const CTxIn& txin, vin)
				if (!txin.IsFinal())
					return false;
			return true;
		}

		bool IsNewerThan(const CTransaction& old) const
		{
			if (vin.size() != old.vin.size())
				return false;
			for (unsigned int i = 0; i < vin.size(); i++)
				if (vin[i].prevout != old.vin[i].prevout)
					return false;

			bool fNewer = false;
			unsigned int nLowest = std::numeric_limits<unsigned int>::max();
			for (unsigned int i = 0; i < vin.size(); i++)
			{
				if (vin[i].nSequence != old.vin[i].nSequence)
				{
					if (vin[i].nSequence <= nLowest)
					{
						fNewer = false;
						nLowest = vin[i].nSequence;
					}
					if (old.vin[i].nSequence < nLowest)
					{
						fNewer = true;
						nLowest = old.vin[i].nSequence;
					}
				}
			}
			return fNewer;
		}

		/** Coinbase Transaction Rules: **/
		bool IsCoinBase() const
		{
			/** A] Input Size must be 1. **/
			if(vin.size() != 1)
				return false;
				
			/** B] First Input must be Empty. **/
			if(!vin[0].prevout.IsNull())
				return false;
				
			/** C] Outputs Count must Exceed 1. **/
			if(vout.size() < 1)
				return false;
				
			return true;
		}

		/** Coinstake Transaction Rules: **/
		bool IsCoinStake() const
		{
			/** A] Must have more than one Input. **/
			if(vin.size() <= 1)
				return false;
				
			/** B] First Input Script Signature must be 8 Bytes. **/
			if(vin[0].scriptSig.size() != 8)
				return false;
				
			/** C] First Input Script Signature must Contain Fibanacci Byte Series. **/
			if(!vin[0].IsStakeSig())
				return false;
				
			/** D] All Remaining Previous Inputs must not be Empty. **/
			for(int nIndex = 1; nIndex < vin.size(); nIndex++)
				if(vin[nIndex].prevout.IsNull())
					return false;
				
			/** E] Must Contain only 1 Outputs. **/
			if(vout.size() != 1)
				return false;
				
			return true;
		}
		
		/** Flag to determine if the transaction is a genesis transaction. **/
		bool IsGenesis() const
		{
			/** A] Genesis Transaction must be Coin Stake. **/
			if(!IsCoinStake())
				return false;
				
			/** B] First Input Previous Transaction must be Empty. **/
			if(!vin[0].prevout.IsNull())
				return false;
				
			return true;
		}
		
		
		/** Flag to determine if the transaction is a Trust Transaction. **/
		bool IsTrust() const
		{
			/** A] Genesis Transaction must be Coin Stake. **/
			if(!IsCoinStake())
				return false;
				
			/** B] First Input Previous Transaction must not be Empty. **/
			if(vin[0].prevout.IsNull())
				return false;
				
			/** C] First Input Previous Transaction Hash must not be 0. **/
			if(vin[0].prevout.hash == 0)
				return false;
				
			/** D] First Input Previous Transaction Index must be 0. **/
			if(vin[0].prevout.n != 0)
				return false;
				
			return true;
		}

		/** Check for standard transaction types
			@return True if all outputs (scriptPubKeys) use only standard transaction forms
		*/
		bool IsStandard() const;

		/** Check for standard transaction types
			@param[in] mapInputs	Map of previous transactions that have outputs we're spending
			@return True if all inputs (scriptSigs) use only standard transaction forms
			@see CTransaction::FetchInputs
		*/
		bool AreInputsStandard(const MapPrevTx& mapInputs) const;

		/** Count ECDSA signature operations the old-fashioned (pre-0.6) way
			@return number of sigops this transaction's outputs will produce when spent
			@see CTransaction::FetchInputs
		*/
		unsigned int GetLegacySigOpCount() const;

		/** Count ECDSA signature operations in pay-to-script-hash inputs.

			@param[in] mapInputs	Map of previous transactions that have outputs we're spending
			@return maximum number of sigops required to validate this transaction's inputs
			@see CTransaction::FetchInputs
		 */
		unsigned int TotalSigOps(const MapPrevTx& mapInputs) const;

		/** Amount of Coins spent by this transaction.
			@return sum of all outputs (note: does not include fees)
		 */
		int64 GetValueOut() const
		{
			int64 nValueOut = 0;
			BOOST_FOREACH(const CTxOut& txout, vout)
			{
				nValueOut += txout.nValue;
				if (!MoneyRange(txout.nValue) || !MoneyRange(nValueOut))
					throw std::runtime_error("CTransaction::GetValueOut() : value out of range");
			}
			return nValueOut;
		}

		/** Amount of Coins coming in to this transaction
			Note that lightweight clients may not know anything besides the hash of previous transactions,
			so may not be able to calculate this.

			@param[in] mapInputs	Map of previous transactions that have outputs we're spending
			@return	Sum of value of all inputs (scriptSigs)
			@see CTransaction::FetchInputs
		 */
		int64 GetValueIn(const MapPrevTx& mapInputs) const;

		static bool AllowFree(double dPriority)
		{
			// Large (in bytes) low-priority (new, small-coin) transactions
			// need a fee.
			return dPriority > COIN * 144 / 250;
		}

		int64 GetMinFee(unsigned int nBlockSize=1, bool fAllowFree=false, enum GetMinFee_mode mode=GMF_BLOCK) const
		{
			if(nVersion >= 2)
				return 0;
				
			// Base fee is either MIN_TX_FEE or MIN_RELAY_TX_FEE
			int64 nBaseFee = (mode == GMF_RELAY) ? MIN_RELAY_TX_FEE : MIN_TX_FEE;

			unsigned int nBytes = ::GetSerializeSize(*this, SER_NETWORK, PROTOCOL_VERSION);
			unsigned int nNewBlockSize = nBlockSize + nBytes;
			int64 nMinFee = (1 + (int64)nBytes / 1000) * nBaseFee;

			if (fAllowFree)
			{
				if (nBlockSize == 1)
				{
					// Transactions under 10K are free
					// (about 4500bc if made of 50bc inputs)
					if (nBytes < 10000)
						nMinFee = 0;
				}
				else
				{
					// Free transaction area
					if (nNewBlockSize < 27000)
						nMinFee = 0;
				}
			}

			// To limit dust spam, require MIN_TX_FEE/MIN_RELAY_TX_FEE if any output is less than 0.01
			if (nMinFee < nBaseFee)
			{
				BOOST_FOREACH(const CTxOut& txout, vout)
					if (txout.nValue < CENT)
						nMinFee = nBaseFee;
			}

			// Raise the price as the block approaches full
			if (nBlockSize != 1 && nNewBlockSize >= MAX_BLOCK_SIZE_GEN/2)
			{
				if (nNewBlockSize >= MAX_BLOCK_SIZE_GEN)
					return MAX_TXOUT_AMOUNT;
				nMinFee *= MAX_BLOCK_SIZE_GEN / (MAX_BLOCK_SIZE_GEN - nNewBlockSize);
			}

			if (!MoneyRange(nMinFee))
				nMinFee = MAX_TXOUT_AMOUNT;
				
			return nMinFee;
		}


		bool ReadFromDisk(CDiskTxPos pos, FILE** pfileRet=NULL)
		{
			CAutoFile filein = CAutoFile(OpenBlockFile(pos.nFile, 0, pfileRet ? "rb+" : "rb"), SER_DISK, DATABASE_VERSION);
			if (!filein)
				return error("CTransaction::ReadFromDisk() : OpenBlockFile failed");

			// Read transaction
			if (fseek(filein, pos.nTxPos, SEEK_SET) != 0)
				return error("CTransaction::ReadFromDisk() : fseek failed");

			try {
				filein >> *this;
			}
			catch (std::exception &e) {
				return error("%s() : deserialize or I/O error", __PRETTY_FUNCTION__);
			}

			// Return file pointer
			if (pfileRet)
			{
				if (fseek(filein, pos.nTxPos, SEEK_SET) != 0)
					return error("CTransaction::ReadFromDisk() : second fseek failed");
				*pfileRet = filein.release();
			}
			return true;
		}

		friend bool operator==(const CTransaction& a, const CTransaction& b)
		{
			return (a.nVersion  == b.nVersion &&
					a.nTime     == b.nTime &&
					a.vin       == b.vin &&
					a.vout      == b.vout &&
					a.nLockTime == b.nLockTime);
		}

		friend bool operator!=(const CTransaction& a, const CTransaction& b)
		{
			return !(a == b);
		}


		std::string ToStringShort() const
		{
			std::string str;
			str += strprintf("%s %s", GetHash().ToString().c_str(), IsCoinBase()? "base" : (IsCoinStake()? "stake" : "user"));
			return str;
		}

		std::string ToString() const
		{
			std::string str;
			str += IsCoinBase() ? "Coinbase" : (IsGenesis() ? "Genesis" : (IsTrust() ? "Trust" : "Transaction"));
			str += strprintf("(hash=%s, nTime=%d, ver=%d, vin.size=%d, vout.size=%d, nLockTime=%d)\n",
				GetHash().ToString().substr(0,10).c_str(),
				nTime,
				nVersion,
				vin.size(),
				vout.size(),
				nLockTime);
			for (unsigned int i = 0; i < vin.size(); i++)
				str += "    " + vin[i].ToString() + "\n";
			for (unsigned int i = 0; i < vout.size(); i++)
				str += "    " + vout[i].ToString() + "\n";
			return str;
		}

		void print() const
		{
			printf("%s", ToString().c_str());
		}


		bool GetCoinstakeInterest(LLD::CIndexDB& txdb, int64& nInterest) const;
		bool GetCoinstakeAge(LLD::CIndexDB& txdb, uint64& nAge) const;

		
		bool ReadFromDisk(LLD::CIndexDB& indexdb, COutPoint prevout, CTxIndex& txindexRet);
		bool ReadFromDisk(LLD::CIndexDB& indexdb, COutPoint prevout);
		bool ReadFromDisk(COutPoint prevout);
		bool DisconnectInputs(LLD::CIndexDB& indexdb);

		/** Fetch from memory and/or disk. inputsRet keys are transaction hashes.

		 @param[in] txdb	Transaction database
		 @param[in] mapTestPool	List of pending changes to the transaction index database
		 @param[in] fBlock	True if being called to add a new best-block to the chain
		 @param[in] fMiner	True if being called by CreateNewBlock
		 @param[out] inputsRet	Pointers to this transaction's inputs
		 @param[out] fInvalid	returns true if transaction is invalid
		 @return	Returns true if all inputs are in txdb or mapTestPool
		 */
		bool FetchInputs(LLD::CIndexDB& indexdb, const std::map<uint512, CTxIndex>& mapTestPool,
						 bool fBlock, bool fMiner, MapPrevTx& inputsRet, bool& fInvalid);

		/** Sanity check previous transactions, then, if all checks succeed,
			mark them as spent by this transaction.

			@param[in] inputs	Previous transactions (from FetchInputs)
			@param[out] mapTestPool	Keeps track of inputs that need to be updated on disk
			@param[in] posThisTx	Position of this transaction on disk
			@param[in] pindexBlock
			@param[in] fBlock	true if called from ConnectBlock
			@param[in] fMiner	true if called from CreateNewBlock
			@param[in] fStrictPayToScriptHash	true if fully validating p2sh transactions
			@return Returns true if all checks succeed
		 */
		bool ConnectInputs(LLD::CIndexDB& indexdb, MapPrevTx inputs,
						   std::map<uint512, CTxIndex>& mapTestPool, const CDiskTxPos& posThisTx,
						   const CBlockIndex* pindexBlock, bool fBlock, bool fMiner);
		bool ClientConnectInputs();
		bool CheckTransaction() const;
		bool AcceptToMemoryPool(LLD::CIndexDB& indexdb, bool fCheckInputs=true, bool* pfMissingInputs=NULL);
	

	protected:
		const CTxOut& GetOutputFor(const CTxIn& input, const MapPrevTx& inputs) const;
	};
	
	
	/** A transaction with a merkle branch linking it to the block chain. */
	class CMerkleTx : public CTransaction
	{
	public:
		uint1024 hashBlock;
		std::vector<uint512> vMerkleBranch;
		int nIndex;

		// memory only
		mutable bool fMerkleVerified;


		CMerkleTx()
		{
			Init();
		}

		CMerkleTx(const CTransaction& txIn) : CTransaction(txIn)
		{
			Init();
		}

		void Init()
		{
			hashBlock = 0;
			nIndex = -1;
			fMerkleVerified = false;
		}


		IMPLEMENT_SERIALIZE
		(
			nSerSize += SerReadWrite(s, *(CTransaction*)this, nType, nVersion, ser_action);
			nVersion = this->nVersion;
			READWRITE(hashBlock);
			READWRITE(vMerkleBranch);
			READWRITE(nIndex);
		)


		int SetMerkleBranch(const CBlock* pblock=NULL);
		int GetDepthInMainChain(CBlockIndex* &pindexRet) const;
		int GetDepthInMainChain() const { CBlockIndex *pindexRet; return GetDepthInMainChain(pindexRet); }
		bool IsInMainChain() const { return GetDepthInMainChain() > 0; }
		int GetBlocksToMaturity() const;
		bool AcceptToMemoryPool(LLD::CIndexDB& indexdb, bool fCheckInputs=true);
		bool AcceptToMemoryPool();
	};
	
	
	
	/** Position on disk for a particular transaction. **/
	class CDiskTxPos
	{
	public:
		unsigned int nFile;
		unsigned int nBlockPos;
		unsigned int nTxPos;

		CDiskTxPos()
		{
			SetNull();
		}

		CDiskTxPos(unsigned int nFileIn, unsigned int nBlockPosIn, unsigned int nTxPosIn)
		{
			nFile = nFileIn;
			nBlockPos = nBlockPosIn;
			nTxPos = nTxPosIn;
		}

		IMPLEMENT_SERIALIZE( READWRITE(FLATDATA(*this)); )
		void SetNull() { nFile = -1; nBlockPos = 0; nTxPos = 0; }
		bool IsNull() const { return (nFile == -1); }

		friend bool operator==(const CDiskTxPos& a, const CDiskTxPos& b)
		{
			return (a.nFile     == b.nFile &&
					a.nBlockPos == b.nBlockPos &&
					a.nTxPos    == b.nTxPos);
		}

		friend bool operator!=(const CDiskTxPos& a, const CDiskTxPos& b)
		{
			return !(a == b);
		}

		std::string ToString() const
		{
			if (IsNull())
				return "null";
			else
				return strprintf("(nFile=%d, nBlockPos=%d, nTxPos=%d)", nFile, nBlockPos, nTxPos);
		}

		void print() const
		{
			printf("%s", ToString().c_str());
		}
	};


	/**  A txdb record that contains the disk location of a transaction and the
	 * locations of transactions that spend its outputs.  vSpent is really only
	 * used as a flag, but having the location is very helpful for debugging.
	 */
	class CTxIndex
	{
	public:
		CDiskTxPos pos;
		std::vector<CDiskTxPos> vSpent;

		CTxIndex()
		{
			SetNull();
		}

		CTxIndex(const CDiskTxPos& posIn, unsigned int nOutputs)
		{
			pos = posIn;
			vSpent.resize(nOutputs);
		}

		IMPLEMENT_SERIALIZE
		(
			if (!(nType & SER_GETHASH))
				READWRITE(nVersion);
			READWRITE(pos);
			READWRITE(vSpent);
		)

		void SetNull();
		bool IsNull();
		
		int GetDepthInMainChain() const;

		friend bool operator==(const CTxIndex& a, const CTxIndex& b)
		{
			return (a.pos    == b.pos &&
					a.vSpent == b.vSpent);
		}

		friend bool operator!=(const CTxIndex& a, const CTxIndex& b)
		{
			return !(a == b);
		}
	};
	
	
	class CTxMemPool
	{
	public:
		mutable CCriticalSection cs;
		
		std::map<uint512, CTransaction> mapTx;
		std::map<COutPoint, CInPoint> mapNextTx;

		bool accept(LLD::CIndexDB& indexdb, CTransaction &tx,
					bool fCheckInputs, bool* pfMissingInputs);
		bool addUnchecked(CTransaction &tx);
		bool remove(CTransaction &tx);
		void queryHashes(std::vector<uint512>& vtxid);

		unsigned long size()
		{
			LOCK(cs);
			return mapTx.size();
		}

		bool exists(uint512 hash)
		{
			return (mapTx.count(hash) != 0);
		}

		CTransaction& lookup(uint512 hash)
		{
			return mapTx[hash];
		}
	};


	
	/* The global Memory Pool to Hold New Transactions. */
	extern CTxMemPool mempool;
	
	
	
	/* __________________________________________________ (Transaction Methods) __________________________________________________  */
	
	
	
	
	/* Add an oprhan transaction to the Orphans Memory Map. */
	bool AddOrphanTx(const CDataStream& vMsg);
	
	
	/* Remove an Orphaned Transaction from the Memory Mao. */
	void EraseOrphanTx(uint512 hash);
	
	
	/* Set the Limit of the Orphan Transaction Sizes Dynamically. */
	unsigned int LimitOrphanTxSize(unsigned int nMaxOrphans);
	
	
	/* Get a specific transaction from the Database or from a block's binary position. */
	bool GetTransaction(const uint512 &hash, CTransaction &tx, uint1024 &hashBlock);
	
}


#endif
