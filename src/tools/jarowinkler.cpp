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


#include "jarowinkler.hpp"
#include "tools.hpp"

#include <algorithm>

namespace haku 
{
	/*
		analyse distance d'edition (algo de Jaro Winkler l�g�rement modifi�).
	*/
	float jaroWinkler(const std::string &s1, const std::string &s2) 
	{
		debug_vvv("comparing " << s1 << " to " << s2);
		
		//~ Coefficient de winkler, favorise les chaines ayant un d�but commun.
		const float c_p = 0.1;
		
		//~ Autre coefficient qui vaut le nombre de caract�re commun en d�but de chaine, 
		//~ avec une valeur maximale � 4
		unsigned int c_l; 
		
		//~ Utile pour ne pas appeler X fois les methodes size()
		const size_t s1size = s1.size(), s2size = s2.size();
		
		//~ Distance std::max pour que deux caract�res correspondant mais plac�s 
		//~ � des offsets diff�rents soient consid�r�s comme "matching" 
		const float dstMax = (float)std::max(s1size, s2size) / 2.0 - 1.0; 
		
		//~ Listes des caracteres "matching"
		std::string s1_match, s2_match;

		//~ Nombre de transposition: caract�re identique mais pas � la m�me place
		float t = 0;
		
		//~ Nombre de caract�re "matching"
		float m;
		
		//~ It�rateurs
		size_t i, j;
		
		//~ Distances de jaro pour simplifier la formule de jaro-winker
		float dj;
		float dw;
		float ds = 0; // scouf addition
		
		
		//~ Caracteres identiques et identiquement plac�s (on se limite � 4)
		for (c_l = 0 ; c_l < std::min((const size_t)4, std::min(s1size,s2size)) && s1[c_l] == s2[c_l] ; c_l++ ) {}
		
		//~ scouf addition to speed up
		//~ if ( std::min(s1size,s2size) > c_l && std::abs((signed int)s1size-(signed int)s2size) > c_l ) 
			//~ return 0;
		
		//~ Recherches des caract�res correspondants de s1 dans s2
		//~ -------------------------------------------------------------------------------------
		//~ Pour chaque caract�re de s1 � l'offset i, on cherche un caract�re identique dans s2 
		//~ a l'offset j allant de i-dstMax � i+dstMax
		//~ L'interval i-dstMax I+dstMax n'est pas clairement cod�, car c'est plus lent (experiment�).
		//~ mais le principe est l�.
		//~ De plus, si un caract�re correspond, on l'enregistre dans une liste
		for ( i = 0 ; i < s1size ; i++ ) 
		{	
			for ( j = 0 ; j < s2size ; j++ ) 
			{
				if ( (i>j ? i-j : j-i) <= dstMax && s1[i] == s2[j] )
				{
					/* special: considere '/' comme tres "fort" */
					if ( s1[i] == '/' )
						ds++;
					/* fin special */
					
					s1_match += s1[i++];	
					j=0;
				}
			}
		}
		
		//~ Recherche des caract�res correspondants de s2 dans s1
		for ( i = 0 ; i < s2size ; i++ ) 
		{
			for ( j = 0 ; j < s1size ; j++ ) 
			{
				if ( (i>j ? i-j : j-i) <= dstMax && s1[j] == s2[i] )
				{
					s2_match += s2[i++];
					j=0;
				}
			}
		}
		
		//~ m vaut le nombre de caract�re correspondant
		m = std::min(s1_match.size(),s2_match.size());
		//~ if ( m == 0 ) return 0;
		if ( m*2 < std::min(s1size,s2size) ) return 0;
	
		//~ Recherche du nombre de transposition, dans les listes de correspondances
		for ( i = 0 ; i < m ; i++ ) 
		{
			if ( s1_match[i] != s2_match[i] )
			{
				t++;
			}
		}
		
		//~ On divise par deux car une transposition est compt�e 
		//~ pour l'�change de deux caract�res
		t = t / 2.0;
		
		//~ distance de jaro
		dj = 1.0/3.0 * ( m/s1size + m/s2size + (m-t)/m);
		//~ distance de jaro-winkler
		dw = dj + ((float)c_l*c_p*(1.0-dj));
		//~ distance scouf (avec modif pour le /)
		ds = dw + (1.0-dw)*ds*0.1;
		
		debug_vvv("ds:" << ds << " dw: " << dw << " dj: " << dj << " t: " << t << " m: " << m << " c_l: " << c_l);
		
		return ds;
	}
}
