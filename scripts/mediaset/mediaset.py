#!/usr/bin/python
# mediaset.py  by Ambrosa http://www.ambrosa.net
# this module is used for download EPG data from Mediaset website
# derived from E2_LOADEPG
# 29-Dec-2011

__author__ = "ambrosa http://www.ambrosa.net"
__copyright__ = "Copyright (C) 2008-2011 Alessandro Ambrosini"
__license__ = "CreativeCommons by-nc-sa http://creativecommons.org/licenses/by-nc-sa/3.0/"

import gc
import os
import sys
import time
import codecs
import socket
import urllib
import urllib2
import ConfigParser
#from xml.dom import minidom

# import CrossEPG functions
import crossepg

# location of local python modules under "scripts/lib" dir.
# add it to sys.path()
crossepg_instroot = crossepg.epgdb_get_installroot()
if crossepg_instroot == False:
	sys.exit(1)
libdir = os.path.join(crossepg_instroot, 'scripts/lib')
sys.path.append(libdir)

# import local modules
import sgmllib
import scriptlib

# =================================================================


class main(sgmllib.SGMLParser):

	# main config file
	CONF_CONFIGFILENAME = "mediaset.conf"

	# Network socket timeout (in seconds)
	CONF_SOCKET_TIMEOUT = 20

	# log text
	CONF_LOG_SCRIPT_NAME = "Mediaset FTA (IT)"
	CONF_LOG_FILENAME = '' # if empty, log to crossepg.log

	# max chars in description
	CONF_DLDESCMAXCHAR = 250

	# retry number if HTTP error
	HTTP_ERROR_RETRY = 3
	# seconds to wait between retries
	HTTP_ERROR_WAIT_RETRY = 5

	# charset used in remote website epg data
	REMOTE_EPG_CHARSET = 'utf-8'

	TODAYMP = ''
	DAYCACHEMP = []
	FIELD_SEPARATOR = '###'
	CHANNELLIST = {}

	DESCRIPTIONS_WEBCACHE = {}


