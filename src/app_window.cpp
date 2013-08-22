//: Copyright (C) 2010 Farny Serge
//: contact@scawf.info -- utf8-encode

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


#include "application.hpp"
#include "app_window.hpp"
#include "cmdline.hpp"
#include "tools/tools.hpp"
#include "tools/adblock.hpp"
#include "tools/string.hpp"
#include "tools/list.hpp"
#include "config.hpp"



/*
	Constructeur, initialise/place les widgets
	connect les callback...
*/

app_window::app_window(application* app)
{
	this->app = app; // application mere
	this->cmd = new cmdline(app);
	this->mode = mode_command;
	this->zoomed = false;
	this->osl = false;	
	this->download = NULL;	
	
	
	//~ INTERFACE ---------------------------------
	this->win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_wmclass(GTK_WINDOW(this->win), APP_NAME, APP_NAME);
	gtk_window_set_default_size(GTK_WINDOW(this->win), 800, 600);
	g_signal_connect(G_OBJECT(this->win), "destroy", G_CALLBACK(this->destroy),this);
	g_signal_connect(G_OBJECT(this->win), "key-press-event", G_CALLBACK(this->keypress), this);
	g_signal_connect(G_OBJECT(this->win), "size-allocate", G_CALLBACK(this->resize), this);

	/* VBox */
	this->vbox = gtk_vbox_new(FALSE, 0);

	/* Scrolled Window */
	this->scroll = gtk_scrolled_window_new(NULL, NULL);
	
	#if SHOW_SCROLLBAR
		gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(this->scroll),GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	#else
		gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(this->scroll),GTK_POLICY_NEVER, GTK_POLICY_NEVER);
	#endif
	
	/* Webview */
	this->view = WEBKIT_WEB_VIEW(webkit_web_view_new());  
	this->settings = webkit_web_view_get_settings(this->view);
	
	/* settings */
	g_object_set(G_OBJECT(this->settings), "user-agent", USER_AGENT, NULL);
	g_object_set(G_OBJECT(this->settings), "enable-caret-browsing", TRUE, NULL);
	g_object_set(G_OBJECT(this->settings), "enable-page-cache", OPT_PAGE_CACHE, NULL);
	g_object_set(G_OBJECT(this->settings), "enable-spatial-navigation", OPT_SPATIAL_NAVIGATION, NULL);
	g_object_set(G_OBJECT(this->settings), "tab-key-cycles-through-elements", OPT_TAB_CYCLE, NULL);
	g_object_set(G_OBJECT(this->settings), "auto-load-images", OPT_LOAD_IMAGE, NULL);
	
	#ifdef OPT_USER_STYLESHEET
		g_object_set(G_OBJECT(this->settings), "user-stylesheet-uri", OPT_USER_STYLESHEET, NULL);
	#endif
	
	#ifdef SPELL_CHEKING
		g_object_set(G_OBJECT(this->settings), "enable-spell-checking", true, NULL);
		g_object_set(G_OBJECT(this->settings), "spell-checking-languages", SPELL_CHEKING, NULL);
	#endif
	
	
	/* signaux */
	g_signal_connect(G_OBJECT(this->view), "notify::title", G_CALLBACK(titlechange), this);
	g_signal_connect(G_OBJECT(this->view), "notify::progress", G_CALLBACK(progresschange), this);
	g_signal_connect(G_OBJECT(this->view), "notify::load-status", G_CALLBACK(loadstat), this);
	g_signal_connect(G_OBJECT(this->view), "hovering-over-link", G_CALLBACK(this->linkhover), this);
	g_signal_connect(G_OBJECT(this->view), "create-web-view", G_CALLBACK(this->createwindow), this);
	g_signal_connect(G_OBJECT(this->view), "navigation-policy-decision-requested", G_CALLBACK(this->decidewindow), this);
	g_signal_connect(G_OBJECT(this->view), "mime-type-policy-decision-requested", G_CALLBACK(this->decidedownload), this);
	g_signal_connect(G_OBJECT(this->view), "download-requested", G_CALLBACK(this->initdownload), this);
	g_signal_connect(G_OBJECT(this->view), "window-object-cleared", G_CALLBACK(objectcleared), this);
	g_signal_connect(G_OBJECT(this->view), "button-press-event", G_CALLBACK(clicked), this);
	g_signal_connect(G_OBJECT(this->view), "resource-request-starting", G_CALLBACK(this->resource_request), this);

	/* Indicator */
	this->indicator = gtk_drawing_area_new();
	this->indicator_pl = gtk_widget_create_pango_layout(this->indicator, NULL);
	gtk_widget_set_size_request(this->indicator, 0, 2);
	g_signal_connect (G_OBJECT (this->indicator), "expose_event",G_CALLBACK (this->exposeindicator), this);

	/* Arranging with scrollbar */
	gtk_container_add(GTK_CONTAINER(this->scroll), GTK_WIDGET(this->view));
	gtk_container_add(GTK_CONTAINER(this->win), this->vbox);
	gtk_container_add(GTK_CONTAINER(this->vbox), this->scroll);
	gtk_container_add(GTK_CONTAINER(this->vbox), this->indicator);	

	/* Setup */
	gtk_box_set_child_packing(GTK_BOX(this->vbox), this->indicator, FALSE, FALSE, 0, GTK_PACK_START);
	gtk_box_set_child_packing(GTK_BOX(this->vbox),this->scroll, TRUE, TRUE, 0, GTK_PACK_START);
	gtk_widget_grab_focus(GTK_WIDGET(this->view));
	gtk_widget_show(this->vbox);
	gtk_widget_show(this->indicator);
	gtk_widget_show(this->scroll);
	gtk_widget_show(GTK_WIDGET(this->view));
	gtk_widget_show(this->win);
	
	gdk_window_set_events(GTK_WIDGET(this->win)->window, GDK_ALL_EVENTS_MASK);
	webkit_web_view_set_full_content_zoom(this->view, TRUE);
	
}


