#pragma once
#ifndef _FCONSTRUCT
#define _FCONSTRUCT

#include "file_list.h"

//Replace "\" with "/" and remove the trailing "/"
int process_path(const char* in, char* out);
enum templateType {
	tt_PAGE,
	tt_CONTAINER,
	tt_MIMETYPE,
	tt_TOC,
	tt_OPF,
	tt_CSS
};
char comicGenTemplate[5][MAX_PATH];

int templateInit(char comicGenTemplate[5][MAX_PATH], char* path);

int get_time_str(char* time_str);

//Generate a uuid string. <uuid_str> should have a length of 37.
int get_uuid(char* uuid_str);

//Generate a list of files and directories for children of 'path'. Returned Flist should be freed by the user.
Flist* get_list(TCHAR* path);

int genTextFile(char* path, char* src, char* fname);
int gen_toc(Flist* flist, char* path);
int gen_opf(Flist* flist, char* path);
#endif