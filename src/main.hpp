#include <iostream>
#include <cstdlib>
#include <vector>
#include <string>
#include <sstream>

#include <gtk/gtk.h>
#include <webkit/webkit.h>

#include "config.hpp"

#include "app_window.hpp"
#include "application.hpp"
#include "tools/tools.hpp"
#include "tools/string.hpp"
#include "tools/jarowinkler.hpp"

/**
	\mainpage HAKU Webbrowser Documentation
	\section intro_sec Introduction
		\a <a href="http://scawf.info/haku/">Haku</a> is a simple Web browser based on WebKit/GTK+ and inspired by Vi.
	\section install_sec Installation
		\subsection requirement_subsec Required libraries
			- gtk+-2.0
			- gthread-2.0
			- webkit-gtk > 1.2.1
		\subsection installprocess_subsec Instalation process
			- cd src && make
			- su -c 'make install'
**/
