#ifndef __STRING_HPP__
#define __STRING_HPP__

	#include <vector>
	#include <string>
	#include "sregex.hpp"
	
	namespace haku
	{
		namespace string
		{
			//~ tolower
			std::string tolower(const std::string &);
			//~ supression espace debut et fin
			std::string trim(const std::string &);
			//~ supression espace debut et fin autour du caractère i-ème
			std::string trim(const std::string &, size_t i);
			//~ supression double espace
			std::string cleanWhitespace(const std::string &);
			//~ string -> array
			std::string implode(const std::vector<std::string>&, const char);
			std::vector<std::string> explode(const std::string&, const char);
			std::vector<std::string> explode(const std::string&);
			//~ Remplace une sous chaine par une chaine
			std::string replaceSubstring(const std::string &find, const std::string &replace, std::string &str);
			std::string replaceSubstring(const std::string &find, const std::string &replace, std::string &str, int&o);
			//~ caractere dans la liste ?
			bool inCharset(const char, const std::string&);
			//~ Comparaison avec wildcard
			bool cmp(std::string, std::string);
		}
	}
	
	
	
#endif // __STRING_HPP__
 
