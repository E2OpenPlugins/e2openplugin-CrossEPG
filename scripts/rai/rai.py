#!/usr/bin/python
# rai.py  by Ambrosa http://www.ambrosa.net
# this module is used for download EPG data from Rai website
# derived from E2_LOADEPG
# 29-Dec-2011

__author__ = "ambrosa http://www.ambrosa.net"
__copyright__ = "Copyright (C) 2008-2011 Alessandro Ambrosini"
__license__ = "CreativeCommons by-nc-sa http://creativecommons.org/licenses/by-nc-sa/3.0/"

import os
import sys
import time
import codecs
import socket
import string
import random
import urllib2
import ConfigParser

# import CrossEPG functions
import crossepg

# location of local python modules under "scripts/lib" dir.
# add it to sys.path()
crossepg_instroot = crossepg.epgdb_get_installroot()
if crossepg_instroot == False:
	sys.exit(1)
libdir = os.path.join(crossepg_instroot , 'scripts/lib')
sys.path.append(libdir)

# import local modules
import sgmllib
import scriptlib

# =================================================================
# HTML PARSER

class Titolo_parser(sgmllib.SGMLParser):

	def parse(self, s):
		self.feed(s)
		self.close()

	def __init__(self, day_get, verbose=0):
		sgmllib.SGMLParser.__init__(self, verbose)
		self.daynow = day_get
		self.daynext = time.strftime("%Y%m%d",time.localtime(time.mktime(time.strptime(day_get,"%Y%m%d"))+86400))
		self.day = self.daynow
		self.guidatoday = []
		self.guidatomorrow = []
		self.sera = False
		self.tomorrow = False
		self.start_orario = False
		self.start_titolo = False
		self.inside_a_titolo = False
		self.inside_palinsesto = False


	def start_div(self,attributes):
		for name,value in attributes:
			if name == "class":
				if value == "intG":
					self.inside_palinsesto = True

	def start_span(self, attributes):
		if self.inside_palinsesto == True:
			for name, value in attributes:
				if name == "class":
					if value == "ora":
						self.start_orario = True
					if value == "info":
						self.start_titolo = True

	def start_a(self,attributes):
		if self.inside_palinsesto == True:
			if self.start_titolo == True:
				self.inside_a_titolo = True

	def handle_data(self, data):
		if self.inside_palinsesto == True:

			if self.start_orario == True:

				# if time < 06:00 is a next day event
				if int(time.strftime("%H",time.strptime(data,"%H:%M"))) < 6 :
					self.day = self.daynext
					self.tomorrow = True
				else:
					if self.tomorrow == True:
						self.inside_a_titolo = False
						self.start_titolo = False
						self.inside_palinsesto = False
						return

				self.dataoraevento = time.strftime("%Y-%m-%d %H:%M",time.strptime(self.day+'-'+data,"%Y%m%d-%H:%M"))
				self.start_orario = False

			if self.inside_a_titolo == True:
				if self.tomorrow == False:
					self.guidatoday.append((self.dataoraevento,data.strip()))
				else:
					self.guidatomorrow.append((self.dataoraevento,data.strip()))

				self.inside_a_titolo = False
				self.start_titolo = False
				self.inside_palinsesto = False


	def get_guida(self):
		return ((self.guidatoday,self.guidatomorrow))


# =================================================================


