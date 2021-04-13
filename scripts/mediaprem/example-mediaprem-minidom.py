#!/usr/bin/python
# mediaprem.py  by Ambrosa http://www.ambrosa.net
# this module is used for download EPG data from Mediaset website
# derived from E2_LOADEPG

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
from xml.dom import minidom

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
# HTML PARSER


class Description_parser(sgmllib.SGMLParser):
	def parse(self, s):
		self.feed(s)
		self.close()

	def __init__(self, verbose=0):
		sgmllib.SGMLParser.__init__(self, verbose)
		self.start_div_box = False
		self.start_div_boxtxt = False
		self.description = ''


	def start_div(self, attributes):
		for name, value in attributes:
			if name == "class":
				if value == "box_Text":
					self.start_div_box = True
				elif value == "txtBox_cms":
					self.start_div_boxtxt = True

	def end_div(self):
		if self.start_div_boxtxt == True:
			self.start_div_box = False
			self.start_div_boxtxt = False


	def handle_data(self, data):
		if self.start_div_boxtxt == True:
			self.description += data.decode('iso-8859-1')

	def get_descr(self):
		return (self.description.strip(' \n\r'))

# =================================================================


class main:

	# main config file
	CONF_CONFIGFILENAME = "mediaprem.conf"

	# Network socket timeout (in seconds)
	CONF_SOCKET_TIMEOUT = 20

	# log text
	CONF_LOG_SCRIPT_NAME = "MediasetPremium (Italy)"
	CONF_LOG_PREFIX = ""

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


	def log(self, s, video=0):
		self.logging.log(self.CONF_LOG_PREFIX + str(s))
		if video == 1:
			self.log2video(str(s))

	def log2video(self, s):
		self.logging.log2video_status(str(s))

	def convert_daymp(self, dmp):
		daystandard = time.strftime("%Y%m%d", time.strptime(dmp, "%Y/%m/%d"))
		return daystandard


	def get_description(self, url):

		if url[:7] != 'http://':
			return('')

		if (url[-5:] != '.html') and (url[-4:] != '.htm'):
			return('')

		self.log("   downloading description \'" + url + "\'")
		url = str(urllib.quote(url, safe=":/"))

		try:
			sock = urllib2.urlopen(url)
			data = sock.read()
		except IOError, e:
			serr = "unknown"
			if hasattr(e, 'reason'):
				serr = str(e.reason)
			elif hasattr(e, 'code'):
				serr = str(e.code)
				if hasattr(e, 'msg'):
					serr += " , " + str(e.msg)

			self.log(url + "      error, reason: " + serr + ". Skip it.")
			return('')

		else:
			sock.close()
			dsparser = Description_parser()
			dsparser.parse(data)
			return(dsparser.get_descr())

		return('')



	def __init__(self, confdir, dbroot):

		# initialize logging
		self.logging = scriptlib.logging_class()
		# write to video OSD the script name
		self.logging.log2video_scriptname(self.CONF_LOG_SCRIPT_NAME)


		# check swap memory available
		osp = os.popen('free | awk \'/Swap/ { print $2 }\'', 'r')
		ret = osp.readlines()
		if len(ret) > 0:
			try:
				m = int(ret[0]) / 1024
			except:
				self.log("Error get SWAP value, abort", 1)
				time.sleep(10)
				sys.exit(1)

			if m < 60:
				self.log("SWAP Not Enabled (<60MB), abort", 1)
				time.sleep(10)
				sys.exit(1)
		else:
			self.log("Error get SWAP value, abort", 1)
			time.sleep(10)
			sys.exit(1)

		osp.close()


		CONF_FILE = os.path.join(confdir, self.CONF_CONFIGFILENAME)
		if not os.path.exists(CONF_FILE):
			self.log("ERROR: %s not present" % CONF_FILE, 1)
			sys.exit(1)

		config = ConfigParser.ConfigParser()
		#config.optionxform = str  # needed to return case sensitive index
		config.read(CONF_FILE)

		# reading [global] section options
		self.CONF_DEFAULT_PROVIDER = config.get("global", "DEFAULT_PROVIDER")
		# save cache under dbroot
		self.CONF_CACHEDIR = os.path.join(dbroot, config.get("global", "CACHE_DIRNAME"))

		self.CONF_DL_DESC = config.getint("global", "DL_DESC")
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
		#self.log("Website timezone - UTC = %d seconds" % self.DELTA_UTC)

		if not os.path.exists(self.CONF_CACHEDIR):
			self.log("Creating \'%s\' directory for caching" % self.CONF_CACHEDIR)
			os.mkdir(self.CONF_CACHEDIR)

		# reading [channels] section
		temp = config.items("channels")

		# create a dictionary (Python array) with index = channel ID
		for i in temp:
			self.CHANNELLIST[i[0].strip(' \n\r').lower()] = unicode(i[1].strip(' \n\r').lower(), 'utf-8')

		if len(self.CHANNELLIST) == 0:
			self.log("ERROR: [channels] section empty ?", 1)
			sys.exit(1)

		# set network socket timeout
		socket.setdefaulttimeout(self.CONF_SOCKET_TIMEOUT)

		self.TODAYMP = time.strftime("%Y/%m/%d")
		# create a list filled with dates (format AAAA/MM/DD) from today to today+ MAX_DAY_EPG
		self.DAYCACHEMP = [self.TODAYMP]
		for day in range(1, self.CONF_MAX_DAY_EPG):
			self.DAYCACHEMP.append(time.strftime("%Y/%m/%d", time.localtime(time.time() + 86400 * day)))



