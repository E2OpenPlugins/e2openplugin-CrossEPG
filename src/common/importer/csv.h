#ifndef _CSV_H_
#define _CSV_H_

bool csv_read (char *file, void(*progress_callback)(int, int), volatile bool *stop);

#endif // _CSV_H_
