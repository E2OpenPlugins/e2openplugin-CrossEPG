from enigma import getDesktop, iPlayableService, eTimer, eServiceReference, eEPGCache
from crossepglib import *
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
import sys

class CrossEPG_Loader(Screen):
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
		self["action"] = Label("Loading data")
		self["status"] = Label("")
		self["progress"] = ProgressBar()
		self["progress"].hide()
		
		self.ret = True	
		self.config = CrossEPG_Config()
		self.config.load()
		self.db_root = self.config.db_root
		if not pathExists(self.db_root):
			if not createDir(self.db_root):
				self.db_root = "/hdd/crossepg"
				
		self.endCallback = endCallback
		self.wrapper = None
		# check for common patches
		try:
			self.epgpatch = new.instancemethod(_enigma.eEPGCache_load,None,eEPGCache)
			print "[CrossEPG_Loader] patch epgcache.load() found"
		except Exception, e:
			self.epgpatch = None
			print "[CrossEPG_Loader] patch epgcache.load() not found"
			
		try:
			self.edgpatch = new.instancemethod(_enigma.eEPGCache_reloadEpg,None,eEPGCache)
			print "[CrossEPG_Loader] patch EDG NEMESIS found"
		except Exception, e:
			self.edgpatch = None
			print "[CrossEPG_Loader] patch EDG NEMESIS not found"
			
		try:
			self.oudeispatch = new.instancemethod(_enigma.eEPGCache_importEvent,None,eEPGCache)
			print "[CrossEPG_Loader] patch Oudeis found"
		except Exception, e:
			self.oudeispatch = None
			print "[CrossEPG_Loader] patch Oudeis not found"
		
		if self.epgpatch:
			self.timer = eTimer()
			self.timer.callback.append(self.__loadEPG)
			self.timer.start(200, 1)
			
		elif self.edgpatch:
			self.timer = eTimer()
			self.timer.callback.append(self.__loadEDG)
			self.timer.start(200, 1)
			
		elif self.oudeispatch:
			self["actions"] = NumberActionMap(["WizardActions", "InputActions"],
			{
				"back": self.__quit
			}, -1)
			
			self.wrapper = CrossEPG_Wrapper()
			self.wrapper.addCallback(self.__wrapperCallback)
			
			self.timeout = eTimer()
			self.timeout.callback.append(self.__quit)
			
			self.hideprogress = eTimer()
			self.hideprogress.callback.append(self["progress"].hide)
			
			self.epg_channel = None;
			self.epg_tuple = ()
			self.epg_starttime = 0
			self.epg_length = 0
			self.epg_name = ""
			
			self.wrapper.init(CrossEPG_Wrapper.CMD_CONVERTER, self.config.db_root)
		else:
			print "No patch found... please reboot enigma2 manually"
			if self.endCallback:
				self.endCallback(self.session, True)
			self.close()
	
	def quit(self):
		if self.epgpatch or self.edgpatch:
			return
			
		self.__quit()
		
	def __loadEPG(self):
		os.system("cp %s/ext.epg.dat /hdd/epg.dat" % (self.db_root))
		self.epgpatch(eEPGCache.getInstance())
		if self.endCallback:
			self.endCallback(self.session, True)
		self.close()
		
	def __loadEDG(self):
		os.system("cp %s/ext.epg.dat /hdd/epg.dat" % (self.db_root))
		self.edgpatch(eEPGCache.getInstance())
		if self.endCallback:
			self.endCallback(self.session, True)
		self.close()
		
	def __wrapperCallback(self, event, param):
		if event == CrossEPG_Wrapper.EVENT_READY:
			self.wrapper.text()
			
		elif event == CrossEPG_Wrapper.EVENT_END:
			self.wrapper.quit()
			
		elif event == CrossEPG_Wrapper.EVENT_ACTION:
			self["action"].text = param
			
		elif event == CrossEPG_Wrapper.EVENT_STATUS:
			self["status"].text = param
			
		elif event == CrossEPG_Wrapper.EVENT_PROGRESS:
			self["progress"].setValue(param)
			
		elif event == CrossEPG_Wrapper.EVENT_CHANNEL:
			if self.epg_channel:
				if len(self.epg_tuple) > 0:
					self.oudeispatch(eEPGCache.getInstance(), self.epg_channel, self.epg_tuple)
					self.epg_tuple = ()
			self.epg_channel = param
			
		elif event == CrossEPG_Wrapper.EVENT_STARTTIME:
			self.epg_starttime = param
			
		elif event == CrossEPG_Wrapper.EVENT_LENGTH:
			self.epg_length = param
			
		elif event == CrossEPG_Wrapper.EVENT_NAME:
			self.epg_name = param
			
		elif event == CrossEPG_Wrapper.EVENT_DESCRIPTION:
			if self.epg_channel:
				self.epg_tuple += ((self.epg_starttime, self.epg_length, self.epg_name, self.epg_name, param, 0),)
			
		elif event == CrossEPG_Wrapper.EVENT_PROGRESSONOFF:
			if param:
				self.hideprogress.stop()
				self["progress"].setValue(0)
				self["progress"].show()
			else:
				self["progress"].setValue(100)
				self.hideprogress.start(500, 1)
		elif event == CrossEPG_Wrapper.EVENT_QUIT:
			if self.epg_channel:
				if len(self.epg_tuple) > 0:
					self.oudeispatch(eEPGCache.getInstance(), self.epg_channel, self.epg_tuple)
			if self.endCallback:
				self.endCallback(self.session, self.ret)
			self.close()
			
		elif event == CrossEPG_Wrapper.EVENT_ERROR:
			self.session.open(MessageBox, "CrossEPG error: %s" % (param), type = MessageBox.TYPE_INFO, timeout = 20)
			self.ret = False
			self.quit()
			
	def __quit(self):
		if self.wrapper:
			if self.wrapper.running():
				self.ret = False
				self.wrapper.quit()
				return
				
		if self.endCallback:
			self.endCallback(self.session, False)
		self.close()