# ----------------------------------------------------------------------


	def download_and_cache(self):
		self.log("--- START DOWNLOAD AND CACHE DATA ---")
		self.log2video("STARTING DOWNLOAD")

		self.log("Removing old cached files")
		scriptlib.cleanup_oldcachedfiles(self.CONF_CACHEDIR, self.FIELD_SEPARATOR)

		chlist = self.CHANNELLIST

		self.log("Start download XML data from \'" + self.CONF_URL + "\'")
		self.log2video("downloading XML data ...")

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

				self.log("\'" + self.CONF_URL + "\' connection error. Reason: " + serr + ". Waiting " + str(self.HTTP_ERROR_WAIT_RETRY) + " sec. and retry [" + str(i) + "] ...")
				time.sleep(self.HTTP_ERROR_WAIT_RETRY) # add sleep
				i -= 1

			else:
				i = -99
				sock.close()

		if (i != -99):
			self.log("Cannot retrieve data from \'" + self.CONF_URL + "\'. Abort script")
			self.log2video("Error: cannot download XML data, abort")
			time.sleep(5)
			sys.exit(1)

		self.log("End download XML data, now processing XML code.")
		self.log2video("preprocessing XML data, wait ...")
		try:
			xmldoc = minidom.parseString(data)
		except:
			self.log("Warning ! Data are not in a valid XML format. Abort script")
			self.log2video("Error: no valid XML data, abort")
			time.sleep(5)
			sys.exit(1)


		self.log("End process XML data")
		self.log2video("end process XML data")

		# days list
		xmlref_giorno = xmldoc.getElementsByTagName('giorno')
		for xml_gg in xmlref_giorno:
			gg = xml_gg.attributes["data"].value
			if gg not in self.DAYCACHEMP:
				continue

			xmlref_canale = xml_gg.getElementsByTagName('canale')
			for xml_ch in xmlref_canale:
				chid = xml_ch.attributes["id"].value.strip(' \n\r').lower()
				if not chlist.has_key(chid):
						self.log("Warning: new channel \"id=%s name=%s\" found in XML data" % (xml_ch.attributes["id"].value, xml_ch.attributes["description"]))
						continue

				clist = [chid]
				if self.CHANNELLIST.has_key(chid + '+1'):
					clist.append(chid + '+1')

				for c in clist:

					# get cache option
					#  0 : don't download/cache
					#  1 : download and cache (optional 1,new_name )
					#  2 : always download overwriting existing files (optional 2,new_name )
					#  3 : always download overwriting existing files only for TODAY (optional 3,new_name )

					cacheopt = int(chlist[c].split(",")[0])

					# if cacheopt == 0, do nothing
					if cacheopt == 0:
						continue

					channel_name = ''
					if len(chlist[c].split(",")) > 1:
						if chlist[c].split(",")[1] != '':
							# channel renamed, new name provided by user
							channel_name = chlist[c].split(",")[1].strip(' \n\r').lower()

					# if channel name is not present as option, quit with error
					if channel_name == '':
						self.log("ERROR ! ID=%s channel name not present" % c)
						sys.exit(1)

					channel_provider = self.CONF_DEFAULT_PROVIDER
					if len(chlist[c].split(",")) > 2:
						if chlist[c].split(",")[2] != '':
							channel_provider = chlist[c].split(",")[2].strip(' \n\r').lower()

					# if channel name is not present as option in channel_list.conf , quit with error
					if channel_name == '':
						self.log("ERROR ! ID=" + str(c) + " channel name not present. Skip !")
						continue

					# download only if file doesn't exist or cacheopt == 2 (always download),
					# using open(...,"w") files will be overwritten (saving a delete + create)

					day = str(self.convert_daymp(gg))
					eventfilename = scriptlib.fn_escape(str(c) + self.FIELD_SEPARATOR + channel_name + self.FIELD_SEPARATOR + day)
					eventfilepath = os.path.join(self.CONF_CACHEDIR, eventfilename)
					if (cacheopt == 1) and os.path.exists(eventfilepath):
						continue
					if (cacheopt == 3) and os.path.exists(eventfilepath) and (gg != self.TODAYMP):
						continue
					if (cacheopt != 1) and (cacheopt != 2) and (cacheopt != 3):
						self.log("Warning: unknown cache option " + str(cacheopt))
						exit_for_loop = True
						continue

					num_events = 0
					self.log("  Writing in cache \'" + eventfilename + "\'", 2)
					self.log2video(" extracting \"%s\" [%d] (%s)" % (channel_name, num_events, day))

					fd = codecs.open(eventfilepath, "w", 'utf-8')

					fd.write(str(c) + self.FIELD_SEPARATOR + channel_name + self.FIELD_SEPARATOR + channel_provider + self.FIELD_SEPARATOR + day + '\n')
					fd.write("Local Time (human readeable)###Unix GMT Time###Event Title###Event Description\n")

					xmlref_events = xml_ch.getElementsByTagName('prg')
					for xml_ee in xmlref_events:
						orainiz = xml_ee.attributes["orainizio"].value

						if (orainiz >= '00:00') and (orainiz <= '05:59'):
							nextdayevent = 86400
						else:
							nextdayevent = 0

						event_starttime = gg + " " + orainiz

						if c == (chid + '+1'):
							# manage channel "+1"
							event_startime_unix_gmt = str(int(time.mktime(time.strptime(event_starttime, "%Y/%m/%d %H:%M"))) - self.DELTA_UTC + 3600 + nextdayevent)
						else:
							# normal channel, not "+1"
							event_startime_unix_gmt = str(int(time.mktime(time.strptime(event_starttime, "%Y/%m/%d %H:%M"))) - self.DELTA_UTC + nextdayevent)


						event_title = unicode(xml_ee.getElementsByTagName('titolo')[0].firstChild.data)
						event_title = event_title.replace('\r', '')
						event_title = event_title.replace('\n', '')
						event_title = event_title.strip(u' ')

						event_description = ''
						if self.CONF_DL_DESC == 1:
							url_desc = xml_ee.getElementsByTagName('linkScheda')[0].firstChild.data
							event_description = unicode(self.get_description(url_desc.strip(' \n\r'))[:self.CONF_DLDESCMAXCHAR])
							event_description = event_description.replace('\r', '')
							event_description = event_description.replace('\n', u' ')
							event_description = event_description.strip(u' ')

						fd.write(event_starttime + self.FIELD_SEPARATOR + event_startime_unix_gmt + self.FIELD_SEPARATOR + event_title + self.FIELD_SEPARATOR + event_description + '\n')
						num_events += 1
						self.log2video(" extracting \"%s\" [%d] (%s)" % (channel_name, num_events, day))


					fd.close()

		del xmldoc

