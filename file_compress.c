#include <string.h>
#include <stdio.h>
#include <zip.h>
#include <strsafe.h>
#include "file_compress.h"
const wchar_t* comicGenTemp = "comicGenTemp.epub";

int genEpub(Flist* flist, char* path) {
	/*char* title = strrchr(path, '/') + 1;
	char fname[MAX_PATH];
	char buf[100];*/
	return 0;
}

int gen_epub_test(Flist* flist) {
	zip_t* book = NULL;
	zip_source_t* zs = NULL;
	FLNode* tmp = NULL;
	int err;
	char zip_path[MAX_PATH] = { 0 };

	char* book_struct[] = {
		"META-INF",
		"OPS",
		"OPS/css",
		"OPS/images",
		"OPS/xhtml"
	};
	book = zip_open(comicGenTemp, ZIP_TRUNCATE | ZIP_CREATE, &err);
	if (!book) {
		puts("failed to open file.");
		return -1;
	}
	for (int i = 0; i < 5; i++) {
		zip_dir_add(book, book_struct[i], ZIP_FL_ENC_UTF_8);
	}

	tmp = flist->first_node;
	while (tmp != NULL) {
		switch (tmp->type) {
		case FT_TOC:
			strcpy_s(zip_path, MAX_PATH, book_struct[4]);
			strcat_s(zip_path, MAX_PATH, "/");
			strcat_s(zip_path, MAX_PATH, tmp->name);
			break;

		case FT_OPF:
			strcpy_s(zip_path, MAX_PATH, book_struct[1]);
			strcat_s(zip_path, MAX_PATH, "/");
			strcat_s(zip_path, MAX_PATH, tmp->name);
			break;

		case FT_CSS:
			strcpy_s(zip_path, MAX_PATH, book_struct[2]);
			strcat_s(zip_path, MAX_PATH, "/");
			strcat_s(zip_path, MAX_PATH, tmp->name);
			break;

		case FT_IMAGE:
			strcpy_s(zip_path, MAX_PATH, book_struct[3]);
			strcat_s(zip_path, MAX_PATH, "/");
			strcat_s(zip_path, MAX_PATH, tmp->name);
			break;

		case FT_XML:
			strcpy_s(zip_path, MAX_PATH, book_struct[4]);
			strcat_s(zip_path, MAX_PATH, "/");
			strcat_s(zip_path, MAX_PATH, tmp->name);
			break;

		default:
			tmp = tmp->next;
			continue;
		}
		
		zs = zip_source_win32w(book, tmp->path, 0, 0);
		if (!zs) {
			puts("sourse file failed.");
		}
		zip_file_add(book, zip_path, zs, ZIP_FL_OVERWRITE | ZIP_FL_ENC_UTF_8);
		tmp = tmp->next;
	}

	zs = zip_source_file(book, comicGenTemplate[tt_CONTAINER], 0, 0);
	zip_file_add(book, "META-INF/container.xml", zs, ZIP_FL_OVERWRITE | ZIP_FL_ENC_UTF_8);
	zs = zip_source_file(book, comicGenTemplate[tt_CSS], 0, 0);
	zip_file_add(book, "OPS/css/default.css", zs, ZIP_FL_OVERWRITE | ZIP_FL_ENC_UTF_8);
	zs = zip_source_file(book, comicGenTemplate[tt_MIMETYPE], 0, 0);
	zip_file_add(book, "mimetype", zs, ZIP_FL_OVERWRITE | ZIP_FL_ENC_UTF_8);
	zip_close(book);
	return 0;
}