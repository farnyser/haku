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

#include "main.hpp"

int main(int argc, char **argv) 
{
	application app(argv[0], (std::string(getenv("HOME")) + "/.config/" + APP_NAME), getenv("HOME"));	
	
	std::string args("");
	for ( int i = 1 ; i < argc ; i++ ) args += std::string(argv[i]) + " ";
		
	//~ traitement  des arguments
	if (argc >= 2)
	{
		if ( !strcmp(argv[1],"--exec") or !strcmp(argv[1],"-e") )
		{
			args = args.substr(strlen(argv[1]),-1);
			args = haku::string::trim(args);
			debug_v("execcmd from cmdline: " << args);
			app.execcmd(exec_context_shell, args, NULL);			
		}
		else if ( !strcmp(argv[1],"--help") or !strcmp(argv[1],"-h") )
		{
			app.execcmd(exec_context_shell, "help", NULL);			
		}
		else
		{
			if ( argc == 2 && g_file_test(argv[1],G_FILE_TEST_IS_REGULAR) )
				args = "file://" + std::string(argv[1]);
			
			app.loaduri(args);
		}
	}
	//~ pas d'argument
	else
	{
		#if DEFAULT_SESSION_RESTORE
			app.session_restore("default");	
		#else
			app.loaduri("default");
		#endif
	}
	
	//~ boucle evenementielle
	app.mainloop();	

	return EXIT_SUCCESS;
}
