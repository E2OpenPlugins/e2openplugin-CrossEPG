#!/usr/bin/python
# stuff.py  by Ambrosa http://www.ambrosa.net
# derived from E2_LOADEPG

__author__ = "ambrosa http://www.ambrosa.net"
__copyright__ = "Copyright (C) 2008-2010 Alessandro Ambrosini"
__license__ = "CreativeCommons by-nc-sa http://creativecommons.org/licenses/by-nc-sa/3.0/"


import os
import sys
import time
import codecs
import crossepg

# escape some incorrect chars from filename
def fn_escape(s):
	if type(s).__name__ == 'str':
		s = s.decode('utf-8')

	s = s.replace(' ','_')
	s = s.replace('/','_')
	s = s.replace(':','_')
	s = s.replace('.','_')
	s = s.replace('|','_')
	s = s.replace('!','_')

	return(s.encode('utf-8'))

# logging class
class logging_class:
	LOG_FD=0

	def __init__(self,logfilename):
		self.LOG_FD=codecs.open(logfilename,"w",'utf-8')

	def log2file(self,s):
		st = time.strftime("%d/%m/%Y %H:%M:%S")
		self.LOG_FD.write(st + ' ' + s + '\n')



# decompress gzipped data
class zlib_class:
	GZTMP_FILE = "gunzip_temp.gz"
	UNGZTMP_FILE = "gunzip_temp"
	BIN_GZUNZIP = "gunzip -c " + GZTMP_FILE

	def gzuncompress(self,data):
		fd = open(self.GZTMP_FILE,'w')
		fd.write(data)
		fd.close()

		fd = os.popen(self.BIN_GZUNZIP)
		data_ungz = fd.read()
		fd.close()
		os.unlink(self.GZTMP_FILE)
		return(data_ungz)


# removing old cached epg files **
def cleanup_oldcachedfiles(cachedir, field_separator):
	TODAY = time.strftime("%Y%m%d")

	for cachedfile in os.listdir(cachedir):
		# extract date from filename
		if cachedfile.split(field_separator)[-1] < TODAY :
			os.unlink(os.path.join(cachedir,cachedfile))


# return LOCALTIME - GMTIME (with DST)
# return negative number if timezone is east of GMT (like Italy)
# return postive number if timezone is west of GMT (like USA)
def delta_utc():
	if time.localtime().tm_isdst == 0 :
		# return localtime - gmtime (in seconds)
		return time.timezone
	else:
		# return (localtime - gmtime - DST)
		return time.altzone


# return DST time difference (in seconds)
def delta_dst():
	if time.localtime().tm_isdst == 0 :
		return 0
	else:
		# return DST difference
		return abs(time.altzone - time.timezone)


