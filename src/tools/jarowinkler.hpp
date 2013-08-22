//: Copyright (C) 2011 Farny Serge
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

#ifndef __JAROWINKLER_HPP_
#define __JAROWINKLER_HPP_

	#include <string>
	
	namespace haku 
	{
		//~ analyse distance d'edition
		float jaroWinkler(const std::string&, const std::string&);
	}
	
#endif // __JAROWINKLER_HPP_
 
