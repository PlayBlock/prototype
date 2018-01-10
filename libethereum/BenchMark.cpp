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
#define BenchMarkFlag 1
using namespace dev;
using namespace dev::eth;

BenchMark* BenchMark::_current = nullptr;

BenchMark::BenchMark(std::string _name)
{
	Name = _name;
}
void BenchMark::makeCurrent()
{
	_current = this;
}

void BenchMark::showSummary(double passTime)
{
	std::cout << "Count: "<< _current->Name;
	std::cout << " Transfer      : " << _current->Transfer;
	std::cout << " WrongDeal     : " << _current->WrongDeal;
	std::cout << " ContractCall  : " << _current->ContractCall;
	std::cout << " ContractCreate: " << _current->ContractCreate;
	std::cout << " Speed: ";
	std::cout << " Transfer      : " << _current->Transfer/ passTime;
	std::cout << " WrongDeal     : " << _current->WrongDeal/ passTime;
	std::cout << " ContractCall  : " << _current->ContractCall/ passTime;
	std::cout << " ContractCreate: " << _current->ContractCreate/ passTime<<std::endl;
}
void BenchMark::IncreaseInvalidDeal()
{
	_current->WrongDeal++;
}
void BenchMark::IncreaseTransfer()
{
	_current->Transfer++;
}
void BenchMark::IncreaseContractCall()
{
	_current->ContractCall++;
}
void BenchMark::IncreaseContractCreate()
{
	_current->ContractCreate++;
}

void BenchMark::restartCount()
{
	WrongDeal = 0;
	Transfer = 0;
	ContractCall = 0;
	ContractCreate = 0;
}
