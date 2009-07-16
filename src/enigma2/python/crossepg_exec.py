from enigma import *
from crossepglib import CrossEPG_Config
from crossepg_locale import _
from Screens.Screen import Screen
from Components.ScrollLabel import ScrollLabel
from Components.Pixmap import Pixmap
from Components.ActionMap import NumberActionMap
from threading import Thread
from Components.ActionMap import ActionMap
from Tools import Notifications
from Tools.Directories import crawlDirectory, pathExists, createDir
from Screens.MessageBox import MessageBox
from Screens.Console import Console
from Components.Button import Button

import os
import re
import _enigma
import new
import time
import os
import sys

class CrossEPG_Exec(Screen):
	def __init__(self, session, cmd, stop = False):
		self.session = session
		if (getDesktop(0).size().width() < 800):
			skin = "%s/skins/exec_sd.xml" % (os.path.dirname(sys.modules[__name__].__file__))
		else:
			skin = "%s/skins/exec_hd.xml" % (os.path.dirname(sys.modules[__name__].__file__))
		f = open(skin, "r")
		self.skin = f.read()
		f.close()
		self.skin_path = "%s/skins/" % (os.path.dirname(sys.modules[__name__].__file__))
		Screen.__init__(self, session)
		self.cmd = cmd
		self.stop = stop
		self.app = eConsoleAppContainer()
		self.config = CrossEPG_Config()
		self.config.load()
		self["log"] = ScrollLabel("")
		
		self["actions"] = ActionMap(["SetupActions", "ColorActions", "DirectionActions"],
		{
			"red": self.__quit,
			"cancel": self.__quit,
			"up": self["log"].pageUp,
			"down": self["log"].pageDown,
		}, -2)
		
		self.timer = eTimer()
		self.timer.callback.append(self.__execute)
		self.quit = eTimer()
		self.quit.callback.append(self.__quit)
		self.timer.start(200, 1)
	
	def __execute(self):
		importdir = "%s/import_scripts/" % (self.config.db_root)
		if not pathExists(importdir):
			importdir = "/hdd/crossepg/import_scripts/"
		script = "%s/%s" % (importdir, self.cmd)
		self.setTitle("CrossEPG Exec (%s)" % (self.cmd))
		self.app.appClosed.append(self.__cmdFinished)
		self.app.dataAvail.append(self.__cmdData)
		if self.app.execute("%s/%s" % (importdir, self.cmd)):
			self.close()
			
	def __cmdFinished(self, retval):
		self.app.appClosed.remove(self.__cmdFinished)
		self.app.dataAvail.remove(self.__cmdData)
		if not self.stop:
			self.quit.start(2000, 1)

	def __cmdData(self, data):
		self["log"].setText(self["log"].getText() + data)
		
	def __quit(self):
		if self.app.running():
			self.app.kill()
		else:
			self.close()

