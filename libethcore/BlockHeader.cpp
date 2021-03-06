﻿/*
	This file is part of cpp-ethereum.

	cpp-ethereum is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	cpp-ethereum is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>.
*/
/** @file BlockHeader.cpp
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 */

#include <libdevcore/Common.h>
#include <libdevcore/Log.h>
#include <libdevcore/RLP.h>
#include <libdevcore/TrieDB.h>
#include <libdevcore/MemoryDB.h>
#include <libdevcore/TrieHash.h>
#include <libethcore/Common.h>

#include "Exceptions.h"
#include "BlockHeader.h"
using namespace std;
using namespace dev;
using namespace dev::eth;


BlockHeader::BlockHeader()
{
}

BlockHeader::BlockHeader(bytesConstRef _block, BlockDataType _bdt, h256 const& _hashWith)
{
	RLP header = _bdt == BlockData ? extractHeader(_block) : RLP(_block);
	m_hash = _hashWith ? _hashWith : sha3(header.data());
	populate(header);
}

BlockHeader::BlockHeader(BlockHeader const& _other) :
	m_parentHash(_other.parentHash()),
	m_sha3Uncles(_other.sha3Uncles()),
	m_stateRoot(_other.stateRoot()),
	m_transactionsRoot(_other.transactionsRoot()),
	m_receiptsRoot(_other.receiptsRoot()),
	m_logBloom(_other.logBloom()),
	m_number(_other.number()),
	m_gasLimit(_other.gasLimit()),
	m_gasUsed(_other.gasUsed()),
	m_extraData(_other.extraData()),
	m_timestamp(_other.timestamp()),
	m_author(_other.author()),
	m_difficulty(_other.difficulty()),
	m_seal(_other.seal()),
	m_hash(_other.hashRawRead()),
	m_hashWithout(_other.hashWithoutRawRead()),
	m_signature(_other.signature()),
	m_running_ver(_other.runningVersion()),
	m_hardfork_vote(_other.hardforkVote())
{
	assert(*this == _other);
}

BlockHeader& BlockHeader::operator=(BlockHeader const& _other)
{
	if (this == &_other)
		return *this;
	m_parentHash = _other.parentHash();
	m_sha3Uncles = _other.sha3Uncles();
	m_stateRoot = _other.stateRoot();
	m_transactionsRoot = _other.transactionsRoot();
	m_receiptsRoot = _other.receiptsRoot();
	m_logBloom = _other.logBloom();
	m_number = _other.number();
	m_gasLimit = _other.gasLimit();
	m_gasUsed = _other.gasUsed();
	m_extraData = _other.extraData();
	m_timestamp = _other.timestamp();
	m_author = _other.author();
	m_difficulty = _other.difficulty();
	m_signature = _other.signature();
	m_running_ver = _other.runningVersion();
	m_hardfork_vote = _other.hardforkVote();

	std::vector<bytes> seal = _other.seal();
	{
		Guard l(m_sealLock);
		m_seal = std::move(seal);
	}
	h256 hash = _other.hashRawRead();
	h256 hashWithout = _other.hashWithoutRawRead();
	{
		Guard l(m_hashLock);
		m_hash = std::move(hash);
		m_hashWithout = std::move(hashWithout);
	}
	assert(*this == _other);
	return *this;
}

void BlockHeader::clear()
{
	m_parentHash = h256();
	m_sha3Uncles = EmptyListSHA3;
	m_author = Address();
	m_stateRoot = EmptyTrie;
	m_transactionsRoot = EmptyTrie;
	m_receiptsRoot = EmptyTrie;
	m_logBloom = LogBloom();
	m_difficulty = 0;
	m_number = 0;
	m_gasLimit = 0;
	m_gasUsed = 0;
	m_timestamp = Invalid256;
	m_extraData.clear();
	m_seal.clear();
	m_signature = { h256(), h256(), 0 };

	const eth::chain::version emptyVer = dev::eth::chain::version(0, 0, 0);
	const eth::chain::hardfork_version emptyHfVer = dev::eth::chain::hardfork_version(emptyVer);

	m_running_ver = emptyVer;
	m_hardfork_vote = dev::eth::chain::hardfork_version_vote(emptyHfVer,fc::time_point_sec(0));

	noteDirty();
}

h256 BlockHeader::hash(IncludeSeal _i) const
{
	h256 dummy;
	Guard l(m_hashLock);
	h256& memo = _i == WithSeal ? m_hash : _i == WithoutSeal ? m_hashWithout : dummy;
	if (!memo)
	{
		RLPStream s;
		streamRLP(s, _i);
		memo = sha3(s.out());
	}
	return memo;
}

