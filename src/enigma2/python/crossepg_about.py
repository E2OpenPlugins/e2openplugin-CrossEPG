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
		self["rytec_pix"] = Pixmap()
		self["rytec_text"] = Label("")
		self["krkadoni_pix"] = Pixmap()
		self["krkadoni_text"] = Label("")

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

		credit = "SIFTeam CrossEPG %s (c) 2009-2011 Sandro Cavazzoni\n" % version
		credit += "http://code.google.com/p/crossepg/\n\n"
		credit += "Application credits:\n"
		credit += "- Sandro Cavazzoni aka skaman (main developer)\n"
		credit += "- Ambrosa (scripts developer)\n"
		credit += "- Sergiotas (mhw2epgdownloader author)\n"
		credit += "- u Killer Bestia (server side application maintainer)\n"
		credit += "- Spaeleus (italian translations)\n"
		credit += "- Bodyan (ukrainian translations)\n"
		credit += "- Kosmacz (polish translations)\n"
		credit += "- Ku4a (russian translations)\n\n"
		credit += "Sources credits:\n"
		credit += "- Rytec http://www.rytec.be (xmltv providers for many countries)\n"
		credit += "- Krkadoni http://www.krkadoni.com/ (xmltv provider for Ex Yugoslavia)\n"
		credit += "- Bodyan and dillinger http://linux-sat.tv/ (xmltv provider for ex USSR channels)\n"
		credit += "- Devilcosta http://sgcpm.com/ (xmltv provider for nova channels in greek and english)"
		self["about"].setText(credit)

		self.onFirstExecBegin.append(self.setImages)

	def setImages(self):
		self["rytec_pix"].instance.setPixmapFromFile("%s/images/rytec.png" % (os.path.dirname(sys.modules[__name__].__file__)))
		self["krkadoni_pix"].instance.setPixmapFromFile("%s/images/krkadoni.png" % (os.path.dirname(sys.modules[__name__].__file__)))

	def quit(self):
		self.close()
	
