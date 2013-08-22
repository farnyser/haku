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

#include "interpreter.hpp"
#include "config.hpp"
#include "help.h"
#include <cstring>

namespace interpreter
{
	/**
		\brief Execute a command or list of commands
		\param p Pointer to either the application or the current window or NULL.
		\param context Execution context.
		\param cmd Command line(s).
		\return depend on the command line
	**/
	int execcmd(exec_context context, std::string cmd, void *p)
	{
		//~ return value,		
		//~ used to ask the window to redraw its content, for example
		int return_value = return_void;
		
		//~ some useful pointers
		application *app;
		app_window *w;
		
		//~ if cmd is empty, return an error
		if ( cmd.size() == 0 ) return interpreter::return_error;		
		
		//~ get w or app by casting p
		if ( context == exec_context_commandline or context == exec_context_callback )
		{
			w = (app_window*)p;
			w->osl = (context == exec_context_callback);
			app = w->app;
		}
		else
		{
			w = NULL;
			app = (application*)p;
		}
	
	
		//~ First step: explode the cmd using \n as a separator
		//~ and then, execcmd on each part

		if ( cmd.find('\n') != std::string::npos )
		{
			std::vector<std::string> arg ( haku::string::explode( haku::string::cleanWhitespace(cmd), '\n' ) );
			
			for ( unsigned int i = 0 ; i < arg.size() ; i++ ) 
			{
				debug_vv("EXECCMD script: will execute this: " << arg[i]);
				return_value |= interpreter::execcmd(context, arg[i], p);
			}
			
			return return_value;
		}
		
		
		//~ Second step: explode a command into a list of arguments and interpret it
		std::vector<std::string> arg ( haku::string::explode( haku::string::trim( haku::string::cleanWhitespace(cmd)  ) ) );
		//~ std::vector<std::string> arg ( haku::string::explode(cmd) );
		std::string tmp, uri;
		size_t offset;
		
		//~ Use the interpreter's map to execute the command
		for ( int i = 0 ; interpreter::map[i].name != "" ; i++ )
		{
			if ( interpreter::map[i].name == arg[0] )
			{
				debug_vv("interpreter :: found one command matching input : " << interpreter::map[i].name );
			
				if ( interpreter::map[i].function != NULL )
				{
					debug_vv("interpreter :: callback function" );
					
					return_value |= (interpreter::map[i].function)(app,w,context,arg);
				}
				//~ There is a script associated to the command
				//~ Execute it after prsing some argument like %uri, %clipboard...
				if ( interpreter::map[i].script != "" )
				{
					debug_vv("interpreter :: callback script : " <<interpreter::map[i].script  );
					
					tmp = interpreter::map[i].script;
					uri = "";
					
					if ( w != NULL ) uri = w->geturi();
					
					while ( tmp.find("%uri") != std::string::npos )
						tmp.replace(tmp.find("%uri"),4,uri);
					while ( (offset = tmp.find("%Xclipboard")) != std::string::npos )
						tmp.replace(offset,11, application::getclipboard(GDK_SELECTION_PRIMARY) );
					while ( (offset = tmp.find("%clipboard")) != std::string::npos )
						tmp.replace(offset,10, application::getclipboard(GDK_SELECTION_CLIPBOARD) );
					
					for ( unsigned int j = 1 ; j < 10 ; j++ )
					{
						while ( (offset = tmp.find(std::string("%")+(char)('1'-1+j))) != std::string::npos )
						{
							debug_vv("interpreter :: argument %" << (char)('1'-1+j) <<" found at offset "<< offset );							
							
							if ( j < arg.size() )
								tmp.replace(offset, 2, arg[j]);
							else
								tmp.replace(offset, 2, "");
						}
					}
					
					debug_v("interpreter :: will execute : " << tmp );
					
					return_value |= interpreter::execcmd(context,tmp,p);
				}
			}
		}
		
		return return_value;
	}
	

