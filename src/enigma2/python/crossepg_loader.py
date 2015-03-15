from enigma import getDesktop, eTimer

from Components.config import config
from Components.Label import Label
from Components.Sources.StaticText import StaticText
from Components.Pixmap import Pixmap
from Components.ProgressBar import ProgressBar
from Components.ActionMap import NumberActionMap
from Components.config import config

from Screens.Screen import Screen
from Screens.MessageBox import MessageBox

from boxbranding import getImageDistro
from crossepglib import *
from crossepg_locale import _

import _enigma
import os
import sys

class CrossEPG_Loader(Screen):
	def __init__(self, session, pcallback = None, noosd = False):
		self.session = session
		if (getDesktop(0).size().width() < 800):
			skin = "%s/skins/downloader_sd.xml" % os.path.dirname(sys.modules[__name__].__file__)
			self.isHD = 0
		else:
			skin = "%s/skins/downloader_hd.xml" % os.path.dirname(sys.modules[__name__].__file__)
			self.isHD = 1
		f = open(skin, "r")
		self.skin = f.read()
		f.close()
		Screen.__init__(self, session)
		Screen.setTitle(self, _("CrossEPG"))

		self["background"] = Pixmap()
		self["action"] = Label(_("Loading data"))
		self["summary_action"] = StaticText(_("Loading data"))
		self["status"] = Label("")
		self["progress"] = ProgressBar()
		self["progress"].hide()

		self.retValue = True
		self.config = CrossEPG_Config()
		self.config.load()
		if getImageDistro() == 'openvix':
			self.db_root = config.misc.epgcachepath.value + 'crossepg'
		else:
			self.db_root = self.config.db_root
		if not pathExists(self.db_root):
			if not createDir(self.db_root):
				self.db_root = "/hdd/crossepg"

		self.pcallback = pcallback
		self.wrapper = None

		self.pcallbacktimer = eTimer()
		self.pcallbacktimer.callback.append(self.doCallback)

		if pathExists("/usr/crossepg"):
			self.home_directory = "/usr/crossepg"
		elif pathExists("/var/crossepg"):
			self.home_directory = "/var/crossepg"
		else:
			print "[CrossEPG_Config] ERROR!! CrossEPG binaries non found"

		# check for common patches
		try:
			self.xepgpatch = new.instancemethod(_enigma.eEPGCache_crossepgImportEPGv21,None,eEPGCache)
			print "[CrossEPG_Loader] patch crossepg v2.1 found"
		except Exception, e:
			self.xepgpatch = None

		try:
			self.epgpatch = new.instancemethod(_enigma.eEPGCache_load,None,eEPGCache)
			print "[CrossEPG_Loader] patch epgcache.load() found"
		except Exception, e:
			self.epgpatch = None

		try:
			self.edgpatch = new.instancemethod(_enigma.eEPGCache_reloadEpg,None,eEPGCache)
			print "[CrossEPG_Loader] patch EDG NEMESIS found"
		except Exception, e:
			self.edgpatch = None

		try:
			self.oudeispatch = new.instancemethod(_enigma.eEPGCache_importEvent,None,eEPGCache)
			print "[CrossEPG_Loader] patch Oudeis found"
		except Exception, e:
			self.oudeispatch = None

		if self.xepgpatch:
			self.timer = eTimer()
			self.timer.callback.append(self.loadEPG2)
			self.timer.start(200, 1)

		elif self.epgpatch:
			self.timer = eTimer()
			self.timer.callback.append(self.loadEPG)
			self.timer.start(200, 1)

		elif self.edgpatch:
			self.timer = eTimer()
			self.timer.callback.append(self.loadEDG)
			self.timer.start(200, 1)

		elif self.oudeispatch:
			self["actions"] = NumberActionMap(["WizardActions", "InputActions"],
			{
				"back": self.quit
			}, -1)

			self.wrapper = CrossEPG_Wrapper()
			self.wrapper.addCallback(self.wrapperCallback)

			self.timeout = eTimer()
			self.timeout.callback.append(self.quit)

			self.hideprogress = eTimer()
			self.hideprogress.callback.append(self["progress"].hide)

			self.epg_channel = None;
			self.epg_tuple = ()
			self.epg_starttime = 0
			self.epg_length = 0
			self.epg_name = ""

			self.wrapper.init(CrossEPG_Wrapper.CMD_CONVERTER, self.db_root)
		else:
			print "No patch found... please reboot enigma2 manually"
			self.closeAndCallback(True)

		if not noosd:
			self.onFirstExecBegin.append(self.firstExec)

	def firstExec(self):
		if self.isHD:
			self["background"].instance.setPixmapFromFile("%s/images/background_hd.png" % (os.path.dirname(sys.modules[__name__].__file__)))
		else:
			self["background"].instance.setPixmapFromFile("%s/images/background.png" % (os.path.dirname(sys.modules[__name__].__file__)))

	def loadEPG2(self):
		print "[CrossEPG_Loader] loading data with crossepg patch v2"
		self.xepgpatch(eEPGCache.getInstance(), self.db_root)
		self.closeAndCallback(True)

	def loadEPG(self):
		try:
			cmd = "%s/crossepg_epgcopy %s/ext.epg.dat %s" % (self.home_directory, self.db_root, config.misc.epgcache_filename.value)
		except Exception, e:
			cmd = "%s/crossepg_epgcopy %s/ext.epg.dat /hdd/epg.dat" % (self.home_directory, self.db_root)

		print "[CrossEPG_Loader] %s" % (cmd)
		os.system(cmd)
		self.epgpatch(eEPGCache.getInstance())
		self.closeAndCallback(True)

	def loadEDG(self):
		cmd = "%s/crossepg_epgcopy %s/ext.epg.dat %s/epg.dat" % (self.home_directory, self.db_root, config.nemepg.path.value)
		print "[CrossEPG_Loader] %s" % (cmd)
		os.system(cmd)
		self.edgpatch(eEPGCache.getInstance())
		self.closeAndCallback(True)

	def wrapperCallback(self, event, param):
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
			self.closeAndCallback(self.retValue)

		elif event == CrossEPG_Wrapper.EVENT_ERROR:
			self.session.open(MessageBox, _("CrossEPG error: %s") % (param), type = MessageBox.TYPE_INFO, timeout = 20)
			self.retValue = False
			self.quit()

	def quit(self):
		if self.wrapper:
			if self.wrapper.running():
				self.retValue = False
				self.wrapper.quit()
				return

		self.closeAndCallback(False)

	def closeAndCallback(self, ret):
		self.retValue = ret
		self.close(ret)
		self.pcallbacktimer.start(0, 1)

	def doCallback(self):
		if self.pcallback:
			self.pcallback(self.retValue)