/*
	Destructeur
*/

app_window::~app_window()
{
	debug_vv("delete called on window");
	this->close();	
}


/* 
************************************************************************************************
	
	Les callback, en static
	
************************************************************************************************
*/


/*
	Nouvelle fenetre (externe)
*/

WebKitWebView* app_window::createwindow(WebKitWebView  *v, WebKitWebFrame *f, app_window *w)  
{
	//~ unused parameter
	(void)v; (void)f;
	
	app_window *wn = w->app->create_window();
	return wn->view;
}


/*
	Decide si il faut ouvrir une nouvelle fenetre ou charger la page dans la meme fenetre
*/

gboolean app_window::decidedownload(WebKitWebView *v, WebKitWebFrame *f, WebKitNetworkRequest *r, gchar *m,  WebKitWebPolicyDecision *p, app_window *w) 
{
	//~ unused parameter
	(void)f; (void)r; (void)p; (void)w;
	
	//~ on ne peut afficher = on telecharge
	if ( !webkit_web_view_can_show_mime_type(v, m) ) 
	{
		webkit_web_policy_decision_download(p);
		return TRUE;
	}
	
	return FALSE;
}


/*
	Afficher ? normalement? Nouvelle fenetre? ActionSpeciale?
*/

gboolean app_window::decidewindow(WebKitWebView *v, WebKitWebFrame *f, WebKitNetworkRequest *r, WebKitWebNavigationAction *n, WebKitWebPolicyDecision *p, app_window *w) 
{
	//~ unused parameter
	(void)f; (void)v; (void)p; (void)n; (void)w;
	
	std::string uri( webkit_network_request_get_uri(r) );
	
	//~ reglage par url à faire avant de lancer la requete http
	w->c_set("useragent", w->app->get_useragent_setting(uri) );
	
	#if OPEN_SPECIAL_LINK
		//~ s'est déjà appliqué, on ne traite pas 2 fois
		if ( w->osl ) { w->osl = false ; return FALSE; }
	
		//~ vérification de tous les liens spéciaux
		for ( int i = 0 ; open_special_link[i].uri != "" ; i++ )
		{
			debug_vv("testing for special_link: " << open_special_link[i].uri);
			
			if ( 
				( haku::sregex::cmp(open_special_link[i].uri, uri).match )
			)
			{
				std::string tmp(open_special_link[i].cmd);
				
				if ( tmp.find("%uri") != std::string::npos )
					tmp = tmp.replace(tmp.find("%uri"),4,uri);
				
				debug_v("open special link (cmd): " << tmp); 
				w->app->execcmd(exec_context_callback, tmp, w); 
				
				return TRUE;
			}
		}
	#endif	
		
	// middle click = nouvelle page
	if ( webkit_web_navigation_action_get_button(n) == 2 )
	{
		webkit_web_policy_decision_ignore(p);
		//~ (w->app->create_window())->seturi(uri);
		w->app->execcmd(exec_context_callback, ":n " + uri, w);

		w->linkh = "";
		return TRUE;
	}
	
	// On ne traite pas
	return FALSE;
}


/*
	Lance un telechargement 
*/

gboolean app_window::initdownload(WebKitWebView *v, WebKitDownload *o, app_window *w) 
{
	std::string filename, save_as, html;
	std::string uri;
	
	app_window *wn;
	
	uri = webkit_download_get_uri(o);
	
	#if DOWNLOAD_AUTO 
		(void)v; // unused
		
		//~ download dans une new fenetre
		wn = w->app->create_window();
		wn->download = o;
	#else
		if ( v != NULL )
		{
			wn = w->app->create_window();
			wn->download = o;
			
			html = "<span style=\"color: red;\">This file can not be shown</span> <br />";
			html += "Original uri: <br /><b>" + uri + "</b><br />";	
			
			wn->sethtml(uri, html); 
			return TRUE;
		}
		else
		{
			if ( !WEBKIT_IS_DOWNLOAD(w->download) )
			{
				wn = w->app->create_window();
				wn->download = o;
			}
			else
			{
				// deja pret
				wn = w;
				
				if ( webkit_download_get_destination_uri(o) != NULL )
					webkit_download_set_destination_uri( wn->download, webkit_download_get_destination_uri(o) );
			}
		}
	#endif	
	
	//~ Chemin, nom et html
	save_as = wn->getLocalFilename();	
	filename = save_as.substr( save_as.find_last_of("/")+1, -1 );	
	
	debug_v("save_as " << save_as);
	html = "<span style=\"color: orange;\">Download in progress</span> <br />";
	html += "Save as: <br /><b>" + save_as + "</b><br />";	
	html += "Original uri: <br /><b>" + uri + "</b><br />";	
	
	wn->setprogress(0);
	wn->settitle("Download of " + filename);
	wn->sethtml(uri, html);
	
	g_signal_connect(wn->download, "notify::progress", G_CALLBACK(wn->updatedownload), wn);
	g_signal_connect(wn->download, "notify::status", G_CALLBACK(wn->updatedownload), wn);
	
	//~ lancement
	webkit_download_set_destination_uri(wn->download, save_as.c_str());
	
	#if DOWNLOAD_AUTO
		webkit_download_start(wn->download);
	#else
		if ( v == NULL )
			webkit_download_start(wn->download);
	#endif
	
	return TRUE; // on traite la demande
}


