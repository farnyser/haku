#ifndef __TOOLS_HPP_
#define __TOOLS_HPP_

	#include <string>
	
	#if DEBUG
		#include <iostream>
		#if DEBUG >= 1
			#define debug_v(var) std::cout << "(debug:v) " << var << std::endl;
		#endif
		#if DEBUG >= 2
			#define debug_vv(var) std::cout << "(debug:vv) " << var << std::endl;
		#endif
		#if DEBUG >= 3
			#define debug_vvv(var) std::cout << "(debug:vvv) " << var << std::endl;
		#endif
	#endif
	
	#ifndef debug_v
		#define debug_v(var)
	#endif
	#ifndef debug_vv
		#define debug_vv(var)
	#endif
	#ifndef debug_vvv
		#define debug_vvv(var)
	#endif
		
	namespace haku 
	{
		namespace tools
		{			
			//~ caractères spéciaux dans url
			std::string urlencode(const std::string &str);
			
			//~ simplification url
			std::string base_uri(std::string);
			std::string base_domain(std::string);
			std::string paragraphless_uri(const std::string&);
		}
	}
	
#endif // __TOOLS_HPP_
 
