#include <libxml/parser.h>
#include "file_construct.h"
#include "file_compress.h"
#include <direct.h>
#include <tchar.h>
#include <stdbool.h>

int _tmain(int argc, TCHAR* argv[]) {
	char path[MAX_PATH], fname[MAX_PATH], src[MAX_PATH], exePath[MAX_PATH];
	WideCharToMultiByte(CP_ACP, 0, argv[1], -1, path, MAX_PATH, NULL, NULL);
	WideCharToMultiByte(CP_ACP, 0, argv[0], -1, exePath, MAX_PATH, NULL, NULL);
	int len;
	//puts(exePath);
	len = strlen(exePath);
	do {
		len--;
	} while (exePath[len] != '\\');

	
	exePath[len+1] = '\0';
	//puts(exePath);
	templateInit(comicGenTemplate, exePath);

	len = strlen(path);
	if (path[len - 1] == '\\') {
		path[len - 1] = '\0';
	}
	//puts(path);
	char* title = strrchr(path, '\\') + 1;
	//puts(title);

	Flist* flist = get_list(argv[1]);

	FLNode* tmp_node = flist->first_node;
	int order = 1;
	while (tmp_node) {
		if (tmp_node->type == FT_IMAGE) {
			strcpy_s(src, MAX_PATH, "../images/");
			strcat_s(src, MAX_PATH, tmp_node->name);
			sprintf_s(fname, MAX_PATH, "%s/part%04d.xhtml", path, order);
			order++;
			genTextFile(title, src, fname);
		}
		tmp_node = tmp_node->next;
	}
	
	flist_destroy(&flist);

	flist = get_list(argv[1]);
	gen_toc(flist, path);
	flist_destroy(&flist);
	
	flist = get_list(argv[1]);
	gen_opf(flist, path);
	flist_destroy(&flist);

	flist = get_list(argv[1]);
	gen_epub_test(flist);
	flist_destroy(&flist);

	return 0;
}