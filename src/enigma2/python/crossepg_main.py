from enigma import *
from Screens.MessageBox import MessageBox

from crossepglib import *
from crossepg_downloader import CrossEPG_Downloader
from crossepg_importer import CrossEPG_Importer
from crossepg_converter import CrossEPG_Converter
from crossepg_loader import CrossEPG_Loader
from crossepg_setup import CrossEPG_Setup
from crossepg_menu import CrossEPG_Menu
from crossepg_auto import crossepg_auto


class CrossEPG_Main:
	def __init__(self):
		self.config = CrossEPG_Config()
		self.patchtype = getEPGPatchType()
		
	def downloader(self, session):
		self.session = session
		crossepg_auto.lock = True
		crossepg_auto.stop()
		self.config.load()
		if self.config.configured == 0:
			self.session.open(MessageBox, _("Please configure crossepg before start downloader"), type=MessageBox.TYPE_ERROR)
		else:
			self.config.deleteLog()
			self.session.openWithCallback(self.downloadCallback, CrossEPG_Downloader, self.config.providers)

	def loaderAsPlugin(self, session):
		self.session = session
		crossepg_auto.lock = True
		crossepg_auto.stop()
		self.loader()

	def downloadCallback(self, ret):
		if ret:
			if self.config.csv_import_enabled == 1:
				self.importer()
			else:
				if self.patchtype != 3:
					self.converter()
				else:
					self.loader()
		else:
			crossepg_auto.lock = False

	def importer(self):
		self.session.openWithCallback(self.importerCallback, CrossEPG_Importer)

	def importerCallback(self, ret):
		if ret:
			if self.patchtype != 3:
				self.converter()
			else:
				self.loader()
		else:
			crossepg_auto.lock = False

	def converter(self):
		self.session.openWithCallback(self.converterCallback, CrossEPG_Converter)

	def converterCallback(self, ret):
		if ret:
			if self.patchtype != -1:
				self.loader()
			else:
				if self.config.download_manual_reboot:
					from Screens.Standby import TryQuitMainloop
					self.session.open(TryQuitMainloop, 3)
				else:
					crossepg_auto.lock = False
		else:
			crossepg_auto.lock = False

	def loader(self):
		self.session.openWithCallback(self.loaderCallback, CrossEPG_Loader)

	def loaderCallback(self, ret):
		crossepg_auto.lock = False

	def setup(self, session, **kwargs):
		crossepg_auto.lock = True
		crossepg_auto.stop()
		session.openWithCallback(self.setupCallback, CrossEPG_Menu)

	def setupCallback(self):
		crossepg_auto.lock = False

	def autostart(self, reason, session):
		crossepg_auto.init(session)
		

crossepg_main = CrossEPG_Main()
