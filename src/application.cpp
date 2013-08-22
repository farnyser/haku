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


#include "application.hpp"
#include "tools/tools.hpp"
#include "config.hpp"
#include <fstream>
#include <signal.h>

#define SHM_SIZE sizeof(int)+sizeof(char)*150


/*
	Fonction de base
*/

application::application(std::string binary, std::string configdir, std::string downloaddir)
{
	// ignore child (do not create zombis)
	struct sigaction s;
	s.sa_handler = SIG_IGN;
	sigaction(SIGCHLD, &s, NULL);

	//~ tmp
	signed int shmid;
	std::ifstream inTmp;
	std::string  strTmp;
	
	//~ path
	g_mkdir_with_parents((configdir+"/session").c_str(),0755);
	g_mkdir_with_parents((configdir+"/uri").c_str(),0755);
	g_mkdir_with_parents(downloaddir.c_str(),0755);
	
	this->binarypath = binary;
	this->configdir = configdir;
	this->downloaddir = downloaddir;
	
	//~ init gtk
	gtk_init(NULL, NULL);
	
	if ( !g_thread_supported() )
		g_thread_init(NULL);
		
	this->session = webkit_get_default_session();
	
	#if OPT_STRICT_SSL
    const gchar* certificate_files[] =
    {
        "/etc/pki/tls/certs/ca-bundle.crt",
        "/etc/ssl/certs/ca-certificates.crt",
        NULL
    };
    guint i;

    for (i = 0; i < G_N_ELEMENTS (certificate_files); i++)
	{
		if ( g_file_test(certificate_files[i], G_FILE_TEST_IS_REGULAR) )
        {
            g_object_set (this->session,
                "ssl-ca-file", certificate_files[i],
                "ssl-strict", FALSE,
                NULL);
            break;
        }
	}
	#endif
	
	//~ cookie persistance
	this->cookies = soup_cookie_jar_new();
	soup_session_add_feature(this->session, SOUP_SESSION_FEATURE(this->cookies));
	
	#if COOKIES_NO_THIRDPARTY
		g_object_set(this->cookies, "accept-policy", SOUP_COOKIE_JAR_ACCEPT_NO_THIRD_PARTY, NULL);
	#endif
	
	#if OPT_CACHE
		webkit_set_cache_model(WEBKIT_CACHE_MODEL_WEB_BROWSER);
	#else
		webkit_set_cache_model(WEBKIT_CACHE_MODEL_DOCUMENT_VIEWER);
	#endif
	
	g_signal_connect(this->cookies, "changed", G_CALLBACK(this->changecookie), this);
	this->reloadcookies();
	
	//~ proxy
	std::string proxy;
	SoupURI *puri;
	
	if ( getenv("http_proxy") )
	{
		proxy = getenv("http_proxy");
		proxy = (proxy.substr(0,7) == "http://") ? proxy : "http://"+proxy;

		std::cout << "Using HTTP proxy -> " << proxy << std::endl;
		
		puri = soup_uri_new(proxy.c_str());
		g_object_set(G_OBJECT(this->session), "proxy-uri", puri, NULL);
		soup_uri_free(puri);
	}

	//~ chargement bookmark
	this->bookmark_list = haku::list::mapStrStr_load(this->configdir+"/bookmark.txt");
	
	if ( this->bookmark_list.find("default") == this->bookmark_list.end() ) 
		this->bookmark_list["default"] = DEFAULT_HOMEPAGE;
	
	//~ chargement searchengine	
	this->searchengine_list = haku::list::mapStrStr_load(this->configdir+"/searchengine.txt");
	
	if ( this->searchengine_list.find("default") == this->searchengine_list.end() ) 
		this->searchengine_list["default"] = DEFAULT_SEARCHENGINE;
		
	//~ chargement javascript/plugins/cookies	
	this->javascript_list = haku::list::mapStrBool_load(this->configdir+"/uri/javascript.txt");
	this->plugins_list = haku::list::mapStrBool_load(this->configdir+"/uri/plugins.txt");
	this->cookies_list = haku::list::mapStrBool_load(this->configdir+"/uri/cookies.txt");
	this->adblock_list = haku::list::mapStrBool_load(this->configdir+"/uri/adblock.txt");
	this->useragent_list = haku::list::mapStrStr_load(this->configdir+"/uri/useragent.txt");
	
	//~ chargement fichier de regle adblock
	inTmp.open((this->configdir+"/adblock.txt").c_str());
		
	if ( !inTmp ) 
	{
		debug_v( "Unable to open file adblock.txt" ); 
	}
	else
	{
		while (inTmp >> strTmp) 
				this->adblock_parser_list.push_back(strTmp); 

		inTmp.close();
	}
	
	//~ gestion mémoire partagée
	//~ ---------------------------------------------------------
	
	//~ Obtient l'adresse de la memoire partagée
	shmid = shmget(ftok(this->configdir.c_str(),0), SHM_SIZE, IPC_CREAT|0666);
	if ( shmid == -1 ) { debug_v("***shmget failed***" << std::endl <<"size:" << SHM_SIZE << " errno:" << errno); exit(EXIT_FAILURE); }
	this->sharedMemoryAddr = shmat(shmid, NULL, 0);
	if ( this->sharedMemoryAddr == (void*)-1 ) { debug_v("***shmat failed***"); exit(EXIT_FAILURE); }

	//~ Récupération de l'int servant de "flag" et compteur de changement
	this->sharedMemoryId = *(int*)(this->sharedMemoryAddr);
}

