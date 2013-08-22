//: Copyright (C) 2011Farny Serge
//: Encoding: utf-8
//: Contact/Information: contact@scawf.info 

//: This file is part of Haku.
//:
//: SRegex is free software: you can redistribute it and/or modify
//: it under the terms of the GNU General Public License as published by
//: the Free Software Foundation, either version 3 of the License, or
//: (at your option) any later version.
//:
//: Haku is distributed in the hope that it will be useful,
//: but WITHOUT ANY WARRANTY; without even the implied warranty of
//: MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//: GNU General Public License for more details.
//:
//: You should have received a copy of the GNU General Public License
//: along with Haku.  If not, see <http://www.gnu.org/licenses/>.

#ifndef __SREGEX_HPP__
#define __SREGEX_HPP__

#include <vector>
#include <string>

namespace haku
{
	namespace sregex
	{
		typedef std::vector<std::string> result_itemlist;
		typedef struct { bool match; result_itemlist itemlist; } result;
		result cmp(const std::string &pattern, const std::string &str);
	};
};

#endif // __SREGEX_HPP__