class main:

	# main config file
	CONF_CONFIGFILENAME = "rai.conf"

	# Network socket timeout (in seconds)
	CONF_SOCKET_TIMEOUT = 20

	# log text
	CONF_LOG_SCRIPT_NAME = "RAI (Italy)"

	# retry number if HTTP error
	HTTP_ERROR_RETRY = 3
	# seconds to wait between retries
	HTTP_ERROR_WAIT_RETRY = 5

	# random time delay (in seconds) between access to remote web pages
	CONF_RANDOM_MIN = 0.0
	CONF_RANDOM_MAX = 2.0

	# charset used in remote website epg data
	REMOTE_EPG_CHARSET = 'utf-8'

	TODAY = ''
	DAYCACHE = []
	FIELD_SEPARATOR = '###'
	CHANNELLIST = {}


	def __init__(self,confdir,dbroot):

		# initialize logging
		self.log = scriptlib.logging_class()
		# write to video OSD the script name
		self.log.log2video_scriptname(self.CONF_LOG_SCRIPT_NAME)
		
		self.log.log("=== RUNNING SCRIPT %s ===" % self.CONF_LOG_SCRIPT_NAME)

		CONF_FILE = os.path.join(confdir,self.CONF_CONFIGFILENAME)
		if not os.path.exists(CONF_FILE) :
			self.log.log("ERROR: %s not present" % CONF_FILE)
			sys.exit(1)

		config = ConfigParser.ConfigParser()
		config.optionxform = str  # needed to return case sensitive index
		config.read(CONF_FILE)

		# reading [global] section options
		self.CONF_DEFAULT_PROVIDER = config.get("global","DEFAULT_PROVIDER")
		# save cache under dbroot
		self.CONF_CACHEDIR = os.path.join(dbroot,config.get("global","CACHE_DIRNAME"))

		self.CONF_MAX_DAY_EPG = config.getint("global","MAX_DAY_EPG")
		self.CONF_URL = config.get("global","URL")

		self.CONF_GMT_ZONE = config.get("global","GMT_ZONE")
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
			self.log.log("Creating \'%s\' directory for caching" % self.CONF_CACHEDIR)
			os.mkdir(self.CONF_CACHEDIR)

		# reading [channels] section
		temp = config.items("channels")

		# create a dictionary (Python array) with index = channel ID
		for i in temp:
			self.CHANNELLIST[i[0]] = unicode(i[1],'utf-8')

		if len(self.CHANNELLIST) == 0 :
			self.log.log("ERROR: [channels] section empty ?")
			sys.exit(1)

		# set network socket timeout
		socket.setdefaulttimeout(self.CONF_SOCKET_TIMEOUT)

		# initialize random generator
		random.seed()

		# today date (format AAAAMMDD)
		self.TODAY = time.strftime("%Y%m%d")

		# create a list filled with dates (format AAAAMMDD) from today to today+MAX_DAY_EPG
		self.DAYCACHE=[self.TODAY]
		for day in range(1,self.CONF_MAX_DAY_EPG):
			self.DAYCACHE.append(time.strftime("%Y%m%d",time.localtime(time.time()+86400*day)))


