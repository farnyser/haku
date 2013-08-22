//: Copyright (C) 2011Farny Serge
//: contact@scawf.info 

//: This file is part of Haku.
//:
//: Haku is free software: you can redistribute it and/or modify
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

#ifndef __ADBLOCK_HPP_
#define __ADBLOCK_HPP_

	#include <string>
	#include <vector>
	
	namespace haku 
	{
		namespace adblock
		{
			enum result {block,allow,ignore};
			
			result parse(std::string uri, std::vector<std::string> rules);
			result ruleCheck(std::string uri, std::string rule);
			bool cmp(std::string uri, std::string rule_uri);
		}
	}
	
#endif // __ADBLOCK_HPP_
 
