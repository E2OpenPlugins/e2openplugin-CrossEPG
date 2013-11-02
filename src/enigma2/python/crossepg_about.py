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

from Tools.Directories import resolveFilename, SCOPE_CURRENT_SKIN
try:
	from Tools.Directories import SCOPE_ACTIVE_SKIN
except:
	pass

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
		Screen.setTitle(self, _("CrossEPG") + " - " + _("About"))

		self.config = CrossEPG_Config()
		self.config.load()

		self["about"] = Label("")
		self["rytec_pix"] = Pixmap()
		self["rytec_text"] = Label("")
		self["krkadoni_pix"] = Pixmap()
		self["krkadoni_text"] = Label("")

		self["actions"] = ActionMap(["SetupActions", "ColorActions", "MenuActions"],
		{
			"red": self.quit,
			"cancel": self.quit,
			"menu": self.quit,
		}, -2)

		self["key_red"] = Button(_("Close"))
		self["key_green"] = Button("")
		self["key_yellow"] = Button("")
		self["key_blue"] = Button("")

		try:
			from version import version
		except Exception, e:
			version = "unknow version"

		credit = "CrossEPG %s (c) 2009-2013 Sandro Cavazzoni\n" % version
		credit += "https://github.com/oe-alliance/e2openplugin-CrossEPG\n\n"
		credit += "Application credits:\n"
		credit += "- Sandro Cavazzoni aka skaman (main developer)\n"
		credit += "- Andy Blackburn aka andyblac (co-developer)\n"
		credit += "- Ambrosa (scripts developer)\n"
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
		try:
			rytecpng = resolveFilename(SCOPE_ACTIVE_SKIN, "crossepg/rytec.png")
		except:
			rytecpng = resolveFilename(SCOPE_CURRENT_SKIN, "skin_default/crossepg/rytec.png")
		if rytecpng == None or not os.path.exists(rytecpng):
			rytecpng = "%s/images/rytec.png" % (os.path.dirname(sys.modules[__name__].__file__))
		self["rytec_pix"].instance.setPixmapFromFile(rytecpng)
		try:
			krkadonipng = resolveFilename(SCOPE_ACTIVE_SKIN, "crossepg/krkadoni.png")
		except:
			krkadonipng = resolveFilename(SCOPE_CURRENT_SKIN, "skin_default/crossepg/krkadoni.png")
		if krkadonipng == None or not os.path.exists(krkadonipng):
			krkadonipng = "%s/images/krkadoni.png" % (os.path.dirname(sys.modules[__name__].__file__))
		self["krkadoni_pix"].instance.setPixmapFromFile(krkadonipng)

	def quit(self):
		self.close()