void application::mainloop()
{
	//~ ne fais rien si il n y a pas de fenetre ouverte
	if ( this->windows_list.size() == 0 ) return;
	
	//~ syncrhonisation toutes les 500 millisecondes
	gint func_ref = g_timeout_add(500, application::sync, this);

	//~ boucle evenementielle
	gtk_main();

	//~ cleaning
	g_source_remove(func_ref);
}


/*
	Check et synchro si necessaire (callback)
*/

gboolean application::sync(void *_app)
{
	application *app = (application*)_app;
	
	//~ update demandée
	if ( app->sharedMemoryId != *((int*)(app->sharedMemoryAddr)) )
	{
		app->sharedMemoryId = *((int*)(app->sharedMemoryAddr));
		debug_v("SYNCHRONISATION : " << (const char*)((int*)(app->sharedMemoryAddr)+1));
		app->execcmd( exec_context_synchronisation, std::string((const char*)((int*)(app->sharedMemoryAddr)+1)), app);
	}
	
	//~ Return true so the function will be called again
	return TRUE;
}

void application::sync_request(const std::string& request)
{
	(*(int*)(this->sharedMemoryAddr))++;
	this->sharedMemoryId = *((int*)(this->sharedMemoryAddr));
	memcpy((int*)(this->sharedMemoryAddr)+1, (request+"\0").c_str(), (request.size()+1) * sizeof(char));
	debug_v("sync request : " << request);
}


/*
	Ajout et supression de fenetre dans la liste de fenetre
*/

app_window* application::create_window()
{	
	app_window *w = new app_window(this);	
	this->windows_list.push_back(w);
	return w;
}

int application::remove_window(app_window** w)
{	
	app_window *tmp = *w;
	unsigned int i = 0;
	while ( i < this->windows_list.size() && tmp != (this->windows_list[i]) )
		i++;
	if ( i == this->windows_list.size() ) return 0;
	
	
	if ( i < this->windows_list.size() )
	{
		this->windows_list.erase( this->windows_list.begin() + i );	
		w = NULL;
		delete tmp;
	}
	
	if ( !this->windows_list.size() ) 
		gtk_main_quit();

	return 1;
}


/*
	Parsage d'url et chargement dans la derniere fenetre
*/

void application::loaduri(std::string uri)
{
	if ( this->windows_list.size() == 0 )
		this->create_window();
	
	(*(this->windows_list.rbegin()))->seturi( this->parseuri(uri) );
}

