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

#include <fstream>

#include "list.hpp"
#include "string.hpp"
#include "tools.hpp"
#include "../config.hpp"

namespace haku
{
	namespace list
	{
		/*
			mapStrStr -> file
		*/
		bool mapStrStr_save(const mapStrStr &M,  const std::string &path)
		{
			mapStrStr::const_iterator i = M.begin(), iend = M.end();
			std::ofstream fs(path.c_str() , std::ofstream::out);
			
			while ( i != iend )
			{
				fs << i->first << " " << i->second << std::endl;		
				i++;
			}
			
			fs.close();	
			return true;
		}


		/*
			file -> mapStrStr
		*/
		mapStrStr mapStrStr_load(const std::string& path)
		{
			mapStrStr ret;
			
			std::ifstream fs(path.c_str(),std::ifstream::in);
			std::vector<std::string> tmp_v;
			std::string tmp_s;
			
			while (fs.good())
			{
				getline(fs,tmp_s);
				tmp_v = haku::string::explode(tmp_s);
				
				//~ ligne ok
				if ( tmp_v.size() == (2) ) 
				{
					ret[tmp_v[0]] = tmp_v[1];
				}
			}
			
			fs.close();
			
			return ret;
		}

		/*
			mapStrBool -> file
		*/
		bool mapStrBool_save(const mapStrBool &M,  const std::string &path)
		{
			mapStrBool::const_iterator i = M.begin(), iend = M.end();
			std::ofstream fs(path.c_str() , std::ofstream::out);
			
			while ( i != iend )
			{
				fs << i->first << " " << (i->second ? "on" : "off") << std::endl;		
				i++;
			}
			
			fs.close();	
			return true;
		}


		/*
			file -> mapStrBool
		*/
		mapStrBool mapStrBool_load(const std::string& path)
		{
			mapStrBool ret;
			
			std::ifstream fs(path.c_str(),std::ifstream::in);
			std::vector<std::string> tmp_v;
			std::string tmp_s;
			
			while (fs.good())
			{
				getline(fs,tmp_s);
				tmp_v = haku::string::explode(tmp_s);
				
				//~ ligne ok
				if ( tmp_v.size() == (2) ) 
				{
					ret[tmp_v[0]] = (tmp_v[1] == "on");
				}
			}
			
			fs.close();
			
			return ret;
		}

		/*
			file -> vectorStr
		*/
		vectorStr vectorStr_load(const std::string& path)
		{
			vectorStr ret;
			
			std::ifstream fs(path.c_str(),std::ifstream::in);
			std::string tmp_s;
			
			while (fs.good())
			{
				getline(fs,tmp_s);
				ret.push_back(tmp_s);
			}
			
			fs.close();
			
			return ret;
		}

		/*
			vectorStr -> file
		*/		
		bool vectorStr_save(const vectorStr &M,  const std::string &path)
		{
			vectorStr::const_iterator i = M.begin(), iend = M.end();
			std::ofstream fs(path.c_str() , std::ofstream::out);
			
			while ( i != iend )
			{
				if ( (*i).size() )
					fs << *i << std::endl;		
				i++;
			}
			
			fs.close();	
			return true;
		}
		
		/*
			vectorStr -> html
		*/
		std::string toHTML(const std::string &type, const vectorStr &M)
		{
			std::string html;
			vectorStr::const_iterator it = M.begin(), iend = M.end();
			
			html = HTML_HEADER;
			
			while ( it != iend )
			{
				if ( type == "session_content" )
					html += HTML_CONTENT_SESSION;
				
				html = haku::string::replaceSubstring("%item_uri%", *it, html);
				
				it++;
			}
			
			html += HTML_FOOTER;
			html = haku::string::replaceSubstring("%type%", type, html);
			
			return html;
		}
		
		/*
			mapStrStr -> html
		*/
		std::string toHTML(const std::string &type, const mapStrStr &M)
		{
			std::string html, buffer;
			mapStrStr::const_iterator it = M.begin(), iend = M.end();
			
			html = HTML_HEADER;
			
			while ( it != iend )
			{
				if ( type == "bookmark" )
					html += HTML_CONTENT_BOOKMARK;
				else if ( type == "searchengine" )
					html += HTML_CONTENT_BOOKMARK;
				
				html = haku::string::replaceSubstring("%item_uri%", it->second, html);
				html = haku::string::replaceSubstring("%item_title%", it->first, html);
				
				it++;
			}
			
			html += HTML_FOOTER;
			html = haku::string::replaceSubstring("%type%", type, html);
			
			return html;
		}
	}
}

