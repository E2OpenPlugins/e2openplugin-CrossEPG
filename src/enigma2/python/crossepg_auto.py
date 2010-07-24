from enigma import *
from Components.ServiceEventTracker import ServiceEventTracker
from Tools.Directories import fileExists
from crossepglib import *
from crossepg_downloader import CrossEPG_Downloader
from crossepg_converter import CrossEPG_Converter
from crossepg_loader import CrossEPG_Loader
from crossepg_importer import CrossEPG_Importer
from crossepg_exec import CrossEPG_Exec
from crossepg_locale import _
from Screens.Screen import Screen

from time import *

import os

class CrossEPG_Auto(Screen):
	def __init__(self):
		self.session = None
		self.timer = eTimer()
		self.timer.callback.append(self.__dailyDownload)
		self.standbyTimer = eTimer()
		self.standbyTimer.callback.append(self.__backToStandby)
		self.ddelayedTimer = eTimer()
		self.ddelayedTimer.callback.append(self.__dailyDownloadDelayed)
		self.providers = list()
		self.providers_id = list()
		self.providers_last = list()
		self.current_id = -1
		self.downloader = None
		self.converter = None
		self.loader = None
		self.auto_tune = 0
		self.auto_tune_osd = 0
		self.enabled = True
	
	def enable(self):
		self.enabled = True
		
	def disable(self):
		self.enabled = False
		
	def init(self, session):
		self.session = session
		Screen.__init__(self, session)
		config = CrossEPG_Config()
		config.load()
		providers = config.getAllProviders()
		for provider in providers:
			self.providers.append(provider)
			self.providers_id.append(config.getChannelID(provider))
			self.providers_last.append(0)
		
		self.auto_tune = config.auto_tune
		self.auto_tune_osd = config.auto_tune_osd
		db_root = config.db_root
		if not pathExists(db_root):
			if not createDir(db_root):
				db_root = "/hdd/crossepg"
				
		self.__event_tracker = ServiceEventTracker(screen=self, eventmap=
		{
			iPlayableService.evEnd: self.__stopped,
			iPlayableService.evTunedIn: self.__tuned,
		})
		
		if config.auto_boot == 1:
			try:
				f = open("%s/ext.epg.dat" % (db_root), "r")
			except Exception, e:
				self.converter = CrossEPG_Converter(self.session, self.__convertOnInitEnded)
			else:
				f.seek(4);
				if f.read(13) == "ENIGMA_EPG_V7":
					self.loader = CrossEPG_Loader(self.session, self.__loaderEnded)
				else:
					self.converter = CrossEPG_Converter(self.session, self.__convertOnInitEnded)
				f.close()
		
		self.dailyStart()
		if fileExists("/tmp/crossepg.standby"):
			os.system("rm /tmp/crossepg.standby")
			print "[CrossEPG_Auto] coming back in standby in 30 seconds"
			self.standbyTimer.start(30000, 1)
	
	def __backToStandby(self):
		from Screens.Standby import inStandby
		if inStandby == None:
			print "[CrossEPG_Auto] coming back in standby"
			from Screens.Standby import Standby
			self.session.open(Standby)

	#def __wakeFromStandby(self):
	#	from Screens.Standby import inStandby
	#	if inStandby != None:
	#		print "[CrossEPG_Auto] wake up from standby"
	#		inStandby.Power()
			#from Screens.Standby import Standby
			#self.session.open(Standby)
	
	def dailyStart(self, hours = None, minutes = None, tomorrow = False):
		config = CrossEPG_Config()
		config.load()
		
		if not hours or not minutes:
			if not config.auto_daily:
				print "[CrossEPG_Auto] daily download disabled"
				return
			self.hours = config.auto_daily_hours
			self.minutes = config.auto_daily_minutes
		else:
			self.hours = hours
			self.minutes = minutes
		
		self.timer.stop()
		now = time()
		ttime = localtime(now)
		ltime = (ttime[0], ttime[1], ttime[2], self.hours, self.minutes, 0, ttime[6], ttime[7], ttime[8])
		stime = mktime(ltime)
		if tomorrow:
			stime += 60*60*24
		if stime < now + 2:
			stime += 60*60*24
		
		delta = int(stime - now);
		if delta <= 0:
			delta = 1
			
		print "[CrossEPG_Auto] enabled timer in %d minutes" % (delta / 60)
		self.timer.start(1000*delta, 1)
		
	def dailyStop(self):
		print "[CrossEPG_Auto] daily download disabled"
		self.timer.stop()
		
	def stop(self):
		if self.downloader:
			self.current_id = -1
			self.downloader.quit()
			self.downloader = None
		if self.converter:
			self.converter.quit()
			self.converter = None
		if self.loader:
			self.loader.quit()
			self.loader = None
	
	def __dailyDownload(self):
		print "[CrossEPG_Auto] daily action! starting downloader"
		if self.enabled:
			if self.session.nav.RecordTimer.isRecording():
				now = time()
				ttime = localtime(now)
				hour = ttime[3] + 1;
				if hour > 23:
					hour = 0;
				print "[CrossEPG_Auto] record in progress.. download delayed"
				self.dailyStart(hour, ttime[4], False)
			else:
				self.stop()
				
				# we mark the stanby status in a file in a /tmp dir.. so it's reachable still after a gui reboot
				from Screens.Standby import inStandby
				if inStandby == None:
					os.system("rm /tmp/crossepg.standby")
				else:
					print "[CrossEPG_Auto] decoder in standby.. waking up"
					os.system("touch /tmp/crossepg.standby")
					#self.wakeupTimer.start(500, 1)
					inStandby.Power()
				
				self.ddelayedTimer.start(5000, 1)
				#self.session.open(CrossEPG_Downloader, self.__dailyDownloadEnded)
				#self.enabled = False
				#self.dailyStart(self.hours, self.minutes, True)
		else:
			print "[CrossEPG_Auto] another download is in progress... skipped"

	def __dailyDownloadDelayed(self):
		self.session.open(CrossEPG_Downloader, self.__dailyDownloadEnded)
		self.enabled = False
		self.dailyStart(self.hours, self.minutes, True)
		
	def __dailyDownloadEnded(self, session, ret):
		if ret:
			config = CrossEPG_Config()
			config.load()
			if config.enable_importer == 1:
				self.scripts = config.getAllImportScripts()
				self.scripts_index = 0;
				self.__dailyStartScripts(session)
			else:
				self.session.open(CrossEPG_Converter, self.__dailyConvertEnded)
		else:
			self.enabled = True
	
	def __dailyStartScripts(self, session):
		if len(self.scripts) > self.scripts_index:
			self.session.open(CrossEPG_Exec, self.scripts[self.scripts_index], False, self.__dailyStartScripts)
			self.scripts_index += 1
		else:
			self.session.open(CrossEPG_Importer, self.__dailyImporterEnded)

	def __dailyImporterEnded(self, session, ret):
		if ret:
			self.session.open(CrossEPG_Converter, self.__dailyConvertEnded)
		else:
			self.enabled = True
	
	def __dailyConvertEnded(self, session, ret):
		reboot = False
		if ret:
			config = CrossEPG_Config()
			config.load()
				
			patchtype = getEPGPatchType()
			if patchtype == 0 or patchtype == 1:
				session.open(CrossEPG_Loader, self.__dailyLoaderEnded)
			elif patchtype == 2 and config.auto_daily_reboot == 0:
				session.open(CrossEPG_Loader, self.__dailyLoaderEnded)
			elif config.auto_daily_reboot == 1:
				#from Screens.Standby import inStandby
				#if inStandby == None:
				#	os.system("rm /tmp/crossepg.standby")
				#else:
				#	print "[CrossEPG_Auto] decoder in standby"
				#	os.system("touch /tmp/crossepg.standby")
				reboot = True
				print "[CrossEPG_Auto] rebooting"
				from Screens.Standby import TryQuitMainloop
				session.open(TryQuitMainloop, 3)
			else:
				self.enabled = True
		else:
			self.enabled = True
			
		if fileExists("/tmp/crossepg.standby") and not reboot:
			os.system("rm /tmp/crossepg.standby")
			#print "[CrossEPG_Auto] coming back in standby now"
			self.__backToStandby()
			#self.standbyTimer.start(5000, 1)
			
		
	def __dailyLoaderEnded(self, session, ret):
		self.enabled = True
		
	def __downloadEnded(self, session, ret):
		self.downloader = None
		if ret and self.current_id > -1:
			print "[CrossEPG_Auto] download ok! ignore others download on this provider for 60 minutes"
			self.providers_last[self.current_id] = time() + 3600
			if self.auto_tune_osd == 1:
				self.session.open(CrossEPG_Converter, self.__convertEnded)
			else:
				self.converter = CrossEPG_Converter(self.session, self.__convertEnded)
		self.current_id = -1

	def __convertOnInitEnded(self, session, ret):
		self.converter = None
		if ret:
			self.loader = CrossEPG_Loader(self.session, self.__loaderEnded)

	def __convertEnded(self, session, ret):
		self.converter = None
		if ret:
			if self.auto_tune_osd == 1:
				self.session.open(CrossEPG_Loader, self.__loaderEnded)
			else:
				self.loader = CrossEPG_Loader(self.session, self.__loaderEnded)
	
	def __loaderEnded(self, session, ret):
		self.loader = None
		
	def __stopped(self):
		if self.downloader:
			self.current_id = -1
			self.downloader.quit()
			self.downloader = None
			
	def __tuned(self):
		if self.auto_tune == 1 and self.enabled:
			sservice = self.session.nav.getCurrentlyPlayingServiceReference()
			if sservice:
				service = sservice.toString()
				count = 0
				for provider in self.providers:
					if self.providers_id[count] == service:
						print "[CrossEPG_Auto] match with provider %s" % (provider)
						if self.providers_last[count] > time():
							print "[CrossEPG_Auto] epg already downloaded... download ignored"
						else:
							self.stop()
							self.current_id = count
							if self.auto_tune_osd == 1:
								self.session.open(CrossEPG_Downloader, self.__downloadEnded, provider)
							else:
								self.downloader = CrossEPG_Downloader(self.session, self.__downloadEnded, provider)
					count += 1
		
crossepg_auto = CrossEPG_Auto()