from enigma import eConsoleAppContainer
from Tools.Directories import crawlDirectory, pathExists, createDir
from types import *

import sys, traceback
import os
import re
import time
import new
import mutex
import _enigma

class CrossEPG_Config:
	providers = [ "skyitalia" ]
	db_root = "/hdd/crossepg"
	lamedb = "lamedb"
	home_directory = ""
	
	auto_boot = 1
	auto_daily = 0
	auto_daily_hours = 4
	auto_daily_minutes = 0
	auto_tune = 1
	auto_tune_osd = 0
	
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
			print "[CrossEPG_Config] %s" % (e)
			return
			
		commentRe = re.compile(r"#(.*)")
		entryRe = re.compile(r"(.*)=(.*)")
		
		for line in f.readlines(): 
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
					elif key == "otv_provider":
						self.providers = value.split("|")
					elif key == "auto_boot":
						self.auto_boot = int(value);
					elif key == "auto_daily":
						self.auto_daily = int(value);
					elif key == "auto_daily_hours":
						self.auto_daily_hours = int(value);
					elif key == "auto_daily_minutes":
						self.auto_daily_minutes = int(value);
					elif key == "auto_tune":
						self.auto_tune = int(value);
					elif key == "auto_tune_osd":
						self.auto_tune_osd = int(value);
						
		f.close()
		print "[CrossEPG_Config] configuration loaded"
		
	def save(self):
		try:
			f = open("%s/crossepg.config" % (self.home_directory), "w")
		except Exception, e:
			print "[CrossEPG_Config] %s" % (e)
			return
		
		f.write("db_root=%s\n" % (self.db_root))
		f.write("lamedb=%s\n" % (self.lamedb))
		f.write("otv_provider=%s\n" % ("|".join(self.providers)))
		f.write("auto_boot=%d\n" % (self.auto_boot))
		f.write("auto_daily=%d\n" % (self.auto_daily))
		f.write("auto_daily_hours=%d\n" % (self.auto_daily_hours))
		f.write("auto_daily_minutes=%d\n" % (self.auto_daily_minutes))
		f.write("auto_tune=%d\n" % (self.auto_tune))
		f.write("auto_tune_osd=%d\n" % (self.auto_tune_osd))
		
		f.close()
		print "[CrossEPG_Config] configuration saved"
		
	def getChannelID(self, provider):
		try:
			f = open("%s/providers/%s.conf" % (self.home_directory, provider), "r")
		except Exception, e:
			print "[CrossEPG_Config] %s" % (e)
			return
			
		nid = -1;
		tsid = -1;
		sid = -1;
		namespace = -1;
		nidRe = re.compile(r"nid=(.*)")
		tsidRe = re.compile(r"tsid=(.*)")
		sidRe = re.compile(r"sid=(.*)")
		namespaceRe = re.compile(r"namespace=(.*)")
		
		for line in f.readlines(): 
			znid = re.findall(nidRe, line)
			if znid:
				nid = int(znid[0]);
			zsid = re.findall(sidRe, line)
			if zsid:
				sid = int(zsid[0]);
			ztsid = re.findall(tsidRe, line)	
			if ztsid:
				tsid = int(ztsid[0]);
			znamespace = re.findall(namespaceRe, line)	
			if znamespace:
				namespace = int(znamespace[0]);
		
		if nid == -1 or sid == -1 or tsid == -1:
			print "[CrossEPG_Config] invalid configuration file"
			return
		
		f.close()
		return "1:0:1:%X:%X:%X:%X:0:0:0:" % (sid, tsid, nid, namespace)
		
	def getAllProviders(self):
		providers = list()
		cfiles = crawlDirectory("%s/providers/" % (self.home_directory), ".*\.conf$")
		for cfile in cfiles:
			providers.append(cfile[1].replace(".conf", ""))
			
		return providers
		
	def getAllLamedbs(self):
		lamedbs = list()
		cfiles = crawlDirectory("/etc/enigma2/", "^lamedb.*")
		for cfile in cfiles:
			lamedbs.append(cfile[1])

		return lamedbs

