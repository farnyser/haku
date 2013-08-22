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

#ifndef __INTERPRETER_HPP__
#define __INTERPRETER_HPP__

	#include <map>
	#include <vector>
	#include <string>
	#include "tools/tools.hpp"
	#include "tools/string.hpp"
	#include "app_window.hpp"
	#include "application.hpp"
	
	/// \namespace interpreter
	/// \brief Namespace for all functions used to interpret commands from the commandline.
	namespace interpreter
	{
		/// \enum return_t
		/// \brief This type is used in return value for all interpreter's callback functions.
		/// Multiple values can be returned using the | as a separator. 
		/// This is usefull when more than one callback is used.
		enum return_t 
		{
			return_void = 0, 								/// Default return value
			return_error = 1 << 0,							/// An error occured
			return_update_request = 1 << 1,					/// The window should be redrawn
			return_reload_request = 1 << 2,					/// The window content should be reload
			return_reload_clean_request = 1 << 3,			/// The window content should be reload without using the cache
			return_sync_request = 1 << 4,					/// The command should be shared to all  Haku's process
			return_newline_request = 1 << 5,				/// The commandline should be closed
			return_window_deleted = 1 << 6					/// The window has been ddeleted, do not use the pointer again
		};
		
		/// \struct map_t
		/// \brief This structure is used to link a command name to a callback.
		typedef struct {
			/// The command name
			std::string name;
			/// The callback function to call (or NULL)
			int (*function)(application*,app_window*,const exec_context,std::vector<std::string>);
			/// The script to execute (or an empty line)
			std::string script;
		} map_t;
		
		int foreach(application *app, app_window *w, const exec_context context, std::vector<std::string> arg);
		int print(application *app, app_window *w, const exec_context context, std::vector<std::string> arg);
		int shexec(application *app, app_window *w, const exec_context context, std::vector<std::string> arg);
		int save(application *app, app_window *w, const exec_context context, std::vector<std::string> arg);
		int session(application *app, app_window *w, const exec_context context, std::vector<std::string> arg);
		int setmode(application *app, app_window *w, const exec_context context, std::vector<std::string> arg);
		int open(application *app, app_window *w, const exec_context context, std::vector<std::string> arg);
		int open_new(application *app, app_window *w, const exec_context context, std::vector<std::string> arg);
		int open_newpid(application *app, app_window *w, const exec_context context, std::vector<std::string> arg);
		int session_add(application *app, app_window *w, const exec_context context, std::vector<std::string> arg);
		int source(application *app, app_window *w, const exec_context context, std::vector<std::string> arg);
		int reload(application *app, app_window *w, const exec_context context, std::vector<std::string> arg);
		int stop(application *app, app_window *w, const exec_context context, std::vector<std::string> arg);
		int quit(application *app, app_window *w, const exec_context context, std::vector<std::string> arg);
		int navigate(application *app, app_window *w, const exec_context context, std::vector<std::string> arg);
		int zoom(application *app, app_window *w, const exec_context context, std::vector<std::string> arg);
		int scroll(application *app, app_window *w, const exec_context context, std::vector<std::string> arg);
		int yank(application *app, app_window *w, const exec_context context, std::vector<std::string> arg);
		int help(application *app, app_window *w, const exec_context context, std::vector<std::string> arg);
		int about(application *app, app_window *w, const exec_context context, std::vector<std::string> arg);
		int set(application *app, app_window *w, const exec_context context, std::vector<std::string> arg);
		int bookmark(application *app, app_window *w, const exec_context context, std::vector<std::string> arg);
		int searchengine(application *app, app_window *w, const exec_context context, std::vector<std::string> arg);
		
		int execcmd(exec_context, std::string cmd, void *p);
	}

#endif // __INTERPRETER_HPP__
