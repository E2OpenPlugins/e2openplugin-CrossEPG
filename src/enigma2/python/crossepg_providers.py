from enigma import getDesktop

from Screens.Screen import Screen

from Components.Label import Label
from Components.Button import Button
from Components.ActionMap import ActionMap
from Components.Sources.List import List

from Tools.LoadPixmap import LoadPixmap

from crossepg_downloader import CrossEPG_Downloader
from crossepg_converter import CrossEPG_Converter
from crossepg_loader import CrossEPG_Loader
from crossepg_locale import _

from crossepglib import *


class CrossEPG_Providers(Screen):
	def __init__(self, session, protocol):
		if (getDesktop(0).size().width() < 800):
			skin = "%s/skins/providers_sd.xml" % os.path.dirname(sys.modules[__name__].__file__)
		else:
			skin = "%s/skins/providers_hd.xml" % os.path.dirname(sys.modules[__name__].__file__)
		f = open(skin, "r")
		self.skin = f.read()
		f.close()
		Screen.__init__(self, session)

		self.patchtype = getEPGPatchType()
		self.config = CrossEPG_Config()
		self.config.load()
		self.providers = self.config.getAllProviders()
		self.protocol = protocol

		self.old_service = None
		self.list = []

		self["list"] = List(self.list)
		self["list"].onSelectionChanged.append(self.selectionChanged)
		self["key_red"] = Button(_("Back"))
		self["key_green"] = Button(_("Enable"))
		self["key_yellow"] = Button(_("Download"))
		self["key_blue"] = Button("")
		self["setupActions"] = ActionMap(["SetupActions", "ColorActions"],
		{
			"red": self.quit,
			"cancel": self.quit,
			"green": self.switchState,
			"ok": self.switchState,
			"yellow": self.download,
		}, -2)

		self.buildList()
		self.onFirstExecBegin.append(self.setTitleByProtocol)
		self.onFirstExecBegin.append(self.selectionChanged)

	def setTitleByProtocol(self):
		if self.protocol == "xmltv":
			self.setTitle("CrossEPG - XMLTV providers")
		elif self.protocol == "opentv":
			self.setTitle("CrossEPG - OpenTV providers")
		elif self.protocol == "xepgdb":
			self.setTitle("CrossEPG - XEPGDB providers")
		elif self.protocol == "script":
			self.setTitle("CrossEPG - Scripts providers")
		elif self.protocol == "mhw2":
			self.setTitle("CrossEPG - MHW2 providers")

	def buildList(self):
		self.list = []
		i = 0
		protocol = self.protocol
		if protocol == "mhw2":
			protocol = "script"
		print protocol
		print self.protocol
		for provider in self.providers[0]:
			if self.providers[2][i] == protocol:
				if protocol == "script":
					description = self.providers[1][i].lower()
					# we use find("mhw2") as workaround because mhw2 doesn't exist as provider type
					if self.protocol == "mhw2" and description.find("mhw2") != -1:
						self.list.append(self.buildListEntry(provider, self.providers[1][i], self.config.providers.count(provider) > 0))
					elif self.protocol == "script" and description.find("mhw2") == -1:
						self.list.append(self.buildListEntry(provider, self.providers[1][i], self.config.providers.count(provider) > 0))
				else:
					self.list.append(self.buildListEntry(provider, self.providers[1][i], self.config.providers.count(provider) > 0))
			i += 1

		self["list"].setList(self.list)

	def buildListEntry(self, name, description, enabled):
		if enabled:
			pixmap = LoadPixmap(cached=True, path="%s/images/enabled.png" % os.path.dirname(sys.modules[__name__].__file__))
			return((description, pixmap, name))
		else:
			return((description, None, name))
		
	def selectionChanged(self):
		if len(self.list) == 0:
			return

		index = self["list"].getIndex()
		provider = self.list[index][2]
		if self.config.providers.count(provider) > 0:
			self["key_green"].setText(_("Disable"))
		else:
			self["key_green"].setText(_("Enable"))

	def switchState(self):
		if len(self.list) == 0:
			return

		index = self["list"].getIndex()
		provider = self.list[index][2]
		if self.config.providers.count(provider) > 0:
			self.config.providers.remove(provider)
		else:
			self.config.providers.append(provider)

		self.buildList()
		self["list"].setIndex(index)

	def download(self):
		if len(self.list) == 0:
			return

		index = self["list"].getIndex()
		self.session.openWithCallback(self.downloadCallback, CrossEPG_Downloader, [self.list[index][2], ])

	def downloadCallback(self, ret):
		if ret:
			if self.patchtype != 3:
				self.converter()
			else:
				self.loader()

	def converter(self):
		self.session.openWithCallback(self.converterCallback, CrossEPG_Converter)

	def converterCallback(self, ret):
		if ret and self.patchtype != -1:
			self.loader()

	def loader(self):
		self.session.open(CrossEPG_Loader)

	def quit(self):
		self.config.save()
		self.close()

