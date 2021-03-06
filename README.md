haku
==================================
haku is a simple Web browser based on WebKit/GTK+ and inspired by Vi.


Requirements
------------
In order to build haku you need GTK+ and Webkit/GTK+ header files.


Installation
------------
Edit src/Makefile to match your local setup (haku is installed into /usr/bin by default) 
and edit src/config.hpp to match your preferences.

Afterwards enter the following command to build and install haku (if
necessary as root):

   cd src && make install


Running haku
------------
Run one of the following commands:

	$ haku
	$ haku http://host.domain
	$ haku file://path/file.html
	$ haku bookmark_name
	$ haku searchengine_name keywords
	$ haku keywords

You can also execute a command using:

	$ haku -e cmd
	$ haku --exec cmd

See help (in french):

	$ haku -h
	$ haku --help

	In a running instance of haku:
		:help


Configuring haku
----------------
You can edit src/config.hpp to match your preference.
Warning: help will *not* be updated.


Killing haku
----------------
If DEFAULT_SESSION_SAVE is off, you can still get the session 
back when there is multiple instance of haku running and one is frozen.
The frozen one (and it's multiple windows) won't be saved.

	haku -e session save tmp
	killall haku
	haku -e session restore tmp
	
That will save all working window uri's and restore them. 


Bugs ?
----------------
If you find a bug, please activate the debug output by adding
-DDEBUG=2  at the end of the CPPFLAGS in the Makefile.
Then, send me the output (somes of the last lines) before the bug occures.
