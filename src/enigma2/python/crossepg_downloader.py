from enigma import getDesktop, iPlayableService, eTimer, eServiceReference

from Components.Label import Label
from Components.Pixmap import Pixmap
from Components.ProgressBar import ProgressBar
from Components.ServiceEventTracker import ServiceEventTracker
from Components.ActionMap import NumberActionMap
from Components.config import config
from Components.NimManager import nimmanager

from Screens.Screen import Screen
from Screens.MessageBox import MessageBox

from crossepglib import *
from crossepg_locale import _

import os
import sys

from Tools.Directories import resolveFilename, SCOPE_CURRENT_SKIN
try:
	from Tools.Directories import SCOPE_ACTIVE_SKIN
except:
	pass

class CrossEPG_Downloader(Screen):
	LOCK_TIMEOUT = 300 	# 100ms for tick - 30 sec
	
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

		self.frontend = None
		self.rawchannel = None
		self.retValue = True
		self.provider_index = 0
		self.status = 0
		self.open = False
		self.saved = False
		self.oldService = None
		self.config = CrossEPG_Config()
		self.config.load()
		self.providers = providers
		self.pcallback = pcallback

		self.wrapper = CrossEPG_Wrapper()
		self.wrapper.addCallback(self.wrapperCallback)

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
			try:
				png = resolveFilename(SCOPE_ACTIVE_SKIN, "crossepg/background_hd.png")
			except:
				png = resolveFilename(SCOPE_CURRENT_SKIN, "skin_default/crossepg/background_hd.png")
			if png == None or not os.path.exists(png):
				png = "%s/images/background_hd.png" % os.path.dirname(sys.modules[__name__].__file__)
		else:
			try:
				png = resolveFilename(SCOPE_ACTIVE_SKIN, "crossepg/background.png")
			except:
				png = resolveFilename(SCOPE_CURRENT_SKIN, "skin_default/crossepg/background.png")
			if png == None or not os.path.exists(png):
				png = "%s/images/background.png" % os.path.dirname(sys.modules[__name__].__file__)
		self["background"].instance.setPixmapFromFile(png)
		self.wrappertimer.start(100, 1)

	def initWrapper(self):
		if len(self.providers) == 0:
			self.closeAndCallback(True)
		else:
			if getDistro() != "openvix":
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

		transponder = self.config.getTransponder(self.providers[self.provider_index])
		if transponder:
			self.doTune(transponder)
		else:
			self.wrapper.download(self.providers[self.provider_index])
			
	def error(self, message):
		print "[CrossEPG_Downloader] Error: %s" % message
		self.session.open(MessageBox, _("CrossEPG error: %s") % (message), type = MessageBox.TYPE_INFO, timeout = 20)
		self.retValue = False
		self.quit()
		
	def doTune(self, transponder):
		nimList = nimmanager.getNimListOfType("DVB-S")
		if len(nimList) == 0:
			self.error("No DVB-S NIMs founds")
			return

		resmanager = eDVBResourceManager.getInstance()
		if not resmanager:
			self.error("Cannot retrieve Resource Manager instance")
			return

		print "[CrossEPG_Downloader] Search NIM for orbital position %d" % transponder["orbital_position"]
		current_slotid = -1
		if self.rawchannel:
			del(self.rawchannel)

		self.frontend = None
		self.rawchannel = None

		nimList.reverse() # start from the last
		for slotid in nimList:
			if slotid <= 3: # only support first 4 frontends.
				sats = nimmanager.getSatListForNim(slotid)
				for sat in sats:
					if sat[0] == transponder["orbital_position"]:
						if current_slotid == -1:	# mark the first valid slotid in case of no other one is free
							current_slotid = slotid

						self.rawchannel = resmanager.allocateRawChannel(slotid)
						if self.rawchannel:
							print "[CrossEPG_Downloader] Nim found on slot id %d with sat %s" % (slotid, sat[1])
							current_slotid = slotid
							break

			if self.rawchannel:
				break

		if current_slotid == -1:
			self.error("No valid NIM found")
			return

		if not self.rawchannel:
			print "[CrossEPG_Downloader] Nim found on slot id %d but it's busy. Stop current service" % current_slotid

			self.oldService = self.session.nav.getCurrentlyPlayingServiceReference()
			self.session.nav.stopService()
			if self.session.pipshown:
				self.session.pipshown = False

			self.rawchannel = resmanager.allocateRawChannel(current_slotid)
			if not self.rawchannel:
				self.error("Cannot get the NIM")
				return

		self.frontend = self.rawchannel.getFrontend()
		if not self.frontend:
			self.error("Cannot get frontend")
			return

		demuxer_id = self.rawchannel.reserveDemux()
		if demuxer_id < 0:
			self.error("Cannot allocate the demuxer")
			return

		params = eDVBFrontendParametersSatellite()
		params.frequency = transponder["frequency"]
		params.symbol_rate = transponder["symbol_rate"]
		params.polarisation = transponder["polarization"]
		params.fec = transponder["fec_inner"]
		params.inversion = transponder["inversion"]
		params.orbital_position = transponder["orbital_position"]
		params.system = transponder["system"]
		params.modulation = transponder["modulation"]
		params.rolloff = transponder["roll_off"]
		params.pilot = transponder["pilot"]
		params_fe = eDVBFrontendParameters()
		params_fe.setDVBS(params, False)
		self.frontend.tune(params_fe)
		self.wrapper.demuxer("/dev/dvb/adapter%d/demux%d" % (0, demuxer_id)) # FIX: use the correct device
		self.wrapper.frontend(current_slotid)

		self.lockcounter = 0
		self.locktimer = eTimer()
		self.locktimer.callback.append(self.checkTunerLock)
		self.locktimer.start(100, 1)

	def checkTunerLock(self):
		dict = {}
		self.frontend.getFrontendStatus(dict)
		if dict["tuner_state"] == "TUNING":
			print "[CrossEPG] TUNING"
		elif dict["tuner_state"] == "LOCKED":
			print "[CrossEPG] ACQUIRING TSID/ONID"
			self.timer = eTimer()
			self.timer.callback.append(self.doDownload)
			self.timer.start(500, 1)
			return
		elif dict["tuner_state"] == "LOSTLOCK" or dict["tuner_state"] == "FAILED":
			print "[CrossEPG] FAILED"

		self.lockcounter += 1
		if self.lockcounter > self.LOCK_TIMEOUT:
			self.error("Timeout for tuner lock")
			return

		self.locktimer.start(100, 1)
		
	def doDownload(self):
		self.wrapper.download(self.providers[self.provider_index])

	def wrapperCallback(self, event, param):
		if event == CrossEPG_Wrapper.EVENT_READY:
			self.download()

		elif event == CrossEPG_Wrapper.EVENT_END:
			if self.rawchannel:
				del(self.rawchannel)

			self.frontend = None
			self.rawchannel = None
			
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

	def quit(self):
		if self.rawchannel:
			del(self.rawchannel)

		self.frontend = None
		self.rawchannel = None
		
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

