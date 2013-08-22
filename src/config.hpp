//~ Protège contre une doucle inclusion, ne pas supprimer
#ifndef __CONFIG_HPP__
#define __CONFIG_HPP__
	
	//~ Necessaire pour les bindings et la carte de fonction - ne pas modifier
	#include "app_window.hpp"	
	#include "interpreter.hpp"	


	//~ Il peut etre interessant de se faire passer 
	//~ pour chrome ou safari, dans le cas d'un site
	//~ executant un script en fonction du navigateur
	//~ ----------------------------------------------------
	#define USER_AGENT 							"Mozilla/5.0 HAKU (AppleWebKit Safari Chrome)"


	//~ Option session/cookies/historique... 
	//~ ----------------------------------------------------
	#define SESSION_TIME 						3600*24*7


	//~ Homepage et moteur de recherche par defaut
	//~ ----------------------------------------------------
	#define DEFAULT_SEARCHENGINE				"http://www.google.fr/search?btnI&q="
	#define DEFAULT_HOMEPAGE					"http://www.google.fr"


	//~ (Des)Activation du javascript et des plugins 
	//~ ----------------------------------------------------

	#define DEFAULT_ENABLE_ADBLOCK				true
	#define DEFAULT_ENABLE_JAVASCRIPT			false
	#define DEFAULT_ENABLE_PLUGINS				false
	#define DEFAULT_ENABLE_COOKIES				true
	#define COOKIES_NO_THIRDPARTY				true


	//~ Comportement session 
	//~ ----------------------------------------------------

	#define DEFAULT_SESSION_SAVE				true
	#define DEFAULT_SESSION_RESTORE				true
	#define DEFAULT_SESSION_CLEAN				true


	//~ Options interface 
	//~ ----------------------------------------------------

	//~ Couleur barre indicatrice
	#define	COLOR_INDICATOR_COMMAND_SECURE		"#FF9045"
	#define	COLOR_INDICATOR_INSERT_SECURE		"#FF3060"
	#define	COLOR_INDICATOR_COMMAND				"#00FF00"
	#define	COLOR_INDICATOR_INSERT				"#FF0000"
	#define	COLOR_INDICATOR_COMMANDLINE			"#CCBBFF"
	#define	COLOR_INDICATOR_COMMANDLINE_FG		"#000000"
	#define	COLOR_INDICATOR_COMMANDLINE_FG2		"#444499"
	

	//~ Affiche ou non les scrollbars
	#define	SHOW_SCROLLBAR				false


	//~ Gestion user agent dynamique
	//~ ----------------------------------------------------
	//~ Ne pas modifier ces definitions
	typedef struct {
			std::string name;
			std::string agent;
	} useragent_t;

	const static useragent_t useragent[] = 
	{
		{"default",USER_AGENT},
		{"haku",USER_AGENT},
		{"firefox","Mozilla/5.0 X11 Linux x86_64 rv 5.0 Gecko/20100101 Firefox/5.0"},
		{"opera","Opera/9.80 (X11; Linux x86_64; U; fr) Presto/2.2.15 Version/10.00"},
		{"chrome"," Mozilla/5.0 (X11; U; Linux x86_64; en-US) AppleWebKit/532.0 (KHTML, like Gecko) Chrome/4.0.202.0 Safari/532.0"},
		{"safari","Mozilla/5.0 (Macintosh; U; Intel Mac OS X 10_5_5; fr-fr) AppleWebKit/525.18 (KHTML, like Gecko) Version/3.1.2 Safari/525.20.1"},
		{"iphone"," Mozilla/5.0 (iPhone; U; CPU iPhone OS 4_0 like Mac OS X; en-us) AppleWebKit/532.9 (KHTML, like Gecko) Version/4.0.5 Mobile/8A293 Safari/6531.22.7"},
		{"dhd","Mozilla/5.0 (Linux; U; Android 2.2.1; fr-fr; HTC_DesireHD_A9191 Build/FRG83D) AppleWebKit/ 533.1 (KHTML, like Gecko) Version/4.0 Mobile Safari/533.1"},
		{"",""}
	};
	
	
	
	//~ Gestion url particulières 
	//~ ----------------------------------------------------
	#define OPEN_SPECIAL_LINK					true
	
	#if OPEN_SPECIAL_LINK
		//~ Ne pas modifier ces definitions
		typedef struct {
			std::string uri, cmd;
		} OSL;

		//~ Doit terminer par un element vide {"",""}
		//~ Le format est: { regex, commande }
		//~ dans commande, %uri est remplacer par l'url
		//~ et %localpath par le path vers un fichier tout juste telecharge
		const static OSL open_special_link[] = 
		{
			{"magnet:*", "exec /usr/bin/transmission '%uri'"},
			{"*.torrent:", "exec /usr/bin/transmission '%uri'"},
			{"",""}
		};
	#endif // OPEN_SPECIAL_LINK


	//~ Options diverses 
	//~ ----------------------------------------------------

	// l'ouverture d'une nouvelle fenetre (à l'aide de :n ou :new) se fait dans un nouveau process ?
	#define NEW_WINDOW_NEW_PROCESS true
	
	// cache en ram, utilisé pour precedent/suivant
	#define OPT_PAGE_CACHE 						true
	// cache en ram, utilisé pour la navigation
	#define OPT_CACHE 							true
	// charge les images
	#define OPT_LOAD_IMAGE						true

	// Desactive(false) ou active(langue) la correction orthographique
	// necessite hunspell (?)
	#define SPELL_CHEKING 						"fr_FR"

	// tab permet de se deplacer entre les elements de la page
	#define OPT_TAB_CYCLE 						true
	// depalcement entre les elements possibles avec les fleches directionnelles
	#define OPT_SPATIAL_NAVIGATION 				true

	// Active ou non le support des scripts utilisateurs
	#define USERJS_ENABLED						true

	// Path vers un .css custom a appliquer à chaque page, ou false
	#define OPT_USER_STYLESHEET					false

	// Un clic dans la page switch le mode insert/command selon l'endroit clické
	#define OPT_CLICK_SWITCH_MODE				true

	// N'accepte que les certificats reconnus par le système (openssl)
	#define OPT_STRICT_SSL						true

	// Dézoom auto lorsque la fenetre  est trop petite.
	#define OPT_ADAPTATIVE_ZOOM					true

	// Active ou non le telechargement auto quand inaffichable
	#define DOWNLOAD_AUTO						false

	// Coefficient d'égalité necessaire pour qu'un resultat soit utilisé en complétion
	#define COMPLETION_MINIMAL_DISTANCE 		0.70
	#define COMPLETION_USEFORCE_DISTANCE 		0.90 // mettre 1 pour desactiver
	#define COMPLETION_USE_WORDLIST 			false
	
	// Empeche l'insertion de caractère lorsque l'on est en mode command
	#define COMMAND_PREVENT_INSERTING			true
	


	//~ KeyBinding
	//~ ----------------------------------------------------
	//~ ne pas modifier cette definition

	typedef struct {
		app_mode mode;
		unsigned int key;
		unsigned int modifier;
		std::string cmd;
	} keybinding_t;

	//~ Doit terminer par un element vide {mode_null,0,0,""}
	//~ Le format est: { keycode, modifier(ou 0), commande interne }
	//~ dans "commande interne", %uri est remplacé par l'url courante,
	//~ %clipboard est remplacé par le contenu du presse-papier (ctrl-c/v)
	//~ et %Xclipboard est remplacé par le contenu du presse-papier X (middle click)
	const static keybinding_t keybinding[] = 
	{
		{mode_all, GDK_Escape, false, "setmode command"},
		
		//firefox style
		{mode_all, GDK_g, GDK_CONTROL_MASK, "setmode commandline :open google %empty"},
		{mode_all, GDK_o, GDK_CONTROL_MASK, "setmode commandline :open %empty"},
		{mode_all, GDK_t, GDK_CONTROL_MASK, "setmode commandline :new %empty"},
		{mode_all, GDK_s, GDK_CONTROL_MASK, "save"},
		{mode_all, GDK_S, GDK_CONTROL_MASK, "setmode commandline :save %empty"},
		{mode_all, GDK_w, GDK_CONTROL_MASK, "quit"},
		
		//vi style
		{mode_command, GDK_i, false, "setmode insert"},
		{mode_command, GDK_colon, false, "setmode commandline :"},
		{mode_command, GDK_semicolon, false, "setmode commandline :open %uri"},
		{mode_command, GDK_slash, false, "setmode commandline /"},
		{mode_command, GDK_KP_Divide, false, "setmode commandline /"},
		{mode_command, GDK_c, false, "stop"},
		{mode_command, GDK_r, false, "reload"},
		{mode_command, GDK_s, false, "source"},
		{mode_command, GDK_o, false, "open default"},
		{mode_command, GDK_n, false, "new default"},
		{mode_command, GDK_g, false, "open google"},
		{mode_command, GDK_KP_Add, false, "zoom in"},
		{mode_command, GDK_plus, false, "zoom in"},
		{mode_command, GDK_KP_Subtract, false, "zoom out"},
		{mode_command, GDK_minus, false, "zoom out"},
		{mode_command, GDK_H, false, "previous"},
		{mode_command, GDK_L, false, "next"},
		{mode_command, GDK_h, false, "scroll horizontal -1"},
		{mode_command, GDK_j, false, "scroll vertical +1"},
		{mode_command, GDK_k, false, "scroll vertical -1"},
		{mode_command, GDK_l, false, "scroll horizontal +1"},
		{mode_command, GDK_Page_Up, false, "scroll vertical -4"},
		{mode_command, GDK_Page_Down, false, "scroll vertical +4"},
		{mode_command, GDK_Home, false, "scroll vertial -10000"},
		{mode_command, GDK_End, false, "scroll vertial +10000"},
		{mode_command, GDK_p, false, "open %clipboard"},
		{mode_command, GDK_y, false, "yank %uri"},
		
		//~ demo mini-script
		{mode_command, GDK_x, false, "js on \n plugins on \n  cookies on"},
		{mode_command, GDK_x, GDK_CONTROL_MASK, "js off \n plugins off \n  cookies off"},
		
		{mode_null, 0, 0, ""}
	};
	
	
	
	//~ Ligne de commande
	//~ ----------------------------------------------------
	//~ ne pas modifier ces definitions
	
	namespace interpreter 
	{			
		//~ Doit terminer par un eleement vide {"",null,""}
		//~ Le format est: { nom, fonction, nom_commande }
		const static map_t map[] = 
		{
			{"foreach", interpreter::foreach, ""},
			{"exec", interpreter::shexec, ""},
			{"print", interpreter::print, ""},
			{"o", interpreter::open, ""},
			{"open", interpreter::open, ""},
			#if NEW_WINDOW_NEW_PROCESS
			{"n", interpreter::open_newpid, ""},
			{"new", interpreter::open_newpid, ""},
			#else // NEW_WINDOW_SAME_PROCESS
			{"n", interpreter::open_new, ""},
			{"new", interpreter::open_new, ""},
			#endif
			{"q", interpreter::quit, ""},
			{"quit", interpreter::quit, ""},
			{"source", interpreter::source, ""},
			{"stop", interpreter::stop, ""},
			{"reload", interpreter::reload, ""},
			{"setmode", interpreter::setmode, ""},
			{"navigate", interpreter::navigate, ""},
			{"session_add", interpreter::session_add, ""},
			{"zoom", interpreter::zoom, ""},
			{"scroll", interpreter::scroll, ""},
			{"yank", interpreter::yank, ""},
			{"help", interpreter::help, ""},
			{"about", interpreter::about, ""},
			{"version", interpreter::about, ""},
			{"session", interpreter::session, ""},
			{"bm", interpreter::bookmark, ""},
			{"bookmark", interpreter::bookmark, ""},
			{"se", interpreter::searchengine, ""},
			{"searchengine", interpreter::searchengine, ""},
			{"save", interpreter::save, ""},
			{"set", interpreter::set, ""},
			{"plugins", NULL, "set plugins %1 %2"},
			{"javascript", NULL, "set javascript %1 %2"},
			{"js", NULL, "set javascript %1 %2"},
			{"cookies", NULL, "set cookies %1 %2"},
			{"qw", NULL, "session_add \n quit"},
			{"qa", NULL, "foreach q"},
			{"qaw", NULL, "foreach qw"},
			{"prev", NULL, "navigate -1"},
			{"previous", NULL, "navigate -1"},
			{"next", NULL, "navigate +1"},
			// les deux commandes suivantes ne fonctionnent que
			// sur une page de telechargement (en cours ou terminé)
			{"mplayer", NULL, "exec mplayer %localpath"},
			{"acroread", NULL, "exec acroread %localpath"},
			{"", NULL, ""}
		};	
	}
	
	//~ Template pour :session/bookmark/searchengine show
	//~ ----------------------------------------------------

	#define HTML_HEADER "<html><head><title>%type% show</title></head><body>"
	#define HTML_CONTENT_BOOKMARK "<a href=\"%item_uri%\">%item_title%</a> [ %item_uri% ] <br />"
	#define HTML_CONTENT_SEARCHENGINE "<a href=\"%item_uri%\">%item_title%</a> [ %item_uri% ] <br />"
	#define HTML_CONTENT_SESSION "<a href=\"%item_uri%\">%item_uri%</a> <br />"
	#define HTML_FOOTER "</body>"

	//~ Quelques constante par default 
	//~ (au cas ou non defini a la compilation) 
	//~ ne pas supprimer
	//~ ----------------------------------------------------
	#define APP_NAME "haku"
	#define APP_HOMEPAGE "http://scawf.info/haku"
	#ifndef APP_VERSION
		#define APP_VERSION "APP_VERSION not set"
	#endif
	#ifndef DEBUG
		#define DEBUG false
	#endif
	#ifndef DEBUG_VERBOSE
		#define DEBUG_VERBOSE false
	#endif

#endif // __CONFIG_HPP__
