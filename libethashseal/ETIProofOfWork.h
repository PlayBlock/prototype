#pragma once
/// Proof of work definition for ETI
#include <libdevcrypto/Common.h>
#include <libproducer/types.hpp>
#include <libdevcore/Guards.h>
#include <libevm/Vote.h>

using namespace dev;

struct ETIProofOfWork
{
	struct Solution
	{
		POW_Operation op;
	};

	struct WorkPackage
	{
		WorkPackage() {}
		WorkPackage(WorkPackage const& _other);
		WorkPackage(h256 _blockId, Secret _privateKey, types::AccountName _workerAccount, uint64_t _nonce, dev::h256 _target): 
			blockId(_blockId), privateKey(_privateKey), workerAccount(_workerAccount), target(_target), nonce(_nonce) {}

		WorkPackage& operator=(WorkPackage const& _other);

		void reset() { Guard l(m_blockIdLock); blockId = h256(); }
		operator bool() const { Guard l(m_blockIdLock); return blockId != h256(); }
		h256 headerHash() const { Guard l(m_blockIdLock); return blockId; }


		h256 blockId;
		Secret privateKey;
		types::AccountName workerAccount;
		dev::h256 target;
		uint64_t nonce;

	private:
		mutable Mutex m_blockIdLock;
	};
};