# -------- xml processing using SGMLLIB -----------
# best way is use xml.minidom but it's very memory hungry (about 40MB memory for 2 MB XML file)
# sgmllib can simple parse xml data
	SGML_PALINSESTO_INSIDE = False
	SGML_PROGRAMMI_INSIDE = False
	
	SGML_GIORNOMP = None
	SGML_FD = {}
	SGML_TOTAL_EVENTS = 0

	SGML_EVENT_IDREF = None
	SGML_EVENT_TIMESTAMP = None
	SGML_EVENT_TITLE = None
	
	SGML_PBAR_MAXVALUE = 1
	SGML_PBAR_INDEX = 0

	def parse(self, s):
		self.feed(s)
		self.close()

	def start_palinsesto(self, attr):
		self.SGML_PALINSESTO_INSIDE = True
		self.SGML_PBAR_MAXVALUE = 100.0 / self.SGML_PBAR_MAXVALUE

	def end_palinsesto(self):
		self.SGML_PALINSESTO_INSIDE = False
		self.SGML_GIORNOMP = None
		self.log.log("extracted %d events" % self.SGML_TOTAL_EVENTS)

	def start_guidatv(self, attr):
		if self.SGML_PALINSESTO_INSIDE == True:
			self.SGML_GIORNOMP = None
			for name, value in attr:
				if name == "data":
					self.log.log2video_pbar(self.SGML_PBAR_INDEX * self.SGML_PBAR_MAXVALUE)
					if str(value).strip(' \n\r') in self.DAYCACHEMP:
						self.SGML_GIORNOMP = str(value).strip(' \n\r')
						self.log.log2video_status("processing XML %s (%d)" % (self.SGML_GIORNOMP, self.SGML_TOTAL_EVENTS))
						self.SGML_FD = {}
						
						for c in sorted(self.CHANNELLIST.keys()):
							cacheopt = int(self.CHANNELLIST[c].split(",")[0])
							
							# if cacheopt == 0, do nothing
							if cacheopt == 0:
								continue

							channel_name = ''
							if len(self.CHANNELLIST[c].split(",")) > 1:
								if self.CHANNELLIST[c].split(",")[1] != '':
									# channel renamed, new name provided by user
									channel_name = self.CHANNELLIST[c].split(",")[1].strip(' \n\r').lower()

							# if channel name is not present as option, quit with error
							if channel_name == '':
								self.log.log("ERROR ! ID=%s channel name not present" % c)
								sys.exit(1)

							channel_provider = self.CONF_DEFAULT_PROVIDER
							if len(self.CHANNELLIST[c].split(",")) > 2:
								if self.CHANNELLIST[c].split(",")[2] != '':
									channel_provider = self.CHANNELLIST[c].split(",")[2].strip(' \n\r').lower()

							# if channel name is not present as option in channel_list.conf , skip it
							if channel_name == '':
								self.log.log("ERROR ! ID=%s channel name not present. Skip !" % c)
								continue

							day = str(self.convert_daymp(self.SGML_GIORNOMP))
							eventfilename = scriptlib.fn_escape(c + self.FIELD_SEPARATOR + channel_name + self.FIELD_SEPARATOR + day)
							eventfilepath = os.path.join(self.CONF_CACHEDIR, eventfilename)
							
							if (cacheopt == 1) and os.path.exists(eventfilepath):
								continue
							if (cacheopt == 3) and os.path.exists(eventfilepath) and (self.SGML_GIORNOMP != self.TODAYMP):
								continue
							if (cacheopt != 1) and (cacheopt != 2) and (cacheopt != 3):
								self.log.log("WARNING: unknown cache option %s" % cacheopt)
								continue

							self.log.log("  Opening for caching \'%s\'" % eventfilename)

							self.SGML_FD[c] = codecs.open(eventfilepath, "w", 'utf-8')
							self.SGML_FD[c].write(c + self.FIELD_SEPARATOR + channel_name + self.FIELD_SEPARATOR + channel_provider + self.FIELD_SEPARATOR + day + '\n')
							self.SGML_FD[c].write("Local Time (human readeable)###Unix GMT Time###Event Title###Event Description\n")

	def end_guidatv(self):
		self.SGML_GIORNOMP = None
		for c in self.SGML_FD.keys():
			self.SGML_FD[c].close()

	def start_programmi(self, attr):
		if self.SGML_GIORNOMP != None:
			self.SGML_PROGRAMMI_INSIDE = True

	def end_programmi(self):
		self.SGML_PROGRAMMI_INSIDE = False

	def start_programma(self, attr):
		self.SGML_PBAR_INDEX += 1
		self.SGML_TOTAL_EVENTS += 1
		if (self.SGML_PROGRAMMI_INSIDE == True):
			for name, value in attr:
				if name == "idref":
					self.SGML_EVENT_IDREF = str(value).strip(' \n\r').lower()
					if not self.CHANNELLIST.has_key(self.SGML_EVENT_IDREF):
						self.log.log("WARNING: new channel id=%s found in XML data" % self.SGML_EVENT_IDREF)
				elif name == "timestamp":
					self.SGML_EVENT_TIMESTAMP = str(value).strip(' \n\r')
				elif name == "titolo":
					self.SGML_EVENT_TITLE = str(value).strip(' \n\r')
					self.SGML_EVENT_TITLE = self.SGML_EVENT_TITLE.encode('utf-8')
					
	def end_programma(self):
		if self.SGML_FD.has_key(self.SGML_EVENT_IDREF):
				
			event_starttime = self.SGML_EVENT_TIMESTAMP
			event_startime_unix_gmt = str(int(time.mktime(time.strptime(event_starttime, "%Y%m%d%H%M"))) - self.DELTA_UTC)

			event_title = unicode(self.SGML_EVENT_TITLE)
			event_title = event_title.replace('\r', '')
			event_title = event_title.replace('\n', '')
			event_title = event_title.strip(u' ')
			#self.log.log("  event_title=" + event_title)

			# no descriptions available for Mediaset
			event_description = ''

			self.SGML_FD[self.SGML_EVENT_IDREF].write(event_starttime + self.FIELD_SEPARATOR + event_startime_unix_gmt + self.FIELD_SEPARATOR + event_title + self.FIELD_SEPARATOR + event_description + '\n')
			self.SGML_TOTAL_EVENTS += 1
			
			self.SGML_EVENT_IDREF = None
			self.SGML_EVENT_TIMESTAMP = None
			self.SGML_EVENT_TITLE = None
		
					#pbar_value = int(self.SGML_PBAR_INDEX * 100 / self.SGML_PBAR_MAXVALUE)
					#if pbar_value > 100:
					#	pbar_value = 100
					#self.log.log2video_pbar(pbar_value)
					#self.SGML_PBAR_INDEX += 1

		
