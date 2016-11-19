from enigma import getDesktop

from Screens.Screen import Screen
from Screens.MessageBox import MessageBox

from Components.Label import Label
from Components.Sources.StaticText import StaticText
from Components.Button import Button
from Components.ActionMap import ActionMap
from Components.Sources.List import List

from Tools.LoadPixmap import LoadPixmap

from crossepg_downloader import CrossEPG_Downloader
from crossepg_converter import CrossEPG_Converter
from crossepg_loader import CrossEPG_Loader
from crossepg_locale import _
from crossepglib import *

from Tools.Directories import resolveFilename, SCOPE_CURRENT_SKIN
try:
	from Tools.Directories import SCOPE_ACTIVE_SKIN
except:
	pass

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
		if protocol == "xmltv":
			protocol_title = _("XMLTV")
		elif protocol == "opentv":
			protocol_title = _("OpenTV")
			from Components.NimManager import nimmanager
			self.configured_sats = nimmanager.getConfiguredSats()
		elif protocol == "xepgdb":
			protocol_title = _("XEPGDB")
		elif protocol == "script":
			protocol_title = _("Scripts")
		elif protocol == "mhw2":
			protocol_title = _("MHW2")
		self.setup_title = _("CrossEPG") + " - " + protocol_title + ' ' + _("providers")
		Screen.setTitle(self, self.setup_title)

		self.patchtype = getEPGPatchType()
		self.config = CrossEPG_Config()
		self.config.load()
		self.providers = self.config.getAllProviders()
		self.protocol = protocol

		self.has_chnaged = False
		self.old_service = None
		self.onChangedEntry = [ ]
		self.list = []

		self["list"] = List(self.list)
		self["list"].onSelectionChanged.append(self.selectionChanged)
		self["key_red"] = Button(_("Cancel"))
		self["key_green"] = Button(_("Save"))
		self["key_yellow"] = Button(_("Download"))
		self["key_blue"] = Button("")
		self["setupActions"] = ActionMap(["SetupActions", "ColorActions", "MenuActions"],
		{
			"red": self.keyCancel,
			"cancel": self.keyCancel,
			"green": self.keySave,
			"ok": self.switchState,
			"yellow": self.download,
			"menu": self.keyCancel,
		}, -2)

		self.buildList()

	# for summary:
	def changedEntry(self):
		for x in self.onChangedEntry:
			x()

	def getCurrentEntry(self):
		return self["list"].getCurrent() and self["list"].getCurrent()[0] or ""

	def getCurrentValue(self):
		try:
			if self["list"].getCurrent()[1]:
				return _("Enabled")
			else:
				return _("Disabled")
		except:
			return ""

	def createSummary(self):
		from crossepg_menu import CrossEPG_MenuSummary
		return CrossEPG_MenuSummary

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
				elif protocol == "opentv":
					if self.config.getTransponder(provider)["orbital_position"] in self.configured_sats:
						self.list.append(self.buildListEntry(provider, self.providers[1][i], self.config.providers.count(provider) > 0))
				else:
					self.list.append(self.buildListEntry(provider, self.providers[1][i], self.config.providers.count(provider) > 0))
			i += 1

		self["list"].setList(self.list)

	def buildListEntry(self, name, description, enabled):
		if enabled:
			try:
				png = resolveFilename(SCOPE_ACTIVE_SKIN, "crossepg/enabled.png")
			except:
				png = resolveFilename(SCOPE_CURRENT_SKIN, "skin_default/crossepg/enabled.png")
			if png == None or not os.path.exists(png):
				png = "%s/images/enabled.png" % os.path.dirname(sys.modules[__name__].__file__)
			pixmap = LoadPixmap(cached=True, path=png)
			return((description, pixmap, name))
		else:
			return((description, None, name))

	def selectionChanged(self):
		if len(self.list) == 0:
			return
		self.has_chnaged = True

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
		self.session.openWithCallback(self.downloadCallback, CrossEPG_Downloader, [self.list[index][2],])

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

	def cancelConfirm(self, result):
		if not result:
			return

		self.close()

	def keyCancel(self):
		if self.has_chnaged :
			self.session.openWithCallback(self.cancelConfirm, MessageBox, _("Really close without saving settings?"))
		else:
			self.close()

	def keySave(self):
		self.config.save()
		self.close()

