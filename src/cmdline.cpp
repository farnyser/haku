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


#include "cmdline.hpp"
#include "tools/tools.hpp"
#include "tools/string.hpp"
#include "tools/jarowinkler.hpp"
#include "config.hpp"

/** 
	\brief Constructor
	\param app Pointer to the min application. Useful for the bestMatch-completion
**/
cmdline::cmdline(application *app)
{
	this->mApp = app;
}

/** 
	\brief Get the current line
	\return A string : The current (displayed) line
**/
std::string cmdline::get()
{	
	if ( this->mCurrent >= this->mList.size() )
	{
		throw("mCurrent > mList.size()");
		return std::string("");
	}
	else
	{
		return this->mList[this->mCurrent];
	}
}

/** 
	\brief Get the current line with cursor
	\return A string : The current (displayed) line with a cursor '|'
**/
std::string cmdline::getSpecial()
{	
	if ( this->mCurrent >= this->mList.size() )
	{
		throw("mCurrent > mList.size()");
		return std::string("");
	}
	else
	{
		std::string tmp = this->get();
		int offset = this->mOffset;
		
		haku::string::replaceSubstring("&","&amp;",tmp,offset);

		return tmp.insert(offset, "<span foreground=\"" COLOR_INDICATOR_COMMANDLINE_FG2 "\">|</span>");
	}
}


/** 
	\brief Get the current bestMatch item
	\param b when true, return the best bestMatch item
	\return A matchingItem : The current bestMatch item
**/
matchingItem cmdline::getBest(bool b)
{	
	if ( this->mMatch.size() > 0 )
	{
		if ( b )
			return *(mMatch.rbegin());
		else
			return *mMatchIndex;
	}
	else
	{
		return matchingItem();
	}
}
/** 
	\brief Get the current bestMatch item
	\return A matchingItem : The current bestMatch item
**/
matchingItem cmdline::getBest(void)
{	
	return this->getBest(false);
}

/** 
	\brief Go to the next bestMatch item
	\param aMove A signed integer used to move inside of the bestMatch list
**/
void cmdline::moveBest(signed int aMove)
{
	if ( this->mMatch.size() <= 0 ) return;
	signed int i;
	
	for ( i = 0 ; i < aMove ; i++ ) 
	{
		this->mMatchIndex++;
		
		if ( this->mMatchIndex == this->mMatch.rend() )
			this->mMatchIndex = this->mMatch.rbegin();
	}
	
	for ( i = 0 ; i > aMove ; i-- ) 
	{
		if ( this->mMatchIndex == this->mMatch.rbegin() )
			this->moveBest(this->mMatch.size() - 1);
		else
			this->mMatchIndex--;
	}
}


/**
	\brief Replace the current argument (space separated) by the current bestMAtch item
	\param b when true, use the best bestMatch item
**/
void cmdline::useBest(bool b)
{
	//~ ceci ne met pas a jour la liste des bestmatch
	//~ histoire de pouvoir continuer a switcher
	
	std::string bestmatch = this->getBest(b).string;
	
	if ( bestmatch.size() > 0 )
	{
		std::string tmp = this->get();
		std::string arg = haku::string::trim(tmp, this->mOffset);
		
		debug_v("cmdline::useBest best=" << bestmatch << " arg=" << arg)
		
		int startIndex = 0;
		int i;
		
		for ( i = 0 ; i < this->mOffset ; i++ )
			if ( tmp.at(i) == ' ' ) startIndex = i+1;
		
		this->mOffset += bestmatch.size() - arg.size();
		this->mList[this->mCurrent].replace(startIndex, arg.size(), bestmatch);
	}
}

/**
	\brief Replace the current argument (space separated) by the current bestMatch item
**/
void cmdline::useBest(void)
{
	this->useBest(false);
}


