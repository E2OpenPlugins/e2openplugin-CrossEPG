from enigma import *
from Screens.MessageBox import MessageBox

from crossepglib import *
from crossepg_downloader import CrossEPG_Downloader
from crossepg_importer import CrossEPG_Importer
from crossepg_converter import CrossEPG_Converter
from crossepg_loader import CrossEPG_Loader
from crossepg_setup import CrossEPG_Setup
from crossepg_menu import CrossEPG_Menu
from crossepg_auto import CrossEPG_Auto

class CrossEPG_Main:
	def __init__(self):
		self.config = CrossEPG_Config()
		self.patchtype = getEPGPatchType()

	def downloader(self, session):
		self.session = session
		CrossEPG_Auto.instance.lock = True
		CrossEPG_Auto.instance.stop()
		self.config.load()
		if self.config.configured == 0:
			self.session.openWithCallback(self.configureCallback, MessageBox, _("You need to configure crossepg before starting downloader.\nWould You like to do it now ?"), type = MessageBox.TYPE_YESNO)
		else:
			self.config.deleteLog()
			self.session.openWithCallback(self.downloadCallback, CrossEPG_Downloader, self.config.providers)
			
	def configureCallback(self, result):
		if result is True:
			self.session.open(CrossEPG_Setup)
		      
	def loaderAsPlugin(self, session):
		self.session = session
		CrossEPG_Auto.instance.lock = True
		CrossEPG_Auto.instance.stop()
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
			CrossEPG_Auto.instance.lock = False

	def importer(self):
		self.session.openWithCallback(self.importerCallback, CrossEPG_Importer)

	def importerCallback(self, ret):
		if ret:
			if self.patchtype != 3:
				self.converter()
			else:
				self.loader()
		else:
			CrossEPG_Auto.instance.lock = False

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
					CrossEPG_Auto.instance.lock = False
		else:
			CrossEPG_Auto.instance.lock = False

	def loader(self):
		self.session.openWithCallback(self.loaderCallback, CrossEPG_Loader)

	def loaderCallback(self, ret):
		CrossEPG_Auto.instance.lock = False

	def setup(self, session, **kwargs):
		CrossEPG_Auto.instance.lock = True
		session.openWithCallback(self.setupCallback, CrossEPG_Menu)

	def setupCallback(self):
		CrossEPG_Auto.instance.lock = False
		CrossEPG_Auto.instance.doneConfiguring()

crossepg_main = CrossEPG_Main()
