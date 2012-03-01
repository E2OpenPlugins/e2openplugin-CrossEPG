from enigma import getDesktop

from Screens.Screen import Screen

from Components.Label import Label
from Components.Button import Button
from Components.ActionMap import ActionMap
from Components.Sources.List import List

from Tools.LoadPixmap import LoadPixmap

from crossepg_locale import _

from crossepglib import *

class CrossEPG_Ordering(Screen):
	def __init__(self, session):
		if (getDesktop(0).size().width() < 800):
			skin = "%s/skins/providers_sd.xml" % os.path.dirname(sys.modules[__name__].__file__)
		else:
			skin = "%s/skins/providers_hd.xml" % os.path.dirname(sys.modules[__name__].__file__)
		f = open(skin, "r")
		self.skin = f.read()
		f.close()
		Screen.__init__(self, session)
		self.setup_title = _("CrossEPG") + " - " + _("Providers start order")
		Screen.setTitle(self, self.setup_title)

		self.config = CrossEPG_Config()
		self.config.load()
		self.providers = self.config.getAllProviders()
		self.onChangedEntry = [ ]
		self.list = []

		self["list"] = List(self.list)
		self["list"].onSelectionChanged.append(self.selectionChanged)
		self["key_red"] = Button(_("Back"))
		self["key_green"] = Button("")
		self["key_yellow"] = Button("")
		self["key_blue"] = Button("")
		self["setupActions"] = ActionMap(["SetupActions", "ColorActions", "MenuActions"],
		{
			"red": self.quit,
			"cancel": self.quit,
			"green": self.moveUp,
			"yellow": self.moveDown,
			"menu": self.quit,
		}, -2)

		self.buildList()
		self.onFirstExecBegin.append(self.selectionChanged)

	# for summary:
	def changedEntry(self):
		for x in self.onChangedEntry:
			x()

	def getCurrentEntry(self):
		if self["list"].getCurrent():
			return str(self["list"].getCurrent()[0])
		else:
			return ""

	def getCurrentValue(self):
		return ""

	def createSummary(self):
		from crossepg_menu import CrossEPG_MenuSummary
		return CrossEPG_MenuSummary

	def getProviderDescription(self, provider):
		i = 0
		for prv in self.providers[0]:
			if prv == provider:
				return self.providers[1][i]
			i += 1
			
		return provider
	
	def buildList(self):
		self.list = []
		for provider in self.config.providers:
			self.list.append(self.buildListEntry(provider, self.getProviderDescription(provider)))

		self["list"].setList(self.list)

	def buildListEntry(self, name, description):
		return((description, None, name))
		
	def selectionChanged(self):
		if len(self.list) <= 1:
			return

		index = self["list"].getIndex()
		if index == 0:
			self["key_green"].setText("")
			self["key_yellow"].setText(_("Move down"))
		elif index == len(self.list) - 1:
			self["key_green"].setText(_("Move up"))
			self["key_yellow"].setText("")
		else:
			self["key_green"].setText(_("Move up"))
			self["key_yellow"].setText(_("Move down"))

	def moveUp(self):
		if len(self.list) <= 1:
			return

		index = self["list"].getIndex()
		if index > 0:
			tmp = self.config.providers[index - 1]
			self.config.providers[index - 1] = self.config.providers[index]
			self.config.providers[index] = tmp
			
			self.buildList()
			self["list"].setIndex(index - 1)
			
	def moveDown(self):
		if len(self.list) <= 1:
			return

		index = self["list"].getIndex()
		if index < len(self.list) - 1:
			tmp = self.config.providers[index + 1]
			self.config.providers[index + 1] = self.config.providers[index]
			self.config.providers[index] = tmp

			self.buildList()
			self["list"].setIndex(index + 1)

	def quit(self):
		self.config.save()
		self.close()

