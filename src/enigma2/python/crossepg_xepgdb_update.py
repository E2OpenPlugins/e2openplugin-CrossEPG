from enigma import getDesktop, eTimer

from Components.Label import Label
from Components.Pixmap import Pixmap
from Components.ProgressBar import ProgressBar

from Screens.Screen import Screen
from Screens.MessageBox import MessageBox

from crossepglib import *
from crossepg_locale import _

import httplib
import xml.etree.cElementTree
import re
import os

SIFTEAM_HOST = "crossepg.sifteam.eu"

class CrossEPG_Xepgdb_Source(object):
	def __init__(self):
		self.headers_url = ""
		self.descriptors_url = ""
		self.description = ""

class CrossEPG_Xepgdb_Update(Screen):
	def __init__(self, session):
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
		
		self.sources = []
		self.session = session
		
		self["background"] = Pixmap()
		self["action"] = Label(_("Updating xepgdb providers..."))
		self["status"] = Label("")
		self["progress"] = ProgressBar()
		self["progress"].hide()
		
		self.config = CrossEPG_Config()
		self.config.load()
		
		self.timer = eTimer()
		self.timer.callback.append(self.start)
		self.timer.start(100, 1)
		
	def start(self):
		if self.load():
			self.save(self.config.home_directory + "/providers/")
			self.session.open(MessageBox, _("Providers updated"), type = MessageBox.TYPE_INFO, timeout = 20)	
		else:
			self.session.open(MessageBox, _("Cannot retrieve xepgdb sources"), type = MessageBox.TYPE_ERROR, timeout = 20)	
		self.close()
		
	def load(self):
		conn = httplib.HTTPConnection(SIFTEAM_HOST)
		conn.request("GET", "/sources.xml")
		httpres = conn.getresponse()
		if httpres.status == 200:
			f = open ("/tmp/crossepg_xepgdb_tmp", "w")
			f.write(httpres.read())
			f.close()
			self.loadFromFile("/tmp/crossepg_xepgdb_tmp")
			os.unlink("/tmp/crossepg_xepgdb_tmp")
			return True
		return False

	def loadFromFile(self, filename):
		mdom = xml.etree.cElementTree.parse(filename)
		root = mdom.getroot()
		
		for node in root:
			if node.tag == "source":
				source = CrossEPG_Xepgdb_Source()
				for childnode in node:
					if childnode.tag == "description":
						source.description = childnode.text
					elif childnode.tag == "headers":
						source.headers = childnode.text
					elif childnode.tag == "descriptors":
						source.descriptors = childnode.text
		
				self.sources.append(source)
				
	def save(self, destination):
		os.system("rm -f " + destination + "/xepgdb_*.conf")
		for source in self.sources:
			p = re.compile('[/:()<>|?*\s-]|(\\\)')
			filename = p.sub('_', source.description).lower()
			if filename[:7] != "xepgdb_":
				filename = "xepgdb_" + filename
			f = open(destination + "/" + filename + ".conf", "w")
			f.write("description=" + source.description + "\n")
			f.write("protocol=xepgdb\n");
			f.write("headers_url =" + source.headers + "\n")
			f.write("descriptors_url =" + source.descriptors + "\n")
			f.close()
			