/*
	Met a jour l'avancement d'un downlaod
*/

void app_window::updatedownload(WebKitDownload *o, GParamSpec *pspec, app_window *w) 
{
	//~ unused parameter
	(void)o; (void)pspec;
	
	WebKitDownloadStatus status = webkit_download_get_status(w->download);
	
	if(status == WEBKIT_DOWNLOAD_STATUS_STARTED || status == WEBKIT_DOWNLOAD_STATUS_CREATED) 
	{
		w->setprogress(webkit_download_get_progress(w->download)*100);
	}
	else if(status == WEBKIT_DOWNLOAD_STATUS_FINISHED)
	{
		w->sethtml(w->gettitle(),"<span style=\"color: green;\">Download finished !</span><br />Saved as: <br /><b>" + w->getLocalFilename() + "</b><br />  Original uri: <br /> <b>" + w->geturi() + "</b></b>");
		
		//~ permet une reutilisation de la fenetre (est inutile car un nouveau downlaod s'ouvrirais dans une nouvelle fenetre, mais bon)
		g_object_unref(w->download);
		w->download = NULL;
		w->download_as.erase();
	}
}


/*
	Reaction au evenement clavier
*/

gboolean app_window::keypress(GtkWidget* widget, GdkEventKey *ev, app_window *w) 
{
	(void)widget;
	
	gboolean processed = FALSE;
	
	ev->state = (ev->state & ~(GDK_MOD2_MASK)); // clear numlock 
	ev->state = (ev->state & ~(GDK_SHIFT_MASK)); // clear shift (shift+a = A)
	
	debug_vvv("ev->keyval: " << ev->keyval << " // ev->state: " << ev->state );
	debug_vvv("ev->keyval unicode: " << gdk_keyval_to_unicode(ev->keyval));
	
	
	// Traitement de la touche
	
	//~ vérification de tous les bindings
	for ( int i = 0 ; keybinding[i].mode != mode_null ; i++ )
	{
		debug_vvv("testing key cmd: '" << keybinding[i].cmd 
			<< "' // modifier: '" << keybinding[i].modifier << "'"
			<< "' // key: '" << keybinding[i].key << "'");
		
		if ( 
			( 
				keybinding[i].mode == mode_all 
				or 
				keybinding[i].mode == w->getmode() 
			) 
			&& 
			keybinding[i].key == ev->keyval 
			&& 
			(
				(!keybinding[i].modifier  && (ev->state == 0))
				or 
				(ev->state == keybinding[i].modifier) 
			)
		)
		{
			std::string tmp( keybinding[i].cmd );
			
			if ( tmp.find("%uri") != std::string::npos )
				tmp = tmp.replace(tmp.find("%uri"),4,w->geturi());
			if ( tmp.find("%clipboard") != std::string::npos )
				tmp = tmp.replace(tmp.find("%clipboard"),10, application::getclipboard(GDK_SELECTION_CLIPBOARD));
			if ( tmp.find("%Xclipboard") != std::string::npos )
				tmp = tmp.replace(tmp.find("%Xclipboard"),11, application::getclipboard(GDK_SELECTION_PRIMARY));
			
			debug_v("keybinding exec: '" << tmp << "'");
			w->app->execcmd(exec_context_callback, tmp, w); 
			w->update();
			
			//~ on est dans le cas d'une combinaison
			processed = TRUE;
			break;
		}
	}

	// Mode: commande
	if ( w->mode == mode_command )
	{
		//~ combinaison ne correspondant pas à un Binding
		if ( !processed )
		{
			#if COMMAND_PREVENT_INSERTING
				processed = TRUE;

				if ( ev->state & GDK_CONTROL_MASK )
					return false;
				else if ( gdk_keyval_to_lower(ev->keyval) == GDK_Return or gdk_keyval_to_lower(ev->keyval) == GDK_KP_Enter )
					return false;
				else if ( (ev->keyval) ==  GDK_Tab or (ev->keyval) ==  GDK_Down or (ev->keyval) ==  GDK_Up or (ev->keyval) ==  GDK_Right or (ev->keyval) ==  GDK_Left )
					return false;
			#else
				processed = FALSE;
			#endif
			
		}
	}
	
	// Mode: insertion
	else if ( w->mode == mode_insert )
	{
		
	}
	
	// Mode: ligne de commande
	else if ( w->mode == mode_commandline )
	{
		if ( processed )
		{
			
		}
		else if ( gdk_keyval_to_lower(ev->keyval) == GDK_Return or gdk_keyval_to_lower(ev->keyval) == GDK_KP_Enter or (gdk_keyval_to_unicode(ev->keyval) == ' ' and !(ev->state & GDK_CONTROL_MASK) ) )
		{
			if ( w->cmd->get().size() )
			{
				matchingItem mi = w->cmd->getBest(true);
				debug_vv("completion::distance with bestmatch (" << mi.string << ") : " << mi.distance);
				
				if ( mi.distance > COMPLETION_USEFORCE_DISTANCE && mi.distance < 1 )
				{
					debug_v("completion::used by force");
					w->cmd->useBest(true);
				}
				
				//~ insert space
				if ( gdk_keyval_to_unicode(ev->keyval) == ' ')
				{
					if ( w->cmd->get().at(w->cmd->get().size()-1) != ' ' )
					{
						std::string tmp = "";
						tmp += gdk_keyval_to_unicode(ev->keyval);
						w->cmd->insert(tmp);
					}
				}
				//~ exec cmdline
				else
				{
					w->app->execcmd(exec_context_commandline, w->cmd->get(), w);
					//~ w->app->execcmd(exec_context_commandline, haku::string::trim(haku::string::cleanWhitespace(w->cmd->get())), w);
					
					// retour, car on ne dois pas faire de w->update()
					return true;
				}
			}
		}		
		else if ( gdk_keyval_to_lower(ev->keyval) == GDK_Tab )
		{
			std::string tmp( w->cmd->getBest().string );
			
			if ( ev->state & GDK_CONTROL_MASK )
			{
				w->cmd->useBest();	
				w->cmd->moveBest(-1);
			}
			else
			{	
				w->cmd->useBest();	
				w->cmd->moveBest(+1);
			}
		}

		else if ( ev->state & GDK_CONTROL_MASK && ev->keyval ==  GDK_v )
		{
			w->cmd->insert( application::getclipboard(GDK_SELECTION_CLIPBOARD) );
		}
		else if ( gdk_keyval_to_lower(ev->keyval) == GDK_BackSpace )
		{
			w->cmd->remove(-1);
		}
		else if ( gdk_keyval_to_lower(ev->keyval) == GDK_Delete )
		{
			w->cmd->remove(+1);
		}
		else if ( gdk_keyval_to_lower(ev->keyval) == GDK_Up )
		{					
			w->cmd->history(-1);
		}		
		else if ( gdk_keyval_to_lower(ev->keyval) == GDK_Down )
		{					
			w->cmd->history(+1);
		}
		else if ( gdk_keyval_to_lower(ev->keyval) == GDK_Home )
		{
			w->cmd->move(-1000);
		}
		else if ( gdk_keyval_to_lower(ev->keyval) == GDK_Left )
		{
			w->cmd->move(-1);
		}		
		else if ( gdk_keyval_to_lower(ev->keyval) == GDK_Right )
		{
			w->cmd->move(+1);
		}		
		else if ( gdk_keyval_to_lower(ev->keyval) == GDK_End )
		{
			w->cmd->move(1000);
		}
		else if ( haku::string::inCharset(gdk_keyval_to_unicode(ev->keyval), "azertyuiopqsdfghjklmwxcvbnAZERTYUIOPQSDFGHJKLMWXCVBN0123456789Ã©Ã¨Ã Ã¹$â‚¬Â£Â°.?!,+-*/:;_ ~#()[]&=<>%" ) )
		{
			std::string tmp = "";
			tmp += gdk_keyval_to_unicode(ev->keyval);
			w->cmd->insert(tmp);
		}	
		
		// mode recherche: validation auto (recherche Ã  la frappe)
		if ( gdk_keyval_to_lower(ev->keyval) != GDK_Escape && w->cmd->get().at(0) == '/'  )
		{
			debug_vv("Search in page");
			w->app->execcmd(exec_context_commandline, w->cmd->get() , w);
		}
		
		//~ prevent inserting into the webview
		processed = TRUE;
	}
	
	debug_vvv("key processed ? : " << (int) processed);
	w->update();
	return processed;
}


