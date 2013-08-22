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


#include "tools.hpp"

namespace haku 
{
	namespace tools
	{
		/*
			Encode des caractères pour les passer dans une URL
		*/
		std::string urlencode(const std::string &str)
		{
			std::string tmp;

			for ( unsigned int i = 0 ; i < str.length() ; i++ )
			{
				switch ( str.at(i) )
				{
					case '!': tmp += "%21"; break;
					case '*': tmp += "%2A"; break;
					case '\'': tmp += "%27"; break;
					case '(': tmp += "%28"; break;
					case ')': tmp += "%29"; break;
					case ';': tmp += "%3B"; break;
					case ':': tmp += "%3A"; break;
					case '@': tmp += "%40"; break;
					case '&': tmp += "%26"; break;
					case '=': tmp += "%3D"; break;
					case '+': tmp += "%2B"; break;
					case '$': tmp += "%24"; break;
					case ',': tmp += "%2C"; break;
					case '/': tmp += "%2F"; break;
					case '?': tmp += "%3F"; break;
					case '%': tmp += "%25"; break;
					case '#': tmp += "%23"; break;
					case '[': tmp += "%5B"; break;
					case ']': tmp += "%5D"; break;
					default: tmp += str.at(i);
				}
			}
			
			return tmp;
		}
		
		/*
			Supprime #abc en fin de chaine
		*/
		std::string paragraphless_uri(const std::string &uri) 
		{
			if ( uri.rfind("#") != std::string::npos ) 
				return uri.substr(0, uri.rfind('#'));
			
			return uri;
		}

		/*
			Supprime "http://", ou équivalent en début de chaine
		*/
		std::string base_uri(std::string uri) 
		{
			if ( uri.substr(0,7) == "http://" ) uri.erase(0,7);
			else if ( uri.substr(0,8) == "https://" ) uri.erase(0,8);
			else if ( uri.substr(0,6) == "http:/" ) uri.erase(0,6);
			else if ( uri.substr(0,7) == "file://" ) uri.erase(0,7);
			else if ( uri.substr(0,6) == "file:/" ) uri.erase(0,6);
				
			if ( uri.find('/') != std::string::npos )
				uri = uri.substr(0, uri.find('/'));
			
			return uri;
		}

		/*
			Retourne une version très simplifiée de l'url
		*/
		std::string base_domain(std::string uri) 
		{
			signed int i;
			unsigned int countDot = 0;
			unsigned int countChar = 0;
			std::string ret("");
			
			uri = base_uri(uri);
			
			for ( i = uri.size()-1 ; i >= 0 ; i-- )
			{
				ret = uri.at(i) + ret;
				countChar++;
				
				if ( uri.at(i) == '.' )
				{
					countDot++;
				
					if ( countDot > 2 or (countDot == 2 && countChar >= 7) )
						break;
				}
				
			}
			
			if ( countDot < 2 ) ret = "." + ret;
			return ret;
		}

	}
}