# ----------------------------------------------------------------------


	def process_cache(self):
		self.log("--- START PROCESSING CACHE ---")
		self.log2video("START PROCESSING CACHE")
		if not os.path.exists(self.CONF_CACHEDIR):
			self.log("ERROR: %s not present" % self.CONF_CACHEDIR, 1)
			sys.exit(1)

		self.log("Loading lamedb")
		lamedb = scriptlib.lamedb_class()

		self.log("Initialize CrossEPG database")
		crossdb = scriptlib.crossepg_db_class()
		crossdb.open_db()

		events = []
		previous_id = ''
		channels_name = ''
		total_events = 0

		self.log("Start data processing")
		filelist = sorted(os.listdir(self.CONF_CACHEDIR))
		filelist.append('***END***')

		for f in filelist:
			id = f.split(self.FIELD_SEPARATOR)[0]
			if previous_id == '':
				previous_id = id

			if id != previous_id:
				total_events += len(events)
				self.log("  ...processing \'%s\' , nr. events %d" % (previous_id, len(events)))
				self.log2video("processed %d events ..." % total_events)

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
				self.log("Reading  \'%s\'" % f)
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
		self.log("TOTAL EPG EVENTS PROCESSED: %d" % total_events)
		self.log("--- END ---")
		self.log2video("END , events processed: %d" % total_events)



# ****************************************************************************************************************************

# MAIN CODE: SCRIPT START HERE

# increase this process niceness (other processes have higher priority)
os.nice(10)

# set Garbage Collector to do a "generational jump" more frequently than default 700
# memory saving: about 50% (!!), some performance loss (obviously)
gc.set_threshold(50, 10, 10)

SCRIPT_DIR = 'scripts/mediaprem/'

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

