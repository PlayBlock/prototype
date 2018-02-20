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
/** @file HostCapability.cpp
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 */

#include "HostCapability.h"

#include "Session.h"
#include "Host.h"
using namespace std;
using namespace dev;
using namespace dev::p2p;

std::vector<std::pair<std::shared_ptr<SessionFace>, PeerSortObject>> HostCapabilityFace::peerSessions() const
{
	return peerSessions(version());
}

std::vector<std::pair<std::shared_ptr<SessionFace>, PeerSortObject>> HostCapabilityFace::peerSessions(u256 const& _version) const
{
	RecursiveGuard l(m_host->x_sessions);
	std::vector<std::pair<std::shared_ptr<SessionFace>, PeerSortObject>> ret;
	for (auto const& i: m_host->m_sessions)
		if (std::shared_ptr<SessionFace> s = i.second.lock())
			if (s->capabilities().count(std::make_pair(name(), _version))) {
				//此处将second定义为PeerSortObject，因为在多线程环境下如果在排序中直接使用Session
				//会导致中途评分及连接时间变化，把排序搞烂
				ret.push_back(make_pair(s, PeerSortObject(s->peer()->id, s->rating(),s->connectionTime()) ));
			}
	return ret;
}