std::string application::parseuri(std::string uri)
{
	uri = haku::string::trim(uri);
	std::vector<std::string> a = haku::string::explode(uri);
	
	//~ #if OPEN_SPECIAL_LINK
		//~ for ( int i = 0 ; open_special_link[i].url != "" ; i++ )
		//~ {
			//~ if ( 
				//~ ( haku::sregex::cmp(open_special_link[i].uri, uri).match )
			//~ )
				//~ return uri;
		//~ }
	//~ #endif	
		
	if ( uri.size() > 6 && uri.substr(0,6) == "http:/" )
		return uri;
	else if ( uri.size() > 7 && uri.substr(0,7) == "https:/" )
		return uri;
	else if ( uri.size() > 3 && uri.substr(0,3) == "www" )
		return "http://" + uri;
	else if ( uri.size() > 6 && uri.substr(0,6) == "file:/" )
		return uri;
	else if ( this->bookmark_list.find(uri) != this->bookmark_list.end() )
		return this->bookmark_list[uri];
	else if ( this->searchengine_list.find(a[0]) != this->searchengine_list.end() )
		return (this->searchengine_list[a[0]]) + haku::tools::urlencode(haku::string::trim(uri.substr(a[0].length(),-1)));
	else if ( uri.find(".") != std::string::npos )
		return "http://" + uri;
	else if ( this->searchengine_list.find("default") != this->searchengine_list.end() )
		return (this->searchengine_list["default"]) + haku::tools::urlencode(uri);
	else
		return uri;
}


/*
	CMD Line : parsage et execution d'une ligne de code
*/

void application::execcmd(exec_context context, std::string cmd, void *p)
{
	std::vector<app_window*> wlist;
	std::vector<app_window*>::iterator wt;
	app_window *w = NULL;
	int result;
	
	//~ debug_v("execcmd called with cmd = '" << cmd << "<");
		
	if ( cmd.at(0) != '/' )
	{
		//~ supression des ':' de debut de commande
		if ( cmd.at(0) == ':'  ) cmd = cmd.substr(1);	
		
		//~ execution commande
		if ( context == exec_context_callback && p == NULL )
		{
			wlist = this->windows_list ;
			
			for ( wt = wlist.begin() ; wt != wlist.end() ; wt++ )
				this->execcmd(context,cmd,*wt);				
			
			return;
		}
		else if ( context == exec_context_shell )
		{
			result = interpreter::execcmd(context,cmd,this);
		}
		else
		{
			result = interpreter::execcmd(context,cmd,p);
		}
		
		if ( result & interpreter::return_error ) 
			debug_v("interpreter :: argument error !!!");
		
		
		if ( result & interpreter::return_sync_request ) 
			this->sync_request(cmd);

		//~ after process
		//~ p est une instance de app_window
		if ( p != NULL && !(result & interpreter::return_window_deleted) && (context == exec_context_commandline or context == exec_context_callback) )
		{
			debug_vv("interpreter :: after process");
			w = (app_window*)p;
			w->osl = (context == exec_context_callback);
		
			if ( result & interpreter::return_reload_clean_request ) 
				w->c_reload(true);
			else if ( result & interpreter::return_reload_request ) 
				w->c_reload(false);
			
			
			if ( result & interpreter::return_newline_request )
				w->setmode(mode_command);
			
			if ( result & interpreter::return_update_request or context == exec_context_commandline ) 
				w->update();			
		}
	}
	else
	{
		//~ searchmode
		if ( context == exec_context_commandline )
		{
			w = (app_window*) p;
			
			if ( w != NULL && cmd.size() > 1 )
				w->c_find(cmd.substr(1,-1));
		}
	}
}

/*
	Gestion des cookies
*/

void application::reloadcookies() 
{
	SoupCookieJar *jar;
	GSList *l, *e;

	this->lockcookie = TRUE;
	
	for(l = e = soup_cookie_jar_all_cookies(this->cookies); e; e = e->next)
		soup_cookie_jar_delete_cookie(this->cookies, (SoupCookie *)e->data);
	
	soup_cookies_free(l);
	jar = soup_cookie_jar_text_new((this->configdir+"/cookies").c_str(), TRUE);
	
	for(l = e = soup_cookie_jar_all_cookies(jar); e; e = e->next)
		soup_cookie_jar_add_cookie(this->cookies, (SoupCookie *)e->data);
	g_slist_free(l);
	
	this->lockcookie = FALSE;
	g_object_unref(jar);
}

