#!/usr/bin/python
# alias.py  by Ambrosa http://www.ambrosa.net
# this module is used for copy epg from a channel to another
# 22-Dec-2011

__author__ = "ambrosa http://www.ambrosa.net"
__copyright__ = "Copyright (C) 2008-2011 Alessandro Ambrosini"
__license__ = "CreativeCommons by-nc-sa http://creativecommons.org/licenses/by-nc-sa/3.0/"

import os
import sys
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

	# log text
	CONF_LOG_SCRIPT_NAME = "ALIAS EPG"


	def __init__(self,confdir,dbroot):

		# initialize logging
		self.log = scriptlib.logging_class()
		# write to video OSD the script name
		self.log.log2video_scriptname(self.CONF_LOG_SCRIPT_NAME)


		CONF_FILE = os.path.join(confdir,self.CONF_CONFIGFILENAME)
		if not os.path.exists(CONF_FILE) :
			self.log.log("ERROR: %s not present" % CONF_FILE)
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
			self.log.log("ERROR: [aliases] section empty ?")
			sys.exit(1)


# ----------------------------------------------------------------------


	def do_epg_alias(self):
		self.log.log("--- START PROCESSING ---")
		self.log.log("Loading lamedb indexed by channel name")
		self.log.log2video_status("start copying EPG from channel to another")
		lamedb = scriptlib.lamedb_class()

		self.log.log("Initialize CrossEPG database")
		crossdb = scriptlib.crossepg_db_class()
		crossdb.open_db()

		total_events = 0

		pbar_maxvalue = 100.0 / (len(self.CHANNELLIST) + 1.0)
		pbar_index = 1
		self.log.log2video_pbar_on()
		self.log.log2video_pbar(0)


		for src_channel in self.CHANNELLIST :
			self.log.log2video_pbar( int(pbar_index * pbar_maxvalue) )
			pbar_index += 1

			dst_channels = self.CHANNELLIST[src_channel].split(',')

			if src_channel.count('-') == 1 :
				src_chname = src_channel.split('-')[0].strip(' \n\r').lower()
				src_provname = src_channel.split('-')[1].strip(' \n\r').lower()

				src_sidbyname = []
				src_sidprovidbyname = lamedb.get_sidprovid_byname(src_chname)
				for p in src_sidprovidbyname:
					if p[1] == src_provname :
						# a channel can have zero or more SID (different channel with same name)
						# return the list [ 0e1f:00820000:0708:00c8:1:0 , 1d20:00820000:2fa8:013e:1:0 , ..... ]
						# return [] if channel name is not in lamedb
						src_sidbyname = p[0]
						ch_sid = lamedb.convert_sid(src_sidbyname)
						if len(ch_sid) == 0 :
							self.log.log("SID \"%s\" invalid, try next" % src_sidbyname)
							continue

						src_epgdb_channel = crossepg.epgdb_channels_get_by_freq(ch_sid[2],ch_sid[1],ch_sid[0]);
						if not src_epgdb_channel :
							self.log.log('Source channel "%s" with SID "%s" has not entry in epgdb, try next' % (src_channel,src_sidbyname) )
							continue
						else:
							break

				# if not sid then exit, go ahead with next src_channel
				if len(src_sidbyname) == 0:
					self.log.log('Source channel "%s" has not SID in lamedb, skip it' % src_channel)
					continue

			elif src_channel.count('-') == 2 :
				tmp = src_channel.split('-')
				src_sidbyname = "%s:xxxxxxxx:%s:%s:x:x" % (tmp[0].strip(' \n\r'),tmp[1].strip(' \n\r'),tmp[2].strip(' \n\r'))

			else:
				self.log.log("Channel source \"%s\" invalid" % src_channel)
				continue

			# convert "0e1f:00820000:0708:00c8:1:0" to sid,tsid,onid
			# return the list [sid,tsid,onid]
			src_sid = lamedb.convert_sid(src_sidbyname)
			if len(src_sid) == 0 :
				self.log.log("SID \"%s\" invalid, try next" % src_sidbyname)
				continue

			src_epgdb_channel = crossepg.epgdb_channels_get_by_freq(src_sid[2],src_sid[1],src_sid[0]);
			if not src_epgdb_channel :
				self.log.log('Source channel "%s" with SID "%s" has not entry in epgdb, skip it' % (src_channel,src_sidbyname) )
				continue

			self.log.log('Source channel "%s" with SID "%s" found in epgdb, using it' % (src_channel,src_sidbyname) )

			for dst in dst_channels:
				dst = dst.strip(' \n\r').lower()
				dst_sidbyname = lamedb.get_sid_byname(dst)
				if len(dst_sidbyname) == 0:
					self.log.log('   dest. channel "%s" has not SID in lamedb, skip it' % dst)
					continue

				for dsid in dst_sidbyname:

					# convert "0e1f:00820000:0708:00c8:1:0" to sid,tsid,onid
					# return the list [sid,tsid,onid]
					dst_sid = lamedb.convert_sid(dsid)

					if dst_sid == src_sid:
						# skip if source=destination
						self.log.log('   dest. channel "%s" is eq. to source, skip it' % dst)
						continue

					num_events = 0
					self.log.log('   copying EPG data from "%s" to "%s" sid "%s")' % (src_channel,dst,dsid) )
					self.log.log2video_status("copy %s -> %s (%d/%d)" % (src_channel,dst,num_events,total_events))


					# add channel into db
					# doesn't matter if the channel already exist... epgdb do all the work
					# this make a reference to the dest. channel
					crossdb.add_channel(dst_sid)

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

						if (num_events % 25) == 0:
							self.log.log2video_status("copy %s -> %s (%d/%d)" % (src_channel,dst,num_events,total_events))


					total_events += num_events
					self.log.log("   copied %d events" % num_events)


		self.log.log2video_pbar(0)
		self.log.log2video_pbar_off()

		self.log.log2video_status("END, copied %d events" % (total_events))
		# end process, close CrossEPG DB saving data
		crossdb.close_db()
		self.log.log("Copied %d events" % total_events)
		self.log.log("--- END ---")



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
script_class.do_epg_alias()