	/**
		\brief For each window, do ...
		\param app Pointer to the application object (or NULL).
		\param w Pointer to the current window object (or NULL).
		\param context Execution context.
		\param arg List of arguments.
		\return return_window_deleted which stop "after process" execution in the app->execcmd function
		\return return_sync_request when this is the first call for the specified command
	**/
	int foreach(application *app, app_window *w, const exec_context context, std::vector<std::string> arg)
	{
		//~ unused parameters
		(void) w;
		
		arg.erase( arg.begin() );
		app->execcmd(exec_context_callback, haku::string::implode(arg,' '), NULL);
		
		// "return_window_deleted" to stop after_process (fixme, this is a hack) 
		if ( context != exec_context_synchronisation )
			return return_sync_request | return_window_deleted;
		else
			return return_window_deleted;		
	}	

	/**
		\brief Print current page
		\param app Pointer to the application object (or NULL).
		\param w Pointer to the current window object (or NULL).
		\param context Execution context.
		\param arg List of arguments.
		\return return_newline_request (do not keep :print in the commandline)
		\return return_error if the context is incorrect
	**/
	int print(application *app, app_window *w, const exec_context context, std::vector<std::string> arg)
	{
		//~ unused parameters
		(void) app;
		
		
		if ( w != NULL )
		{
			w->c_print();
			return return_newline_request;
		}
		else
		{
			return return_error;		
		}	
	}

	/**
		\brief Exec a shell cmd
		\param app Pointer to the application object (or NULL).
		\param w Pointer to the current window object (or NULL).
		\param context Execution context.
		\param arg List of arguments.
		\return return_newline_request (do not keep :print in the commandline)
		\return return_error if the context is incorrect
	**/
	int shexec(application *app, app_window *w, const exec_context context, std::vector<std::string> arg)
	{
		//~ unused parameters
		(void) app;
		
		
		if ( w != NULL )
		{
			std::string tmp;
		
			for ( std::vector<std::string>::iterator it = arg.begin() ; it != arg.end() ; it++ )
			{
				if ( *it == "%uri" )
					tmp += " " + w->geturi();
				else if ( *it == "%localpath" )
					tmp += " " + w->getLatestLocalFilename().substr(7,-1);
				else if ( *it == "%Xclipboard" )
					tmp += " " + application::getclipboard(GDK_SELECTION_PRIMARY);
				else if ( *it == "%clipboard" )
					tmp += " " + application::getclipboard(GDK_SELECTION_CLIPBOARD);					
				else
					tmp += " " + *it;
			}
			
			if ( fork() == 0 ) // fils
			{
				if ( system(tmp.c_str()) )
					exit(EXIT_SUCCESS);
				else
					exit(EXIT_FAILURE);
			}
			
			return return_newline_request;
		}
		else
		{
			return return_error;		
		}	
	}

	/**
		\brief Save the content of the current page.
		\param app Pointer to the application object (or NULL).
		\param w Pointer to the current window object (or NULL).
		\param context Execution context.
		\param arg List of arguments.
		\return return_void on success.
		\return return_error if there is no window or the argument list is badly formed.
	**/
	int save(application *app, app_window *w, const exec_context context, std::vector<std::string> arg)
	{
		unsigned arg_size = arg.size();
		std::string cmd = haku::string::implode(arg,' ');
		
		if ( w == NULL && arg_size >= 2 )
		{
			WebKitNetworkRequest *r = webkit_network_request_new(arg[1].c_str());			
			
			w = app->create_window();
			w->download = webkit_download_new(r);
			w->osl = (context == exec_context_callback);
			
			if ( arg_size >= 3 )
				webkit_download_set_destination_uri(w->download, haku::string::trim(cmd.substr(arg[0].size()+2+arg[1].size(), -1)).c_str());
			
			app_window::initdownload(NULL, w->download, w);
			
			return return_void;
		}
		else if ( w != NULL )
		{
			WebKitNetworkRequest *r = webkit_network_request_new(w->geturi().c_str());			
			WebKitDownload *o = webkit_download_new(r);
			
			if ( arg_size >= 2 )
				webkit_download_set_destination_uri(o, haku::string::trim(cmd.substr(arg[0].size(), -1)).c_str());
			
			app_window::initdownload(NULL, o, w);
			
			return return_void;
		}
		
		return return_error;
	}



