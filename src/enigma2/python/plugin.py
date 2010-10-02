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
	if config.show_plugin == 1 and config.show_extension == 1:
		plugins.append(PluginDescriptor(name="CrossEPG Downloader",
										description=_("An EPG downloader"),
										where = [ PluginDescriptor.WHERE_EXTENSIONSMENU, PluginDescriptor.WHERE_PLUGINMENU ],
										fnc = crossepg_main.downloader))
	elif config.show_extension == 1:
		plugins.append(PluginDescriptor(name="CrossEPG Downloader",
										description=_("An EPG downloader"),
										where = PluginDescriptor.WHERE_EXTENSIONSMENU,
										fnc = crossepg_main.downloader))
	elif config.show_plugin == 1:
		plugins.append(PluginDescriptor(name="CrossEPG Downloader",
										description=_("An EPG downloader"),
										where = PluginDescriptor.WHERE_PLUGINMENU,
										fnc = crossepg_main.downloader))
	
	if config.isQBOXHD():
		plugins.append(PluginDescriptor(name="CrossEPG",
										description=_("CrossEPG setup panel"),
										where = PluginDescriptor.WHERE_PLUGINMENU,
										fnc = crossepg_main.setup))
	else:
		plugins.append(PluginDescriptor(name="CrossEPG",
										description=_("CrossEPG setup panel"),
										where = PluginDescriptor.WHERE_MENU,
										fnc = setup))
										
	plugins.append(PluginDescriptor(name="CrossEPG Auto",
									description = _("CrossEPG automatic actions"),
									where = PluginDescriptor.WHERE_SESSIONSTART,
									fnc = crossepg_main.autostart))
	return plugins;