void BlockHeader::streamRLPFields(RLPStream& _s) const
{
	_s	<< m_parentHash << m_sha3Uncles << m_author << m_stateRoot << m_transactionsRoot << m_receiptsRoot << m_logBloom
		<< m_difficulty << m_number << m_gasLimit << m_gasUsed << m_timestamp << m_extraData
	//当前版本信息
	<< (m_running_ver.v_num)
	//写入hardfork投票信息
	<< (m_hardfork_vote.hf_version.v_num) << (m_hardfork_vote.hf_time.sec_since_epoch());
}

void BlockHeader::streamRLP(RLPStream& _s, IncludeSeal _i) const
{
	if (_i != OnlySeal)
	{
		_s.appendList(BlockHeader::BasicFields + (_i == WithoutSeal ? 0 : BlockHeader::SignatureFields));
		BlockHeader::streamRLPFields(_s);

		//for (unsigned i = 0; i < m_seal.size(); ++i)
		//	_s.appendRaw(m_seal[i]);
	}

	if (_i != WithoutSeal)
		_s << (m_signature.v + 27) << (u256)m_signature.r << (u256)m_signature.s;

}

h256 BlockHeader::headerHashFromBlock(bytesConstRef _block)
{
	return sha3(RLP(_block)[0].data());
}

RLP BlockHeader::extractHeader(bytesConstRef _block)
{
	RLP root(_block);
	if (!root.isList())
		BOOST_THROW_EXCEPTION(InvalidBlockFormat() << errinfo_comment("Block must be a list") << BadFieldError(0, _block.toString()));
	RLP header = root[0];
	if (!header.isList())
		BOOST_THROW_EXCEPTION(InvalidBlockFormat() << errinfo_comment("Block header must be a list") << BadFieldError(0, header.data().toString()));
	if (!root[1].isList())
		BOOST_THROW_EXCEPTION(InvalidBlockFormat() << errinfo_comment("Block transactions must be a list") << BadFieldError(1, root[1].data().toString()));
	if (!root[2].isList())
		BOOST_THROW_EXCEPTION(InvalidBlockFormat() << errinfo_comment("Block uncles must be a list") << BadFieldError(2, root[2].data().toString()));
	return header;
}

void BlockHeader::populate(RLP const& _header)
{
	int field = 0;
	try
	{
		m_parentHash = _header[field = 0].toHash<h256>(RLP::VeryStrict);
		m_sha3Uncles = _header[field = 1].toHash<h256>(RLP::VeryStrict);
		m_author = _header[field = 2].toHash<Address>(RLP::VeryStrict);
		m_stateRoot = _header[field = 3].toHash<h256>(RLP::VeryStrict);
		m_transactionsRoot = _header[field = 4].toHash<h256>(RLP::VeryStrict);
		m_receiptsRoot = _header[field = 5].toHash<h256>(RLP::VeryStrict);
		m_logBloom = _header[field = 6].toHash<LogBloom>(RLP::VeryStrict);
		m_difficulty = _header[field = 7].toInt<u256>();
		m_number = _header[field = 8].toInt<u256>();
		m_gasLimit = _header[field = 9].toInt<u256>();
		m_gasUsed = _header[field = 10].toInt<u256>();
		m_timestamp = _header[field = 11].toInt<u256>();
		m_extraData = _header[field = 12].toBytes();

		//读取出块人的客户端版本信息
		uint32_t run_ver = _header[field = 13].toInt<uint32_t>();
		//读取hardfork投票
		uint32_t hf_ver = _header[field = 14].toInt<uint32_t>();
		uint32_t hf_t = _header[field = 15].toInt<uint32_t>();

		m_running_ver.v_num = run_ver;
		m_hardfork_vote.hf_version.v_num = hf_ver;
		m_hardfork_vote.hf_time = fc::time_point_sec(hf_t);

		m_seal.clear();
		uint32_t sealCount = 0;

		//uint32_t sealCount = _header.itemCount() - BlockHeader::BasicFields - BlockHeader::SignatureFields;
		//for (unsigned i = 16; i < 16 + sealCount; ++i)
		//	m_seal.push_back(_header[i].data().toBytes());

		byte v = _header[field = 16 + sealCount].toInt<byte>() - 27;
		h256 r = _header[field = 17 + sealCount].toInt<u256>();
		h256 s = _header[field = 18 + sealCount].toInt<u256>();
		m_signature = { r, s, v };

		

	}
	catch (Exception const& _e)
	{
		_e << errinfo_name("invalid block header format") << BadFieldError(field, toHex(_header[field].data().toBytes()));
		throw;
	}
}

