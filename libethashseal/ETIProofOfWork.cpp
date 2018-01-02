#include "ETIProofOfWork.h"

using namespace dev;

ETIProofOfWork::WorkPackage::WorkPackage(WorkPackage const& _other) :
	blockId(_other.headerHash()),
	privateKey(_other.privateKey),
	workerAccount(_other.workerAccount),
	target(_other.target),
	nonce(_other.nonce)
{}

ETIProofOfWork::WorkPackage& ETIProofOfWork::WorkPackage::operator=(ETIProofOfWork::WorkPackage const& _other)
{
	if (this == &_other)
		return *this;

	privateKey = _other.privateKey;
	workerAccount = _other.workerAccount;
	nonce = _other.nonce;
	target = _other.target;
	
	
	blockId = _other.blockId;

	h256 headerHash = _other.headerHash();
	{
		Guard l(m_blockIdLock);
		blockId = std::move(headerHash);
	}
	return *this;
}