#!/usr/bin/python
# alias.py  by Ambrosa http://www.ambrosa.net
# this module is used for copy epg from a channel to another

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


class main:

	# main config file
	CONF_CONFIGFILENAME = "alias.conf"

	# log file
	CONF_LOGFILENAME = "log.txt"

	# charset used in remote website epg data
	REMOTE_EPG_CHARSET = 'utf-8'


	def log(self,s):
		crossepg.log_add(str(s))
		self.logging.log2file(str(s))


	def __init__(self,confdir,dbroot):

		self.CROSSEPG_DBROOT = dbroot

		LOG_FILE = os.path.join(confdir, self.CONF_LOGFILENAME)
		self.logging = scriptlib.logging_class(LOG_FILE)

		CONF_FILE = os.path.join(confdir,self.CONF_CONFIGFILENAME)
		if not os.path.exists(CONF_FILE) :
			self.log("ERROR: %s not present" % CONF_FILE)
			sys.exit(1)

		config = ConfigParser.ConfigParser()
		#config.optionxform = str  # needed to return case sensitive index
		config.read(CONF_FILE)

		# reading [aliases] section
		temp=config.items("aliases");

		self.CHANNELLIST = {}
		# create a dictionary (Python array) with index = channel ID
		for i in temp:
			self.CHANNELLIST[i[0]] = unicode(i[1],'utf-8')

		if len(self.CHANNELLIST) == 0 :
			self.log("ERROR: [aliases] section empty ?")
			sys.exit(1)


# ----------------------------------------------------------------------


	def copy_epg(self):
		self.log("--- START PROCESSING ---")
		self.log("Loading lamedb")
		lamedb = scriptlib.lamedb_class()

		self.log("Initialize CrossEPG database")
		crossdb = scriptlib.crossepg_db_class()
		crossdb.open_db(self.CROSSEPG_DBROOT)

		total_events = 0

		for src_channel in self.CHANNELLIST :

			dst_channels = self.CHANNELLIST[src_channel].split('|')

			# a channel can have zero or more SID (different channel with same name)
			# return the list [0e1f:00820000:0708:00c8:1:0 , 1d20:00820000:2fa8:013e:1:0 , ..... ]
			# return [] if channel name is not in lamedb
			src_sidbyname = lamedb.get_sid_byname(src_channel.strip(' \n').lower())

			# if not sid then exit, go ahead with next src_channel
			if len(src_sidbyname) == 0:
				self.log('Source channel "%s" has not SID in lamedb, skip it' % src_channel)
				continue

			for s in src_sidbyname:
				# convert "0e1f:00820000:0708:00c8:1:0" to sid,tsid,onid
				# return the list [sid,tsid,onid]
				ch_sid = lamedb.convert_sid(s)

				src_epgdb_channel = crossepg.epgdb_channels_get_by_freq(ch_sid[2],ch_sid[1],ch_sid[0]);
				if not src_epgdb_channel :
					self.log('Source channel "%s" with SID "%s" has not entry in epgdb, skip it' % (src_channel,s) )
					continue

				self.log('Source channel "%s" with SID "%s" found in epgdb, using it' % (src_channel,s) )

				for dst in dst_channels:
					dst = dst.strip(' \n').lower()
					dst_sidbyname = lamedb.get_sid_byname(dst)
					if len(dst_sidbyname) == 0:
						self.log('   dest. channel "%s" has not SID in lamedb, skip it' % dst)
						continue

					for dsid in dst_sidbyname:
						self.log('   copying EPG data from "%s" to "%s" sid "%s")' % (src_channel,dst,dsid) )

						# convert "0e1f:00820000:0708:00c8:1:0" to sid,tsid,onid
						# return the list [sid,tsid,onid]
						d_3sid = lamedb.convert_sid(dsid)

						# add channel into db
						# doesn't matter if the channel already exist... epgdb do all the work
						# this make a reference to the dest. channel
						crossdb.add_channel(d_3sid)

						num_events = 0
						title = src_epgdb_channel.title_first;
						while (title != None) :
							#print str(title.start_time)
							#print str(title.length)
							#print str(crossepg.epgdb_read_description(title))
							#print str(crossepg.epgdb_read_long_description(title))
							#print "-----------------------------------"

							e_starttime = int(title.start_time)
							e_length = int(title.length)
							e_title = crossepg.epgdb_read_description(title).encode('utf-8')
							e_summarie = crossepg.epgdb_read_long_description(title).encode('utf-8')
							e_countrycode = "%c%c%c" % (title.iso_639_1, title.iso_639_2, title.iso_639_3)

							# add_event(start_time , duration , title , summarie , ISO639_language_code , strings_encoded_with_UTF-8)
							crossdb.add_event(e_starttime, e_length, e_title, e_summarie, e_countrycode, True )
							num_events += 1

							title = title.next

						total_events += num_events

						self.log("   copied %d events" % num_events)

			# end "for s in src_sidbyname"
			break

		# end process, close CrossEPG DB saving data
		crossdb.close_db()
		self.log("Copied %d events" % total_events)
		self.log("--- END ---")



# ****************************************************************************************************************************

# MAIN CODE: SCRIPT START HERE

SCRIPT_DIR = 'scripts/alias/'

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

# copy epg
script_class.copy_epg()


