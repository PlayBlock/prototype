/*
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
/** @file BenchMark.cpp
 * @author dz <i@gavwood.com>
 * @date 2014
 */

#pragma once
#include "BenchMark.h"

using namespace dev;
using namespace dev::eth;
const char* BenchMarkChannel::name() { return EthGreen "BenchMark"; }

BenchMark* BenchMark::_current = nullptr;

double BenchMark::MainTime = 0.0;
double BenchMark::SerielizeTime = 0.0;

double BenchMark::record_1 = 0.0;
double BenchMark::record_2 = 0.0;
double BenchMark::record_3 = 0.0;
bool BenchMark::lastTxtInBlock = false;
bool BenchMark::importBlock = false;
BenchMark::BenchMark(std::string _name)
{
	Name = _name;
}
void BenchMark::makeCurrent(BenchMark* current)
{
	_current = current;
}

void BenchMark::showSummary(double passTime)
{
	
	//clog(BenchMarkChannel) << "Count: "<< _current->Name<< " Transfer  : " << _current->Transfer << " WrongDeal : " << _current->WrongDeal<< " ContractCall  : " << _current->ContractCall<< " ContractCreate: " << _current->ContractCreate << " Speed: " << " Transfer      : " << _current->Transfer/ passTime << " WrongDeal     : " << _current->WrongDeal/ passTime << " ContractCall  : " << _current->ContractCall/ passTime<< " ContractCreate: " << _current->ContractCreate/ passTime;
}
void BenchMark::IncreaseInvalidDeal()
{
	if (_current)
	{
		_current->WrongDeal++;
	}

}
void BenchMark::IncreaseTransfer()
{
	if (_current)
	{
		_current->Transfer++;
	}
}
void BenchMark::IncreaseContractCall()
{
	if (_current)
	{
		_current->ContractCall++;
	}
}
void BenchMark::IncreaseContractCreate()
{
	if (_current)
	{
		_current->ContractCreate++;
	}
}

void BenchMark::restartCount()
{
	WrongDeal = 0;
	Transfer = 0;
	ContractCall = 0;
	ContractCreate = 0;
}