void application::changecookie(SoupCookieJar *j, SoupCookie *oc, SoupCookie *c, application *app) 
{
	//~ on ignore le cookieJar en argument, et on en recharge un 
	(void)j;
	
	SoupDate *e;
	SoupCookieJar *jar;

	// ne pas traiter un callback de changement de cookies issu de reloadcookies
	if(app->lockcookie)
		return;
	
	jar = soup_cookie_jar_text_new((app->configdir+"/cookies").c_str(), FALSE);
		
	//~ if ( c && c->expires == NULL ) // on enmerde la date d'expiration demandée
	if ( c != NULL && c->domain )
	{
		if ( app->get_cookies_setting(c->domain)  )
		{
			debug_v("cookie OK pour " << haku::tools::base_domain(c->domain) << " (domaine complet: " << c->domain << ")");

			e = soup_date_new_from_time_t(time(NULL) + SESSION_TIME);
			c = soup_cookie_copy(c);
			soup_cookie_set_expires(c, e);
			soup_cookie_jar_add_cookie(jar, soup_cookie_copy(c));
		}
		else
		{
			debug_v("cookie BLOQUE pour " << haku::tools::base_domain(c->domain) << " (domaine complet: " << c->domain << ")");
		}
	}
	else
	{		
		if ( oc != NULL )
			soup_cookie_jar_delete_cookie(jar, oc);
	}
	
	g_object_unref(jar);

}


/*
	Setting par url
*/

bool application::get_javascript_setting(std::string uri)
{
	uri = haku::tools::base_uri(uri);
	
	if ( this->session_javascript_list.find(uri) != this->session_javascript_list.end() )
		return (this->session_javascript_list[uri]);
	else if ( this->javascript_list.find(uri) != this->javascript_list.end() )
		return (this->javascript_list[uri]);
	else
		return DEFAULT_ENABLE_JAVASCRIPT;
}

bool application::get_plugins_setting(std::string uri)
{
	uri = haku::tools::base_uri(uri);
	
	if ( this->session_plugins_list.find(uri) != this->session_plugins_list.end() )
		return (this->session_plugins_list[uri]);
	else if ( this->plugins_list.find(uri) != this->plugins_list.end() )
		return (this->plugins_list[uri]);
	else
		return DEFAULT_ENABLE_PLUGINS;
}

bool application::get_adblock_setting(std::string uri)
{
	uri = haku::tools::base_uri(uri);
	
	if ( this->session_adblock_list.find(uri) != this->session_adblock_list.end() )
		return (this->session_adblock_list[uri]);
	else if ( this->adblock_list.find(uri) != this->adblock_list.end() )
		return (this->adblock_list[uri]);
	else
		return DEFAULT_ENABLE_ADBLOCK;
}

const std::string application::get_useragent_setting(std::string uri)
{
	uri = haku::tools::base_uri(uri);
	
	if ( this->session_useragent_list.find(uri) != this->session_useragent_list.end() )
		return (this->session_useragent_list[uri]);
	else if ( this->useragent_list.find(uri) != this->useragent_list.end() )
		return (this->useragent_list[uri]);
	else
		return "default";
}

bool application::get_cookies_setting(std::string uri)
{
	uri = haku::tools::base_domain(uri);
	
	if ( this->session_cookies_list.find(uri) != this->session_cookies_list.end() )
		return (this->session_cookies_list[uri]);
	else if ( this->cookies_list.find(uri) != this->cookies_list.end() )
		return (this->cookies_list[uri]);
	else
		return DEFAULT_ENABLE_COOKIES;
}


/*
	Gestion session
*/

haku::list::vectorStr application::session_getList(void)
{
	haku::list::vectorStr tmp;
	
	GError *error = NULL;
	const char *scriptfile;
	std::string name;
	GDir *dir = g_dir_open((this->configdir+"/session/").c_str(), 0, &error);
		
	while ( (scriptfile = g_dir_read_name(dir)) )
	{
		g_clear_error(&error);
		
		name = scriptfile;
		if ( name.size() >= 5 ) tmp.push_back( name.substr(0,name.size()-4) );
	}
	
	g_dir_close(dir);
	g_clear_error(&error);
		
	return tmp;
}

