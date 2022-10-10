#pragma once
#ifndef _FLIST
#define _FLIST

#include <Windows.h>

enum f_type {
	FT_DIR = 1,
	FT_IMAGE,
	FT_XML,
	FT_OPF,
	FT_TOC,
	FT_NCX,
	FT_CSS,
	FT_OTHER
};

typedef struct _flnode {
	char name[MAX_PATH];
	TCHAR path[MAX_PATH];
	int type;
	struct _flnode* next;
} FLNode;

typedef struct _flist {
	int length;
	FLNode* first_node;
	FLNode* last_node;
} Flist;

int get_type(char* path);

//Create a new Flist struct.
Flist* new_flist();

//Print the content of flist.
int print_flist(Flist* flist);

//Append a node to the end of flist.
int flist_append_node(Flist* flist, char* name, TCHAR* path, int type);

//Free flist that was created by new_flist().
int flist_destroy(Flist** flist);

#endif