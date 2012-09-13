from enigma import getDesktop

from Screens.Screen import Screen

from Components.Label import Label
from Components.Button import Button
from Components.ActionMap import ActionMap
from Components.config import config

from crossepglib import *
from crossepg_locale import _

import os
import sys

class CrossEPG_Info(Screen):
	def __init__(self, session):
		self.session = session
		if (getDesktop(0).size().width() < 800):
			skin = "%s/skins/info_sd.xml" % (os.path.dirname(sys.modules[__name__].__file__))
		else:
			skin = "%s/skins/info_hd.xml" % (os.path.dirname(sys.modules[__name__].__file__))
		f = open(skin, "r")
		self.skin = f.read()
		f.close()

		Screen.__init__(self, session)
		Screen.setTitle(self, _("CrossEPG") + " - " + _("Info"))

		self.config = CrossEPG_Config()
		self.config.load()

		self["version"] = Label("")
		self["create"] = Label("")
		self["last_update"] = Label("")
		self["headersdb_size"] = Label("")
		self["descriptorsdb_size"] = Label("")
		self["indexesdb_size"] = Label("")
		self["aliasesdb_size"] = Label("")
		self["total_size"] = Label("")
		self["channels_count"] = Label("")
		self["events_count"] = Label("")
		self["hashes_count"] = Label("")
		self["actions"] = ActionMap(["SetupActions", "ColorActions", "MenuActions"],
		{
			"red": self.quit,
			"cancel": self.quit,
			"menu": self.quit,
		}, -2)

		self["key_red"] = Button(_("Close"))
		self["key_green"] = Button("")
		self["key_yellow"] = Button("")
		self["key_blue"] = Button("")

		self.wrapper = CrossEPG_Wrapper()
		self.wrapper.addCallback(self.__wrapperCallback)
		if getDistro() == 'ViX':
			self.wrapper.init(CrossEPG_Wrapper.CMD_INFO, config.misc.epgcachepath.value + 'crossepg')
		else:
			self.wrapper.init(CrossEPG_Wrapper.CMD_INFO, self.config.db_root)


	def quit(self):
		if not self.wrapper.running():
			self.close()

	def __wrapperCallback(self, event, param):
		if event == CrossEPG_Wrapper.INFO_HEADERSDB_SIZE:
			self["headersdb_size"].text = _("Headers db size: %s") % (param)
		elif event == CrossEPG_Wrapper.INFO_DESCRIPTORSDB_SIZE:
			self["descriptorsdb_size"].text = _("Descriptors db size: %s") % (param)
		elif event == CrossEPG_Wrapper.INFO_INDEXESDB_SIZE:
			self["indexesdb_size"].text = _("Indexes db size: %s") % (param)
		elif event == CrossEPG_Wrapper.INFO_ALIASESDB_SIZE:
			self["aliasesdb_size"].text = _("Aliases db size: %s") % (param)
		elif event == CrossEPG_Wrapper.INFO_TOTAL_SIZE:
			self["total_size"].text = _("Total size: %s") % (param)
		elif event == CrossEPG_Wrapper.INFO_CHANNELS_COUNT:
			self["channels_count"].text = _("Channels count: %s") % (param)
		elif event == CrossEPG_Wrapper.INFO_EVENTS_COUNT:
			self["events_count"].text = _("Events count: %s") % (param)
		elif event == CrossEPG_Wrapper.INFO_HASHES_COUNT:
			self["hashes_count"].text = _("Hashes count: %s") % (param)
		elif event == CrossEPG_Wrapper.INFO_CREATION_TIME:
			self["create"].text = _("Creation time: %s") % (param)
		elif event == CrossEPG_Wrapper.INFO_UPDATE_TIME:
			self["last_update"].text = _("Last update time: %s") % (param)
		elif event == CrossEPG_Wrapper.INFO_VERSION:
			self["version"].text = _("Version: %s") % (param)