	/**
		\brief SearchEngine Add or Del.
		\param app Pointer to the application object (or NULL).
		\param w Pointer to the current window object (or NULL).
		\param context Execution context.
		\param arg List of arguments.
		\return return_newline_request|return_update_request to ask the window to redraw its content, and re-enter command mod.
		\return return_error if there is something wrong.
	**/
	int searchengine(application *app, app_window *w, const exec_context context, std::vector<std::string> arg)
	{
		unsigned arg_size = arg.size();
		
		if ( arg_size >= 3 && arg[1] == "add" )
		{
			if ( arg_size == 3 && w != NULL ) arg.push_back( w->geturi() );
			else if ( arg_size == 3 ) return return_error;
			
			app->searchengine_list = haku::list::mapStrStr_load(app->configdir+"/searchengine.txt");
			app->searchengine_list[arg[2]] = arg[3];
			haku::list::mapStrStr_save(app->searchengine_list, app->configdir+"/searchengine.txt");

			if ( app->searchengine_list.find("default") == app->searchengine_list.end() ) 
				app->searchengine_list["default"] = DEFAULT_SEARCHENGINE;
			
			debug_v("searchEngine add " << arg[2] << " => " << arg[3])
			
			if ( context != exec_context_synchronisation )
				app->sync_request(arg[0] + " "  + arg[1] + " "  + arg[2] + " "  + arg[3]);
		}	
		else if ( arg_size >= 2 && arg[1] == "del" )
		{
			app->searchengine_list = haku::list::mapStrStr_load(app->configdir+"/searchengine.txt");
			haku::list::mapStrStr::iterator ifind = app->searchengine_list.find(arg[2]);
			
			if ( ifind != app->searchengine_list.end() )
			{
				app->searchengine_list.erase(ifind);
				haku::list::mapStrStr_save(app->searchengine_list, app->configdir+"/searchengine.txt");
				
				debug_v("searchEngine del " << arg[2])
				
				if ( context != exec_context_synchronisation )
					app->sync_request(arg[0] + " "  + arg[1] + " "  + arg[2]);
			}
			
			if ( app->searchengine_list.find("default") == app->searchengine_list.end() ) 
				app->searchengine_list["default"] = DEFAULT_SEARCHENGINE;
		}
		else if ( arg_size >= 2 && arg[1] == "show" )
		{
			if ( w == NULL )  
				w = app->create_window();
		
			w->sethtml("searchengine", haku::list::toHTML("searchengine", app->searchengine_list));
		}		
		else
		{
			return return_error;
		}
		
		return return_newline_request|return_update_request;
	}
	
	/**
		\brief BookMark Add or Del.
		\param app Pointer to the application object (or NULL).
		\param w Pointer to the current window object (or NULL).
		\param context Execution context.
		\param arg List of arguments.
		\return return_newline_request|return_update_request to ask the window to redraw its content, and re-enter command mod.
		\return return_error if there is something wrong.
	**/
	int bookmark(application *app, app_window *w, const exec_context context, std::vector<std::string> arg)
	{
		int arg_size = arg.size();
		
		if ( arg_size >= 3 && arg[1] == "add" )
		{
			if ( arg_size == 3 && w != NULL ) arg.push_back( w->geturi() );
			else if ( arg_size == 3 ) return return_error;
			
			app->bookmark_list = haku::list::mapStrStr_load(app->configdir+"/bookmark.txt");
			app->bookmark_list[arg[2]] = arg[3];
			
			haku::list::mapStrStr_save(app->bookmark_list, app->configdir+"/bookmark.txt");
			
			if ( app->bookmark_list.find("default") == app->bookmark_list.end() ) 
				app->bookmark_list["default"] = DEFAULT_HOMEPAGE;
			
			debug_v("bookmark add " << arg[2] << " => " << arg[3])
			
			if ( context != exec_context_synchronisation )
				app->sync_request(arg[0] + " "  + arg[1] + " "  + arg[2] + " "  + arg[3]);
		}	
		else if ( arg_size >= 2 && arg[1] == "del" )
		{
			app->bookmark_list = haku::list::mapStrStr_load(app->configdir+"/bookmark.txt");
			haku::list::mapStrStr::iterator ifind = app->bookmark_list.find(arg[2]);
			
			if ( ifind != app->bookmark_list.end() )
			{
				app->bookmark_list.erase(ifind);
				haku::list::mapStrStr_save(app->bookmark_list, app->configdir+"/bookmark.txt");
				
				if ( app->bookmark_list.find("default") == app->bookmark_list.end() ) 
					app->bookmark_list["default"] = DEFAULT_HOMEPAGE;

				debug_v("bookmark del " << arg[2])
				
				if ( context != exec_context_synchronisation )
					app->sync_request(arg[0] + " "  + arg[1] + " "  + arg[2]);
			}
		}
		else if ( arg_size >= 2 && arg[1] == "show" )
		{
			if ( w == NULL )  
				w = app->create_window();
			
			w->sethtml("bookmark", haku::list::toHTML("bookmark", app->bookmark_list));
		}
		else
		{
			return return_error;
		}
		
		return return_newline_request|return_update_request;
	}
	
