#ifndef _CSV_H_
#define _CSV_H_

bool csv_read (char *file, void(*progress_callback)(int, int), volatile bool *stop);
bool bin_read (char *file, char *label, void(*progress_callback)(int, int), void(*file_callback)(char*));

#endif // _CSV_H_
