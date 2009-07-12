#ifndef _IMPORTER_H_
#define _IMPORTER_H_

bool importer_extension_check (char *filename, char *extension);
void importer_parse_csv (char *dir, char *filename, char *label);
void importer_parse_url (char *dir, char *filename, char *dbroot);
int importer_set_steps (char *dir, void(*step_callback)());
void importer_parse_directory (char *dir, char *dbroot, void(*progress_callback)(int, int), void(*url_callback)(char*), void(*file_callback)(char*));
#endif // _IMPORTER_H_
