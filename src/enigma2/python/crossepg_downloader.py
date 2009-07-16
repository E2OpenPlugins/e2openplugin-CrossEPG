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

class CrossEPG_Downloader(Screen):
	def __init__(self, session, endCallback = None, provider = None):
		self.session = session
		if (getDesktop(0).size().width() < 800):
			skin = "%s/skins/downloader_sd.xml" % (os.path.dirname(sys.modules[__name__].__file__))
		else:
			skin = "%s/skins/downloader_hd.xml" % (os.path.dirname(sys.modules[__name__].__file__))
		f = open(skin, "r")
		self.skin = f.read()
		f.close()
		Screen.__init__(self, session)
		
		self["action"] = Label(_("Starting downloader"))
		self["status"] = Label("")
		self["progress"] = ProgressBar()
		self["progress"].hide()
		self["actions"] = NumberActionMap(["WizardActions", "InputActions"],
		{
			"back": self.__quit
		}, -1)
		
		self.__event_tracker = ServiceEventTracker(screen=self, eventmap=
		{
			iPlayableService.evTunedIn: self.__tuned,
		})
		
		self.ret = True
		self.provider_index = 0
		self.status = 0	
		self.tune_enabled = False
		self.oldService = self.session.nav.getCurrentlyPlayingServiceReference()
		self.config = CrossEPG_Config()
		self.config.load()
		self.provider = provider
		self.endCallback = endCallback
		
		if len(self.config.providers) == 0 and not self.provider:
			if self.endCallback:
				self.endCallback(self.session, True)
			self.close()
		
		self.wrapper = CrossEPG_Wrapper()
		self.wrapper.addCallback(self.__wrapperCallback)
		
		self.timeout = eTimer()
		self.timeout.callback.append(self.__quit)
		
		self.hideprogress = eTimer()
		self.hideprogress.callback.append(self["progress"].hide)
		
		self.wrapper.init(CrossEPG_Wrapper.CMD_DOWNLOADER, self.config.db_root)
	
	def quit(self):
		self.__quit()
	
	def __download(self):
		if self.provider:
			self.wrapper.download(self.provider)
		else:
			service = self.config.getChannelID(self.config.providers[self.provider_index])
			try:
				cservice = self.session.nav.getCurrentlyPlayingServiceReference().toString()
			except Exception, e:
				cservice = None
				
			if service:
				print "[CrossEPG_Downloader] %s service is %s" % (self.config.providers[self.provider_index], service)
				if service == cservice:
					self.wrapper.download(self.config.providers[self.provider_index])
				else:
					self.tune_enabled = True
					self.wrapper.wait()
					self.timeout.start(60000, 1)
					self.session.nav.playService(eServiceReference(service))
			else:
				self.wrapper.quit()
	
	def __wrapperCallback(self, event, param):
		if event == CrossEPG_Wrapper.EVENT_READY:
			self.__download()
			
		elif event == CrossEPG_Wrapper.EVENT_END:
			if self.status == 0:
				self.provider_index += 1
				if self.provider:
					self.status = 1
					self.wrapper.save()
				else:
					if self.provider_index < len(self.config.providers):
						self.__download()
					else:
						self.status = 1
						if self.oldService:
							self.session.nav.playService(self.oldService)
						self.wrapper.save()
			else:
				self.wrapper.quit()
				
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
			
	def __tuned(self):
		if self.tune_enabled:
			self.timeout.stop()
			self.wrapper.download(self.config.providers[self.provider_index])
			self.tune_enabled = False
			
	def __quit(self):
		if self.wrapper.running():
			self.ret = False
			self.wrapper.quit()
		else:
			if self.endCallback:
				self.endCallback(self.session, False)
			self.close()
