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


#include "string.hpp"
#include "tools.hpp"
#include <algorithm>


namespace haku
{
	namespace string
	{
		/*
			Transform en minuscule
		*/
		std::string tolower(const std::string &str)
		{
			std::string result;
			
			for ( size_t i = 0 ; i < str.size() ; i++ )
				result += std::tolower(str[i]);
			
			return result;
		}
		
		/*
			Supprime les espaces en debut et fin de chaine
		*/
		std::string trim(const std::string &str )
		{
			if( str.empty() ) return "";
			
			size_t startIndex = str.find_first_not_of(' '); 
			size_t endIndex = str.find_last_not_of(' ');; 
			
			if ( startIndex == std::string::npos ) startIndex = 0;
			if ( endIndex == std::string::npos ) endIndex = str.size();
			
			return str.substr(startIndex, (endIndex-startIndex+1));
		}

		std::string trim(const std::string &str, size_t index)
		{
			if( str.empty() ) return "";
			if( index > str.size() ) throw "index > str.size()";
			
			debug_vvv("to be stripped =" << str << ";")
			
			size_t i;
			size_t startIndex = std::string::npos; 
			size_t endIndex = std::string::npos; 
			
			for ( i = 0 ; i < index ; i++ )
			{
				if ( str[i] == ' ' ) 
				{
					startIndex = i+1;
				}
			}
			
			for ( i = index ; i < str.size() ; i++ )
			{
				if ( str[i] == ' ' ) 
				{
					endIndex = i-1;
					break;
				}
			}
			
			if ( startIndex == std::string::npos ) startIndex = 0;
			if ( endIndex == std::string::npos ) endIndex = str.size();

			debug_vvv("stripped =" << str.substr(startIndex, (endIndex-startIndex+1)) << ";");
			
			return str.substr(startIndex, (endIndex-startIndex+1));
		}

		/*
			Supprime les espaces doubles
		*/
		std::string cleanWhitespace(const std::string &s)
		{
			size_t i;
			std::string tmp;
			debug_vv("str::clean(" << s << ")");
			
			for ( i = 0 ; i < s.size() ; i++  )
			{
				if ( s[i] == ' ' )
				{
					if ( tmp.size() && tmp[ tmp.size() - 1 ] == ' ' )
						continue;
				}
				
				tmp += s[i];
			}
			
			debug_vv("str::cleaned = " << tmp);
			return tmp;
		}

		/*
			Explose une chaine suivant un séparateur en un tableau (vector)
		*/
		std::string implode(const std::vector<std::string>&arg, const char c)
		{
			std::string str;
			
			for (size_t i = 0 ; i < arg.size() ; i++ )
			{
				if ( i ) str += c + arg[i];
				else str = arg[i];
			}
			
			return str;
		}
		
		std::vector<std::string> explode(const std::string &str, const char c)
		{
			debug_vvv("str::explode(" << str << ")");
			
			std::vector<std::string> array;
			
			for ( size_t i = 0 ; i < str.length() ; i++ )
			{
				if ( i == 0 or str[i] == c )
					array.push_back("");
				
				if ( str[i] != c )
					array[ array.size() - 1 ] += str[i];
			}
			
			debug_vvv("str::explode has found " << array.size() << " argument(s)");
			return array;
		}	
		
		//~ par defaut: on utilise le caractère d'espacement
		std::vector<std::string> explode(const std::string &str)
		{
			return haku::string::explode(str,' ');
		}

		/*
			Un caractère c est il dans l'ensemble charset ?
		*/
		bool inCharset(const char c, const std::string &charset)
		{
			for ( size_t i = 0 ; i < charset.length() ; i++ )
				if ( charset.at(i) == c )
					return true;
			
			return false;
		}

		/*
			Remplacement d'une sous chaine
		*/
		std::string replaceSubstring(const std::string &find, const std::string &replace, std::string& str)
		{
			int i;
			return replaceSubstring(find, replace, str, i);
		}
		
		std::string replaceSubstring(const std::string &find, const std::string &replace, std::string& str, int &o)
		{
			int i, imax = str.size() - find.size(), ds = replace.size() - find.size();
			
			for ( i = 0; i <= imax ; i++ )
			{
				if ( str.substr(i, find.size()) == find )
				{
					debug_v("replace " << str.substr(i, find.size()) << " with " << replace << " (ds:" << ds << ")");
					str.replace(i, find.size(), replace);
					
					//~ On ne parse PAS le bout qui a été remplacé
					if ( i < o ) o += ds;
					i += ds;
					imax += ds;
				}
				
			}
			
			return str;
		}
	
		bool cmp(std::string str, std::string expr)
		{
			bool wild = false;
			size_t str_i, expr_i, start;
			start = 0;
			
			str = tolower(str);
			expr = tolower(expr);
			
			for ( str_i = start, expr_i = 0 ; expr_i < expr.size() && str_i < str.size()  ; expr_i++ )
			{
				if ( expr.at(expr_i) == '*' )
				{
					wild = true;
					continue;
				}
				
				do
				{
					if ( expr.at(expr_i) == str.at(str_i) )
					{
						wild = false;
						str_i++;
					}
					else
					{
						if ( wild )
						{
							str_i++;
						}
						else
						{
							str_i = start++;
							expr_i = 0;
							break;
						}
					}
				} while ( str_i < str.size() && wild );
			
			}
			
			//~ std::cout << "expr_i " << expr_i << std::endl;
			//~ std::cout << "expr_size " << expr.size() << std::endl;
			//~ std::cout << "str_i " << str_i << std::endl;
			//~ std::cout << "str_size " << str.size() << std::endl;
			//~ std::cout << "wild " << wild << std::endl;
			
			if ( expr_i+1 == expr.size() && expr.at(expr_i) == '*' )
			{
				expr_i++;
				wild = true;
			}
			
			return ((expr_i == expr.size()) && (wild or (str_i == str.size())));
		}
		
		
	}
}
