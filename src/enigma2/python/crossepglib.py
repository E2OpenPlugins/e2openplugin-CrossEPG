from enigma import *
from crossepg_locale import _
from Tools.Directories import crawlDirectory, pathExists, createDir
from types import *
from time import *
from boxbranding import getImageDistro

import sys, traceback
import os
import re
import new
import _enigma

# return value
# -1 none
# 0 simple epgcache.load() patch
# 1 edg nemesis patch
# 2 oudeis patch
# 3 crossepg v2.1 patch
def getEPGPatchType():
	try:
		xepgpatch = new.instancemethod(_enigma.eEPGCache_crossepgImportEPGv21,None,eEPGCache)
		return 3
	except Exception, e:
		pass

	try:
		epgpatch = new.instancemethod(_enigma.eEPGCache_load,None,eEPGCache)
		return 0
	except Exception, e:
		pass

	try:
		edgpatch = new.instancemethod(_enigma.eEPGCache_reloadEpg,None,eEPGCache)
		return 1
	except Exception, e:
		pass

	try:
		oudeispatch = new.instancemethod(_enigma.eEPGCache_importEvent,None,eEPGCache)
		return 2
	except Exception, e:
		pass

	return -1

class CrossEPG_Config:
	providers = [ ]
	db_root = "/hdd/crossepg"
	lamedb = "lamedb"
	home_directory = ""

	force_load_on_boot = 0
	download_weekly_enabled = 0
	download_weekday = _("sunday")
	download_xdaily_enabled = 0
	download_xdaily_num = 2
	download_daily_enabled = 0
	download_daily_hours = 4
	download_daily_minutes = 0
	download_daily_reboot = 0
	download_standby_enabled = 0
	next_update_time = _("Not scheduled")
	download_manual_reboot = 0
	csv_import_enabled = 0
	show_plugin = 1
	show_extension = 1
	show_force_reload_as_plugin = 0
	last_partial_download_timestamp = 0
	last_full_download_timestamp = 0
	last_defrag_timestamp = time()
	configured = 0

	def __init__(self):
		if pathExists("/usr/crossepg"):
			self.home_directory = "/usr/crossepg"
		elif pathExists("/var/crossepg"):
			self.home_directory = "/var/crossepg"
		else:
			print "[CrossEPG_Config] ERROR!! CrossEPG binaries non found"

	def load(self):
		try:
			f = open("%s/crossepg.config" % (self.home_directory), "r")
		except Exception, e:
			#print "[CrossEPG_Config] %s" % (e)
			return

		commentRe = re.compile(r"#(.*)")
		entryRe = re.compile(r"(.*)=(.*)")

		for line in f.readlines():
			try:
				comment = re.findall(commentRe, line)
				if not comment:
					entry = re.findall(entryRe, line)
					if entry:
						key = entry[0][0].strip()
						value = entry[0][1].strip()
						if key == "db_root":
							self.db_root = value
						if key == "lamedb":
							self.lamedb = value
						elif key == "providers":
							self.providers = []
							tmp = value.split("|")
							for p in tmp:
								if len(p) > 0 and os.path.exists("%s/providers/%s.conf" % (self.home_directory, p)):
									self.providers.append(p)
						elif key == "force_load_on_boot":
							self.force_load_on_boot = int(value);
						elif key == "download_weekly_enabled":
							self.download_weekly_enabled = int(value);
						elif key == "download_weekday":
							self.download_weekday = value;
						elif key == "download_xdaily_enabled":
							self.download_xdaily_enabled = int(value);
						elif key == "download_xdaily_num":
							self.download_xdaily_num = int(value);
						elif key == "download_daily_enabled":
							self.download_daily_enabled = int(value);
						elif key == "download_daily_hours":
							self.download_daily_hours = int(value);
						elif key == "download_daily_minutes":
							self.download_daily_minutes = int(value);
						elif key == "next_update_time":
							self.next_update_time = value;
						elif key == "download_daily_reboot":
							self.download_daily_reboot = int(value);
						elif key == "download_manual_reboot":
							self.download_manual_reboot = int(value);
						elif key == "download_standby_enabled":
							self.download_standby_enabled = int(value);
						elif key == "last_partial_download_timestamp":
							self.last_partial_download_timestamp = int(value);
						elif key == "last_full_download_timestamp":
							self.last_full_download_timestamp = int(value);
						elif key == "last_defrag_timestamp":
							self.last_defrag_timestamp = int(value);
						elif key == "csv_import_enabled":
							self.csv_import_enabled = int(value);
						elif key == "show_plugin":
							self.show_plugin = int(value);
						elif key == "show_extension":
							self.show_extension = int(value);
						elif key == "show_force_reload_as_plugin":
							self.show_force_reload_as_plugin = int(value);
						elif key == "configured":
							self.configured = int(value);
			except Exception:
				pass

		f.close()

	def save(self):
		try:
			f = open("%s/crossepg.config" % (self.home_directory), "w")
		except Exception, e:
			print "[CrossEPG_Config] %s" % (e)
			return

		tmp = []
		for p in self.providers:
			if len(p) > 0:
					tmp.append(p)
		self.providers = tmp

		f.write("db_root=%s\n" % (self.db_root))
		f.write("lamedb=%s\n" % (self.lamedb))
		f.write("providers=%s\n" % ("|".join(self.providers)))
		f.write("force_load_on_boot=%d\n" % (self.force_load_on_boot))
		f.write("download_weekly_enabled=%d\n" % (self.download_weekly_enabled))
		f.write("download_weekday=%s\n" % (self.download_weekday))
		f.write("download_xdaily_enabled=%d\n" % (self.download_xdaily_enabled))
		f.write("download_xdaily_num=%d\n" % (self.download_xdaily_num))
		f.write("download_daily_enabled=%d\n" % (self.download_daily_enabled))
		f.write("download_daily_hours=%d\n" % (self.download_daily_hours))
		f.write("download_daily_minutes=%d\n" % (self.download_daily_minutes))
		f.write("next_update_time=%s\n" % (self.next_update_time))
		f.write("download_daily_reboot=%d\n" % (self.download_daily_reboot))
		f.write("download_manual_reboot=%d\n" % (self.download_manual_reboot))
		f.write("download_standby_enabled=%d\n" % (self.download_standby_enabled))
		f.write("last_full_download_timestamp=%d\n" % (self.last_full_download_timestamp))
		f.write("last_partial_download_timestamp=%d\n" % (self.last_partial_download_timestamp))
		f.write("last_defrag_timestamp=%d\n" % (self.last_defrag_timestamp))
		f.write("csv_import_enabled=%d\n" % (self.csv_import_enabled))
		f.write("show_plugin=%d\n" % (self.show_plugin))
		f.write("show_extension=%d\n" % (self.show_extension))
		f.write("show_force_reload_as_plugin=%d\n" % (self.show_force_reload_as_plugin))
		f.write("configured=%d\n" % (self.configured))

		f.close()

	def getChannelProtocol(self, provider):
		try:
			f = open("%s/providers/%s.conf" % (self.home_directory, provider), "r")
		except Exception, e:
			print "[CrossEPG_Config] %s" % (e)
			return

		proto = re.compile(r"protocol=(.*)")
		for line in f.readlines():
			zproto = re.findall(proto, line)
			if zproto:
				f.close()
				return zproto[0].strip()

		f.close()
		return ""

	def getTransponder(self, provider):
		try:
			f = open("%s/providers/%s.conf" % (self.home_directory, provider), "r")
		except Exception, e:
			print "[CrossEPG_Config] %s" % (e)
			return
			
		regexp = re.compile(r"(.*)=(.*)")

		transponder_keys = [
				"frequency",
				"symbol_rate",
				"polarization",
				"fec_inner",
				"orbital_position",
				"inversion",
				"system",
				"modulation",
				"roll_off",
				"pilot",
			]
			
		transponder = {}
		for line in f.readlines():
			res = re.findall(regexp, line)
			if res:
				key = res[0][0]
				try:
					value = int(res[0][1])
				except Exception, e:
					value = -1
				
				if key in transponder_keys:
					transponder[key] = value

		if len(transponder.keys()) != len(transponder_keys):
			return

		f.close()

		return transponder
		
	def getAllProviders(self):
		providers = list()
		providersdesc = list()
		providersproto = list()
		cfiles = crawlDirectory("%s/providers/" % (self.home_directory), ".*\.conf$")
		for cfile in cfiles:
			providers.append(cfile[1].replace(".conf", ""))

		providers.sort()

		for provider in providers:
			try:
				descadded = False
				protoadded = False
				f = open("%s/providers/%s.conf" % (self.home_directory, provider), "r")
				desc = re.compile(r"description=(.*)")
				proto = re.compile(r"protocol=(.*)")
				for line in f.readlines():
					zdesc = re.findall(desc, line)
					if zdesc:
						providersdesc.append(zdesc[0].strip())
						descadded = True

					zproto = re.findall(proto, line)
					if zproto:
						providersproto.append(zproto[0].strip())
						protoadded = True

					if descadded and protoadded:
						break

				f.close()

				if not descadded:
					providersdesc.append(provider)

				if not protoadded:
					providersproto.append(None)

			except Exception, e:
				print "[CrossEPG_Config] %s" % (e)
				providersdesc.append(provider)
				providersproto.append(None)

		ret = [providers, providersdesc, providersproto]
		return ret

	def getAllLamedbs(self):
		lamedbs = list()
		cfiles = crawlDirectory("/etc/enigma2/", "^lamedb.*")
		for cfile in cfiles:
			lamedbs.append(cfile[1])

		return lamedbs

	def isQBOXHD(self):
		try:
			ret = False
			f = open("/proc/stb/info/model", "r")
			model = f.read().strip()
			if model == "qboxhd" or model == "qboxhd-mini":
				ret = True
			f.close()
			return ret
		except Exception, e:
			return False

	def deleteLog(self):
		try:
			if getImageDistro() != "openvix":
				os.unlink(self.db_root + "/crossepg.log")
			else:
				os.unlink(config.misc.epgcachepath.value + "/crossepg.log")
		except Exception, e:
			print e

