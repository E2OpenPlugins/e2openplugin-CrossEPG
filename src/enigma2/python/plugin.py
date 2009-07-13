from crossepglib import CrossEPG_Config
from crossepg_main import crossepg_main
from crossepg_locale import _
from Plugins.Plugin import PluginDescriptor

def setup(menuid, **kwargs):
	if menuid == "setup":
		return [("CrossEPG", crossepg_main.setup, "crossepg", None)]
	else:
		return []

def Plugins(**kwargs):
	config = CrossEPG_Config()
	config.load()
	plugins = list()
	if config.show_downloader == 1:
		plugins.append(PluginDescriptor(name="CrossEPG Downloader",
										description=_("An EPG downloader"),
										where = [ PluginDescriptor.WHERE_EXTENSIONSMENU, PluginDescriptor.WHERE_PLUGINMENU ],
										fnc = crossepg_main.downloader))
										
	plugins.append(PluginDescriptor(name="CrossEPG",
									description=_("An EPG downloader (setup panel)"),
									where = [ PluginDescriptor.WHERE_MENU, PluginDescriptor.WHERE_EXTENSIONSMENU ],
									fnc = setup))
										
	plugins.append(PluginDescriptor(name="CrossEPG Auto",
									description = _("An EPG downloader (automatic actions)"),
									where = PluginDescriptor.WHERE_SESSIONSTART,
									fnc = crossepg_main.autostart))
	return plugins;