/**
	\brief Generate a list of bestMatch items
**/
void cmdline::updateBest(void)
{
	//~ effacement precedent
	this->mMatch.clear();
	
	//~ variable utiles
	std::vector<std::string> searchVector;
	
	std::string cmd = this->get();
	std::string arg = haku::string::trim(cmd, this->mOffset);;
	
	matchingItem item;
	unsigned int i;
	
	//~ comptargument
	std::vector<std::string> args = haku::string::explode(cmd);
	unsigned int args_current;
	
	for ( i = 0, args_current = 0 ; i < this->mOffset ; i++ )
		if ( cmd.at(i) == ' ' )
			args_current++;
	
		
	if ( args_current == 0 )
	{
		for ( int i = 0 ; interpreter::map[i].name != "" ; i++ )
			searchVector.push_back(std::string(":") + interpreter::map[i].name);
	}	
	else if ( args_current >= 2 && args[0] == ":exec" )
	{
		searchVector.push_back("%localpath");
		searchVector.push_back("%uri");
		searchVector.push_back("%clipboard");
		searchVector.push_back("%xclipboard");
	}
	else if ( args_current == 1 && (args[0] == ":bookmark" or args[0] == ":bm" or args[0] == ":searchengine" or args[0] == ":se") )
	{
		searchVector.push_back("show");
		searchVector.push_back("add");
		searchVector.push_back("del");
	}	
	else if ( args_current == 1 && args[0] == ":session" )
	{
		searchVector.push_back("restore");
		searchVector.push_back("clean");
		searchVector.push_back("save");
		searchVector.push_back("show");
	}
	else if ( args_current == 2 && args[0] == ":session" )
	{
		debug_vv("bestMatch::get session list && show it");
		searchVector = this->mApp->session_getList();
	}
	else if ( args_current >= 1 && (args[0] == ":plugins" or args[0] == ":cookies" or args[0] == ":javascript" or args[0] == ":js") )
	{
		if ( args_current == 2 && (args[1] == "on" or args[1] == "off") )
		{	
			searchVector.push_back("always");
		}
		else if ( args_current == 1 )
		{
			searchVector.push_back("on");
			searchVector.push_back("off");
		}
	}	
	else if ( args_current >= 1 && args[0] == ":set" )
	{
		if ( args_current == 3 && (args[2] == "on" or args[2] == "off") )
		{	
			searchVector.push_back("always");
		}
		else if ( args_current == 3 && (args[1] == "useragent") )
		{	
			searchVector.push_back("always");
		}
		else if ( args_current == 2 && (args[1] == "useragent") )
		{
			for ( int i = 0 ; useragent[i].name != "" ; i++ )
				searchVector.push_back(useragent[i].name);
		}
		else if ( args_current == 2 )
		{
			searchVector.push_back("on");
			searchVector.push_back("off");
		}
		else if ( args_current == 1 )
		{
			searchVector.push_back("javascript");
			searchVector.push_back("plugins");
			searchVector.push_back("cookies");
			searchVector.push_back("useragent");
			searchVector.push_back("adblock");
		}
	}
	else if ( 
		(args_current == 1 && (args[0] == ":open" or args[0] == ":o" or args[0] == ":new" or args[0] == ":n")) 
		or
		(args_current == 2 && (args[1] == "del") && (args[0] == ":bm" or args[0] == ":bookmark" or args[0] == ":se" or args[0] == ":searchengine"))
	)
	{
		haku::list::mapStrStr::iterator it = this->mApp->bookmark_list.begin(), itend = this->mApp->bookmark_list.end();
		
		while ( it != itend )
		{
			searchVector.push_back(it->first);
			it++;
		}
		
		it = this->mApp->searchengine_list.begin(), itend = this->mApp->searchengine_list.end();
		
		while ( it != itend )
		{
			searchVector.push_back(it->first);
			it++;
		}
	}
	else if ( COMPLETION_USE_WORDLIST )
	{
		searchVector = haku::list::vectorStr_load(this->mApp->configdir+"/completion.txt");		
	}
	
	//~ Compare in lowercase
	arg = haku::string::tolower(arg);
	
	//~ For each possible value, compare...
	for ( i = 0 ; i < searchVector.size() ; i++ )
	{		
		item.distance = haku::jaroWinkler(arg, haku::string::tolower(searchVector[i]));
		item.string = searchVector[i];
		
		//~ minimal score to be selected
		if ( item.distance > COMPLETION_MINIMAL_DISTANCE ) 
			this->mMatch.insert(item);
	}
	
	//~ set the iterator(bestMatch current) to the first element of the new list
	this->mMatchIndex = this->mMatch.rbegin();
}



/**
	\brief Insert chars in a new command line
**/
void cmdline::newline(std::string aInsert)
{	
	//~ new line
	this->mCurrent = this->mList.size();
	this->mOffset = aInsert.size();
	this->mList.push_back(aInsert);
	this->mMatch.clear();
}


/**
	\brief Insert some chars into the current line (at current offset)
**/
void cmdline::insert(std::string aInsert)
{	
	//~ no line -> new line
	if ( this->mCurrent >= this->mList.size() )
	{
		this->newline(aInsert);
	}
	//~ offset is out of limits 
	else if ( this->mOffset > this->mList[ this->mCurrent ].size() )
	{
		throw("mOffset > mList[mCurrent].size()");
	}
	//~ insert
	else
	{
		this->mList[ this->mCurrent ].insert(this->mOffset, aInsert);
		this->mOffset += aInsert.size();
		this->updateBest();
	}
}


/**
	\brief Remove some chars from the current line (starting at current offset)
	\param aSize number of char(s) to remove
**/
void cmdline::remove(signed int aSize)
{	
	//~ there is no current line
	if ( this->mCurrent >= this->mList.size() )
	{
		throw("mCurrent > mList.size()");
	}
	//~ current offset is out of limits
	else if ( this->mOffset > this->mList[ this->mCurrent ].size() )
	{
		throw("mOffset > mList[mCurrent].size()");
	}
	//~ remove
	else
	{
		if ( aSize < 0 )
		{
			//~ reduce aSize in order to not get beyond limits
			unsigned int tmp;
			
			tmp = this->mOffset;
			this->move(aSize);
			tmp -= this->mOffset;
			aSize = tmp;
		}
		
		this->mList[ this->mCurrent ].erase(this->mOffset, aSize);
		this->updateBest();
	}
}

/**
	\brief Move into the current line
	\param aMove move the cursor for aMove number of char
**/
void cmdline::move(signed int aMove)
{	
	if ( this->mCurrent >= this->mList.size() )
	{
		throw("mCurrent > mList.size()");
	}
	else if ( (unsigned)(this->mOffset+aMove) > this->mList[ this->mCurrent ].size() )
	{
		if ( aMove < 0 ) this->mOffset = 1;
		else this->mOffset = this->mList[ this->mCurrent ].size();
	}
	else 
	{
		this->mOffset += aMove;
		this->updateBest();
		
		// the minimal offset is 1 because the first char ':' or '/' is not modifiable
		if ( this->mOffset == 0 )
			this->mOffset = 1;
	}
}

/**
	\brief Move into the command line history
	\param aMove number of line to go back/forward
**/
void cmdline::history(signed int aMove)
{		
	if ( (unsigned)(this->mCurrent+aMove) >= this->mList.size() )
	{
		//~ if it get beyond limits
		//~ don't do a damn thing
	}
	else
	{
		this->mCurrent += aMove;
		this->mOffset = this->mList[ this->mCurrent ].size();
		this->updateBest();
	}
}