/*
	Affichage de l'indicateur entraine son dessinage
*/

gboolean app_window::exposeindicator(GtkWidget *widget, GdkEventExpose *e, app_window *w) 
{
	(void) widget; (void) e;
	w->drawindicator();
	return TRUE;
}


/*
	Survol d'un lien => indication du lien dans le titre
*/

void app_window::linkhover(WebKitWebView *v, const char* t, const char* l, app_window *w) 
{
	(void) v; (void) t;
	
	if ( l )
		w->linkh = l;
	else 
		w->linkh = "";
	
	w->update();
}


/*
	Actualise l'avancement du chargement de la page (indicator+titre)
*/

void app_window::progresschange(WebKitWebView *v, gint p, app_window *w) 
{	
	(void) v; (void) p;
	w->progress = webkit_web_view_get_progress(w->view)*100;
	w->update();
}

void app_window::loadstat(WebKitWebView *view, WebKitWebFrame *f, app_window *w) 
{
	(void) f;
	std::string html;
	
	switch ( webkit_web_view_get_load_status(view) )
	{
		case WEBKIT_LOAD_PROVISIONAL:		
			//~ supprime le lien courant de la liste
			//~ car on change de page
			if ( DEFAULT_SESSION_SAVE ) 
				w->app->session_remove("default", w->geturi());
	
			w->progress = 0;
			w->mode = mode_command;
			break;
		
		case WEBKIT_LOAD_COMMITTED:		
			// ajout nouvelle page dans session sauvegarde
			if ( DEFAULT_SESSION_SAVE && w->geturi().substr(0,4) == "http" ) 
				w->app->session_add("default", w->geturi());

			//~ Page http-secure
			w->trusted = ( (w->geturi()).substr(0,5) == "https" );

			
			//~ Chargement reglage "par url"
			w->c_set("javascript", w->app->get_javascript_setting(w->geturi()) );
			w->c_set("plugins", w->app->get_plugins_setting(w->geturi()) );
			
			//~ Recharge les cookies
			w->app->reloadcookies();
			
			break;
			
		case WEBKIT_LOAD_FIRST_VISUALLY_NON_EMPTY_LAYOUT:
			break;
			
		case WEBKIT_LOAD_FINISHED:
			w->progress = 100;
			
			break;
		
		case WEBKIT_LOAD_FAILED:
			w->progress = -1;
			break;
			
	}
	
	w->update();
}