void application::session_restore(std::string name)
{
	debug_v("session::restore " << name);
	haku::list::vectorStr currently_opened;
	haku::list::vectorStr session_url = haku::list::vectorStr_load(configdir + "/session/" + name + ".txt");
	haku::list::vectorStr::iterator it, jt;
	std::vector<app_window*>::iterator wt;
	
	//~ Si session vide ET  name == default, on restore qaund meme la page d'accueilq
	if ( (session_url.size() < 1 or session_url[0].size() < 1 ) && name == "default" )
		session_url.push_back(this->parseuri("default"));
	
	//~ pour eviter de restaurer une fenetre deja ouverte, on a besoin de la liste des 
	//~ uri ouvertes
	if ( DEFAULT_SESSION_SAVE )
		for (wt = this->windows_list.begin() ; wt != windows_list.end() ; wt++)
				currently_opened.push_back( (*wt)->geturi() );
	
	//~ Ouverture des uri  chargées
	for (it = session_url.begin() ; it != session_url.end() ; it++)
	{
		if ( (*it).size() )
		{
			//~ dans le cas où on sauvegarde auto la liste des fenetres ouvertes, 
			//~ ne pas les recharger si deja ouverte
			if ( DEFAULT_SESSION_SAVE &&  name == "default" )
			{
				for ( jt = currently_opened.begin() ; jt != currently_opened.end() && (*jt) != (*it) ; jt++);
				
				if ( jt != currently_opened.end() )
					currently_opened.erase(jt);
				else
					(this->create_window())->seturi(*it);					
			}
			else
			{
				(this->create_window())->seturi(*it);
			}
		}
	}
	
	if ( DEFAULT_SESSION_CLEAN ) 
		this->session_delete(name);
}

void application::session_save(std::string name)
{
	debug_v("session::save " << name);
	haku::list::vectorStr session_url;
	std::vector<app_window*>::iterator it;
	
	for (it = this->windows_list.begin() ; it != this->windows_list.end() ; it++)
		session_url.push_back((*it)->geturi());
	
	haku::list::vectorStr_save(session_url, configdir + "/session/" + name + ".txt");
}

void application::session_remove(std::string name, std::string uri)
{
	debug_v("session::remove " << uri << " from " << name);
	bool found = false;
	haku::list::vectorStr session_url = haku::list::vectorStr_load(configdir + "/session/" + name + ".txt");
	
	//~ recherche du lien dans le fichier de session
	for (haku::list::vectorStr::iterator it = session_url.begin() ; !found && it != session_url.end() ; ++it)
	{
		if ( *it == uri )
		{
			debug_vv("session::remove uri found");
			session_url.erase(it);
			found = true;
		}
	}
	
	//~ pas trouvé, essai sans le specificateur de paragraphe
	if ( !found && uri != haku::tools::paragraphless_uri(uri) ) 
		this->session_remove( name, haku::tools::paragraphless_uri(uri) );
	//~ autrement, trouvé, donc sauvegarde
	else
		haku::list::vectorStr_save(session_url, configdir + "/session/" + name + ".txt");
}

void application::session_add(std::string name, std::string uri)
{
	debug_v("session::add " << uri << " to " << name);
	haku::list::vectorStr session_url = haku::list::vectorStr_load(configdir + "/session/" + name + ".txt");
	session_url.push_back(uri);
	haku::list::vectorStr_save(session_url, configdir + "/session/" + name + ".txt");
}

void application::session_delete(std::string name)
{
	debug_v("session::delete  " << name);
	if ( std::remove((configdir + "/session/" + name + ".txt").c_str()) )
		std::cerr << "impossible de supprimer la session" << std::endl;;
}


/*
	Gestion clipboard
*/

void application::setclipboard(GdkAtom atom, const std::string& str)
{
	GtkClipboard *cl = gtk_clipboard_get(atom);
	gtk_clipboard_set_text(cl, str.c_str(), str.size());
	gtk_clipboard_store(cl);
}

std::string application::getclipboard(GdkAtom atom)
{
	GtkClipboard *cl;
	std::string tmp;
	gchar *cltmp;
	
	cl = gtk_clipboard_get(atom);
	
	if ( (cltmp = gtk_clipboard_wait_for_text(cl)) != NULL )
	{
		tmp = cltmp;
		free(cltmp);
	}
	
	return tmp;
}
