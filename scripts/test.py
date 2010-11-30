# aleambro 30-Nov-2010
# this test.py program simply do nothing


# import Enigma2 functions
from crossepg import *


# -------------------------------------------------
# this is the main function
def main():

	log_add("START TEST SCRIPT")

	# get dbroot path
	dbroot = epgdb_get_dbroot()

	# open epgdb
	if epgdb_open(dbroot):
		log_add("EPGDB opened (root=%s)" % dbroot);
	else:
		log_add("Error opening EPGDB");
		epgdb_close();
		return

	log_add("END TEST SCRIPT")

# -------------------------------------------------
# run main() function
main()