/*
	Redimensionnement de la fenetre, et zoom adaptatif
*/

void app_window::resize(GtkWidget* widget, GtkAllocation *a, app_window *w) 
{
	//~ unused parameters
	(void) widget; (void) a;
	
	#if OPT_ADAPTATIVE_ZOOM
	//~ Specified zoom level, do not do anything
	if ( w->zoomed ) return;
	
	//~ Current zoom level
	float zoom = webkit_web_view_get_zoom_level(w->view);
	
	//~ Automatic zoom setting
	if ( (a->width < 300 or a->height < 300) && zoom != 0.3 )
	{
		debug_vv("Adaptative zoom set to 0.3 (old was: " << zoom << " )");
		webkit_web_view_set_zoom_level(w->view, 0.3);
	}
	else if ( (a->width < 400 or a->height < 400) && zoom != 0.6 )
	{	
		debug_vv("Adaptative zoom set to 0.6 (old was: " << zoom << " )");
		webkit_web_view_set_zoom_level(w->view, 0.6);
	}
	else if ( (a->width < 500 or a->height < 500) && zoom != 0.8 )
	{
		debug_vv("Adaptative zoom set to 0.8 (old was: " << zoom << " )");
		webkit_web_view_set_zoom_level(w->view, 0.8);
	}
	else if ( (a->width < 600 or a->height < 600) && zoom != 0.9 )
	{
		debug_vv("Adaptative zoom set to 0.9 (old was: " << zoom << " )");
		webkit_web_view_set_zoom_level(w->view, 0.9);
	}
	else if ( zoom != 1.0 )
	{
		debug_vv("Adaptative zoom set to 1 (old was: " << zoom << " )");
		webkit_web_view_set_zoom_level(w->view, 1.0);
	}
	#endif
}


/*
	Changement du titre de la page web => changement du titre de la fenetre
*/

void app_window::titlechange(WebKitWebView *v, GParamSpec *spec, app_window *w) 
{
	//~ Unused paramters
	(void)spec;
	
	debug_vv("app_window::titlechange() called");
	
	char *t = NULL;
	g_object_get (v, "title", &t, NULL);	
	
	if ( t != NULL )
	{
		w->settitle(t);
		w->update();
	}
}


/*
	UserJS
*/

void app_window::objectcleared(GtkWidget *widget, WebKitWebFrame *frame, JSContextRef js, JSObjectRef win, app_window *w) 
{
	(void)widget; (void)frame; (void)js; (void)win; (void)w;
	
	#if USERJS_ENABLED
	JSStringRef jsscript;
	JSValueRef exception = NULL;
	
	GError *error = NULL;
	char *script;
	const char *scriptfile;
	
	GDir *dir;
	dir = g_dir_open((w->app->configdir+"/userjs/").c_str(), 0, &error);
	
	if ( error == NULL && dir != NULL )
	{
		while ( (scriptfile = g_dir_read_name(dir)) )
		{
			g_clear_error(&error);
			
			if ( g_file_get_contents((w->app->configdir+"/userjs/"+scriptfile).c_str(), &script, NULL, &error) ) 
			{	
				debug_v("script " << scriptfile << " loaded")
				
				//~ webkit_web_view_execute_script(w->view, script);
				
				jsscript = JSStringCreateWithUTF8CString(script);
				JSEvaluateScript(js, jsscript, JSContextGetGlobalObject(js), NULL, 0, &exception);
			}
			
			g_free(script);
		}
	
		g_dir_close(dir);
	}
	
	g_clear_error(&error);
	#endif // USERJS_ENABLED
}

/*
	Fermeture de la fenetre (callback)
*/

void app_window::destroy(GtkWidget* widget, app_window *w) 
{
	(void)widget;
	w->app->remove_window(&w);
}	


/*
	Clic (souris)
*/

gboolean app_window::clicked(GtkWidget *widget, GdkEventButton *event, app_window *w)
{
	(void)widget;
	bool is_link = w->linkh.size() != 0;
	
	WebKitHitTestResult* hit;
	WebKitHitTestResultContext hit_context;
	std::string tmp;

	hit = webkit_web_view_get_hit_test_result (w->view, event);
	g_object_get(hit, "context",&hit_context,NULL);

	if ( is_link )
	{
		w->linkh.erase();
		w->update();
	}
	
	//~ middle click
	if ( event->button == 2 )
	{
		//~ l'element survolé est un lien
		if ( is_link )
			return FALSE;
		
		//~ l'element survolé est editable => on colle
		else if ( hit_context & WEBKIT_HIT_TEST_RESULT_CONTEXT_EDITABLE )
			return FALSE;
		
		
		else
		{
			tmp = application::getclipboard(GDK_SELECTION_PRIMARY);
			
			if ( tmp.size() )
			{
				//~ on est en mdoe commandline : insert dans la commande
				if ( w->mode == mode_commandline )
				{
					w->cmd->insert(tmp);
					w->update();
				}
				//~ on charge ce qui est dans le clipboardX en temps qu'url
				else
				{
					debug_v("MiddleClick Clilpboard '" << tmp << "' will be loaded as uri")
					w->seturi( w->app->parseuri(tmp) );
				}
				
				return TRUE;
			}
			
		}
	}
	
	#if OPT_CLICK_SWITCH_MODE
	if ( w->mode == mode_command && (hit_context & WEBKIT_HIT_TEST_RESULT_CONTEXT_EDITABLE) )
	{
		debug_v("click switch to insert");
		w->mode = mode_insert;
		w->update();
	}	
	else if ( w->mode == mode_insert && !(hit_context & WEBKIT_HIT_TEST_RESULT_CONTEXT_EDITABLE)  )
	{
		debug_v("click switch to command");
		w->mode = mode_command;
		w->update();
	}
	#endif
	
	return FALSE; // unhandled
}

