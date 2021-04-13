#!/usr/bin/python
# above is Python interpreter location

# ambrosa 08-Jan-2011 http://www.ambrosa.net
# this test.py program simply do nothing


import os
import sys

# import CrossEPG module
import crossepg

# python local modules under "scripts/lib" directory
# and add it to sys.path()
crossepg_instroot = crossepg.epgdb_get_installroot()
if crossepg_instroot == False:
	print "ERROR: cannot find CrossEPG installation directory"
	sys.exit(1)
libdir = os.path.join(crossepg_instroot , 'scripts/lib')
sys.path.append(libdir)

# import local modules under 'scripts/lib'
import scriptlib

# -------------------------------------------------
# this is the main function
def main():

	# log_add() print to stdout a text message
	crossepg.log_add("---- START EXAMPLE TEST SCRIPT ----")

	# get installation dir
	instdir = crossepg.epgdb_get_installroot()
	if instdir == False:
		crossepg.log_add("ERROR: cannot find CrossEPG installation directory")
		sys.exit(1)

	crossepg.log_add("Installation dir : %s" % instdir)


	# get dbroot path
	dbroot = crossepg.epgdb_get_dbroot()
	if dbroot == False:
		crossepg.log_add("ERROR: cannot find CrossEPG database directory")
		sys.exit(1)

	crossepg.log_add("Database dir : %s" % dbroot)


	# open CrossEPG internal database
	if crossepg.epgdb_open(dbroot):
		crossepg.log_add("EPGDB opened successfully (root = %s)" % dbroot)
	else:
		crossepg.log_add("Error opening EPGDB")
		crossepg.epgdb_close()
		sys.exit(1)

	crossepg.log_add("Closing EPGDB")
	crossepg.epgdb_close()


	delta_timezone = scriptlib.delta_utc()
	crossepg.log_add("GMT vs. LocalTime difference (in seconds): %d" % delta_timezone)
	delta_daylight = scriptlib.delta_dst()
	crossepg.log_add("DayLight Saving (DST) difference now: %d" % delta_daylight)
	
	

	crossepg.log_add("---- END EXAMPLE TEST SCRIPT ----")

# -------------------------------------------------
# run main() function
main()

