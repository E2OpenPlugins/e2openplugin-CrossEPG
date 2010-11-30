#!/usr/local/bin/python
# above is Python location into QboxHD

# aleambro 30-Nov-2010
# this test.py program simply do nothing


# import Enigma2 functions
from crossepg import *


# -------------------------------------------------
# this is the main function
def main():

	log_add("START TEST SCRIPT")

	# get install path
	instdir = epgdb_get_installroot()
	log_add("Install dir : %s" % instdir)

	# get dbroot path
	dbroot = epgdb_get_dbroot()
	log_add("Database dir : %s" % dbroot)

	# open epgdb
	if epgdb_open(dbroot):
		log_add("EPGDB opened (root=%s)" % dbroot);
	else:
		log_add("Error opening EPGDB");
		epgdb_close();
		return

	# close epgdb
	log_add("EPGDB closing")
	epgdb_close();


	log_add("END TEST SCRIPT")

# -------------------------------------------------
# run main() function
main()