	/**
		\brief Session save, restore, clean.
		\param app Pointer to the application object (or NULL).
		\param w Pointer to the current window object (or NULL).
		\param context Execution context.
		\param arg List of arguments.
		\return return_newline_request|return_update_request to ask the window to redraw its content, and re-enter command mod.
		\return return_error if there is something wrong.
	**/
	int session(application *app, app_window *w, const exec_context context, std::vector<std::string> arg)
	{
		int arg_size = arg.size();
		
		//~ FIX ME
		if ( (arg_size == 2 or arg_size == 3) && arg[1] == "save" )
		{
			if ( arg_size < 3 )
					arg.push_back("default");
			
			if ( context == exec_context_synchronisation ) 
			{
				execcmd(context,"foreach session_add" ,app);
			}
			else
			{
				app->session_save(arg[2]);
				return return_newline_request|return_sync_request;
			}
		}
		else if ( (arg_size == 2 or arg_size == 3) && arg[1] == "restore" )
		{
			if ( arg_size == 3 )
				app->session_restore(arg[2]);
			else
				app->session_restore("default");
		}		
		else if ( (arg_size == 2 or arg_size == 3) && arg[1] == "clean" )
		{
			if ( arg_size == 3 )
				app->session_delete(arg[2]);
			else
				app->session_delete("default");				
		}
		else if ( arg_size >= 2 && arg[1] == "show" )
		{
			if ( arg_size == 2 )
				arg.push_back("default");
			
			haku::list::vectorStr sessionList = haku::list::vectorStr_load(app->configdir + "/session/" + arg[2] + ".txt");
			
			if ( w == NULL )  
				w = app->create_window();
			
			w->sethtml("session", haku::list::toHTML("session_content", sessionList));
		}
		else
		{
			return return_error;
		}
		
		return return_newline_request|return_update_request;
	}
		
	/**
		\brief Change uri specific setting (javascipt,plugins,cookies,useragent,adblock).
		\param app Pointer to the application object (or NULL).
		\param w Pointer to the current window object (or NULL).
		\param context Execution context.
		\param arg List of arguments.
		\return return_void on success.
		\return return_reload_request if needed, to force the page to reload its content.
		\return return_error if there is no window or the argument list is badly formed.
	**/
	int set(application *app, app_window *w, const exec_context context, std::vector<std::string> arg)
	{
		int arg_size = arg.size();
		
		//~ This is a "shell" query, broadcast it
		if ( w == NULL && context == exec_context_shell && arg_size >= 4 ) 
		{
			return return_sync_request;
		}
		//~ This is a sync request, set setting in the current application instance
		else if ( w == NULL && context == exec_context_synchronisation && arg_size == 4 ) 
		{
			if ( arg[1] == "plugins" )
				app->session_plugins_list[ haku::tools::base_uri(arg[3]) ] = (arg[2] == "on"); 
			if ( arg[1] == "js" or arg[1] == "javascript" )
				app->session_javascript_list[ haku::tools::base_uri(arg[3]) ] = (arg[2] == "on"); 
			if ( arg[1] == "cookies" )
				app->session_cookies_list[ haku::tools::base_domain(arg[3]) ] = (arg[2] == "on");
			if ( arg[1] == "useragent" )
				app->session_useragent_list[ haku::tools::base_domain(arg[3]) ] = arg[2];
		
			return return_void;
		}
		//~ Commandline input
		else if ( w != NULL )
		{
			if ( arg_size >= 4 && ((arg[2] == "on" or arg[2] == "off") or arg[1] == "useragent") && arg[3] == "always" )
			{
				if ( arg_size == 4 ) arg.push_back( w->geturi() );
				if ( arg[1] != "useragent" )
					w->c_set(arg[1], arg[4], arg[2] == "on", arg[3] == "always");
				else
					w->c_set(arg[1], arg[4], arg[2], arg[3] == "always");				
				app->sync_request(arg[0] + " "  + arg[1] + " "  + arg[2] + " "  + arg[3] + " "  + arg[4]);
				
				if ( arg[1] == "cookies"  or arg[1] == "useragent"  ) return return_reload_clean_request;
				return return_reload_request;
			}			
			else if ( arg_size >= 3 && (arg[2] == "off" or arg[2] == "on" or arg[1] == "useragent") )
			{
				if ( arg_size == 3 ) arg.push_back( w->geturi() );
				if ( arg[1] != "useragent" )
					w->c_set(arg[1], arg[3], arg[2] == "on");
				else
					w->c_set(arg[1], arg[3], arg[2]);
				
				app->sync_request(arg[0] + " "  + arg[1] + " "  + arg[2] + " "  + arg[3]);
				
				if ( arg[1] == "cookies" or arg[1] == "useragent" ) return return_reload_clean_request;
				return return_reload_request;
			}
		}
		
		return return_error;
	}
	
