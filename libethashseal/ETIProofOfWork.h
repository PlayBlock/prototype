#pragma once
/// Proof of work definition for ETI
#include <libdevcrypto/Common.h>
#include <libproducer/types.hpp>

using namespace dev;

struct ETIProofOfWork
{
	struct WorkPackage
	{
		h256 blockId;
		Secret privateKey;
		types::AccountName workerAccount;
		uint64_t nonce;
	};
};