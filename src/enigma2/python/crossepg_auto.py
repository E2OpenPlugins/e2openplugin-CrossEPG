from enigma import * #, quitMainloop
from Components.ServiceEventTracker import ServiceEventTracker
from Screens.MessageBox import MessageBox
from Tools.Directories import fileExists
from crossepglib import *
from crossepg_downloader import CrossEPG_Downloader
from crossepg_converter import CrossEPG_Converter
from crossepg_loader import CrossEPG_Loader
from crossepg_importer import CrossEPG_Importer
from crossepg_defragmenter import CrossEPG_Defragmenter
from crossepg_locale import _
from Screens.Screen import Screen

from time import *

import os

def getWeekdayNum(wday):
	weekdays = []
	weekdays.append(_("monday"))
	weekdays.append(_("tuesday"))
	weekdays.append(_("wednesday"))
	weekdays.append(_("thursday"))
	weekdays.append(_("friday"))
	weekdays.append(_("saturday"))
	weekdays.append(_("sunday"))
	wdays = {}
	index=0
	for day in weekdays:
		wdays[day] = index
		index += 1
	return wdays[wday]

retrycount = 0
autoCrossEPGTimer = None
def CrossEPGautostart(reason, session=None, **kwargs):
	"called with reason=1 to during /sbin/shutdown.sysvinit, with reason=0 at startup?"
	global autoCrossEPGTimer
	global _session
	now = int(time())
	if reason == 0:
		print "[CrossEPG_Auto] AutoStart Enabled"
		if session is not None:
			_session = session
			if autoCrossEPGTimer is None:
				autoCrossEPGTimer = CrossEPG_Auto(session)
	else:
		print "[CrossEPG_Auto] Stop"
		autoCrossEPGTimer.stop()

