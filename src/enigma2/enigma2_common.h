#ifndef _ENIGMA2_COMMON_H_
#define _ENIGMA2_COMMON_H_

static int toBCD (int dec)
{
	if (dec >= 100)
		return -1;
	return (dec/10)*0x10 + dec%10;
}

/*
static int get_mjd (struct tm *value)
{
	int l = 0;
	if (value->tm_mon <= 1)	// Jan or Feb
		l = 1;
	return (14956 + value->tm_mday + ((value->tm_year - l) * 365.25) + ((value->tm_mon + 2 + l * 12) * 30.6001));
}
*/

#endif //_ENIGMA2_COMMON_H_