/*
	Debut de communication http
*/
void app_window::resource_request(WebKitWebView *view, WebKitWebFrame *frame, WebKitWebResource *resource, WebKitNetworkRequest  *request, WebKitNetworkResponse *response, app_window *w)
{
	//~ Unused parameters
	(void) view; (void) frame; (void) resource; (void) response;
	
	//~ recuperation de l'url cible de la requete
	std::string uri = webkit_network_request_get_uri(request); 
	
	//~ traitement
	if ( w->app->get_adblock_setting(w->geturi()) && haku::adblock::parse(uri, w->app->adblock_parser_list) == haku::adblock::block )
	{
		std::cout << "=============================" << std::endl;
		std::cout << "URI BLOCKED ("<<uri<<")" << std::endl;
		std::cout << "=============================" << std::endl;
		webkit_network_request_set_uri(request,"about:blank");
	}
}


/* 
************************************************************************************************
	
	Les méthodes
	
************************************************************************************************
*/


/*
	Imrpimer la page
*/

void app_window::c_print() 
{
	webkit_web_frame_print ( webkit_web_view_get_focused_frame(this->view) );
}


/*
	Actualiser la page
*/

void app_window::c_reload(gboolean nocache) 
{
	if ( nocache )
		webkit_web_view_reload_bypass_cache(this->view);
	else
		webkit_web_view_reload(this->view);
}


/*
	Arreter le chargemlent de la page / du download
*/

void app_window::c_stop() 
{
	if ( WEBKIT_IS_DOWNLOAD(this->download) )
	{
		webkit_download_cancel(this->download);
		this->download = NULL;
	}
	else
	{
		webkit_web_view_stop_loading(this->view);
	}
}


/*
	Fermeture de la fenetre
*/

void app_window::close()
{	
	debug_v("close called");
	
	if ( this->win != NULL )
	{
		//~ std::cout << "(debug) close" << std::endl;

		//~ supprime de la liste de sauvegarde (auto)
		if ( DEFAULT_SESSION_SAVE ) 
			this->app->session_remove("default", this->geturi());

		//~ Arrete le telechargement / chargement
		this->c_stop();
		
		//~ webkit_web_back_forward_list_clear(webkit_web_view_get_back_forward_list(this->view));

		gtk_widget_destroy(GTK_WIDGET(this->view));
		gtk_widget_destroy(this->indicator);
		
		#if SHOW_SCROLLBAR
			gtk_widget_destroy(this->scroll);
		#endif
		
		gtk_widget_destroy(this->vbox);
		gtk_widget_destroy(this->win);
		
		this->win = NULL;
		
		debug_vv("close done");
	}
}


/*
	Lance le chargement d'une page via son url
*/

void app_window::seturi(const std::string &uri)
{
	if ( uri.size() )
	{
		this->uri = uri;
		this->baseuri = haku::tools::base_uri(uri);
		webkit_web_view_load_uri(this->view, uri.c_str());
	}
}

/*
	Définir le titre de la fenetre
*/

void app_window::settitle(const std::string &title)
{
	this->title = title;
	this->update();
}


/*
	Définir l'avancement (barre titre+indicateur)
*/

void app_window::setprogress(int p)
{
	this->progress = p;
	this->update();
}


/*
	Définition du mode (command, insert, commandline)
*/

void app_window::setmode(app_mode m)
{
	if ( m == mode_insert or m == mode_command or m == mode_commandline )
		this->mode = m;
}


/*
	Définition du contenu de la ligne de commande
*/

void app_window::setcmdline(std::string cmd)
{
	this->cmd->newline(cmd);
}

void app_window::sethtml(const std::string &title, const std::string &html)
{
	//~ on ne veut pas que la fausse url soit parsé par OPEN_SPECIAL_LINK
	this->osl = true;
	
	this->settitle(title);
	webkit_web_back_forward_list_add_item(webkit_web_view_get_back_forward_list(this->view) , webkit_web_history_item_new());
	webkit_web_view_load_html_string(this->view, html.c_str(), title.c_str());
}

/*
	Mise à jour des widgets
*/

void app_window::update()
{
	debug_vvv("app_window::update() called");
	std::string title = this->title;
	
	if ( this->linkh != "")
	{
		title = this->linkh;
	}
	else if ( this->progress != 100 )
	{
		std::ostringstream p;
		p << this->progress;
		
		title = "[" + p.str()+"%] " + title;
	}
	
	this->drawindicator();
	gtk_window_set_title(GTK_WINDOW(this->win), title.c_str());
}


/*
	Récupère le mode
*/

app_mode app_window::getmode()
{
	return this->mode;
}


/*
	Récupère le titre
*/

std::string app_window::gettitle()
{
	return this->title;
}