class CrossEPG_Wrapper:
	EVENT_READY = 0
	EVENT_OK = 1
	EVENT_START = 2
	EVENT_END = 3
	EVENT_QUIT = 4
	EVENT_ERROR = 5
	EVENT_ACTION = 9
	EVENT_STATUS = 10
	EVENT_PROGRESS = 11
	EVENT_PROGRESSONOFF = 12
	EVENT_CHANNEL = 13
	EVENT_STARTTIME = 14
	EVENT_LENGTH = 15
	EVENT_NAME = 16
	EVENT_DESCRIPTION = 17
	
	CMD_DOWNLOADER = 0
	CMD_CONVERTER = 1

	home_directory = ""

	def __init__(self):
		self.cmd = eConsoleAppContainer()
		self.cache = None
		self.callbackList = []
		self.type = 0
		self.maxSize = "0 byte"
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
			x = "%s/crossepg_downloader -k 19 -r -d %s" % (self.home_directory, dbdir)
		elif cmd == self.CMD_CONVERTER:
			x = "%s/crossepg_dbconverter -k 19 -r -d %s" % (self.home_directory, dbdir)
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
			if (ttype == "READ CHANNELS"):
				self.type = 0;
				self.__callCallbacks(self.EVENT_ACTION, "Reading channels")
			elif (ttype == "READ TITLES"):
				self.type = 1;
				self.__callCallbacks(self.EVENT_ACTION, "Reading titles")
			elif (ttype == "PARSE TITLES"):
				self.type = 2;
				self.__callCallbacks(self.EVENT_ACTION, "Parsing titles")
			elif (ttype == "READ SUMMARIES"):
				self.type = 3;
				self.__callCallbacks(self.EVENT_ACTION, "Reading summaries")
			elif (ttype == "PARSE SUMMARIES"):
				self.type = 4;
				self.__callCallbacks(self.EVENT_ACTION, "Parsing summaries")
		elif data.find("CHANNELS ") == 0:
			self.__callCallbacks(self.EVENT_STATUS, "%s channels" % (data[9:]))
		elif data.find("SIZE ") == 0:
			if self.type == 1 or self.type == 3:
				self.maxSize = data[5:]
				self.__callCallbacks(self.EVENT_STATUS, "Read %s" % (data[5:]))
			else:
				self.__callCallbacks(self.EVENT_STATUS, "%s of %s" % (data[5:], self.maxSize))
		elif data.find("PROGRESS ") == 0:
			if data[9:] == "ON":
				self.__callCallbacks(self.EVENT_PROGRESSONOFF, True)
			elif data[9:] == "OFF":
				self.__callCallbacks(self.EVENT_PROGRESSONOFF, False)
			else:
				self.__callCallbacks(self.EVENT_PROGRESS, int(data[9:]))

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
		self.cmd.write(cmd, len(cmd))
		
	def epgdat(self, value):
		print "[CrossEPG_Wrapper] -> EPGDAT %s" % (value)
		cmd = "EPGDAT %s\n" % (value)
		self.cmd.write(cmd, len(cmd))
			
	def demuxer(self, value):
		print "[CrossEPG_Wrapper] -> DEMUXER %s" % (value)
		cmd = "DEMUXER %s\n" % (value)
		self.cmd.write(cmd, len(cmd))

	def download(self, provider):
		print "[CrossEPG_Wrapper] -> DOWNLOAD %s" % (provider)
		cmd = "DOWNLOAD %s\n" % (provider)
		self.cmd.write(cmd, len(cmd))
		
	def convert(self):
		print "[CrossEPG_Wrapper] -> CONVERT"
		self.__callCallbacks(self.EVENT_ACTION, "Converting data")
		self.__callCallbacks(self.EVENT_STATUS, "")
		self.cmd.write("CONVERT\n", 8)
		
	def text(self):
		print "[CrossEPG_Wrapper] -> TEXT"
		self.__callCallbacks(self.EVENT_ACTION, "Loading data")
		self.__callCallbacks(self.EVENT_STATUS, "")
		self.cmd.write("TEXT\n", 5)
			
	def stop(self):
		print "[CrossEPG_Wrapper] -> STOP"
		self.cmd.write("STOP\n", 5)

	def save(self):
		print "[CrossEPG_Wrapper] -> SAVE"
		self.__callCallbacks(self.EVENT_ACTION, "Saving data")
		self.__callCallbacks(self.EVENT_STATUS, "")
		self.cmd.write("SAVE\n", 5)

	def wait(self):
		print "[CrossEPG_Wrapper] -> WAIT"
		self.cmd.write("WAIT\n", 5)

	def quit(self):
		print "[CrossEPG_Wrapper] -> QUIT"
		self.cmd.write("QUIT\n", 5)
