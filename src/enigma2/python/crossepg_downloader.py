from enigma import getDesktop, iPlayableService, eTimer, eServiceReference

from Components.Label import Label
from Components.Pixmap import Pixmap
from Components.ProgressBar import ProgressBar
from Components.ServiceEventTracker import ServiceEventTracker
from Components.ActionMap import NumberActionMap
from Components.config import config

from Screens.Screen import Screen
from Screens.MessageBox import MessageBox

from crossepglib import *
from crossepg_locale import _

import os
import sys

from Tools.Directories import resolveFilename, SCOPE_CURRENT_SKIN

class CrossEPG_Downloader(Screen):
	def __init__(self, session, providers, pcallback = None, noosd = False):
		from Components.Sources.StaticText import StaticText
		from Components.Sources.Progress import Progress
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

		self.session = session

		self["background"] = Pixmap()
		self["action"] = Label(_("Starting downloader"))
		self["status"] = Label("")
		self["progress"] = ProgressBar()
		self["progress"].hide()
		self["summary_action"] = StaticText(_("Starting downloader"))
		self["summary_status"] = StaticText()
		self["summary_progress"] = Progress()
		self["actions"] = NumberActionMap(["WizardActions", "InputActions"],
		{
			"back": self.quit
		}, -1)

		self.__event_tracker = ServiceEventTracker(screen=self, eventmap=
		{
			iPlayableService.evTunedIn: self.tuned,
		})

		self.retValue = True
		self.provider_index = 0
		self.status = 0
		self.open = False
		self.saved = False
		self.tune_enabled = False
		self.oldService = self.session.nav.getCurrentlyPlayingServiceReference()
		self.config = CrossEPG_Config()
		self.config.load()
		self.providers = providers
		self.pcallback = pcallback

		self.wrapper = CrossEPG_Wrapper()
		self.wrapper.addCallback(self.wrapperCallback)

		self.timeout = eTimer()
		self.timeout.callback.append(self.quit)

		self.hideprogress = eTimer()
		self.hideprogress.callback.append(self["progress"].hide)

		self.pcallbacktimer = eTimer()
		self.pcallbacktimer.callback.append(self.doCallback)

		self.wrappertimer = eTimer()
		self.wrappertimer.callback.append(self.initWrapper)

		if noosd:
			self.wrappertimer.start(100, 1)
		else:
			self.onFirstExecBegin.append(self.firstExec)

	def firstExec(self):
		if self.isHD:
			png = resolveFilename(SCOPE_CURRENT_SKIN, "crossepg/background_hd.png")
			if png == None or not os.path.exists(png):
				png = "%s/images/background_hd.png" % os.path.dirname(sys.modules[__name__].__file__)
		else:
			png = resolveFilename(SCOPE_CURRENT_SKIN, "crossepg/background.png")
			if png == None or not os.path.exists(png):
				png = "%s/images/background.png" % os.path.dirname(sys.modules[__name__].__file__)
		self["background"].instance.setPixmapFromFile(png)
		self.wrappertimer.start(100, 1)

	def initWrapper(self):
		if len(self.providers) == 0:
			self.closeAndCallback(True)
		else:
			if getDistro() != "ViX" and getDistro() != "AAF" and getDistro() != "openMips":
				self.wrapper.init(CrossEPG_Wrapper.CMD_DOWNLOADER, self.config.db_root)
			else:
				self.wrapper.init(CrossEPG_Wrapper.CMD_DOWNLOADER, config.misc.epgcachepath.value + 'crossepg')

	def download(self):
		if self.config.getChannelProtocol(self.providers[self.provider_index]) != "script":
			if not self.open:
				self.wrapper.open()
				self.open = True
				self.saved = False
		else:
			if self.open:
				self.wrapper.save()
				self.saved = True
				self.provider_index -= 1
				return

		service = self.config.getChannelID(self.providers[self.provider_index])
		try:
			cservice = self.session.nav.getCurrentlyPlayingServiceReference().toString()
		except Exception, e:
			cservice = None

		if service:
			print "[CrossEPG_Downloader] %s service is %s" % (self.providers[self.provider_index], service)
			if service == cservice:
				self.wrapper.download(self.providers[self.provider_index])
			else:
				self.tune_enabled = True
				self.wrapper.wait()
				self.timeout.start(60000, 1)
				self.session.nav.playService(eServiceReference(service))
		else:
			self.wrapper.download(self.providers[self.provider_index])

	def wrapperCallback(self, event, param):
		if event == CrossEPG_Wrapper.EVENT_READY:
			self.download()

		elif event == CrossEPG_Wrapper.EVENT_END:
			if self.saved and self.open:
				self.wrapper.close()
				self.open = False

			if self.status == 0:
				self.provider_index += 1
				if self.provider_index < len(self.providers):
					self.download()
				else:
					self.status = 1
					if self.oldService:
						self.session.nav.playService(self.oldService)
					if self.open:
						self.wrapper.save()
					else:
						self.wrapper.quit()
			else:
				if self.open:
					self.wrapper.close()
				self.wrapper.quit()

		elif event == CrossEPG_Wrapper.EVENT_ACTION:
			self["action"].text = param
			self["status"].text = ""
			self["summary_action"].text = param
			self["summary_status"].text = ""

		elif event == CrossEPG_Wrapper.EVENT_STATUS or event == CrossEPG_Wrapper.EVENT_URL:
			self["status"].text = param
			self["summary_status"].text = param

		elif event == CrossEPG_Wrapper.EVENT_PROGRESS:
			self["progress"].setValue(param)
			self["summary_progress"].setValue(param)

		elif event == CrossEPG_Wrapper.EVENT_PROGRESSONOFF:
			if param:
				self.hideprogress.stop()
				self["progress"].setValue(0)
				self["progress"].show()
				self["summary_progress"].setValue(0)
			else:
				self["progress"].setValue(100)
				self["summary_progress"].setValue(100)
				self.hideprogress.start(500, 1)
		elif event == CrossEPG_Wrapper.EVENT_QUIT:
			self.closeAndCallback(self.retValue)
		elif event == CrossEPG_Wrapper.EVENT_ERROR:
			self.session.open(MessageBox, _("CrossEPG error: %s") % (param), type = MessageBox.TYPE_INFO, timeout = 20)
			self.retValue = False
			self.quit()

	def tuned(self):
		if self.tune_enabled:
			self.timeout.stop()
			self.wrapper.download(self.providers[self.provider_index])
			self.tune_enabled = False

	def quit(self):
		if self.wrapper.running():
			self.retValue = False
			self.wrapper.quit()
		else:
			self.closeAndCallback(False)

	def closeAndCallback(self, ret):
		self.retValue = ret
		self.pcallbacktimer.start(0, 1)
		self.close(ret)

	def doCallback(self):
		if self.pcallback:
			self.pcallback(self.retValue)

