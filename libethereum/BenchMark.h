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
/** @file BenchMark.h
 * @author dz <i@gavwood.com>
 * @date 2014
 */

#pragma once
#include <libdevcore/Log.h>
#include <libdevcore/Common.h>

#define BenchMarkFlag 1
namespace dev
{
namespace eth
{
struct BenchMarkChannel : public LogChannel {
	static const char* name(); static const int verbosity = 0; static const bool debug = false;
};
class BenchMark
{
public:	
	BenchMark(std::string _name);
	static void makeCurrent(BenchMark*);
	void showSummary(double _time);
	static void IncreaseInvalidDeal();
	static void IncreaseTransfer();
	static void IncreaseContractCall();
	static void IncreaseContractCreate();
	
	static BenchMark* _current;

	void restartCount();
	std::string Name;
	int WrongDeal;
	int Transfer;
	int ContractCall;
	int ContractCreate;
	Timer timer;

	static double MainTime;
	static double SerielizeTime;
};

}
}
