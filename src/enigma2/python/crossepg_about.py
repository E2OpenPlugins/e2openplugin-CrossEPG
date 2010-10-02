from enigma import getDesktop

from Screens.Screen import Screen

from Components.Label import Label
from Components.Button import Button
from Components.Pixmap import Pixmap
from Components.ActionMap import ActionMap

from Tools.LoadPixmap import LoadPixmap

from crossepglib import *
from crossepg_locale import _

import os
import sys

class CrossEPG_About(Screen):
	def __init__(self, session):
		self.session = session
		if (getDesktop(0).size().width() < 800):
			skin = "%s/skins/about_sd.xml" % (os.path.dirname(sys.modules[__name__].__file__))
		else:
			skin = "%s/skins/about_hd.xml" % (os.path.dirname(sys.modules[__name__].__file__))
		f = open(skin, "r")
		self.skin = f.read()
		f.close()

		Screen.__init__(self, session)
		
		self.config = CrossEPG_Config()
		self.config.load()
		
		self["about"] = Label("")
		self["rytec_pix"] = Pixmap();
		self["rytec_text"] = Label("")
		self["isakota_text"] = Label("")

		self["actions"] = ActionMap(["SetupActions", "ColorActions"],
		{
			"red": self.quit,
			"cancel": self.quit
		}, -2)
		
		self["key_red"] = Button(_("Back"))
		self["key_green"] = Button("")
		self["key_yellow"] = Button("")
		self["key_blue"] = Button("")

		try:
			from version import version
		except Exception, e:
			version = "unknow version"

		self["about"].setText(_("SIFTeam CrossEPG %s (c) 2009-2010 Sandro Cavazzoni\nhttp://code.google.com/p/crossepg/\n\nFor translations many thanks to:\nSpaeleus (italian translations)\n\nFor XMLTV sources many thanks to:") % version)
		self["rytec_text"].setText(_("Rytec (http://www.rytec.be)\n19 xmltv providers for many countries"))
		self["isakota_text"].setText(_("Isakota (http://www.krkadoni.com/)\nProvider for Ex Yugoslavia"))

		self.onFirstExecBegin.append(self.setImages)

	def setImages(self):
		self["rytec_pix"].instance.setPixmapFromFile("%s/images/rytec.png" % (os.path.dirname(sys.modules[__name__].__file__)))

	def quit(self):
		self.close()
	