	/**
		\brief Show help in the current page (or a new one).
		\param app Pointer to the application object (or NULL).
		\param w Pointer to the current window object (or NULL).
		\param context Execution context.
		\param arg List of arguments.
		\return return_void.
	**/
	int help(application *app, app_window *w, const exec_context context, std::vector<std::string> arg)
	{
		//~ unused parameters
		(void)context; (void)arg; 
		
		if ( w == NULL )  
			w = app->create_window();
		
		w->sethtml("help", (char*)help_html);
		
		return return_void;
	}
	
	/**
		\brief Show version informations in the current page (or a new one).
		\param app Pointer to the application object (or NULL).
		\param w Pointer to the current window object (or NULL).
		\param context Execution context.
		\param arg List of arguments.
		\return return_void.
	**/
	int about(application *app, app_window *w, const exec_context context, std::vector<std::string> arg)
	{
		//~ unused parameters
		(void) context; (void)arg; (void)app;
		
		std::string html;
		html += "This is <b>" APP_NAME "</b> ";
		html += "<br /> ";
		html += "Version " APP_VERSION;
		html += "<br /> ";
		html += "Homepage <a href=\"" APP_HOMEPAGE "\">" APP_HOMEPAGE "</a>";
		
		if ( w == NULL )  
			w = app->create_window();
		
		w->sethtml("about", html);
		
		return return_void;
	}
	
	/**
		\brief Reload the page without cache.
		\param app Pointer to the application object (or NULL).
		\param w Pointer to the current window object (or NULL).
		\param context Execution context.
		\param arg List of arguments.
		\return return_reload_clean_request.
	**/
	int reload(application *app, app_window *w, const exec_context context, std::vector<std::string> arg)
	{
		//~ unused parameters
		(void) w; (void) context; (void)arg; (void)app;
		
		return return_reload_clean_request;
	}
	
	/**
		\brief Stop loading.
		\param w Pointer to the window to stop.
		\return return_void if there is a window.
		\return return_error if the w pointer is NULL.
	**/	
	int stop(application *app, app_window *w, const exec_context context, std::vector<std::string> arg)
	{
		//~ unused parameters
		(void)context; (void)arg; (void)app;
		
		if ( w !=  NULL )
		{
			w->c_stop();
			return return_void;
		}
		
		return return_error;
	}
	
	/**
		\brief Navigate into the history.
		\param w Pointer to the window concerned.
		\return return_void if everything is fine.
		\return return_error if the w pointer is NULL or the argument list incorrect.
	**/	
	int navigate(application *app, app_window *w, const exec_context context, std::vector<std::string> arg)
	{
		//~ unused parameters
		(void) context; (void)arg; (void)app;
		
		if ( w !=  NULL && arg.size() > 1 )
		{
			w->c_navigate( atoi(arg[1].c_str()) );
			return return_void;
		}
		
		return return_error;
	}
	
