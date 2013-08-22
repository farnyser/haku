#ifndef __APPLICATION_HPP__
#define __APPLICATION_HPP__

	#include <map>
	#include <vector>
	#include <string>
	#include <fstream>
	#include <cstring>
	
	#include <sys/ipc.h>
	#include <sys/shm.h>
	#include <errno.h>
	
	#include "app_window.hpp"
	#include "tools/string.hpp"
	#include "tools/list.hpp"
	
	#include <JavaScriptCore/JavaScript.h>
	
	/// \enum exec_context
	/// \brief Exec_context is used to describe the context from which a command come from.
	/// It is damn usefull to know how to react to the command.
	enum exec_context 
	{ 
		exec_context_commandline,		/// Commandline
		exec_context_synchronisation, 	/// Synchronisation request
		exec_context_shell, 					/// From a shell execution (haku -e cmd)
		exec_context_callback					/// From another event (keyboard event...)
	};

	class app_window;
	class application
	{
		private:
			std::vector<app_window*> windows_list;
			void* sharedMemoryAddr;
			int sharedMemoryId;
		
		public:	
			haku::list::mapStrStr bookmark_list;
			haku::list::mapStrStr searchengine_list;
			haku::list::mapStrBool javascript_list;
			haku::list::mapStrBool plugins_list;
			haku::list::mapStrBool cookies_list;
			haku::list::mapStrStr useragent_list;
			haku::list::mapStrBool adblock_list;
			haku::list::mapStrBool session_javascript_list;
			haku::list::mapStrBool session_plugins_list;
			haku::list::mapStrBool session_cookies_list;
			haku::list::mapStrBool session_adblock_list;
			haku::list::mapStrStr session_useragent_list;
		
			//~ #if ADBLOCK
				std::vector<std::string> adblock_parser_list;
			//~ #endif
		
			bool lockcookie; 
			SoupCookieJar *cookies;
			SoupSession *session;
		
		public:
			std::string downloaddir, configdir;
			std::string binarypath;

		private: application(const application&) {};
		private: application& operator=(const application&) { return *this; };
		
		public:
			application(std::string,std::string,std::string);
			void mainloop();
			
			app_window* create_window();
			int remove_window(app_window**);
			
			std::string parseuri(std::string);
		
			void loaduri(std::string uri);
			void execcmd(exec_context, std::string cmd, void *p);
			void session_restore(std::string name);
			void session_save(std::string name);
			void session_delete(std::string name);
			void session_add(std::string name, std::string uri);
			void session_remove(std::string name, std::string uri);
			haku::list::vectorStr session_getList(void);
		
			bool get_plugins_setting(std::string uri);
			bool get_javascript_setting(std::string uri);
			bool get_cookies_setting(std::string uri);
			bool get_adblock_setting(std::string uri);
			const std::string get_useragent_setting(std::string uri);
		
			void reloadcookies();
			void sync_request(const std::string&);
			
			static void setclipboard(GdkAtom, const std::string&);
			static std::string getclipboard(GdkAtom);
			static gboolean sync(void *app);
			static void changecookie(SoupCookieJar *j, SoupCookie *oc, SoupCookie *c, application *app) ;
	};

#endif // __APPLICATION_HPP__
