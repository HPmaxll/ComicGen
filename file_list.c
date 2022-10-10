#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <strsafe.h>
#include "file_list.h"

int get_type(char* path) {
	if (strcmp(path, "toc.xhtml") == 0) {
		return FT_TOC;
	}
	int length = strlen(path);
	char c = path[length - 1];
	int type = 0;
	
	switch (c) {
	case 'g':
		type = FT_IMAGE;
		break;
	case 'l':
		type = FT_XML;
		break;
	case 'f':
		type = FT_OPF;
		break;
	case 'x':
		type = FT_NCX;
		break;
	case 's':
		type = FT_CSS;
		break;
	default:
		type = FT_OTHER;
	}
	return type;
}

Flist* new_flist() {
	Flist* flist = (Flist*)malloc(sizeof(Flist));
	if (!flist) {
		puts("Failed to create head node.");
		return NULL;
	}
	flist->first_node = NULL;
	flist->last_node = NULL;
	flist->length = 0;
	return flist;
}

int print_flist(Flist* flist) {	
	FLNode* tmp = flist->first_node;
	printf("[");
	while (tmp->next) {
		printf("[\"%s\", %d], ", tmp->name, tmp->type);
		tmp = tmp->next;
	}
	printf("\"%s\", %d]]\n", tmp->name, tmp->type);
	return 0;
}

int flist_append_node(Flist* flist, char* name, TCHAR* path, int type) {
	
	int length = strlen(name) + 1;
	FLNode* tmp = (FLNode*)malloc(sizeof(FLNode));
	if (!tmp)
		return -1;
	strcpy_s(tmp->name, sizeof(char) * length, name);
	StringCchCopy(tmp->path, MAX_PATH, path);
	tmp->type = type;
	tmp->next = NULL;
	if (flist->length == 0) {
		flist->first_node = tmp;
		flist->last_node = tmp;
	}
	else {
		flist->last_node->next = tmp;
		flist->last_node = tmp;
	}
	flist->length++;
	return 0;
}

int flist_destroy(Flist** flist) {
	FLNode* first = (*flist)->first_node;
	FLNode* tmp = NULL;
	while (first->next) {
		tmp = first->next;
		first->next = tmp->next;
		free(tmp);
	}
	free(first);
	free(*flist);
	*flist = NULL;
	return 0;
}