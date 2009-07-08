from enigma import getDesktop, iPlayableService, eTimer, eServiceReference, eEPGCache
from crossepglib import *
from Screens.Screen import Screen
from Components.Label import Label
from Components.Pixmap import Pixmap
from Components.ProgressBar import ProgressBar
from Components.ServiceEventTracker import ServiceEventTracker
from Components.ActionMap import NumberActionMap
from Plugins.Plugin import PluginDescriptor
from ServiceReference import ServiceReference
from threading import Thread
from Components.ActionMap import ActionMap
from Tools import Notifications
from Screens.MessageBox import MessageBox

import Screens.Standby

import os
import re
import _enigma
import new
import time
import os

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
		self.skin_path = "%s/skins/" % (os.path.dirname(sys.modules[__name__].__file__))
		Screen.__init__(self, session)
		
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
		self["actions"] = NumberActionMap(["WizardActions", "InputActions"],
		{
			"back": self.quit
		}, -1)
		
		self.wrapper = CrossEPG_Wrapper()
		self.wrapper.addCallback(self.__wrapperCallback)
		self.wrapper.init(CrossEPG_Wrapper.CMD_INFO, self.config.db_root)
			
	def quit(self):
		if not self.wrapper.running():
			self.close()
	
	def __wrapperCallback(self, event, param):
		if event == CrossEPG_Wrapper.INFO_HEADERSDB_SIZE:
			self["headersdb_size"].text = "Headers db size: %s" % (param)
		elif event == CrossEPG_Wrapper.INFO_DESCRIPTORSDB_SIZE:
			self["descriptorsdb_size"].text = "Descriptors db size: %s" % (param)
		elif event == CrossEPG_Wrapper.INFO_INDEXESDB_SIZE:
			self["indexesdb_size"].text = "Indexes db size: %s" % (param)
		elif event == CrossEPG_Wrapper.INFO_ALIASESDB_SIZE:
			self["aliasesdb_size"].text = "Aliases db size: %s" % (param)
		elif event == CrossEPG_Wrapper.INFO_TOTAL_SIZE:
			self["total_size"].text = "Total size: %s" % (param)
		elif event == CrossEPG_Wrapper.INFO_CHANNELS_COUNT:
			self["channels_count"].text = "Channels count: %s" % (param)
		elif event == CrossEPG_Wrapper.INFO_EVENTS_COUNT:
			self["events_count"].text = "Events count: %s" % (param)
		elif event == CrossEPG_Wrapper.INFO_HASHES_COUNT:
			self["hashes_count"].text = "Hashes count: %s" % (param)
		elif event == CrossEPG_Wrapper.INFO_CREATION_TIME:
			self["create"].text = "Creation time: %s" % (param)
		elif event == CrossEPG_Wrapper.INFO_UPDATE_TIME:
			self["last_update"].text = "Last update time: %s" % (param)
		elif event == CrossEPG_Wrapper.INFO_VERSION:
			self["version"].text = "Version: %s" % (param)
			