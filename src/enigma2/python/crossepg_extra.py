from enigma import *
from crossepglib import *
from crossepg_auto import crossepg_auto
from crossepg_info import CrossEPG_Info
from crossepg_downloader import CrossEPG_Downloader
from crossepg_importer import CrossEPG_Importer
from crossepg_converter import CrossEPG_Converter
from crossepg_loader import CrossEPG_Loader
from crossepg_exec import CrossEPG_Exec
from crossepg_locale import _

from Screens.Screen import Screen
from Screens.MessageBox import MessageBox

from Components.Label import Label
from Components.Button import Button
from Components.PluginComponent import plugins
from Components.ActionMap import ActionMap
from Components.MenuList import MenuList
from Components.ServiceEventTracker import ServiceEventTracker
from time import *

import _enigma

class CrossEPG_Extra(Screen):
	def __init__(self, session, auto_action):
		if (getDesktop(0).size().width() < 800):
			skin = "%s/skins/extra_sd.xml" % (os.path.dirname(sys.modules[__name__].__file__))
		else:
			skin = "%s/skins/extra_hd.xml" % (os.path.dirname(sys.modules[__name__].__file__))
		f = open(skin, "r")
		self.skin = f.read()
		f.close()
		Screen.__init__(self, session)
		self.config = CrossEPG_Config()
		self.config.load()
		self.scripts = self.config.getAllImportScripts()
		self.session = session
		self.auto_action = auto_action
		self.providers_count = 0
		self.provider_selected = None
		self.old_service = None
		
		menulist = list()
		menulist.append(_("Start automated task"))
		for provider in self.config.getAllProviders():
			menulist.append(_("Download %s") % (provider[1]))
			self.providers_count += 1
			
		for script in self.scripts:
			menulist.append(_("Execute import script %s" % (script)))
			
		menulist.append(_("Import data"))
		menulist.append(_("Convert db"))
		if getEPGPatchType() > -1:
			menulist.append(_("Load data"))
		
		self["menu"] = MenuList(menulist)
		
		self["key_red"] = Button(_("Exit"))
		self["key_green"] = Button(" ")
		self["key_yellow"] = Button(" ")
		self["key_blue"] = Button(" ")
		self["actions"] = ActionMap(["SetupActions", "ColorActions"],
		{
			"red": self.quit,
			"cancel": self.quit,
			"ok": self.ok,
		}, -2)
		self.__event_tracker = ServiceEventTracker(screen=self, eventmap=
		{
			iPlayableService.evTunedIn: self.__tuned,
		})
		
	def ok(self):
		self.provider_selected = None
		selected = self["menu"].getSelectedIndex()
		if selected == 0: # automated task
			self.auto_action(self.session)
		else:
			selected -= 1
			if selected < self.providers_count: # download ...
				service = self.config.getChannelID(self.config.getAllProviders()[0][selected])
				if service:
					self.provider_selected = self.config.getAllProviders()[0][selected]
					self.old_service = self.session.nav.getCurrentlyPlayingServiceReference()
					self.session.nav.playService(eServiceReference(service))
			else:
				selected -= self.providers_count
				if selected < len(self.scripts): # import script ...
					self.session.open(CrossEPG_Exec, self.scripts[selected], True)
				else:
					selected -= len(self.scripts)
					if selected == 0: # import
						self.session.open(CrossEPG_Importer)
					elif selected == 1: # convert
						self.session.open(CrossEPG_Converter)
					elif selected == 2: # load
						self.session.open(CrossEPG_Loader)
	
	def __tuned(self):
		if self.provider_selected:
			self.session.open(CrossEPG_Downloader, self.__downloaderEnded, self.provider_selected)
			self.provider_selected = None
	
	def __downloaderEnded(self, session, ret):
		if self.old_service:
			self.session.nav.playService(self.old_service)
			self.old_service = None
		
	def quit(self):
		self.close()