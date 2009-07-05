from enigma import *
from crossepglib import *
from crossepg_downloader import CrossEPG_Downloader
from crossepg_converter import CrossEPG_Converter
from crossepg_loader import CrossEPG_Loader
from crossepg_setup import CrossEPG_Setup
from crossepg_auto import crossepg_auto
from Plugins.Plugin import PluginDescriptor

def downloader(session, **kwargs):
	crossepg_auto.stop()
	crossepg_auto.disable()
	session.open(CrossEPG_Downloader, __callbackDownloader)

def __callbackDownloader(session, ret):
	if ret:
		session.open(CrossEPG_Converter, __callbackConverter)
	else:
		crossepg_auto.enable()

def __callbackConverter(session, ret):
	if ret:
		config = CrossEPG_Config()
		config.load()
		patchtype = getEPGPatchType()
		if patchtype == 0 or patchtype == 1:
			session.open(CrossEPG_Loader, __callbackLoader)
		elif patchtype == 2 and config.manual_reboot == 0:
			session.open(CrossEPG_Loader, __callbackLoader)
		elif config.manual_reboot == 1:
			from Screens.Standby import TryQuitMainloop
			session.open(TryQuitMainloop, 3)
		else:
			crossepg_auto.enable()
	else:
		crossepg_auto.enable()

def __callbackLoader(session, ret):
	crossepg_auto.enable()

def configurator(session, **kwargs):
	session.open(CrossEPG_Setup)
	
def autostart(reason, session):
	crossepg_auto.init(session)
		
def Plugins(**kwargs):
	return [PluginDescriptor(name="CrossEPG Downloader", description="An EPG downloader", where = PluginDescriptor.WHERE_PLUGINMENU, fnc = downloader),
			PluginDescriptor(name="CrossEPG Setup", description="An EPG downloader (setup panel)", where = PluginDescriptor.WHERE_PLUGINMENU, fnc = configurator),
			PluginDescriptor(name="CrossEPG Auto", description = "An EPG downloader (automatic actions)", where = PluginDescriptor.WHERE_SESSIONSTART, fnc = autostart)]
