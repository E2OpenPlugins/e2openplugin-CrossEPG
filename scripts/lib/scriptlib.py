#!/usr/bin/python
# scriptlib.py  by Ambrosa http://www.ambrosa.net
# derived from E2_LOADEPG
# 22-Dec-2011

__author__ = "ambrosa http://www.ambrosa.net"
__copyright__ = "Copyright (C) 2008-2011 Alessandro Ambrosini"
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
	
	FDlog = None

	def __init__(self, fname=''):
		# get where CrossEPG save data (dbroot) and use it for opening crossepg.log
		dbroot = crossepg.epgdb_get_dbroot()
		if dbroot != False:
			if fname != '' :
				self.FDlog = open(dbroot+'/'+fname,'w')
			else :
				crossepg.log_open(dbroot)
		else:
			print "[scriptlib] WARNING: cannot open crossepg dbroot. Log not initialized !!"
			

	def log(self,s):
		if self.FDlog != None :
			self.FDlog.write("%s %s\n" % (time.strftime("%d/%m/%Y %H:%M:%S"), s) )
		else:
			crossepg.log_add(str(s))

	def log2video_status(self,s):
		print("LOGTEXT %s" % s)
		sys.stdout.flush()
		
	def log2video_pbar_on(self):
		print("PROGRESS ON")
		sys.stdout.flush()
		
	def log2video_pbar_off(self):
		print("PROGRESS OFF")
		sys.stdout.flush()

	def log2video_pbar(self,i):
		print("PROGRESS %d" % i)
		sys.stdout.flush()
		
	def log2video_scriptname(self,s):
		print("TYPE RUNNING CSCRIPT %s" % s)
		sys.stdout.flush()

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

	# initialize an empty dictionary (Python array) indexed by channel name
	# format: { channel_name : [ (sid , provider) , (sid , provider) , .... ] }
	INDEXBYCHNAME = True
	lamedb_dict = {}

	# lamedb indexed by provider name
	# format: { provider_name : [ (sid , channel_name) , (sid , channel_name) , .... ] }
	INDEXBYPROVID = False # if True, also create the array lamedb_dict_prov, usually false for saving memory
	lamedb_provid_dict = {}

	def __init__(self, index_by_chname = True, index_by_provid = False):
		self.INDEXBYCHNAME = index_by_chname
		self.INDEXBYPROVID = index_by_provid
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

			temp = temp.strip(' \n\r')
			if temp == u"end":
				# next line should be "services"
				temp = self.decode_charset(fd.readline())
				temp = temp.strip(' \n\r')
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

			sid = sid.strip(' \n\r')

			if sid == u'end':
				# reached end of channel section, end loop
				break;

			channel_name = self.decode_charset(fd.readline()) # read channel name, this is the second line

			channel_name = channel_name.strip(' \n\r').lower() # force channel name lowercase

			temp = self.decode_charset(fd.readline()) # read provider , this is the third line
			temp = temp.strip(' \n\r').lower()

			temp_P = temp.find('p:')
			if temp_P == -1 :
				print("ERROR parsing lamedb, channel_name section: provider name \'p:\' not present")
				sys.exit(1)
			else:
				temp = temp[(temp_P + 2):]
				temp = temp.split(',')[0]
				temp = temp.strip(' \n\r')
				if temp == '':
					provider_name = u'noprovider'
				else:
					provider_name = temp.lower()

			#channel_name=channel_name.encode('utf-8')
			#provider_name=provider_name.encode('utf-8')

			if self.INDEXBYCHNAME == True:
				sp = (sid,provider_name)
				if channel_name != '':
					if self.lamedb_dict.has_key(channel_name):
						self.lamedb_dict[channel_name].append(sp)
					else:
						self.lamedb_dict[channel_name]=[sp]

			if self.INDEXBYPROVID == True:
				sp = (sid,channel_name)
				if self.lamedb_provid_dict.has_key(provider_name):
					self.lamedb_provid_dict[provider_name].append(sp)
				else:
					self.lamedb_provid_dict[provider_name]=[sp]

				

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


	def get_provid_byname(self,channel_name):
		provid_list = []

		if self.lamedb_dict.has_key(channel_name) :
			for v in self.lamedb_dict[channel_name]:
				# (sid,provider_name)
				provid_list.append(v[1])

		return(provid_list)

	def get_sidprovid_byname(self,channel_name):
		sidprov_list = []
		if self.lamedb_dict.has_key(channel_name) :
			# (sid,provider_name)
			sidprov_list = self.lamedb_dict[channel_name]

		return(sidprov_list)


	def get_chnames_byprov(self,provider_name):
		if self.INDEXBYPROVID == True:
			if self.lamedb_provid_dict.has_key(provider_name) :
				return self.lamedb_provid_dict[provider_name]
			else:
				return None
		return None

	def convert_sid(self,sid):
		s=[]

		# SID:ns:TSID:ONID:stype:unused

		try:
			tmp = sid.split(":")
			s.append(int(tmp[0],0x10))  # SID
			s.append(int(tmp[2],0X10))  # TSID
			s.append(int(tmp[3],0X10))  # ONID
		except:
			pass

		return(s)


class crossepg_db_class:

	db_channel_ref = ''
	event_id = 1

	title_ref = ''

	def __init__(self):
		pass

	def open_db(self):
		# get where CrossEPG save data (dbroot)
		dbroot = crossepg.epgdb_get_dbroot()
		# open CrossEPG database
		if not crossepg.epgdb_open(dbroot):
			print("ERROR opening CrossEPG database")
			sys.exit(1)

		# load database structures (index, ....)
		crossepg.epgdb_load()

	def close_db(self):
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
	def add_event(self, start_time, duration, title=' ', summarie=' ', language='eng', utf8=False):
		start_time = int(start_time)
		duration = int(duration)

		if (duration < 0) or (duration > 65535) :
			# duration must be >= 0 or < 65536 , skip this event (it's an error)
			print("DEBUG: length error %d" % duration)
			return

		event_ref = crossepg.epgdb_title_alloc() # alloc title structure in memory		
		event_ref.event_id = self.event_id  # event_id is unique inside a channel
		self.event_id += 1

		event_ref.start_time = start_time	# Unix timestamp, always referred to gmt+0 without daylight saving
		event_ref.mjd = crossepg.epgdb_calculate_mjd(event_ref.start_time)	# Modified Julian Date. if you don't know it you can calulate it with epgdb_calculate_mjd()
	
		# print("       title %s , starttime %s , duration %f" % (title, start_time, duration))
		event_ref.length = duration  # event duration in seconds

		# ISO 639 language code. http://en.wikipedia.org/wiki/ISO_639
		event_ref.iso_639_1 = ord(language[0:1])
		event_ref.iso_639_2 = ord(language[1:2])
		event_ref.iso_639_3 = ord(language[2:3])

		# add event in epgdb and return back a reference to the structure
		# remember to use always the new structure reference
		# if the event already exist epgdb update it and automatically destroy the new structure
		event_ref = crossepg.epgdb_titles_add(self.db_channel_ref, event_ref)


		#print("DEBUG , title DATA TYPE: \'%s\'" % type(title).__name__ )
		#print("DEBUG , summarie DATA TYPE: \'%s\'" % type(summarie).__name__ )

		if utf8 == False :
			crossepg.epgdb_titles_set_description(event_ref, title);
			crossepg.epgdb_titles_set_long_description(event_ref, summarie);
		else:
			crossepg.epgdb_titles_set_description_utf8(event_ref, title);
			crossepg.epgdb_titles_set_long_description_utf8(event_ref, summarie);