/*
	Récupère l'uri
*/

std::string app_window::geturi()
{
	if ( webkit_web_view_get_uri(this->view) != NULL )
		return webkit_web_view_get_uri(this->view);
	else
		return "";
}


/*
	Récupère le nom sous lequel le fichier va etre sauvegardé
*/

const std::string app_window::getLatestLocalFilename()
{
	return this->latest_download_as;
}

const std::string app_window::getLocalFilename()
{
	if ( this->download_as.size() == 0 )
	{
		std::string save_as, filename;
		
		//~ le nom est deja specifie (via :save NOM)
		if ( webkit_download_get_destination_uri(this->download) != NULL )
		{
			filename = webkit_download_get_destination_uri(this->download);	
			
			if ( filename.at(0) != '/' ) save_as = this->app->downloaddir + "/" + filename;
			else save_as = filename;
			
			if ( g_file_test(save_as.c_str(), G_FILE_TEST_IS_DIR ) )
			{
				save_as = save_as + '/' + webkit_download_get_suggested_filename(this->download);
			}		
		}
		
		//~ obtention du nom par default
		if ( filename.size() == 0 )
		{
			debug_v("(download) nom par default");
			filename = webkit_download_get_suggested_filename(this->download);	
			if ( filename.size() == 0 ) filename = "no_name";
			
			save_as = this->app->downloaddir + "/" + filename;
			
			//~ le fichier existe
			//~ rempalce par _filename
			while ( g_file_test(save_as.c_str(), G_FILE_TEST_IS_REGULAR) )
			{
				filename = "_" + filename;
				save_as = this->app->downloaddir + "/" + filename;
			}
		}
		
		this->download_as = "file://" + save_as;
		this->latest_download_as = "file://" + save_as;
	}
	
	return this->download_as;
}


/*
	Dessine l'indicateur et le texte de la ligne de commande
*/

void app_window::drawindicator() 
{
	gint width,height;
	GdkGC *gc;
	GdkColor fg;
	
	int x;

	gc = gdk_gc_new(this->indicator->window);
	
		
	if ( this->mode == mode_command ) 
	{
		if ( this->trusted ) 
		{
			height = 4;			
			gdk_color_parse(COLOR_INDICATOR_COMMAND_SECURE, &fg);
		}
		else
		{
			height = 2;			
			gdk_color_parse(COLOR_INDICATOR_COMMAND, &fg);
		}
	}	
	else if ( this->mode == mode_insert ) 
	{
		if ( this->trusted ) 
		{
			height = 4;			
			gdk_color_parse(COLOR_INDICATOR_INSERT_SECURE, &fg);
		}
		else
		{
			height = 2;			
			gdk_color_parse(COLOR_INDICATOR_INSERT, &fg);
		}
	}	
	else // if ( this->mode == mode_commandline ) 
	{
		height = 16;
		gdk_color_parse(COLOR_INDICATOR_COMMANDLINE, &fg);
	}
	
	width = this->progress * this->indicator->allocation.width / 100;
	gtk_widget_set_size_request(this->indicator, this->indicator->allocation.width, height);
	
	gdk_gc_set_rgb_fg_color(gc, &fg);
	gdk_draw_rectangle(this->indicator->window, this->indicator->style->bg_gc[GTK_WIDGET_STATE(this->indicator)], TRUE, 0, 0, this->indicator->allocation.width, height);
	gdk_draw_rectangle(this->indicator->window, gc, TRUE, 0, 0, width, height);
	
	if ( this->mode == mode_commandline )
	{
		gdk_color_parse(COLOR_INDICATOR_COMMANDLINE_FG, &fg);
		gdk_gc_set_rgb_fg_color(gc, &fg);
		pango_layout_set_markup(this->indicator_pl, this->cmd->getSpecial().c_str(), -1);
		gdk_draw_layout(this->indicator->window, gc, 0, 0, this->indicator_pl);
		
		pango_layout_set_text(this->indicator_pl, this->cmd->getBest().string.c_str(), -1);
		pango_layout_get_pixel_size(this->indicator_pl, &x, NULL);
		gdk_draw_layout(this->indicator->window, gc, width - x, 0, this->indicator_pl);
	}
	
	g_object_unref(gc);
}


/* 
************************************************************************************************
	
	Les méthodes de contrôle par l'utilisateur
	
************************************************************************************************
*/


/*
	Commandes de navigation dans l'historique
*/

void app_window::c_navigate(gint steps) 
{
	webkit_web_view_go_back_or_forward(this->view,steps);
}


/* 
	Scroll sur commande
*/

void app_window::c_vscroll(gint steps) 
{
	gdouble v;
	GtkAdjustment *a;

	a = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(this->scroll));
	v = gtk_adjustment_get_value(a);
	v += gtk_adjustment_get_step_increment(a) * steps;
	v = MAX(v, 0.0);
	v = MIN(v, gtk_adjustment_get_upper(a) - gtk_adjustment_get_page_size(a));
	gtk_adjustment_set_value(a, v);
}

void app_window::c_hscroll(gint steps) 
{
	gdouble h;
	GtkAdjustment *a;

	a = gtk_scrolled_window_get_hadjustment(GTK_SCROLLED_WINDOW(this->scroll));
	h = gtk_adjustment_get_value(a);
	h += gtk_adjustment_get_step_increment(a) * steps;
	h = MAX(h, 0.0);
	h = MIN(h, gtk_adjustment_get_upper(a) - gtk_adjustment_get_page_size(a));
	gtk_adjustment_set_value(a, h);
}


