from enigma import getDesktop, iPlayableService, eTimer, eServiceReference, eEPGCache
from crossepglib import *
from crossepg_locale import _
from Screens.Screen import Screen
from Components.Label import Label
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

class CrossEPG_Importer(Screen):
	def __init__(self, session, endCallback = None):
		self.session = session
		if (getDesktop(0).size().width() < 800):
			skin = "%s/skins/downloader_sd.xml" % (os.path.dirname(sys.modules[__name__].__file__))
		else:
			skin = "%s/skins/downloader_hd.xml" % (os.path.dirname(sys.modules[__name__].__file__))
		f = open(skin, "r")
		self.skin = f.read()
		f.close()
		Screen.__init__(self, session)
		
		self["action"] = Label(_("Starting importer"))
		self["status"] = Label("")
		self["progress"] = ProgressBar()
		self["progress"].hide()
		self["actions"] = NumberActionMap(["WizardActions", "InputActions"],
		{
			"back": self.__quit
		}, -1)
		
		self.ret = True	
		self.config = CrossEPG_Config()
		self.config.load()
		self.lamedb = self.config.lamedb
		self.db_root = self.config.db_root
		if not pathExists(self.db_root):
			if not createDir(self.db_root):
				self.db_root = "/hdd/crossepg"
				
		self.endCallback = endCallback
		
		self.wrapper = CrossEPG_Wrapper()
		self.wrapper.addCallback(self.__wrapperCallback)
		
		self.hideprogress = eTimer()
		self.hideprogress.callback.append(self["progress"].hide)
		
		self.status = 0
		self.wrapper.init(CrossEPG_Wrapper.CMD_IMPORTER, self.config.db_root)
	
	def quit(self):
		self.__quit()
	
	def __wrapperCallback(self, event, param):
		if event == CrossEPG_Wrapper.EVENT_READY:
			self.wrapper.importx()
			
		elif event == CrossEPG_Wrapper.EVENT_END:
			if self.status == 0:
				self.status += 1
				self.wrapper.save()
			else:
				self.wrapper.delCallback(self.__wrapperCallback)
				self.wrapper.quit()
				if self.endCallback:
					self.endCallback(self.session, self.ret)
				self.close()
				
		elif event == CrossEPG_Wrapper.EVENT_FILE:
			self["action"].text = _("Parsing")
			self["status"].text = param
			
		elif event == CrossEPG_Wrapper.EVENT_URL:
			self["action"].text = _("Downloading")
			self["status"].text = param
			
		elif event == CrossEPG_Wrapper.EVENT_ACTION:
			self["action"].text = param
			
		elif event == CrossEPG_Wrapper.EVENT_STATUS:
			self["status"].text = param
			
		elif event == CrossEPG_Wrapper.EVENT_PROGRESS:
			self["progress"].setValue(param)
			
		elif event == CrossEPG_Wrapper.EVENT_PROGRESSONOFF:
			if param:
				self.hideprogress.stop()
				self["progress"].setValue(0)
				self["progress"].show()
			else:
				self["progress"].setValue(100)
				self.hideprogress.start(500, 1)
				
		elif event == CrossEPG_Wrapper.EVENT_QUIT:
			if self.endCallback:
				self.endCallback(self.session, self.ret)
			self.close()
			
		elif event == CrossEPG_Wrapper.EVENT_ERROR:
			self.session.open(MessageBox, _("CrossEPG error: %s") % (param), type = MessageBox.TYPE_INFO, timeout = 20)
			self.ret = False
			self.quit()
			
	def __quit(self):
		if self.wrapper.running():
			self.ret = False
			self.wrapper.quit()
		else:
			if self.endCallback:
				self.endCallback(self.session, False)
			self.close()
