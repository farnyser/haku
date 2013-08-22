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

#ifndef __LIST_HPP__
#define __LIST_HPP__

	#include <vector>
	#include <string>
	#include <map>
	
	namespace haku
	{
		namespace list
		{
			typedef std::map<std::string,std::string> mapStrStr;
			typedef std::map<std::string,bool> mapStrBool;
			typedef std::vector<std::string> vectorStr;
			
			//~ mapStrStr <=> file
			bool mapStrStr_save(const mapStrStr &,  const std::string&);
			mapStrStr mapStrStr_load(const std::string&);
			//~ mapStrBool <=> file
			bool mapStrBool_save(const mapStrBool &,  const std::string&);
			mapStrBool mapStrBool_load(const std::string&);
			//~ vectorStr <=> file
			bool vectorStr_save(const vectorStr &,  const std::string&);
			vectorStr vectorStr_load(const std::string&);
			
			std::string toHTML(const std::string &type, const vectorStr &M);
			std::string toHTML(const std::string &type, const mapStrStr &M);
		}
	}
	
#endif // __LIST_HPP__
 
