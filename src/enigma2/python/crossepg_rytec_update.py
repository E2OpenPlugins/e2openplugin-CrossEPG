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
import zipfile
import os

RYTEC_HOST = "www.rytec.be"

class CrossEPG_Rytec_Source(object):
	def __init__(self):
		self.channels_urls = []
		self.epg_urls = []
		self.description = ""

class CrossEPG_Rytec_Update(Screen):
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
		self["action"] = Label(_("Updating rytec providers..."))
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
			self.session.open(MessageBox, _("Cannot retrieve rytec sources"), type = MessageBox.TYPE_ERROR, timeout = 20)	
		self.close()
		
	def getChannelsUrls(self, root, name):
		ret = []
		for node in root:
			if node.tag == "channel":
				if node.get("name") == name:
					for childnode in node:
						if childnode.tag == "url":
							ret.append(childnode.text)
							
					break
		return ret

	def load(self):
		conn = httplib.HTTPConnection(RYTEC_HOST)
		conn.request("GET", "/")
		httpres = conn.getresponse()
		if httpres.status == 200:
			#/tools/rytec.sources.xml.20110419.zip
			p = re.search('a href\s?=\s?"([\w:\/\.\_\-]+rytec\.sources\.xml\.\d+\.zip)"', httpres.read())
			if p == None:
				return False
			conn.request("GET", p.group(1))
			httpres = conn.getresponse()
			if httpres.status == 200:
				f = open ("/tmp/crossepg_rytec_tmp", "w")
				f.write(httpres.read())
				f.close()
				zip = zipfile.ZipFile("/tmp/crossepg_rytec_tmp", "r")
				files = zip.namelist()
				for file in files:
					if file == "rytec.sources.xml":
						f = open ("/tmp/crossepg_rytec_tmp2", "w")
						f.write(zip.read(file))
						f.close()
						self.loadFromFile("/tmp/crossepg_rytec_tmp2")
				os.unlink("/tmp/crossepg_rytec_tmp")
				os.unlink("/tmp/crossepg_rytec_tmp2")
				return True
		return False

	def loadFromFile(self, filename):
		mdom = xml.etree.cElementTree.parse(filename)
		root = mdom.getroot()
		
		for node in root:
			if node.tag == "source":
				source = CrossEPG_Rytec_Source()
				source.channels_urls = self.getChannelsUrls(root, node.get("channels"))
				for childnode in node:
					if childnode.tag == "description":
						source.description = childnode.text
					elif childnode.tag == "url":
						source.epg_urls.append(childnode.text)
		
				self.sources.append(source)
				
	def save(self, destination):
		os.system("rm -f " + destination + "/rytec_*.conf")
		for source in self.sources:
			p = re.compile('[/:()<>|?*\s-]|(\\\)')
			filename = p.sub('_', source.description).lower()
			if filename[:6] != "rytec_":
				filename = "rytec_" + filename
			f = open(destination + "/" + filename + ".conf", "w")
			f.write("description=" + source.description + "\n")
			f.write("protocol=xmltv\n");
			count = 0
			for url in source.channels_urls:
				f.write("channel_url_" + str(count) + "=" + url + "\n")
				count += 1
				
			count = 0
			for url in source.epg_urls:
				f.write("epg_url_" + str(count) + "=" + url + "\n")
				count += 1
			f.write("preferred_language=eng");
			f.close()
			