class CrossEPG_Wrapper:
	EVENT_READY			= 0
	EVENT_OK			= 1
	EVENT_START			= 2
	EVENT_END			= 3
	EVENT_QUIT			= 4
	EVENT_ERROR			= 5
	EVENT_ACTION			= 6
	EVENT_STATUS			= 7
	EVENT_PROGRESS			= 8
	EVENT_PROGRESSONOFF		= 9
	EVENT_CHANNEL			= 10
	EVENT_STARTTIME			= 11
	EVENT_LENGTH			= 12
	EVENT_NAME			= 13
	EVENT_DESCRIPTION		= 14
	EVENT_FILE			= 15
	EVENT_URL			= 16

	INFO_HEADERSDB_SIZE		= 50
	INFO_DESCRIPTORSDB_SIZE	= 51
	INFO_INDEXESDB_SIZE		= 52
	INFO_ALIASESDB_SIZE		= 53
	INFO_TOTAL_SIZE			= 54
	INFO_CHANNELS_COUNT		= 55
	INFO_EVENTS_COUNT		= 56
	INFO_HASHES_COUNT		= 57
	INFO_CREATION_TIME		= 58
	INFO_UPDATE_TIME		= 59
	INFO_VERSION			= 60

	CMD_DOWNLOADER		= 0
	CMD_CONVERTER		= 1
	CMD_INFO			= 2
	CMD_IMPORTER		= 3
	CMD_DEFRAGMENTER	= 4

	home_directory = ""

	def __init__(self):
		self.cmd = eConsoleAppContainer()
		self.cache = None
		self.callbackList = []
		self.type = 0
		self.maxSize = "0 byte"

		versionlist = getEnigmaVersionString().split("-");

		self.oldapi = False
		try:
			if len(versionlist) >= 3:
				self.version = int(versionlist[0]+versionlist[1]+versionlist[2])
				if self.version < 20100716:
					self.oldapi = True
		except Exception:
			pass

		config = CrossEPG_Config()
		if config.isQBOXHD():
				self.oldapi = True

		if pathExists("/usr/crossepg"):
			self.home_directory = "/usr/crossepg"
		elif pathExists("/var/crossepg"):
			self.home_directory = "/var/crossepg"
		else:
			print "[CrossEPG_Config] ERROR!! CrossEPG binaries non found"

	def init(self, cmd, dbdir):
		if not pathExists(dbdir):
			if not createDir(dbdir):
				dbdir = "/hdd/crossepg"

		if cmd == self.CMD_DOWNLOADER:
			x = "%s/crossepg_downloader -r -d %s" % (self.home_directory, dbdir)
		elif cmd == self.CMD_CONVERTER:
			x = "%s/crossepg_dbconverter -r -d %s" % (self.home_directory, dbdir)
		elif cmd == self.CMD_INFO:
			x = "%s/crossepg_dbinfo -r -d %s" % (self.home_directory, dbdir)
		elif cmd == self.CMD_IMPORTER:
			importdir = "%s/import/" % (dbdir)
			x = "%s/crossepg_importer -r -i %s -d %s" % (self.home_directory, importdir, dbdir)
		elif cmd == self.CMD_DEFRAGMENTER:
			x = "%s/crossepg_defragmenter -r -d %s" % (self.home_directory, dbdir)
		else:
			print "[CrossEPG_Wrapper] unknow command on init"
			return

		print "[CrossEPG_Wrapper] executing %s" % (x)
		self.cmd.appClosed.append(self.__cmdFinished)
		self.cmd.dataAvail.append(self.__cmdData)
		if self.cmd.execute(x):
			self.cmdFinished(-1)

	def __cmdFinished(self, retval):
		self.__callCallbacks(self.EVENT_QUIT)
		self.cmd.appClosed.remove(self.__cmdFinished)
		self.cmd.dataAvail.remove(self.__cmdData)

	def __cmdData(self, data):
		if self.cache is None:
			self.cache = data
		else:
			self.cache += data

		if '\n' in data:
			splitcache = self.cache.split('\n')
			if self.cache[-1] == '\n':
				iteration = splitcache
				self.cache = None
			else:
				iteration = splitcache[:-1]
				self.cache = splitcache[-1]
			for mydata in iteration:
				if mydata != '':
					self.__parseLine(mydata)

	def __parseLine(self, data):
		if data.find("CHANNEL ") == 0:
			self.__callCallbacks(self.EVENT_CHANNEL, data[7:])
		elif data.find("STARTTIME ") == 0:
			self.__callCallbacks(self.EVENT_STARTTIME, int(data[10:]))
		elif data.find("LENGTH ") == 0:
			self.__callCallbacks(self.EVENT_LENGTH, int(data[7:]))
		elif data.find("NAME ") == 0:
			self.__callCallbacks(self.EVENT_NAME, data[5:])
		elif data.find("DESCRIPTION ") == 0:
			self.__callCallbacks(self.EVENT_DESCRIPTION, data[12:].replace("\\n", "\n"))
		elif data == "READY":
			print "[CrossEPG_Wrapper] <- READY"
			self.__callCallbacks(self.EVENT_READY, None)
		elif data == "START":
			print "[CrossEPG_Wrapper] <- START"
			self.__callCallbacks(self.EVENT_START, None)
		elif data == "END":
			print "[CrossEPG_Wrapper] <- END"
			self.__callCallbacks(self.EVENT_END, None)
		elif data == "OK":
			print "[CrossEPG_Wrapper] <- OK"
			self.__callCallbacks(self.EVENT_OK, None)
		elif data.find("ERROR ") == 0:
			ttype = data[5:]
			self.__callCallbacks(self.EVENT_ERROR, data[6:])
		elif data.find("TYPE ") == 0:
			ttype = data[5:]
			print "[CrossEPG_Wrapper] <- TYPE %s" % (ttype)
			if ttype == "READ CHANNELS":
				self.type = 0;
				self.__callCallbacks(self.EVENT_ACTION, _("Reading channels"))
			elif ttype == "READ TITLES":
				self.type = 1;
				self.__callCallbacks(self.EVENT_ACTION, _("Reading titles"))
			elif ttype == "PARSE TITLES":
				self.type = 2;
				self.__callCallbacks(self.EVENT_ACTION, _("Parsing titles"))
			elif ttype == "READ SUMMARIES":
				self.type = 3;
				self.__callCallbacks(self.EVENT_ACTION, _("Reading summaries"))
			elif ttype == "PARSE SUMMARIES":
				self.type = 4;
				self.__callCallbacks(self.EVENT_ACTION, _("Parsing summaries"))
			elif ttype == "DOWNLOADING CHANNELS LIST":
				self.type = 5;
				self.__callCallbacks(self.EVENT_ACTION, _("Downloading channels list"))
			elif ttype == "DEFLATING CHANNELS LIST":
				self.type = 6;
				self.__callCallbacks(self.EVENT_ACTION, _("Deflating channels list"))
			elif ttype == "PARSING CHANNELS LIST":
				self.type = 7;
				self.__callCallbacks(self.EVENT_ACTION, _("Parsing channels list"))
			elif ttype == "DOWNLOADING EVENTS":
				self.type = 8;
				self.__callCallbacks(self.EVENT_ACTION, _("Downloading events"))
			elif ttype == "DEFLATING EVENTS":
				self.type = 9;
				self.__callCallbacks(self.EVENT_ACTION, _("Deflating events"))
			elif ttype == "PARSING EVENTS":
				self.type = 10;
				self.__callCallbacks(self.EVENT_ACTION, _("Parsing events"))
			elif ttype == "DOWNLOADING XEPGDB":
				self.type = 11;
				self.__callCallbacks(self.EVENT_ACTION, _("Downloading XEPGDB"))
			elif ttype == "PARSING XEPGDB":
				self.type = 12;
				self.__callCallbacks(self.EVENT_ACTION, _("Merging XEPGDB"))
			elif ttype == "DEFLATING XEPGDB":
				self.type = 13;
				self.__callCallbacks(self.EVENT_ACTION, _("Deflating XEPGDB"))
			elif ttype == "RUNNING SCRIPT":
				self.type = 14;
				self.__callCallbacks(self.EVENT_ACTION, _("Running script"))
			elif ttype.find("RUNNING CSCRIPT ") == 0:
				self.type = 14;
				self.__callCallbacks(self.EVENT_ACTION, _("Running script") + " " + data[21:])

		elif data.find("CHANNELS ") == 0:
			self.__callCallbacks(self.EVENT_STATUS, _("%s channels") % (data[9:]))
		elif data.find("SIZE ") == 0:
			if self.type == 1 or self.type == 3:
				self.maxSize = data[5:]
				self.__callCallbacks(self.EVENT_STATUS, _("Read %s") % (data[5:]))
			else:
				self.__callCallbacks(self.EVENT_STATUS, _("%s of %s") % (data[5:], self.maxSize))
		elif data.find("PROGRESS ") == 0:
			if data[9:] == "ON":
				self.__callCallbacks(self.EVENT_PROGRESSONOFF, True)
			elif data[9:] == "OFF":
				self.__callCallbacks(self.EVENT_PROGRESSONOFF, False)
			else:
				self.__callCallbacks(self.EVENT_PROGRESS, int(data[9:]))
		elif data.find("FILE ") == 0:
			self.__callCallbacks(self.EVENT_FILE, data[5:])
		elif data.find("URL ") == 0:
			self.__callCallbacks(self.EVENT_URL, data[4:])
		elif data.find("VERSION ") == 0:
			self.__callCallbacks(self.INFO_VERSION, data[8:])
		elif data.find("HEADERSDB_SIZE ") == 0:
			self.__callCallbacks(self.INFO_HEADERSDB_SIZE, data[15:])
		elif data.find("DESCRIPTORSDB_SIZE ") == 0:
			self.__callCallbacks(self.INFO_DESCRIPTORSDB_SIZE, data[19:])
		elif data.find("INDEXESDB_SIZE ") == 0:
			self.__callCallbacks(self.INFO_INDEXESDB_SIZE, data[15:])
		elif data.find("ALIASESDB_SIZE ") == 0:
			self.__callCallbacks(self.INFO_ALIASESDB_SIZE, data[15:])
		elif data.find("TOTAL_SIZE ") == 0:
			self.__callCallbacks(self.INFO_TOTAL_SIZE, data[11:])
		elif data.find("CHANNELS_COUNT ") == 0:
			self.__callCallbacks(self.INFO_CHANNELS_COUNT, data[15:])
		elif data.find("EVENTS_COUNT ") == 0:
			self.__callCallbacks(self.INFO_EVENTS_COUNT, data[13:])
		elif data.find("HASHES_COUNT ") == 0:
			self.__callCallbacks(self.INFO_HASHES_COUNT, data[13:])
		elif data.find("CREATION_TIME ") == 0:
			self.__callCallbacks(self.INFO_CREATION_TIME, data[14:])
		elif data.find("UPDATE_TIME ") == 0:
			self.__callCallbacks(self.INFO_UPDATE_TIME, data[12:])
		elif data.find("LOGTEXT ") == 0:
			self.__callCallbacks(self.EVENT_STATUS, data[8:])

	def __callCallbacks(self, event, param = None):
		for callback in self.callbackList:
			callback(event, param)

	def addCallback(self, callback):
		self.callbackList.append(callback)

	def delCallback(self, callback):
		self.callbackList.remove(callback)

	def running(self):
		return self.cmd.running()

	def lamedb(self, value):
		print "[CrossEPG_Wrapper] -> LAMEDB %s" % (value)
		cmd = "LAMEDB %s\n" % (value)
		if self.oldapi:
			self.cmd.write(cmd, len(cmd))
		else:
			self.cmd.write(cmd)

	def epgdat(self, value):
		print "[CrossEPG_Wrapper] -> EPGDAT %s" % (value)
		cmd = "EPGDAT %s\n" % (value)
		if self.oldapi:
			self.cmd.write(cmd, len(cmd))
		else:
			self.cmd.write(cmd)

	def demuxer(self, value):
		print "[CrossEPG_Wrapper] -> DEMUXER %s" % (value)
		cmd = "DEMUXER %s\n" % (value)
		if self.oldapi:
			self.cmd.write(cmd, len(cmd))
		else:
			self.cmd.write(cmd)

	def frontend(self, value):
		print "[CrossEPG_Wrapper] -> FRONTEND %s" % (value)
		cmd = "FRONTEND %d\n" % (value)
		if self.oldapi:
			self.cmd.write(cmd, len(cmd))
		else:
			self.cmd.write(cmd)
	
	def defrag(self,):
		print "[CrossEPG_Wrapper] -> DEFRAGMENT"
		cmd = "DEFRAGMENT\n"
		if self.oldapi:
			self.cmd.write(cmd, len(cmd))
		else:
			self.cmd.write(cmd)
	
	def download(self, provider):
		print "[CrossEPG_Wrapper] -> DOWNLOAD %s" % (provider)
		cmd = "DOWNLOAD %s\n" % (provider)
		if self.oldapi:
			self.cmd.write(cmd, len(cmd))
		else:
			self.cmd.write(cmd)

	def convert(self):
		print "[CrossEPG_Wrapper] -> CONVERT"
		self.__callCallbacks(self.EVENT_ACTION, _("Converting data"))
		self.__callCallbacks(self.EVENT_STATUS, "")
		if self.oldapi:
			self.cmd.write("CONVERT\n", 8)
		else:
			self.cmd.write("CONVERT\n")

	def importx(self):
		print "[CrossEPG_Wrapper] -> IMPORT"
		if self.oldapi:
			self.cmd.write("IMPORT\n", 7)
		else:
			self.cmd.write("IMPORT\n")

	def text(self):
		print "[CrossEPG_Wrapper] -> TEXT"
		self.__callCallbacks(self.EVENT_ACTION, _("Loading data"))
		self.__callCallbacks(self.EVENT_STATUS, "")
		if self.oldapi:
			self.cmd.write("TEXT\n", 5)
		else:
			self.cmd.write("TEXT\n")

	def stop(self):
		print "[CrossEPG_Wrapper] -> STOP"
		if self.oldapi:
			self.cmd.write("STOP\n", 5)
		else:
			self.cmd.write("STOP\n")

	def save(self):
		print "[CrossEPG_Wrapper] -> SAVE"
		self.__callCallbacks(self.EVENT_ACTION, _("Saving data"))
		self.__callCallbacks(self.EVENT_STATUS, "")
		if self.oldapi:
			self.cmd.write("SAVE\n", 5)
		else:
			self.cmd.write("SAVE\n")

	def wait(self):
		print "[CrossEPG_Wrapper] -> WAIT"
		if self.oldapi:
			self.cmd.write("WAIT\n", 5)
		else:
			self.cmd.write("WAIT\n")

	def quit(self):
		print "[CrossEPG_Wrapper] -> QUIT"
		if self.oldapi:
			self.cmd.write("QUIT\n", 5)
		else:
			self.cmd.write("QUIT\n")

	def open(self):
		print "[CrossEPG_Wrapper] -> OPEN"
		if self.oldapi:
			self.cmd.write("OPEN\n", 5)
		else:
			self.cmd.write("OPEN\n")

	def close(self):
		print "[CrossEPG_Wrapper] -> CLOSE"
		if self.oldapi:
			self.cmd.write("CLOSE\n", 6)
		else:
			self.cmd.write("CLOSE\n")