#	def handle_data(self, data):
#		pass

#	def report_unbalanced(self, tag):
#		self.log.log("  WARNING: UNBALANCED TAG REPORTED \"%s\" @ line=%d!!" % (tag,self.line) )
#
#	def unknown_starttag(self, tag, attributes):
#		self.log.log(" UNKNW STARTTAG %s !!" % tag)
#
#	def unknown_endtag(self, tag):
#		self.log.log(" UNKNW ENDTAG %s !!" % tag)


# -----------------------------------------------

	def convert_daymp(self, dmp):
		daystandard = time.strftime("%Y%m%d", time.strptime(dmp, "%d/%m/%Y"))
		return daystandard

	def __init__(self, confdir, dbroot):

		# initialize SGMLLIB
		sgmllib.SGMLParser.__init__(self, 0)

		# initialize logging class
		self.log = scriptlib.logging_class(self.CONF_LOG_FILENAME)
		
		# write to video OSD the script name
		self.log.log2video_scriptname(self.CONF_LOG_SCRIPT_NAME)
		
		self.log.log("=== RUNNING SCRIPT %s ===" % self.CONF_LOG_SCRIPT_NAME)

		CONF_FILE = os.path.join(confdir, self.CONF_CONFIGFILENAME)
		if not os.path.exists(CONF_FILE):
			self.log.log("ERROR: %s not present" % CONF_FILE)
			self.log.log2video_status("ERROR: %s not present" % CONF_FILE)
			sys.exit(1)

		config = ConfigParser.ConfigParser()
		#config.optionxform = str  # needed to return case sensitive index
		config.read(CONF_FILE)

		# reading [global] section options
		self.CONF_DEFAULT_PROVIDER = config.get("global", "DEFAULT_PROVIDER")
		# save cache under dbroot
		self.CONF_CACHEDIR = os.path.join(dbroot, config.get("global", "CACHE_DIRNAME"))

		self.CONF_MAX_DAY_EPG = config.getint("global", "MAX_DAY_EPG")

		self.CONF_URL = config.get("global", "URL")

		self.CONF_GMT_ZONE = config.get("global", "GMT_ZONE")
		if self.CONF_GMT_ZONE.strip(' ').lower() == 'equal':
			#self.DELTA_UTC = -scriptlib.delta_utc() # return negative if timezone is east of GMT (like Italy), invert sign
			self.DELTA_UTC = 0
		else:
			self.DELTA_UTC = float(self.CONF_GMT_ZONE) * 3600.0
			if self.DELTA_UTC >= 0:
				self.DELTA_UTC = self.DELTA_UTC + scriptlib.delta_dst()
			else:
				self.DELTA_UTC = self.DELTA_UTC - scriptlib.delta_dst()

		self.DELTA_UTC = int(self.DELTA_UTC)
		#self.log.log("Website timezone - UTC = %d seconds" % self.DELTA_UTC)

		if not os.path.exists(self.CONF_CACHEDIR):
			self.log.log("Create \'%s\' directory for caching" % self.CONF_CACHEDIR)
			os.mkdir(self.CONF_CACHEDIR)

		# reading [channels] section
		temp = config.items("channels")

		# create a dictionary (Python array) with index = channel ID
		for i in temp:
			self.CHANNELLIST[i[0].strip(' \n\r').lower()] = unicode(i[1].strip(' \n\r').lower(), 'utf-8')

		if len(self.CHANNELLIST) == 0:
			self.log.log("ERROR: [channels] section empty ?")
			sys.exit(1)

		# set network socket timeout
		socket.setdefaulttimeout(self.CONF_SOCKET_TIMEOUT)

		self.TODAYMP = time.strftime("%d/%m/%Y")
		# create a list filled with dates (format DD/MM/AAAA) from today to today+ MAX_DAY_EPG
		self.DAYCACHEMP = []
		for day in range(0, self.CONF_MAX_DAY_EPG):
			self.DAYCACHEMP.append(time.strftime("%d/%m/%Y", time.localtime(time.time() + 86400 * day)))