# ----------------------------------------------------------------------


	def download_and_cache(self):
		self.log.log("--- START DOWNLOAD AND CACHE DATA ---")
		self.log.log2video_status("STARTING DOWNLOAD")

		self.log.log("Removing old cached files")
		scriptlib.cleanup_oldcachedfiles(self.CONF_CACHEDIR, self.FIELD_SEPARATOR)

		#self.log("Start downloading HTML data from \'%s\'" % self.CONF_URL)

		chlist = self.CHANNELLIST

		# get remote XML files
		#   chid format: channel id , 0|1|2(,new name)
		#   i.e. ("101" , "1,SkyCinema1")
		pbar_max = 0
		for c in chlist.keys():
			cacheopt = int(string.split(chlist[c],",")[0])
			if cacheopt == 1:
				pbar_max += 1
				
		pbar_max *= self.CONF_MAX_DAY_EPG
		pbar_max = 100.0 / pbar_max
			
		self.log.log2video_pbar_on()
		self.log.log2video_pbar(0)
		pbar_value = 0
		
		for c in sorted(chlist.keys()):
			self.guidatoday = []
			self.guidatomorrow = []

			# get cache option
			#  0 : don't download/cache
			#  1 : download and cache (optional 1,new_name )
			#  2 : always download overwriting existing files (optional 2,new_name )
			#  3 : always download overwriting existing files only for TODAY (optional 3,new_name )

			cacheopt = int(string.split(chlist[c],",")[0])

			# if cacheopt == 0, do nothing
			if cacheopt == 0:
				continue
		
			self.log.log2video_status("processing %s" % c)
			channel_name = ''
			if len(chlist[c].split(",")) > 1 :
				if chlist[c].split(",")[1] != '' :
					# channel renamed, new name provided by user
					channel_name = chlist[c].split(",")[1].strip(' ').lower()

			# if channel name is not present as option, quit with error
			if channel_name == '':
				self.log.log("ERROR ! ID=%s channel name not present" % c)
				sys.exit(1)

			channel_provider = self.CONF_DEFAULT_PROVIDER
			if len(chlist[c].split(",")) > 2 :
				if chlist[c].split(",")[2] != '' :
					channel_provider = chlist[c].split(",")[2].strip(' ').lower()

			exit_for_loop = False
			for day in self.DAYCACHE:
				if exit_for_loop == True:
					break
				
				pbar_value += 1
				self.log.log2video_pbar(pbar_value * pbar_max)


				day_get = time.strftime("%Y_%m_%d",time.strptime(day,"%Y%m%d"))
				xmlfile = "%s_%s" % (c,day_get)

				# download only if file doesn't exist or cacheopt == 2 (always download),
				# using open(...,"w") files will be overwritten (saving a delete + create)

				eventfilename = scriptlib.fn_escape(str(c) + self.FIELD_SEPARATOR + channel_name + self.FIELD_SEPARATOR + day)
				eventfilepath = os.path.join(self.CONF_CACHEDIR, eventfilename)
				if (cacheopt == 1) and os.path.exists(eventfilepath):
					continue
				if (cacheopt == 3) and os.path.exists(eventfilepath) and (day != self.TODAY):
					continue
				if (cacheopt != 1) and (cacheopt != 2) and (cacheopt != 3):
					self.log.log("Warning: unknown cache option %s" % cacheopt)
					exit_for_loop = True
					continue

				self.log.log("Download HTML data from \'%s?%s\'" % (self.CONF_URL,xmlfile))
				self.log.log2video_status("downloading %s" % xmlfile)

				i = self.HTTP_ERROR_RETRY
				while i > 0  :
					#  wait randomly to avoid overloading website
					time.sleep(random.uniform(self.CONF_RANDOM_MIN, self.CONF_RANDOM_MAX))

					try:
						sock=urllib2.urlopen(self.CONF_URL + '?' + xmlfile)
						data=sock.read()

					except IOError, e:
						serr="unknown"
						if hasattr(e, 'reason'):
							serr=str(e.reason)
						elif hasattr(e, 'code'):
							serr=str(e.code)
							if hasattr(e, 'msg'):
								serr+=" , "+str(e.msg)

						self.log.log("\'%s?%s\' connection error. Reason: %s. Waiting %d sec. and retry [%d] ..." % (self.CONF_URL,xmlfile, serr, self.HTTP_ERROR_WAIT_RETRY, i))
						time.sleep(self.HTTP_ERROR_WAIT_RETRY) # add sleep
						i -= 1

					else:
						i = 0 # force quit WHILE loop
						sock.close()

						dtparser = Titolo_parser(day)
						dtparser.parse(data)
						self.guida = self.guidatomorrow
						(self.guidatoday, self.guidatomorrow) = dtparser.get_guida()

						# if no data, quit for loop and stop downloading
						if len(self.guidatoday) == 0:
							exit_for_loop = True
							break

						self.guida = self.guida + self.guidatoday

						self.log.log("  writing in cache \'%s\'" % eventfilename)
						# write data in cache file using UTF-8 encoding
						fd = codecs.open(eventfilepath, "w", 'utf-8')
						fd.write(str(c) + self.FIELD_SEPARATOR + channel_name + self.FIELD_SEPARATOR + channel_provider + self.FIELD_SEPARATOR + day + '\n')
						fd.write("Local Time (human readeable)###Unix GMT Time###Event Title###Event Description\n")

						# extract all events and put in eventfile
						for event in self.guida:
							(dataora,titolo) = event
							event_starttime = dataora
							# time.mktime return Unix time inside GMT timezone
							event_startime_unix_gmt = str(int(time.mktime(time.strptime(event_starttime,"%Y-%m-%d %H:%M"))) - self.DELTA_UTC )
							#event_startime_unix_gmt = str(int(time.mktime(time.strptime(event_starttime,"%Y-%m-%d %H:%M")))  )
							#self.log(event_starttime + " , " + str(self.DELTA_UTC) + " , " + str(int(time.mktime(time.strptime(event_starttime,"%Y-%m-%d %H:%M")))) + " , " + event_startime_unix_gmt )

							# convert remote data (RAI website use UTF-8) in Python Unicode (UCS2)
							event_title = unicode(titolo,self.REMOTE_EPG_CHARSET)

							event_title = event_title.replace('\r','')
							event_title = event_title.replace('\n',u' ')
							event_title = event_title.strip(u' ')

							event_description = u''

							fd.write(event_starttime + self.FIELD_SEPARATOR + event_startime_unix_gmt + self.FIELD_SEPARATOR + event_title + self.FIELD_SEPARATOR + event_description + '\n')

						fd.close()
		
		self.log.log2video_pbar(0)
		self.log.log2video_pbar_off()



