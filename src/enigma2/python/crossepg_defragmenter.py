from enigma import getDesktop, eTimer

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

import os
import sys

class CrossEPG_Defragmenter(Screen):
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
		self["action"] = Label(_("Defragmentation..."))
		self["summary_action"] = StaticText(_("Defragmentation..."))
		self["status"] = Label("")
		self["progress"] = ProgressBar()
		self["progress"].hide()
		self["actions"] = NumberActionMap(["WizardActions", "InputActions"],
		{
			"back": self.quit
		}, -1)

		self.status = 0
		self.retValue = True
		self.config = CrossEPG_Config()
		self.config.load()
		self.lamedb = self.config.lamedb
		if getImageDistro() != "ViX" and getImageDistro() != "AAF" and getImageDistro() != "openMips":
			self.db_root = self.config.db_root
		else:
			self.db_root = config.misc.epgcachepath.value + 'crossepg'
		if not pathExists(self.db_root):
			if not createDir(self.db_root):
				self.db_root = "/hdd/crossepg"

		self.pcallback = pcallback

		self.wrapper = CrossEPG_Wrapper()
		self.wrapper.addCallback(self.wrapperCallback)

		self.hideprogress = eTimer()
		self.hideprogress.callback.append(self["progress"].hide)

		self.pcallbacktimer = eTimer()
		self.pcallbacktimer.callback.append(self.doCallback)

		if noosd:
			self.wrappertimer = eTimer()
			self.wrappertimer.callback.append(self.startWrapper)
			self.wrappertimer.start(100, 1)
		else:
			self.onFirstExecBegin.append(self.firstExec)

	def firstExec(self):
		if self.isHD:
			self["background"].instance.setPixmapFromFile("%s/images/background_hd.png" % (os.path.dirname(sys.modules[__name__].__file__)))
		else:
			self["background"].instance.setPixmapFromFile("%s/images/background.png" % (os.path.dirname(sys.modules[__name__].__file__)))
		self.startWrapper()

	def startWrapper(self):
		self.wrapper.init(CrossEPG_Wrapper.CMD_DEFRAGMENTER, self.db_root)

	def wrapperCallback(self, event, param):
		if event == CrossEPG_Wrapper.EVENT_READY:
			#self.wrapper.epgdat("%s/ext.epg.dat" % (self.db_root))
			#self.wrapper.lamedb("/etc/enigma2/%s" % (self.lamedb))
			self.wrapper.defrag()

		elif event == CrossEPG_Wrapper.EVENT_END:
			if self.status == 0:
				self.wrapper.save()
			else:
				self.wrapper.delCallback(self.wrapperCallback)
				self.wrapper.quit()
				self.closeAndCallback(self.retValue)
				
			self.status += 1

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
			self.closeAndCallback(self.retValue)

		elif event == CrossEPG_Wrapper.EVENT_ERROR:
			self.session.open(MessageBox, _("CrossEPG error: %s") % (param), type = MessageBox.TYPE_INFO, timeout = 20)
			self.retValue = False
			self.quit()

	def quit(self):
		if self.wrapper.running():
			self.retValue = False
			self.wrapper.quit()
		else:
			self.closeAndCallback(False)

	def closeAndCallback(self, ret):
		self.retValue = ret
		self.close(ret)
		self.pcallbacktimer.start(0, 1)

	def doCallback(self):
		if self.pcallback:
			self.pcallback(self.retValue)

