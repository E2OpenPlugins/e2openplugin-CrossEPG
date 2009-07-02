MyEPG 0.2.5
Program released under the license GNU GPL v2
All credits are in CREDITS.TXT

** INSTALLATION

How to install it:
through the web interface go to Systems->Plugins
In install plugin select the file "myepg-(version)-dgs-sh4.tar.gz" and press "Upload plugin"

** GENERAL INFORMATION

The db files are three. By default they are:
- /media/myepg.headers.db
- /media/myepg.descriptors.db
- /media/myepg.aliases.db

The log files are three. By default thery are:
- /media/myepg.log
- /media/myepg.downloader.log
- /media/myepg.sync.log

The path can be changed from che configration file ('/var/plug-in/myepg.config') or from the configuration menu

Remote keys:
- Arrows		= grid navigation (hold pressed for a fast navigation)
- Ok			= change channel or set the zap (or remove it if already exist)
- Rec			= set the record (or remove it if already exist)
- Page up/down  = change the fav group
- Play			= go to the program transmitted now
- Vol. up/down  = channels page up/down
- i				= more info
- Forward		= go 24h forward
- Backward		= go 24h backward
- Exit 			= quit from plugin

** HINTS AND TIPS

There are some problems with original dgs epg db. It can contain only a limited number of events.
MyEPG download and manage a larger number of events so, if you put all the events into the internal dgs db,
many events are automatically deleted by the dgs software.

Actually the only solution is insert a limited number of events playing with 2 values (in configuration menu):
- sync hours	(how many hours sync with internal db)
- sync groups	(how groups sync with internal db)
They works as a filter on data sync.