# ----------------------------------------------------------------------


	def download_and_cache(self):
		self.log.log("--- START DOWNLOAD AND CACHE DATA ---")
		self.log.log2video_status("STARTING DOWNLOAD")

		self.log.log("Removing old cached files")
		scriptlib.cleanup_oldcachedfiles(self.CONF_CACHEDIR, self.FIELD_SEPARATOR)

		self.log.log("Start download XML data from \'%s\'" % self.CONF_URL)
		self.log.log2video_status("downloading XML data ...")

		i = self.HTTP_ERROR_RETRY
		while i > 0:
			try:
				sock = urllib2.urlopen(self.CONF_URL)
				data = sock.read()
			except IOError, e:
				serr = "unknown"
				if hasattr(e, 'reason'):
					serr = str(e.reason)
				elif hasattr(e, 'code'):
					serr = str(e.code)
				if hasattr(e, 'msg'):
					serr += " , " + str(e.msg)

				self.log.log("\'" + self.CONF_URL + "\' connection error. Reason: " + serr + ". Waiting " + str(self.HTTP_ERROR_WAIT_RETRY) + " sec. and retry [" + str(i) + "] ...")
				time.sleep(self.HTTP_ERROR_WAIT_RETRY) # add sleep
				i -= 1

			else:
				i = -99
				sock.close()

		if (i != -99):
			self.log.log("Cannot retrieve data from \'" + self.CONF_URL + "\'. Abort script")
			self.log.log2video_status("ERROR: cannot download XML data, abort")
			time.sleep(5)
			sys.exit(1)

		self.log.log("end download XML data, now processing")
		self.log.log2video_status("processing XML data, wait ...")

		# replace malformed single end tag <.../> as <...> (SGML doesn't like "/>")
		data = data.replace('/>', '></programma>')

		# set max 'id' occurencies
		self.SGML_PBAR_MAXVALUE = data.count('idref="')

		# start SGMLLIB parsing
		self.log.log2video_pbar_on()
		self.log.log2video_pbar(0)
		self.parse(data)
		self.log.log2video_pbar(0)
		self.log.log2video_pbar_off()
		
		self.log.log("end process XML data")