/*
	Recherche dans la page
*/

void app_window::c_find(std::string token) 
{
	//~ std::cout << webkit_web_view_search_text(this->view,token.c_str(),FALSE,TRUE,TRUE) << std::endl;
	webkit_web_view_search_text(this->view,token.c_str(),FALSE,TRUE,TRUE);
}


/*
	Afficher la source
*/

void app_window::c_source() 
{	
	//~ toggle entre page et sourge
	webkit_web_view_set_view_source_mode(this->view, !webkit_web_view_get_view_source_mode(this->view));
	this->c_reload(false);
}


/*
	Zoom in/out
*/

void app_window::c_zoom(int z) 
{
	this->zoomed = true;
	
	if(z < 0)
	{
		webkit_web_view_zoom_out(this->view);
	}
	else if(z > 0)
	{
		webkit_web_view_zoom_in(this->view);
	}
	else 
	{
		this->zoomed = false;
		webkit_web_view_set_zoom_level(this->view, 1.0);
	}
}


/*
	Active/desactive javascript/plugins/cookies et le met dans la liste de session
*/

void app_window::c_set(const std::string &str, const std::string &stat)
{
	int i;
	
	if ( str == "useragent" )
	{
		for (  i = 0 ; useragent[i].name != "" ; i++ )
			if ( useragent[i].name == stat )
				break;
		
		if ( useragent[i].name ==  "" ) 
			g_object_set(G_OBJECT(this->settings), "user-agent", stat.c_str(), NULL);
		else
			g_object_set(G_OBJECT(this->settings), "user-agent", useragent[i].agent.c_str(), NULL);
		
		debug_v("useragent set to " << stat);
	}
}

void app_window::c_set(const std::string &str, bool stat)
{
	if ( str == "plugins" )
	{
		g_object_set(G_OBJECT(this->settings), "enable-plugins", stat, NULL);		
	}
	else if ( str == "javascript" or str == "js" )
	{
		g_object_set(G_OBJECT(this->settings), "enable-scripts", stat, NULL);
	}
}

void app_window::c_set(const std::string &str, const std::string &uri, bool stat)
{
	this->c_set(str,stat);
	
	if ( str == "cookies" )
	{
		this->app->session_cookies_list[haku::tools::base_domain(uri)] = stat;		
	}
	else if ( str == "plugins" )
	{
		this->app->session_plugins_list[haku::tools::base_uri(uri)] = stat;		
	}
	else if ( str == "javascript" or str == "js" )
	{
		this->app->session_javascript_list[haku::tools::base_uri(uri)] = stat;		
	}
	else if ( str == "adblock" )
	{
		this->app->session_adblock_list[haku::tools::base_uri(uri)] = stat;		
	}
	
	this->app->sync_request("set " + str + " " + (stat ? "on " : "off ") + uri);
}

void app_window::c_set(const std::string &str, const std::string &uri, const std::string &stat)
{
	this->c_set(str,stat);
	
	if ( str == "useragent" )
	{
		this->app->session_useragent_list[haku::tools::base_uri(uri)] = stat;		
	}
	
	this->app->sync_request("set " + str + " " + stat + " " + uri);
}

void app_window::c_set(const std::string &str, const std::string &uri, const std::string &stat, bool always)
{
	this->c_set(str, uri, stat);
	
	if ( always )
	{
		if ( str == "useragent" )
		{
			this->app->useragent_list = haku::list::mapStrStr_load(this->app->configdir+"/uri/useragent.txt");
			this->app->useragent_list[haku::tools::base_uri(uri)] = stat;
			
			haku::list::mapStrStr_save(this->app->useragent_list, this->app->configdir+"/uri/useragent.txt");
		}
	}	
}


void app_window::c_set(const std::string &str, const std::string &uri, bool stat, bool always)
{
	this->c_set(str, uri, stat);
	
	if ( always )
	{
		if ( str == "cookies" )
		{
			this->app->cookies_list = haku::list::mapStrBool_load(this->app->configdir+"/uri/cookies.txt");
			this->app->cookies_list[haku::tools::base_uri(uri)] = stat ? "on" : "off";
			
			haku::list::mapStrBool_save(this->app->cookies_list, this->app->configdir+"/uri/cookies.txt");
		}
		else if ( str == "plugins" )
		{
			this->app->plugins_list = haku::list::mapStrBool_load(this->app->configdir+"/uri/plugins.txt");
			this->app->plugins_list[haku::tools::base_uri(uri)] = stat ? "on" : "off";
			
			haku::list::mapStrBool_save(this->app->plugins_list, this->app->configdir+"/uri/plugins.txt");
		}
		else if ( str == "javascript" or str == "js" )
		{
			this->app->javascript_list = haku::list::mapStrBool_load(this->app->configdir+"/uri/javascript.txt");
			this->app->javascript_list[haku::tools::base_uri(uri)] = stat ? "on" : "off";
			
			haku::list::mapStrBool_save(this->app->javascript_list, this->app->configdir+"/uri/javascript.txt");
		}
		else if ( str == "adblock" )
		{
			this->app->adblock_list = haku::list::mapStrBool_load(this->app->configdir+"/uri/adblock.txt");
			this->app->adblock_list[haku::tools::base_uri(uri)] = stat;
			
			haku::list::mapStrBool_save(this->app->adblock_list, this->app->configdir+"/uri/adblock.txt");
		}
	}	
}