	/**
		\brief Change the window exec mode.
		\param app Pointer to the application object (or NULL).
		\param w Pointer to the current window object (or NULL).
		\param context Execution context.
		\param arg List of arguments.
		\return return_update_request : the window is asked to redraw its decorations.
		\return return_error if the execution context or the argument list is incorrect.
	**/
	int setmode(application *app, app_window *w, const exec_context context, std::vector<std::string> arg)
	{
		//~ unused parameters
		(void) app; (void) context; (void)arg; 
		std::string cmd;
	
		if ( w != NULL and arg.size() > 1 )
		{
			if ( arg[1] == "insert" )
			{
				debug_vv("setmode insert");
				w->setmode(mode_insert);
			}
			if ( arg[1] == "command" )
			{
				debug_vv("setmode command");
				w->setmode(mode_command);
			}
			if ( arg[1] == "commandline" )
			{
				debug_vv("setmode commandline");
				w->setmode(mode_commandline);
				
				if ( arg.size() > 2 )
				{
					
					for ( unsigned int i = 2 ; i < arg.size() ; i++ )
					{
						if ( i > 2 ) { cmd += " "; }
						
						if ( arg[i] != "%empty" )
							cmd += arg[i];
					}
					
					w->setcmdline(cmd);
				}
			}
			
			return return_update_request;
		}	
		
		return return_error;
	}
	
	/**
		\brief Get into/out of source display mode.
		\param app Pointer to the application object (or NULL).
		\param w Pointer to the current window object (or NULL).
		\param context Execution context.
		\param arg List of arguments.
		\return return_reload_request to tell the window to refresh its content.
		\return return_error if there is no window.
	**/
	int source(application *app, app_window *w, const exec_context context, std::vector<std::string> arg)
	{
		//~ unused parameters
		(void) app; (void) context; (void)arg; 
		
		if ( w != NULL )
		{
			w->c_source();
			return return_reload_request;
		}
		
		return return_error;
	}	

	/**
		\brief Close a window.
		\param app Pointer to the application object (or NULL).
		\param w Pointer to the current window object (or NULL).
		\param context Execution context.
		\param arg List of arguments.
		\return return_window_deleted which means that the pointer to the window should not be used anymore.
		\return return_error if there is no window to close.
	**/
	int quit(application *app, app_window *w, const exec_context context, std::vector<std::string> arg)
	{
		//~ unused parameters
		(void) context; (void)arg; 
		
		if ( w != NULL )
		{
			app->remove_window(&w);
			return return_window_deleted;
		}
		
		return return_error;
	}
	
	/**
		\brief Open an URI in a new window (within a new process).
		\param app Pointer to the application object (or NULL).
		\param w Pointer to the current window object (or NULL).
		\param context Execution context.
		\param arg List of arguments.
		\return return_newline_request
	**/
	int open_newpid(application *app, app_window *w, const exec_context context, std::vector<std::string> arg)
	{
		std::string tmp = app->binarypath + " -e open";
		unsigned int i;
		
		for ( i = 1 ; i < arg.size() ; i++ )
			tmp += " " + arg[i];
		
		if ( fork() == 0 ) // fils
		{
			std::vector<std::string> tmp_vec = haku::string::explode(tmp);
			char ** array = new char *[tmp_vec.size()+1];
			for (i = 0; i < tmp_vec.size(); i++)
			{
				array[i] = new char[tmp_vec.at(i).size()];		
				strcpy(array[i], tmp_vec.at(i).c_str());		
			}
			array[tmp_vec.size()] = NULL;
			
			execvp(app->binarypath.c_str(), array);
			exit(EXIT_FAILURE);
		}
		
		return return_newline_request;
	}
	
	/**
		\brief Open an URI in a new window.
		\param app Pointer to the application object (or NULL).
		\param w Pointer to the current window object (or NULL).
		\param context Execution context.
		\param arg List of arguments.
		\return return_newline_request | return_void
		\return return_newline_request | return_error if the execution context or the argument list is incorrect.
	**/
	int open_new(application *app, app_window *w, const exec_context context, std::vector<std::string> arg)
	{
		w = app->create_window();
		return return_newline_request|interpreter::open(app,w,context,arg);
	}
	
