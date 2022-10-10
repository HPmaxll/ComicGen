#pragma once
#ifndef _FCOMPRESS
#define _FCOMPRESS


#include "file_list.h"
#include "file_construct.h"


int genEpub(Flist* flist, char* path);
int gen_epub_test(Flist* flist);
const wchar_t* comicGenTemp;
#endif