#ifndef __APP_WINDOW_HPP__
#define __APP_WINDOW_HPP__

	#include <cstdlib>
	#include <iostream>
	#include <vector>
	#include <string>
	#include <sstream>

	#include <gtk/gtk.h>
	#include <gdk/gdkkeys.h>
	#include <gdk/gdkkeysyms.h>
	#include <webkit/webkit.h>
	#include <JavaScriptCore/JavaScript.h>

	#include "application.hpp"
	#include "cmdline.hpp"

	#ifdef OPEN_MAGNET
	        #include <unistd.h>
	#endif

	class application;
	class cmdline;
	
	enum app_mode { mode_null, mode_all, mode_command, mode_insert, mode_commandline};
	
	class app_window
	{
		public:
			application* app;
			WebKitDownload *download;
			bool osl;
					
		private:
			cmdline* cmd;
		
			std::string title, uri, baseuri, linkh, download_as, latest_download_as;
			gint progress; // 0 à 100
			app_mode mode;
			bool zoomed;
			bool trusted;
		
			GtkWidget *win;
			GtkWidget *indicator;
			GtkWidget *vbox;
			GtkWidget *scroll;
		
			WebKitWebSettings *settings;
		
		public:
			WebKitWebView *view;
		
		private:
			PangoLayout  *indicator_pl;
		
		// callback 
		public:
			static WebKitWebView* createwindow(WebKitWebView*, WebKitWebFrame*, app_window*) ;
			static void linkhover(WebKitWebView *v, const char* t, const char* l, app_window*) ;
			static void loadstat(WebKitWebView *view, WebKitWebFrame *f, app_window *w);
			static void progresschange(WebKitWebView *v, gint p, app_window *);
			static void resize(GtkWidget* widget, GtkAllocation *a, app_window *);
			static void titlechange(WebKitWebView *v, GParamSpec *, app_window *);
			static void destroy(GtkWidget* widget, app_window *);
			static void objectcleared(GtkWidget *widget, WebKitWebFrame *frame, JSContextRef js, JSObjectRef win, app_window *w); 
			static gboolean decidedownload(WebKitWebView *v, WebKitWebFrame *f, WebKitNetworkRequest *r, gchar *m,  WebKitWebPolicyDecision *p, app_window *);
			static gboolean decidewindow(WebKitWebView *v, WebKitWebFrame *f, WebKitNetworkRequest *r, WebKitWebNavigationAction *n, WebKitWebPolicyDecision *p, app_window *);
			static gboolean initdownload(WebKitWebView *view, WebKitDownload *o, app_window *w);
			static void updatedownload(WebKitDownload *o, GParamSpec *pspec, app_window *w);
			static gboolean keypress(GtkWidget*, GdkEventKey *, app_window *w);
			static gboolean exposeindicator(GtkWidget *, GdkEventExpose *, app_window *w);
			static void resource_request(WebKitWebView         *webView,
                                                        WebKitWebFrame        *web_frame,
                                                        WebKitWebResource     *web_resource,
                                                        WebKitNetworkRequest  *request,
                                                        WebKitNetworkResponse *response, app_window *w);

			static gboolean clicked(GtkWidget *widget, GdkEventButton *evb, app_window *w);

		private: app_window(const app_window&) {};
		private: app_window& operator=(const app_window&) { return *this; };
			
		public:
			app_window(application* app);
			~app_window();
		
			//~ obtention donnée
			std::string geturi();
			std::string gettitle();
			const std::string getLocalFilename();
			const std::string getLatestLocalFilename();
			app_mode getmode();
		
			//~ reglage
			void seturi(const std::string&);
			void settitle(const std::string&);
			void setprogress(int);
			void setmode(app_mode);
			void setcmdline(std::string cmd);
			void sethtml(const std::string &title, const std::string &html);
		
			// reload, page suivante/precedente, imprimer ...
			void c_print();
			void c_stop();
			void c_source();
			void c_zoom(int z);
			void c_find(std::string);
			void c_reload(gboolean nocache);
			void c_navigate(gint steps);
			void c_hscroll(gint steps);
			void c_vscroll(gint steps);
		
			void c_set(const std::string&, const std::string&, bool, bool);
			void c_set(const std::string&, const std::string&, bool);
			void c_set(const std::string&, bool);			
			void c_set(const std::string&, const std::string&,const std::string&, bool);
			void c_set(const std::string&, const std::string&, const std::string&);
			void c_set(const std::string&, const std::string&);
			
			//~ traitement
			void update();
			void close();
		
			//~ widget
			void drawindicator() ;
	};

#endif // __APP_WINDOW_HPP__
