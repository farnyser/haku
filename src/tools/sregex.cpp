//: Copyright (C) 2010 Farny Serge
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


#include "sregex.hpp"


namespace haku
{
	namespace sregex
	{
		result cmp(const std::string &pattern, const std::string &str)
		{
			result r, tmp;
			r.match = true;
			tmp.match = true;
			
			result_itemlist::iterator it;
			
			size_t pattern_i = 0, str_i = 0;
			bool wild = false;
			
			while ( pattern_i < pattern.size() && str_i < str.size() )
			{
				if ( pattern.at(pattern_i) == str.at(str_i) )
				{
					pattern_i++;
					str_i++;
				}
				else if ( pattern.at(pattern_i) == '*' )
				{
					if ( !wild )
					{			
						wild = true;
						r.itemlist.push_back(std::string("")+str.at(str_i++)); 
						
						if ( str_i+1 > str.size() )
							break;
					}
					
					if ( pattern_i+1 < pattern.size() )
					{
						tmp = cmp(pattern.substr(pattern_i+1, -1), str.substr(str_i,-1));
						
						if ( tmp.match )
						{
							for ( it = tmp.itemlist.begin() ; it != tmp.itemlist.end() ; it++ )
							{
								r.itemlist.push_back(*it);
							}
							
							break;
						}
						else
						{
							r.itemlist[r.itemlist.size()-1] += str.at(str_i);
							str_i++;
						}
					}
					else
					{
						for ( ; str_i < str.size() ; str_i++ ) 
						{
							r.itemlist[r.itemlist.size()-1] += str.at(str_i);
						}		
					}
				}
				else
				{
					r.match = false;
					r.itemlist.erase(r.itemlist.begin(), r.itemlist.end());
					break;
				}
				
			}
			
			if ( pattern_i >= pattern.size() && str_i+1 < str.size() ) r.match = false;
			else if ( str_i >= str.size() && pattern_i+1 < pattern.size() ) r.match = false;
			else if ( tmp.match == false ) r.match = tmp.match;
			
			return r;
		}
	}
}