# ----------------------------------------------------------------------


	def process_cache(self):
		self.log.log("--- START PROCESSING CACHE ---")
		self.log.log2video_status("START PROCESSING CACHE")
		if not os.path.exists(self.CONF_CACHEDIR):
			self.log.log("ERROR: %s not present" % self.CONF_CACHEDIR)
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

		for f in filelist :
			id = f.split(self.FIELD_SEPARATOR)[0]
			if previous_id == '':
				previous_id = id

			if id != previous_id :
				total_events += len(events)
				self.log.log("  ...processing \'%s\' , nr. events %d" % (previous_id,len(events)))
				self.log.log2video_status("processed %d events ..." % total_events )

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

							e_starttime = int(e.split(self.FIELD_SEPARATOR)[1])

							if i < L :
								e_length = int(events[i+1].split(self.FIELD_SEPARATOR)[1]) - e_starttime
							else:
								# last event, dummy length 90 min.
								e_length = 5400
							i += 1

							# extract title and encode Python Unicode with UTF-8
							e_title = e.split(self.FIELD_SEPARATOR)[2].encode('utf-8')

							# RAI website HAVE NOT long description. (bleah !).
							e_summarie = u' '
							# encode Python Unicode in UTF-8
							e_summarie = e_summarie.encode('utf-8')

							# add_event(start_time , duration , title , summarie , ISO639_language_code , strings_encoded_with_UTF-8)
							crossdb.add_event(e_starttime, e_length, e_title, e_summarie, 'ita', True )

				if f == '***END***':
					break

				events = []
				previous_id = id
				channels_name = ''

			if id == previous_id:
				self.log.log("Reading  \'%s\'" % f)
				# read events from cache file using UTF-8 and insert them in events list
				fd = codecs.open(os.path.join(self.CONF_CACHEDIR, f),"r","utf-8")
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
		self.log.log("TOTAL EPG EVENTS PROCESSED: %d" % total_events)
		self.log.log("--- END ---")
		self.log.log2video_status("END, events processed: %d" % total_events)
		time.sleep(3)



# ****************************************************************************************************************************

# MAIN CODE: SCRIPT START HERE

SCRIPT_DIR = 'scripts/rai/'

# get CrossEPG installation dir.
crossepg_instroot = crossepg.epgdb_get_installroot()
if crossepg_instroot == False:
	sys.exit(1)
scriptlocation = os.path.join(crossepg_instroot , SCRIPT_DIR)

# get where CrossEPG save data (dbroot) and use it as script cache repository
crossepg_dbroot = crossepg.epgdb_get_dbroot()
if crossepg_dbroot == False:
	sys.exit(1)

# initialize script class
script_class = main(scriptlocation , crossepg_dbroot)

# download data and cache them
script_class.download_and_cache()

# read cached data and inject into CrossEPG database
script_class.process_cache()

