from enigma import * #, quitMainloop
from Components.ServiceEventTracker import ServiceEventTracker
from Tools.Directories import fileExists
from crossepglib import *
from crossepg_downloader import CrossEPG_Downloader
from crossepg_converter import CrossEPG_Converter
from crossepg_loader import CrossEPG_Loader
from crossepg_importer import CrossEPG_Importer
from crossepg_locale import _
from Screens.Screen import Screen

from time import *

import os

class CrossEPG_Auto(Screen):
	POLL_TIMER = 1800000	# poll every 30 minutes
	#POLL_TIMER = 18000
	POLL_TIMER_FAST = 10000	# poll every 10 seconds
	POLL_TIMER_BOOT = 60000	# poll every 1 minute

	def __init__(self):
		self.session = None

		self.timer = eTimer()
		self.standbyTimer = eTimer()
		self.delayedInitTimer = eTimer()

		self.timer.callback.append(self.poll)
		self.standbyTimer.callback.append(self.backToStandby)
		self.delayedInitTimer.callback.append(self.init)

		self.config = CrossEPG_Config()
		self.patchtype = getEPGPatchType()

		self.pdownloader = None
		self.pimporter = None
		self.pconverter = None
		self.ploader = None

		self.osd = False
		self.ontune = False
		self.lock = False

		if fileExists("/tmp/crossepg.standby"):
			os.system("rm -f /tmp/crossepg.standby")
			print "[CrossEPG_Auto] coming back in standby in 30 seconds"
			self.standbyTimer.start(30000, 1)
			
		self.config.load()
		
		if self.config.force_load_on_boot:
			self.loader()

	def init(self, session = None):
		if session != None:
			self.session = session
			
		if time() < 1262325600:		# if before 2010 probably the clock isn't yet updated
			self.delayedInitTimer.start(60000, 1)	#initialization delayed of 1 minute
			return
			
		self.resetDailyDownloadDateCache()
		self.timer.start(self.POLL_TIMER_BOOT, 1)

	def forcePoll(self):
		self.timer.stop()
		self.resetDailyDownloadDateCache()
		self.timer.start(self.POLL_TIMER_FAST, 1)
		
	def resetDailyDownloadDateCache(self):
		self.config.load()
		now = time()
		ttime = localtime(now)
		ltime = (ttime[0], ttime[1], ttime[2], self.config.download_daily_hours, self.config.download_daily_minutes, 0, ttime[6], ttime[7], ttime[8])
		stime = mktime(ltime)
		if stime < now:
			ttime = localtime(stime+86400)	# 24 hours in future

		# to avoid problems with internal clock (big changes on date/time)
		# we step forward of 24 hours until the new time is greater than now
		while ttime < now:
			ttime = ttime+86400	# 24 hours in future
		
		self.cacheYear = ttime[0]
		self.cacheMonth = ttime[1]
		self.cacheDay = ttime[2]
			
	def poll(self):
		from Screens.Standby import inStandby
		self.config.load()

		if self.lock:
			print "[CrossEPG_Auto] poll"
			self.timer.start(self.POLL_TIMER_FAST, 1)
		elif self.session.nav.RecordTimer.isRecording() or abs(self.session.nav.RecordTimer.getNextRecordingTime() - time()) <= 900 or abs(self.session.nav.RecordTimer.getNextZapTime() - time()) <= 900:
			print "[CrossEPG_Auto] poll"
			self.timer.start(self.POLL_TIMER, 1)
		elif self.config.download_standby_enabled and inStandby:
			self.providers = []
			now = time()

			if self.config.last_full_download_timestamp <= now - (24*60*60):
				self.config.last_full_download_timestamp = now
				self.config.last_partial_download_timestamp = now
				self.config.save()
				self.providers = self.config.providers
			elif self.config.last_partial_download_timestamp <= now - (60*60): # skip xmltv... we download it only one time a day
				self.config.last_partial_download_timestamp = now
				self.config.save()
				providers = self.config.getAllProviders()
				i = 0
				for provider in providers[0]:
					if self.config.providers.count(provider) > 0:
						if providers[2][i] == "opentv":
							self.providers.append(provider)
						else:
							print "[CrossEPG_Auto] is not OpenTV : skip provider %s (we download it only one time a day)" % provider
					i += 1

			if len(self.providers) == 0:
				print "[CrossEPG_Auto] poll"
				self.timer.start(self.POLL_TIMER, 1)
			else:
				print "[CrossEPG_Auto] automatic download in standby"
				self.osd = False
				self.ontune = False
				self.config.deleteLog()
				self.download(self.providers)
		elif self.config.download_daily_enabled:
			now = time()
			ttime = localtime(now)
			ltime = (self.cacheYear, self.cacheMonth, self.cacheDay, self.config.download_daily_hours, self.config.download_daily_minutes, 0, ttime[6], ttime[7], ttime[8])
			stime = mktime(ltime)
			if stime < now and self.config.last_full_download_timestamp != stime:
				from Screens.Standby import inStandby
				self.osd = (inStandby == None)
				self.ontune = False
				self.config.last_full_download_timestamp = stime
				self.config.last_partial_download_timestamp = stime
				self.config.save()
				ttime = localtime(stime+86400)	# 24 hours in future
				# to avoid problems with internal clock (big changes on date/time)
				# we step forward of 24 hours until the new time is greater than now
				while ttime < now:
					ttime = ttime+86400	# 24 hours in future
				self.cacheYear = ttime[0]
				self.cacheMonth = ttime[1]
				self.cacheDay = ttime[2]
				self.config.deleteLog()
				self.download(self.config.providers)
			elif stime < now + (self.POLL_TIMER / 1000) and self.config.last_full_download_timestamp != stime:
				print "[CrossEPG_Auto] poll"
				delta = int(stime - now);
				self.timer.start((delta + 5)*1000, 1)	# 5 seconds offset
			else:
				print "[CrossEPG_Auto] poll"
				self.timer.start(self.POLL_TIMER, 1)
		elif self.config.download_tune_enabled:
			now = time()
			if self.config.last_partial_download_timestamp <= now - (60*60):
				providerok = None
				sservice = self.session.nav.getCurrentlyPlayingServiceReference()
				if sservice:
					service = sservice.toString()

					providers = self.config.getAllProviders()
					i = 0
					for provider in providers[0]:
						if providers[2][i] == "opentv":
							if self.config.getChannelID(provider) == service:
								providerok = provider
								break;
						i += 1

				if providerok:
					print "[CrossEPG_Auto] automatic download on tune"
					self.osd = False
					self.ontune = True
					self.config.last_partial_download_timestamp = now
					self.config.save()
					self.config.deleteLog()
					self.download([provider,])
				else:
					print "[CrossEPG_Auto] poll"
					self.timer.start(self.POLL_TIMER, 1)
			else:
				print "[CrossEPG_Auto] poll"
				self.timer.start(self.POLL_TIMER, 1)
		else:
			print "[CrossEPG_Auto] poll"
			self.timer.start(self.POLL_TIMER, 1)

	def download(self, providers):
		print "[CrossEPG_Auto] providers selected for download:"
		for provider in providers:
			print "[CrossEPG_Auto] %s" % provider
		if self.osd:
			self.session.openWithCallback(self.downloadCallback, CrossEPG_Downloader, providers)
		else:
			self.pdownloader = CrossEPG_Downloader(self.session, providers, self.downloadCallback, True)

	def downloadCallback(self, ret):
		self.pdownloader = None

		from Screens.Standby import inStandby
		if inStandby: # if in standby force service stop
			self.session.nav.stopService()

		if ret:
			if self.config.csv_import_enabled == 1 and not self.ontune:
				self.importer()
			else:
				if self.patchtype != 3:
					self.converter()
				else:
					self.loader()
		else:
			self.timer.start(self.POLL_TIMER, 1)

	def importer(self):
		print "[CrossEPG_Auto] start csv import"
		if self.osd:
			self.session.openWithCallback(self.importerCallback, CrossEPG_Importer)
		else:
			self.pimporter = CrossEPG_Importer(self.session, self.importerCallback, True)

	def importerCallback(self, ret):
		self.pimporter = None

		if ret:
			if self.patchtype != 3:
				self.converter()
			else:
				self.loader()
		else:
			self.timer.start(self.POLL_TIMER, 1)

	def converter(self):
		print "[CrossEPG_Auto] start epg.dat conversion"
		if self.osd:
			self.session.openWithCallback(self.converterCallback, CrossEPG_Converter)
		else:
			self.pconverter = CrossEPG_Converter(self.session, self.converterCallback, True)

	def converterCallback(self, ret):
		self.pconverter = None

		if ret:
			if self.patchtype != -1:
				self.loader()
			else:
				if self.config.download_daily_reboot:
					from Screens.Standby import inStandby
					if inStandby:
						os.system("touch /tmp/crossepg.standby")
					else:
						os.system("rm /tmp/crossepg.standby")
					print "[CrossEPG_Auto] rebooting"
					from Screens.Standby import TryQuitMainloop
					self.session.open(TryQuitMainloop, 3)
				else:
					self.timer.start(self.POLL_TIMER, 1)
		else:
			self.timer.start(self.POLL_TIMER, 1)

	def loader(self):
		if self.osd:
			self.session.openWithCallback(self.loaderCallback, CrossEPG_Loader)
		else:
			self.ploader = CrossEPG_Loader(self.session, self.loaderCallback, True)

	def loaderCallback(self, ret):
		self.ploader = None
		self.timer.start(self.POLL_TIMER, 1)

	def stop(self):
		if self.pdownloader:
			self.pdownloader.quit()
			self.pdownloader = None
		if self.pimporter:
			self.pimporter.quit()
			self.pimporter = None
		if self.pconverter:
			self.pconverter.quit()
			self.pconverter = None
		if self.ploader:
			self.ploader.quit()
			self.ploader = None

	def backToStandby(self):
		from Screens.Standby import inStandby
		if inStandby == None:
			print "[CrossEPG_Auto] coming back in standby"
			from Screens.Standby import Standby
			self.session.open(Standby)

crossepg_auto = CrossEPG_Auto()