class CrossEPG_Auto:
	instance = None
	def __init__(self, session):
		self.session = session
		self.crossepgtimer = eTimer()
		self.crossepgtimer.callback.append(self.CrossEPGonTimer)
		self.crossepgactivityTimer = eTimer()
		self.crossepgactivityTimer.timeout.get().append(self.crossepgdatedelay)

		self.config = CrossEPG_Config()
		self.patchtype = getEPGPatchType()

		self.pdownloader = None
		self.pimporter = None
		self.pconverter = None
		self.ploader = None
		self.pdefrag = None

		self.osd = False
		self.lock = False

		if fileExists("/tmp/crossepg.standby"):
			# Modded by IAmATeaf 13/04/2012
			# os.system("rm -f /tmp/crossepg.standby")
			os.unlink("/tmp/crossepg.standby")
			print "[CrossEPG_Auto] coming back in standby in 30 seconds"
			self.standbyTimer.start(30000, 1)

		self.config.load()

		if self.config.force_load_on_boot:
			self.loader()

		now = int(time())
		global CrossEPGTime
		if self.config.download_standby_enabled or self.config.download_daily_enabled or self.config.download_xdaily_enabled or self.config.download_weekly_enabled:
			print "[CrossEPG_Auto] Schedule Enabled at ", strftime("%c", localtime(now))
			if now > 1262304000:
				self.crossepgdate()
			else:
				print "[CrossEPG_Auto] Time not yet set."
				CrossEPGTime = 0
				self.crossepgactivityTimer.start(36000)
		else:
			CrossEPGTime = 0
			print "[CrossEPG_Auto] Schedule Disabled at", strftime("%c", localtime(now))
			self.crossepgactivityTimer.stop()

		assert CrossEPG_Auto.instance is None, "class CrossEPG_Auto is a singleton class and just one instance of this class is allowed!"
		CrossEPG_Auto.instance = self

	def __onClose(self):
		CrossEPG_Auto.instance = None

	def forcePoll(self):
		self.crossepgtimer.start(10)

	def crossepgdatedelay(self):
		self.crossepgactivityTimer.stop()
		self.crossepgdate()

	def getCrossEPGTime(self):
		self.config.load()
		nowt = time()
		now = localtime(nowt)
		epgtime = int(mktime((now.tm_year, now.tm_mon, now.tm_mday, self.config.download_daily_hours, self.config.download_daily_minutes, 0, now.tm_wday, now.tm_yday, now.tm_isdst)))
		if self.config.download_weekly_enabled:
			epgtime += (getWeekdayNum(self.config.download_weekday) - getWeekdayNum(strftime("%A", localtime(nowt))))*24*3600
		return epgtime

	def crossepgdate(self, atLeast = 0):
		self.crossepgtimer.stop()
		global CrossEPGTime
		CrossEPGTime = self.getCrossEPGTime()
		now = int(time())
		if CrossEPGTime > 0:
			if CrossEPGTime < now + atLeast:
				if self.config.download_weekly_enabled:
					print "[CrossEPG_Auto] crossepgtime weekly:", strftime("%c", localtime(CrossEPGTime))
					CrossEPGTime += 24*7*3600
				elif self.config.download_xdaily_enabled:
					print "[CrossEPG_Auto] crossepgtime xdaily:", strftime("%c", localtime(CrossEPGTime))
					CrossEPGTime += self.config.download_xdaily_num*24*3600
				elif self.config.download_daily_enabled:
					CrossEPGTime += 24*3600
					while (int(CrossEPGTime)-30) < now:
						CrossEPGTime += 24*3600
				elif self.config.download_standby_enabled:
					CrossEPGTime += 3600
					while (int(CrossEPGTime)-30) < now:
						CrossEPGTime += 3600

			next = CrossEPGTime - now
			self.crossepgtimer.startLongTimer(next)
		else:
			CrossEPGTime = -1
		self.config.next_update_time = "%s" % strftime("%c", localtime(CrossEPGTime))
		self.config.save()
		print "[CrossEPG_Auto] Time set to", strftime("%c", localtime(CrossEPGTime)), strftime("(now=%c)", localtime(now))
		return CrossEPGTime

	def backupstop(self):
		self.crossepgtimer.stop()

	def CrossEPGonTimer(self):
		self.crossepgtimer.stop()
		now = int(time())
		wake = self.getCrossEPGTime()
		# If we're close enough, we're okay...
		atLeast = 0
		if wake - now < 60:
			atLeast = 60
			print "[CrossEPG_Auto] onTimer occurred at", strftime("%c", localtime(now))
			from Screens.Standby import inStandby
			self.config.load()
			if (self.config.download_standby_enabled and inStandby) or self.config.download_daily_enabled or self.config.download_xdaily_enabled or self.config.download_weekly_enabled:
				if self.lock or self.session.nav.RecordTimer.isRecording() or abs(self.session.nav.RecordTimer.getNextRecordingTime() - time()) <= 900 or abs(self.session.nav.RecordTimer.getNextZapTime() - time()) <= 900:
					print "[CrossEPG_Auto] poll delaying as recording."
					self.doCrossEPG(False)
				elif not inStandby:
					message = _("Your epg about to update,\nDo you want to allow this?")
					ybox = self.session.openWithCallback(self.doCrossEPG, MessageBox, message, MessageBox.TYPE_YESNO, timeout = 30)
					ybox.setTitle('Scheduled CrossEPG.')
				else:
					self.doCrossEPG(True)
		self.crossepgdate(atLeast)

	def doCrossEPG(self, answer):
		global retrycount
		now = int(time())
		if answer is False:
			if retrycount < 2:
				print '[CrossEPG_Auto] Number of retries',retrycount
				print "[CrossEPG_Auto] delayed."
				repeat = retrycount
				repeat += 1
				retrycount = repeat
				CrossEPGTime = now + (30 * 60)
				print "[CrossEPG_Auto] Time now set to", strftime("%c", localtime(CrossEPGTime)), strftime("(now=%c)", localtime(now))
				self.crossepgtimer.startLongTimer(30 * 60)
			else:
				atLeast = 60
				print "[CrossEPG_Auto] Enough Retries, delaying till next schedule.", strftime("%c", localtime(now))
				self.session.open(MessageBox, _("Enough Retries, delaying till next schedule."), MessageBox.TYPE_INFO, timeout = 10)
				retrycount = 0
				self.crossepgdate(atLeast)
		else:
			self.timer = eTimer()
			self.timer.callback.append(self.doautostartdownload)
			print "[CrossEPG_Auto] Running CrossEPG", strftime("%c", localtime(now))
			self.timer.start(100, 1)

	def doautostartdownload(self):
		self.config.load()
		from Screens.Standby import inStandby
		if self.config.download_standby_enabled and inStandby:
			self.osd = False
		elif self.config.download_daily_enabled or self.config.download_xdaily_enabled or self.config.download_weekly_enabled:
			self.osd = (inStandby == None)
		self.config.deleteLog()
		self.download(self.config.providers)

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

		if ret:
			if self.config.csv_import_enabled == 1:
				self.importer()
			else:
				if self.patchtype != 3:
					self.converter()
				else:
					self.loader()

	def defrag(self):
		if self.config.last_defrag_timestamp < time() - 7 * 24 * 60 * 60:	 # 1 week
			print "[CrossEPG_Auto] start defragmentation"
			if self.osd:
				self.session.openWithCallback(self.defragCallback, CrossEPG_Defragmenter)
			else:
				self.pdefrag = CrossEPG_Defragmenter(self.session, self.defragCallback, True)
			self.config.last_defrag_timestamp = time()
			self.config.save()
			
	def defragCallback(self, ret):
		self.pdefrag = None
		
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
						# Modded by IAmATeaf 13/04/2012
						# os.system("touch /tmp/crossepg.standby")
						open("/tmp/crossepg.standby", "wb").close()
					else:
						# Modded by IAmATeaf 13/04/2012
						# os.system("rm /tmp/crossepg.standby")
						os.unlink("/tmp/crossepg.standby")
					print "[CrossEPG_Auto] rebooting"
					from Screens.Standby import TryQuitMainloop
					self.session.open(TryQuitMainloop, 3)

	def loader(self):
		if self.osd:
			self.session.openWithCallback(self.loaderCallback, CrossEPG_Loader)
		else:
			self.ploader = CrossEPG_Loader(self.session, self.loaderCallback, True)

	def loaderCallback(self, ret):
		self.ploader = None
		self.defrag()

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
		if self.pdefrag:
			self.pdefrag.quit()
			self.pdefrag = None

	def backToStandby(self):
		from Screens.Standby import inStandby
		if inStandby == None:
			print "[CrossEPG_Auto] coming back in standby"
			from Screens.Standby import Standby
			self.session.open(Standby)

	def doneConfiguring(self):
		now = int(time())
		if self.config.download_standby_enabled or self.config.download_daily_enabled or self.config.download_xdaily_enabled or self.config.download_weekly_enabled:
			if autoCrossEPGTimer is not None:
				print "[CrossEPG_Auto] Schedule Enabled at", strftime("%c", localtime(now))
				autoCrossEPGTimer.crossepgdate()
		else:
			if autoCrossEPGTimer is not None:
				global CrossEPGTime
				CrossEPGTime = 0
				print "[CrossEPG_Auto] Schedule Disabled at", strftime("%c", localtime(now))
				autoCrossEPGTimer.backupstop()
		if CrossEPGTime > 0:
			t = localtime(CrossEPGTime)
			crossepgtext = strftime(_("%a %e %b  %-H:%M"), t)
		else:
			crossepgtext = ""
