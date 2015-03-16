from enigma import getDesktop,eConsoleAppContainer
from Screens.Screen import Screen
from Components.MenuList import MenuList
#from Components.Label import Label
from Components.Button import Button
from Components.ActionMap import ActionMap

from crossepglib import *
from crossepg_locale import _

import os
import sys

class CrossEPG_Log(Screen):
	def __init__(self, session):
		self.session = session
		if (getDesktop(0).size().width() < 800):
			skin = "%s/skins/log_sd.xml" % (os.path.dirname(sys.modules[__name__].__file__))
		else:
			skin = "%s/skins/log_hd.xml" % (os.path.dirname(sys.modules[__name__].__file__))
		f = open(skin, "r")
		self.skin = f.read()
		f.close()

		Screen.__init__(self, session)
		Screen.setTitle(self, _("CrossEPG") + " - " + _("Log"))

		self["actions"] = ActionMap(["OkCancelActions", "ColorActions"],
		{
			"ok": self.close,
			"cancel": self.close,
			"yellow": self.left,
			"blue": self.right,
			"green": self.read,
			"red": self.delete
		}, -1)

		self["key_yellow"] = Button("<<")
		self["key_blue"] = Button(">>")
		self["key_green"] = Button(_("Read"))
		self["key_red"] = Button(_("Delete"))

		self.data = ''
		self.container = eConsoleAppContainer()
		self.container.appClosed.append(self.appClosed)
		self.container.dataAvail.append(self.dataAvail)

		self.offset = 0
		self.maxoffset = 0

		self["linelist"] = MenuList(list=[], enableWrapAround=True)

		self.config = CrossEPG_Config()
		self.config.load()
		self.db_root = self.config.db_root

	def appClosed(self, retval):
		print "appClosed"
		if retval:
			self.data += '\nexecute error %d' % retval
		self.setList()

	def dataAvail(self, str):
		print "dataAvail: " + str
		self.data += str

	def setList(self):
		if self["linelist"] is not None:
			lines = self.data.split('\n')
			list = []
			for line in lines:
				if self.offset > 0:
					list.append(line[self.offset:len(line)])
				else:
					list.append(line)
				if len(line) > self.maxoffset:
					self.maxoffset = len(line)
			self["linelist"].setList(list)

	def execute(self, cmd):
		if self.container.execute(str(cmd)):
			self.appClosed(-1)		

	def read(self):
		cmd = "cat %s/crossepg.log" % (self.db_root)
		print "[CrossEPG_Log] cmd:  %s" % (cmd)
		self.execute(cmd)

	def delete(self):
		cmd = "rm -f %s/crossepg.log" % (self.db_root)
		print "[CrossEPG_Log] cmd:  %s" % (cmd)
		self.data = ''
		self.execute(cmd)		

	def left(self):
		if self.offset > 0:
			self.offset = self.offset - 20
			self.setList()

	def right(self):
		if self.offset < self.maxoffset - 40:
			self.offset = self.offset + 20
			self.setList()
