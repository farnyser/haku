//: Copyright (C) 2011 Farny Serge
//: Encoding: utf-8
//: Contact/Information: contact@scawf.info 

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

#ifndef __CMDLINE_HPP__
#define __CMDLINE_HPP__

	#include <cstdlib>
	#include <set>
	#include <vector>
	#include <string>
	#include <map>

	#include "tools/list.hpp"
	#include "application.hpp"
	class application;
	
	/// \struct matchingItem
	/// \brief Used to represent a result from the bestMatch-completion used in the command line.
	typedef struct
	{
		/// Distance between the input string and this item
		float distance;
		/// One string matching the input
		std::string string;
	} matchingItem;
	
	/// \struct sortMatchingItem Fonctor
	/// \brief Used to sort matchingItem by score.	
	struct sortMatchingItem
	{ 
		bool operator ()(const matchingItem& a1, const matchingItem& a2) const
		{ 
			return a1.distance <= a2.distance; 
		} 
	}; 
	
	/// \class cmdline
	/// \brief Manage the commandline-interface.
	/// This class has to manage the commandline-interface.
	/// It stores the history, and manage deplacement both in history and in a line (cursor).
	/// It also manage the bestMatch-completion.
	class cmdline
	{
		private:
			std::vector<std::string> mList;
			short unsigned int mOffset, mCurrent;
			
			std::vector<std::vector<std::string> > mList2;
			short unsigned int mCurrentLine, mCurrentArg, mOffsetArg;
			
			std::set<matchingItem, sortMatchingItem>::reverse_iterator mMatchIndex;
			std::set<matchingItem, sortMatchingItem> mMatch;
			
			application *mApp;
		
			void updateBest(void);
		
		private: cmdline(const cmdline&) {};
		private: cmdline& operator=(const cmdline&) { return *this; };

		public:		
			cmdline(application *);
			std::string get(void);
			std::string getSpecial(void);
			matchingItem getBest(bool b);
			matchingItem getBest(void);
			
			void newline(std::string aInsert);
			void insert(std::string aInsert);
			void remove(signed int aSize);
			void move(signed int aMove);
			void history(signed int aMove);
			void useBest(bool b);
			void useBest(void);
			void moveBest(signed int aMove);

	};

#endif // __CMDLINE_HPP__