# ----------------------------------------------------------------------

	def process_cache(self):
		self.log.log("--- START PROCESSING CACHE ---")
		self.log.log2video_status("START PROCESSING CACHE")
		
		if not os.path.exists(self.CONF_CACHEDIR):
			self.log.log("ERROR: %s not present" % self.CONF_CACHEDIR)
			self.log.log2video_status("ERROR: %s not present, abort" % self.CONF_CACHEDIR)
			time.sleep(5)
			sys.exit(1)

		self.log.log("Loading lamedb")
		lamedb = scriptlib.lamedb_class()

		self.log.log("Initialize CrossEPG database")
		crossdb = scriptlib.crossepg_db_class()
		crossdb.open_db()

		events = []
		previous_id = ''
		channels_name = ''
		total_events = 0

		self.log.log("Start data processing")
		filelist = sorted(os.listdir(self.CONF_CACHEDIR))
		filelist.append('***END***')

		self.log.log2video_pbar_on()
		self.log.log2video_pbar(0)
		pbar_maxvalue = 100.0 / len(filelist)
		pbar_index = 0

		for f in filelist:
			self.log.log2video_pbar(int(pbar_index * pbar_maxvalue))
			pbar_index += 1
			
			id = f.split(self.FIELD_SEPARATOR)[0]
			if previous_id == '':
				previous_id = id

			if id != previous_id:
				total_events += len(events)
				self.log.log("  ...processing \'%s\' , nr. events %d" % (previous_id, len(events)))
				self.log.log2video_status("processed %d events" % total_events)

				for c in channels_name:
					# a channel can have zero or more SID (different channel with same name)
					# return the list [0e1f:00820000:0708:00c8:1:0 , 1d20:00820000:2fa8:013e:1:0 , ..... ]
					# return [] if channel name is not in lamedb
					sidbyname = lamedb.get_sid_byname(c.strip(' \n').lower())

					# process every SID
					for s in sidbyname:
						# convert "0e1f:00820000:0708:00c8:1:0" to sid,tsid,onid
						# return the list [sid,tsid,onid]
						ch_sid = lamedb.convert_sid(s)
						if len(ch_sid) == 0:
							continue

						# add channel into db
						# doesn't matter if the channel already exist... epgdb do all the work
						crossdb.add_channel(ch_sid)

						i = 0
						L = len(events) - 1

						# process events
						for e in events:

							items = e.split(self.FIELD_SEPARATOR)
							e_starttime = int(items[1])

							if i < L:
								e_length = int(events[i + 1].split(self.FIELD_SEPARATOR)[1]) - e_starttime
							else:
								# last event, dummy length 90 min.
								e_length = 5400
							i += 1

							# extract title and encode Python Unicode with UTF-8
							e_title = items[2].encode('utf-8')

							# extract summarie and encode Python Unicode with UTF-8
							e_summarie = items[3].encode('utf-8')

							# add_event(start_time , duration , title , summarie , ISO639_language_code , strings_encoded_with_UTF-8)
							crossdb.add_event(e_starttime, e_length, e_title, e_summarie, 'ita', True)

				if f == '***END***':
					break

				events = []
				previous_id = id
				channels_name = ''

			if id == previous_id:
				self.log.log("Reading  \'%s\'" % f)
				# read events from cache file using UTF-8 and insert them in events list
				fd = codecs.open(os.path.join(self.CONF_CACHEDIR, f), "r", "utf-8")
				lines = fd.readlines()
				fd.close()
				if channels_name == '':
					# first line has channel data (id,name,provider,date)
					channels_name = lines[0].split(self.FIELD_SEPARATOR)[1].split('|')
				# the second line is only a remark
				# add events starting from third line
				events.extend(lines[2:])

		# end process, close CrossEPG DB saving data
		crossdb.close_db()
		
		self.log.log2video_pbar(0)
		self.log.log2video_pbar_off()
		
		self.log.log("TOTAL EPG EVENTS PROCESSED: %d" % total_events)
		self.log.log("--- END ---")
		self.log.log2video_status("END, processed %d events" % total_events)
		
		time.sleep(3)


# ****************************************************************************************************************************

# MAIN CODE: SCRIPT START HERE
# increase this process niceness (other processes have higher priority)
os.nice(10)

# set Garbage Collector to do a "generational jump" more frequently than default 700
# memory saving: about 50% (!!), some performance loss (obviously)
gc.set_threshold(50, 10, 10)

SCRIPT_DIR = 'scripts/mediaset/'

# get CrossEPG installation dir.
crossepg_instroot = crossepg.epgdb_get_installroot()
if crossepg_instroot == False:
	sys.exit(1)
scriptlocation = os.path.join(crossepg_instroot, SCRIPT_DIR)

# get where CrossEPG save data (dbroot) and use it as script cache repository
crossepg_dbroot = crossepg.epgdb_get_dbroot()
if crossepg_dbroot == False:
	sys.exit(1)

# initialize script class
script_class = main(scriptlocation, crossepg_dbroot)

# download data and cache them
script_class.download_and_cache()

# read cached data and inject into CrossEPG database
script_class.process_cache()

