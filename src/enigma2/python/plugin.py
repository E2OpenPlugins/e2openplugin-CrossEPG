from crossepg_main import crossepg_main
from Plugins.Plugin import PluginDescriptor

def Plugins(**kwargs):
	return [PluginDescriptor(name="CrossEPG Downloader", description="An EPG downloader", where = [ PluginDescriptor.WHERE_EXTENSIONSMENU, PluginDescriptor.WHERE_PLUGINMENU ], fnc = crossepg_main.downloader),
			PluginDescriptor(name="CrossEPG Setup", description="An EPG downloader (setup panel)", where = [ PluginDescriptor.WHERE_EXTENSIONSMENU, PluginDescriptor.WHERE_PLUGINMENU ], fnc = crossepg_main.setup),
			PluginDescriptor(name="CrossEPG Auto", description = "An EPG downloader (automatic actions)", where = PluginDescriptor.WHERE_SESSIONSTART, fnc = crossepg_main.autostart)]
