from crossepg_auto import CrossEPGautostart
from crossepglib import CrossEPG_Config
from crossepg_main import crossepg_main
from crossepg_locale import _
from Plugins.Plugin import PluginDescriptor
from boxbranding import getImageDistro

def setup(menuid, **kwargs):
	if getImageDistro() in ("openvix", "miraclebox", "egami", "miracleboxhd", "egamihd"):
		if menuid == "epg":
			return [("CrossEPG", crossepg_main.setup, "crossepg", None)]
		else:
			return []
	else:
		if menuid == "setup":
			return [("CrossEPG", crossepg_main.setup, "crossepg", None)]
		else:
			return []

def call_downloader(session, **kwargs):
	crossepg_main.downloader(session)

def call_loaderAsPlugin(session, **kwargs):
	crossepg_main.loaderAsPlugin(session)

def call_setup(session, **kwargs):
	crossepg_main.setup(session)

def Plugins(**kwargs):
	config = CrossEPG_Config()
	config.load()
	plugins = list()
	if config.show_plugin == 1 and config.show_extension == 1:
		plugins.append(PluginDescriptor(name=_("CrossEPG") + " " + _("Downloader"),
										description=_("An EPG downloader"),
										where = [ PluginDescriptor.WHERE_EXTENSIONSMENU, PluginDescriptor.WHERE_PLUGINMENU ],
										fnc = call_downloader))
	elif config.show_extension == 1:
		plugins.append(PluginDescriptor(name=_("CrossEPG") + " " + _("Downloader"),
										description=_("An EPG downloader"),
										where = PluginDescriptor.WHERE_EXTENSIONSMENU,
										fnc = call_downloader))
	elif config.show_plugin == 1:
		plugins.append(PluginDescriptor(name=_("CrossEPG") + " " + _("Downloader"),
										description=_("An EPG downloader"),
										where = PluginDescriptor.WHERE_PLUGINMENU,
										fnc = call_downloader))

	if config.isQBOXHD():
		plugins.append(PluginDescriptor(name=_("CrossEPG"),
										description=_("CrossEPG setup panel"),
										where = PluginDescriptor.WHERE_PLUGINMENU,
										fnc = call_setup))
	else:
		plugins.append(PluginDescriptor(name=_("CrossEPG"),
										description=_("CrossEPG setup panel"),
										where = PluginDescriptor.WHERE_MENU,
										fnc = setup))

	plugins.append(PluginDescriptor(name=_("CrossEPG") + " " + _("Auto"),
									description = _("CrossEPG automatic actions"),
									where = PluginDescriptor.WHERE_SESSIONSTART,
									fnc = CrossEPGautostart))

	if config.show_force_reload_as_plugin == 1:
		plugins.append(PluginDescriptor(name=_("CrossEPG") + " " + _("Force Reload"),
										description=_("CrossEPG Force Reload"),
										where = PluginDescriptor.WHERE_PLUGINMENU,
										fnc = call_loaderAsPlugin))

	return plugins;
