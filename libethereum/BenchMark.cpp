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

std::string BenchMark::Name = "";
int BenchMark::WrongDeal = 0;
int BenchMark::Transfer = 0;
int BenchMark::ContractCall = 0;
int BenchMark::ContractCreate = 0;
Timer BenchMark::timer = Timer();

void BenchMark::ShowSummary()
{
	std::cout << "=========Benchmark Total=========" << std::endl;
	std::cout << "name:"<<Name << std::endl;
	double passTime = timer.elapsed();
	std::cout << "TimePassed    : " << passTime / 1000.0 <<"seconds"<<std::endl;
	std::cout << "Transfer      : " << Transfer << std::endl;
	std::cout << "WrongDeal     : " << WrongDeal << std::endl;
	std::cout << "ContractCall  : " << ContractCall << std::endl;
	std::cout << "ContractCreate: " << ContractCreate << std::endl;
	std::cout << "=========Benchmark Speed(k/s)=====" << std::endl;
	std::cout << "Transfer      : " << Transfer/ passTime << std::endl;
	std::cout << "WrongDeal     : " << WrongDeal/ passTime << std::endl;
	std::cout << "ContractCall  : " << ContractCall/ passTime << std::endl;
	std::cout << "ContractCreate: " << ContractCreate/ passTime << std::endl;
	std::cout << "=================================" << std::endl;
}
void BenchMark::IncreaseInvalidDeal()
{
	WrongDeal++;
}
void BenchMark::IncreaseTransfer()
{
	Transfer++;
}
void BenchMark::IncreaseContractCall()
{
	ContractCall++;
}
void BenchMark::IncreaseContractCreate()
{
	ContractCreate++;
}

void BenchMark::Restart(std::string _name)
{
	Name = _name;
	WrongDeal = 0;
	Transfer = 0;
	ContractCall = 0;
	ContractCreate = 0;
	timer.restart();
}