struct BlockInfoDiagnosticsChannel: public LogChannel { static const char* name() { return EthBlue "▧" EthWhite " ◌"; } static const int verbosity = 9; };

void BlockHeader::populateFromParent(BlockHeader const& _parent)
{
	m_stateRoot = _parent.stateRoot();
	m_number = _parent.m_number + 1;
	m_parentHash = _parent.m_hash;
	m_gasLimit = _parent.m_gasLimit;
	m_difficulty = 1;
	m_gasUsed = 0;
}

void BlockHeader::verify(Strictness _s, BlockHeader const& _parent, bytesConstRef _block) const
{
	if (m_number > ~(unsigned)0)
		BOOST_THROW_EXCEPTION(InvalidNumber());

	if (_s != CheckNothingNew && m_gasUsed > m_gasLimit)
		BOOST_THROW_EXCEPTION(TooMuchGasUsed() << RequirementError(bigint(m_gasLimit), bigint(m_gasUsed)));

	if (_parent)
	{
		if (m_parentHash && _parent.hash() != m_parentHash)
			BOOST_THROW_EXCEPTION(InvalidParentHash());

		if (m_timestamp <= _parent.m_timestamp)
			BOOST_THROW_EXCEPTION(InvalidTimestamp());

		if (m_number != _parent.m_number + 1)
			BOOST_THROW_EXCEPTION(InvalidNumber());
	}

	if (_block)
	{
		RLP root(_block);

		auto txList = root[1];
		auto expectedRoot = trieRootOver(txList.itemCount(), [&](unsigned i){ return rlp(i); }, [&](unsigned i){ return txList[i].data().toBytes(); });

		clog(BlockInfoDiagnosticsChannel) << "Expected trie root:" << toString(expectedRoot);
		if (m_transactionsRoot != expectedRoot)
		{
			MemoryDB tm;
			GenericTrieDB<MemoryDB> transactionsTrie(&tm);
			transactionsTrie.init();

			vector<bytesConstRef> txs;

			for (unsigned i = 0; i < txList.itemCount(); ++i)
			{
				RLPStream k;
				k << i;

				transactionsTrie.insert(&k.out(), txList[i].data());

				txs.push_back(txList[i].data());
				cdebug << toHex(k.out()) << toHex(txList[i].data());
			}
			cdebug << "trieRootOver" << expectedRoot;
			cdebug << "orderedTrieRoot" << orderedTrieRoot(txs);
			cdebug << "TrieDB" << transactionsTrie.root();
			cdebug << "Contents:";
			for (auto const& t: txs)
				cdebug << toHex(t);

			if (txs.size() > c_maxBlockTransaction)
			{
				BOOST_THROW_EXCEPTION(BlockTransactions() << RequirementError((bigint)c_maxBlockTransaction,(bigint)(txs.size())));
			}

			BOOST_THROW_EXCEPTION(InvalidTransactionsRoot() << Hash256RequirementError(expectedRoot, m_transactionsRoot));
		}
		clog(BlockInfoDiagnosticsChannel) << "Expected uncle hash:" << toString(sha3(root[2].data()));
		if (m_sha3Uncles != sha3(root[2].data()))
			BOOST_THROW_EXCEPTION(InvalidUnclesHash() << Hash256RequirementError(sha3(root[2].data()), m_sha3Uncles));
	}
}

void BlockHeader::sign(const fc::ecc::private_key& signer)
{
	enum { MixHashField = 0, NonceField = 1 };
	auto sig = dev::sign(signer, hash(WithoutSeal));
	SignatureStruct sigStruct = *(SignatureStruct const*)&sig;
	if (sigStruct.isValid())
		m_signature = sigStruct;
}

bool BlockHeader::verifySign() const
{
	return m_signature.isValid();
}

types::AccountName const& BlockHeader::producer() const
{
	if (!m_producer)
	{
		auto p = recover(m_signature, hash(WithoutSeal));
		if (!p)
			BOOST_THROW_EXCEPTION(InvalidSignature());
		m_producer = right160(dev::sha3(bytesConstRef(p.data(), sizeof(p))));
	}
	return m_producer;
}