# manage channel list from lamedb
class lamedb_class:

	LAMEDB='/etc/enigma2/lamedb'

	# initialize an empty dictionary (Python array)
	# the following format can handle duplicated channel name
	# format: { channel_name : [ (sid , provider) , (sid , provider) , .... ] }
	lamedb_dict={}

	def __init__(self):
		self.read_lamedb()

	# first of all try to decode a string using UTF-8, if it fails then try with ISO-8859-1
	# always return an Unicode string
	def decode_charset(self,s):
		u = None
		charset_list = ('utf-8','iso-8859-1','iso-8859-2','iso-8859-15')

		for charset in charset_list:
			try:
				u = unicode(s,charset,"strict")
			except:
				pass
			else:
				break

		if u == None:
			print("CHARSET ERROR while decoding lamedb")
			sys.exit(1)
		else:
			return(u)


	def read_lamedb(self):
		if not os.path.exists(self.LAMEDB):
			print("ERROR ! \'%s\' NOT FOUND" % self.LAMEDB)
			sys.exit(1)

		# lamedb mix UTF-8 + iso-8859-* inside it
		# need charset decoding line by line
		fd = open(self.LAMEDB,"r")

		# skip transponder section
		# read lamedb until are found "end" and "services" lines
		while True:
			temp = self.decode_charset(fd.readline())
			if temp == '' :
				print("ERROR parsing lamedb, transponder section: end of file")
				sys.exit(1)

			temp = temp.strip(' \n')
			if temp == u"end":
				# next line should be "services"
				temp = self.decode_charset(fd.readline())
				temp = temp.strip(' \n')
				if temp == u'services':
					# reached end of transponder section, end loop and continue with parsing channel section
					break
				else:
					print("ERROR parsing lamedb, transponder section: not found \"end + services\" lines")
					sys.exit(1)

		# parsing lamedb channel section
		while True:
			sid = self.decode_charset(fd.readline()) # read SID , it's the first line

			if sid == '' :
				print("ERROR parsing lamedb, channel_name section: end of file")
				sys.exit(1)

			sid = sid.strip(' \n')

			if sid == u'end':
				# reached end of channel section, end loop
				break;

			channel_name = self.decode_charset(fd.readline()) # read channel name, this is the second line

			channel_name = channel_name.strip(' \n').lower() # force channel name lowercase

			temp = self.decode_charset(fd.readline()) # read provider , this is the third line
			temp = temp.strip(' \n').lower()

			temp_P = temp.find('p:')
			if temp_P == -1 :
				print("ERROR parsing lamedb, channel_name section: provider name \'p:\' not present")
				sys.exit(1)
			else:
				temp = temp[(temp_P + 2):]
				temp = temp.split(',')[0]
				if temp == '':
					provider_name = u'noprovider'
				else:
					provider_name = temp.strip(' ').lower()

			#channel_name=channel_name.encode('utf-8')
			#provider_name=provider_name.encode('utf-8')

			# if name not empty, add sid to dictionary
			sp = (sid,provider_name)
			if channel_name != '':
				if self.lamedb_dict.has_key(channel_name):
					self.lamedb_dict[channel_name].append(sp)
				else:
					self.lamedb_dict[channel_name]=[sp]

		fd.close()

		if len(self.lamedb_dict) == 0 :
			print("ERROR lamedb empty ?")
			sys.exit(1)


	def get_sid_byname(self,channel_name):
		sid_list = []

		if self.lamedb_dict.has_key(channel_name) :
			for v in self.lamedb_dict[channel_name]:
				# (sid,provider_name)
				sid_list.append(v[0])

		return(sid_list)


	def convert_sid(self,sid):
		s=[]

		# SID:ns:TSID:ONID:stype:unused
		tmp = sid.split(":")
		s.append(int(tmp[0],0x10))  # SID
		s.append(int(tmp[2],16))    # TSID
		s.append(int(tmp[3],16))    # ONID

		return(s)


class crossepg_db_class:

	db_channel_ref = ''
	event_id = 1

	title_ref = ''

	def __init__(self):
		pass

	def open_db(self,dbroot):
		# open database
		if not crossepg.epgdb_open(dbroot):
			print("ERROR opening CrossEPG database")
			sys.exit(1)

		# load database structures (hash, ....)
		crossepg.epgdb_load()

	def close_db(self,dbroot):
		# save data
		if crossepg.epgdb_save(None):
			print("CrossEPG data saved")
		else:
			print("CrossEPG Error saving data")

		# close epgdb and clean memory
		crossepg.epgdb_close()
		crossepg.epgdb_clean()


	# add channel into db and get a reference to the structure
	# doesn't matter if the channel already exist... epgdb do all the work
	def add_channel(self,ch_sid):
		# epgdb_channels_add(onid, tsid, sid)
		self.db_channel_ref = crossepg.epgdb_channels_add(ch_sid[2], ch_sid[1], ch_sid[0])
		self.event_id = 1

	# add an EPG event
	def add_event(self, start_time, duration, title=' ', summarie=' ', language='eng', utf8=False ):
		event_ref = crossepg.epgdb_title_alloc() # alloc title structure in memory
		event_ref.event_id = self.event_id  # event_id is unique inside a channel
		self.event_id += 1

		event_ref.start_time = start_time	# Unix timestamp, always referred to gmt+0 without daylight saving
		event_ref.mjd = crossepg.epgdb_calculate_mjd(event_ref.start_time)	# Modified Julian Date. if you don't know it you can calulate it with epgdb_calculate_mjd()
		event_ref.length = duration  # event duration in seconds

		# ISO 639 language code. http://en.wikipedia.org/wiki/ISO_639
		event_ref.iso_639_1 = ord(language[0:1])
		event_ref.iso_639_2 = ord(language[1:2])
		event_ref.iso_639_3 = ord(language[2:3])

		# add event in epgdb and return back a reference to the structure
		# remember to use always the new structure reference
		# if the event already exist epgdb update it and automatically destroy the new structure
		event_ref = crossepg.epgdb_titles_add(self.db_channel_ref, event_ref)

		# now we need to add title and summarie

		if utf8 == False:
			# TITLE
			crossepg.epgdb_titles_set_description(event_ref, title);
			# SUMMARIE
			crossepg.epgdb_titles_set_long_description(event_ref, summarie);
		else:
			# TITLE
			crossepg.epgdb_titles_set_description_unicode(event_ref, title);
			# SUMMARIE
			crossepg.epgdb_titles_set_long_description_unicode(event_ref, summarie);

