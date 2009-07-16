from enigma import *
from crossepglib import *
from crossepg_downloader import CrossEPG_Downloader
from crossepg_importer import CrossEPG_Importer
from crossepg_converter import CrossEPG_Converter
from crossepg_loader import CrossEPG_Loader
from crossepg_setup import CrossEPG_Setup
from crossepg_auto import crossepg_auto

class CrossEPG_Main:
	def __init__(self):
		pass
		
	def downloader(self, session, **kwargs):
		crossepg_auto.stop()
		crossepg_auto.disable()
		session.open(CrossEPG_Downloader, self.__callbackDownloader)

	def __callbackDownloader(self, session, ret):
		if ret:
			config = CrossEPG_Config()
			config.load()
			if config.enable_importer == 1:
				session.open(CrossEPG_Importer, self.__callbackImporter)
			else:
				session.open(CrossEPG_Converter, self.__callbackConverter)
		else:
			crossepg_auto.enable()
			
	def __callbackImporter(self, session, ret):
		if ret:
			session.open(CrossEPG_Converter, self.__callbackConverter)
		else:
			crossepg_auto.enable()
			
	def __callbackConverter(self, session, ret):
		if ret:
			config = CrossEPG_Config()
			config.load()
			patchtype = getEPGPatchType()
			if patchtype == 0 or patchtype == 1:
				session.open(CrossEPG_Loader, self.__callbackLoader)
			elif patchtype == 2 and config.manual_reboot == 0:
				session.open(CrossEPG_Loader, self.__callbackLoader)
			elif config.manual_reboot == 1:
				from Screens.Standby import TryQuitMainloop
				session.open(TryQuitMainloop, 3)
			else:
				crossepg_auto.enable()
		else:
			crossepg_auto.enable()

	def __callbackLoader(self, session, ret):
		crossepg_auto.enable()

	def setup(self, session, **kwargs):
		session.open(CrossEPG_Setup, self.downloader)

	def autostart(self, reason, session):
		crossepg_auto.init(session)
		
crossepg_main = CrossEPG_Main()