	/**
		\brief  Open an URI in the current window.
		\param app Pointer to the application object (or NULL).
		\param w Pointer to the current window object (or NULL).
		\param context Execution context.
		\param arg List of arguments.
		\return return_void
		\return return_error if the execution context or the argument list is incorrect.
	**/
	int open(application *app, app_window *w, const exec_context context, std::vector<std::string> arg)
	{
		//~ (may be) unused parameters
		(void) context;
		
		unsigned int  arg_size = arg.size();
		
		if ( w != NULL )
		{
			if ( arg_size == 1 )
			{
				//~ Ouverture homepage
				w->seturi(app->parseuri("default"));
			}
			else if ( arg_size >= 2 )
			{
				//~ Parsage de l'url
				std::string uri = "";
				
				for ( unsigned int i = 1 ; i < arg_size ; i++ )
					uri += arg[i] + " ";
				
				w->seturi(  app->parseuri(uri) );
			}			
			
			return return_void;
		}
		else
		{
			return interpreter::open_new(app, NULL, context,arg);
		}
		
		return return_error;
	}
	
	/**
		\brief  Zoom or unzoom the current view.
		\param app Pointer to the application object (or NULL).
		\param w Pointer to the current window object (or NULL).
		\param context Execution context.
		\param arg List of arguments.
		\return return_void
		\return return_error if the execution context or the argument list is incorrect.
	**/
	int zoom(application *app, app_window *w, const exec_context context, std::vector<std::string> arg)
	{
		//~ unused parameters
		(void) context; (void)app;
		
		if ( w != NULL && arg.size() > 1 )
		{
			if ( arg[1] == "in" )
				w->c_zoom(1);
			else if ( arg[1] == "out" )
				w->c_zoom(-1);
			else if ( arg[1] == "off" )
				w->c_zoom(0);
			else
				return return_error;
			
			return return_void;
		}
	
		return return_error;
	}
	
	/**
		\brief  Scroll a webview either horizontally or vertically .
		\param app Pointer to the application object (or NULL).
		\param w Pointer to the current window object (or NULL).
		\param context Execution context.
		\param arg List of arguments.
		\return return_void
		\return return_error if the execution context or the argument list is incorrect.
	**/
	int scroll(application *app, app_window *w, const exec_context context, std::vector<std::string> arg)
	{
		//~ unused parameters
		(void) context; (void)app;
		
		if ( w != NULL && arg.size() > 2 )
		{
			if ( arg[1] == "horizontal" )
				w->c_hscroll( atoi(arg[2].c_str()) );
			else if ( arg[1] == "vertical" )
				w->c_vscroll( atoi(arg[2].c_str()) );
			else
				return return_error;
			
			return return_void;
		}
	
		return return_error;
	}
	
	/**
		\brief  Save current URI into the specified or default session file.
		\param app Pointer to the application object (or NULL).
		\param w Pointer to the current window object (or NULL).
		\param context Execution context.
		\param arg List of arguments.
		\return return_void
		\return return_error if the execution context or the argument list is incorrect.
	**/
	int session_add(application *app, app_window *w, const exec_context context, std::vector<std::string> arg)
	{
		//~ unused parameters
		(void) app; (void) context;
		
		if ( w != NULL )
		{
			if ( arg.size() < 1 ) arg.push_back("");
			if ( arg.size() < 2 ) arg.push_back("default");
			
			w->app->session_add(arg[1],w->geturi());
			return return_void;
		}
		
		return return_error;
	}
	
	/**
		\brief  Copy some text to the X Window Selection Clipboad.
		\param app Pointer to the application object (or NULL).
		\param w Pointer to the current window object (or NULL).
		\param context Execution context.
		\param arg List of arguments.
		\return return_void
	**/
	int yank(application *app, app_window *w, const exec_context context, std::vector<std::string> arg)
	{
		//~ unused parameters
		(void) context; (void) w; (void) app;
		
		std::string tmp = "";
		
		for ( unsigned int i = 1 ; i < arg.size() ; i++ )
			tmp += arg[i] + " ";
		
		application::setclipboard(GDK_SELECTION_CLIPBOARD,tmp);
		return return_void;
	}
}
