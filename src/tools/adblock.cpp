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

#include "adblock.hpp"
#include "tools.hpp"
#include "string.hpp"

namespace haku
{
	namespace adblock
	{
		result parse(std::string uri, std::vector<std::string> rules)
		{
			result r = ignore;
			std::vector<std::string>::iterator it;
			
			for ( it = rules.begin() ; it != rules.end() ; it++ )
			{
				switch (ruleCheck(uri, *it))
				{
					case block:
						if ( r == ignore ) {r = block;}
						break;
						
					case allow:
						r = allow;
						break;
					
					case ignore:
						break;
				}
			}
			
			return r;
		}
	
		result ruleCheck(std::string uri, std::string rule)
		{
			result r = ignore;
			
			if ( uri.length() && rule.length() )
			{
				switch ( rule.at(0) )
				{
					// cette regle est un commentaire
					case '!':
						break;
					
					// cette regle est une autorisation
					case '@':
						if ( rule.length() > 2 && rule.substr(0,2) == "@@" )
						{
							if ( cmp(uri, rule.substr(2,-1)) )
								r = allow;
						}
						break;
					
					// cette regle est une interdiction
					default:
						if ( rule.find("##") != std::string::npos or rule.find("$") != std::string::npos ) 
							r = ignore;
						else if ( cmp(uri, rule) )
							r = block;
						break;
				}
			}
			
			return r;
		}
		
		bool cmp(std::string uri, std::string rule_uri)
		{
			if ( rule_uri.length() && rule_uri.at(0) != '|' ) rule_uri = '*' + rule_uri;
			else rule_uri = rule_uri.substr(1,-1);
			if ( rule_uri.at(rule_uri.length()-1) != '|' ) rule_uri += '*';
			else rule_uri = rule_uri.substr(0,-rule_uri.length()-2);
			
			return haku::string::cmp(uri, rule_uri);
		}
	}
}
