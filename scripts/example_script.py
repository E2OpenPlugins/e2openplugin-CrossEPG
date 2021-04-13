#!/usr/bin/python
# Author: Skaman
# Date: October 2010

from crossepg import *

def dump_title(title):
	log_add("----------EVENT----------")
	log_add("ID: %d" % title.event_id)
	log_add("MJD: %d" % title.mjd)
	log_add("START TIME: %d" % title.start_time)
	log_add("DURATION: %d" % title.length)
	log_add("GENRE ID: %d" % title.genre_id)
	log_add("REVISION: %d" % title.revision)
	log_add("ISO639: %c%c%c" % (title.iso_639_1, title.iso_639_2, title.iso_639_3))
	log_add("IS UTF8: %d" % IS_UTF8(title.flags))
	log_add("DESCRIPTION_CRC: 0x%x" % title.description_crc)
	log_add("DESCRIPTION_SEEK: %d" % title.description_seek)
	log_add("DESCRIPTION LENTH: %d" % title.description_length)
	log_add("DESCRIPTION: %s" % epgdb_read_description(title))
	log_add("LONG DESCRIPTION CRC: 0x%x" % title.long_description_crc)
	log_add("LONG DESCRIPTION SEEK: %d" % title.long_description_seek)
	log_add("LONG DESCRIPTION LENTH: %d" % title.long_description_length)
	log_add("LONG DESCRIPTION: %s" % epgdb_read_long_description(title))

def main():
	# get dbroot path
	dbroot = epgdb_get_dbroot()

	# open epgdb
	if epgdb_open(dbroot):
		log_add("EPGDB opened (root=%s)" % dbroot)
	else:
		log_add("Error opening EPGDB")
		epgdb_close()
		return

	# load indexes and hashes in memory
	epgdb_load()

	####################
	# WRITE OPERATIONS #
	####################

	# add an event into db
	log_add("TEST: add an event into db")
	nid = 0x01		# original network id
	tsid = 0x01		# transport service id
	sid = 0x01		# service id
					# the 3 values above identify the channel

	channel = epgdb_channels_add(nid, tsid, sid)	# add channel into db and get a reference to the structure
													# doesn't matter if the channel already exist... epgdb do all the work

	title = epgdb_title_alloc()							# alloc title structure in memory
	title.event_id = 1									# event id.. it's unique inside a channel
	title.start_time = 1293840000						# 1/1/2011 timestamp.. it's always referred to gmt+0 without daylight saving
	title.mjd = epgdb_calculate_mjd(title.start_time)	# Modified Julian Date.. if you don't know it you can calulate it with epgdb_calculate_mjd()
	title.length = 600									# ten minutes.. event duration in seconds
	title.iso_639_1 = ord('e')							# ISO 639 language code.. http://en.wikipedia.org/wiki/ISO_639
	title.iso_639_2 = ord('n')
	title.iso_639_3 = ord('g')

	title.flags = SET_UTF8(title.flags)		# necessary only if description and/or long description have utf-8 charset

	title = epgdb_titles_add(channel, title)	# add event in epgdb and return back a reference to the structure
												# remember to use always the new structure reference
												# if the event already exist epgdb update it and automatically destroy the new structure

	# now the event structure is already in epgdb
	# but we still need to add descriptions
	epgdb_titles_set_description (title, "our custom event short description")
	epgdb_titles_set_long_description (title, "our custom event long description")

	###################
	# READ OPERATIONS #
	###################

	# walk inside channels and events
	log_add("TEST: walk inside channels and events")
	channel_count = 0
	channel = epgdb_channels_get_first()
	while channel != None:
		log_add("---------CHANNEL---------")
		log_add("NID: 0x%x" % channel.nid)
		log_add("TSID: 0x%x" % channel.tsid)
		log_add("SID: 0x%x" % channel.sid)
		title_count = 0
		title = channel.title_first
		while title != None:
			dump_title(title)
			title = title.next
			title_count += 1
			if title_count == 2:		# it's only a test so we halt the loop after 2 titles
				break

		channel = channel.next
		channel_count += 1
		if channel_count == 3:		# it's only a test so we halt the loop after 3 channels
			break

	# get a channel by nid, tsid, sid
	log_add("TEST: get a channel by nid, tsid, sid")
	channel = epgdb_channels_get_by_freq(nid, tsid, sid)
	if channel:
		# get an event by channel and timestamp
		log_add("TEST: get an event by channel and timestamp")
		title = epgdb_titles_get_by_time (channel, 1293840000) # exist also the api epgdb_titles_get_by_id_and_mjd(channel, event_id, mjd_time)
		if title:
			dump_title(title)
		else:
			log_add("Title not found")
	else:
		log_add("Channel not found")

	# saving data
	# it's necessary only if you add new events
	if epgdb_save(None):
		log_add("Data saved")
	else:
		log_add("Error saving data")

	# close epgdb and clean memory
	epgdb_close()
	epgdb_clean()
	log_add("EPGDB closed")

main()
