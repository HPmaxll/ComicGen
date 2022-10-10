#include <stdio.h>
#include <time.h>
#include <Rpc.h>
#include <windows.h>
#include <tchar.h>
#include <strsafe.h>
#include <libxml/tree.h>
#include <libxml/parser.h>
#include "file_construct.h"

int process_path(const char* in, char* out) {
	strcpy_s(out, MAX_PATH, in);
	int len = strlen(in);
	for (int i = 0; i < len; i++) {
		if (out[i] == '\\')
			out[i] = '/';
	}
	if (out[len - 1] == '/')
		out[len - 1] = '\0';
	return 0;
}

int templateInit(char comicGenTemplate[5][MAX_PATH], char* path) {
	strcpy_s(comicGenTemplate[tt_PAGE], MAX_PATH, path);
	strcat_s(comicGenTemplate[tt_PAGE], MAX_PATH, "resource\\page.xhtml");
	strcpy_s(comicGenTemplate[tt_CONTAINER], MAX_PATH, path);
	strcat_s(comicGenTemplate[tt_CONTAINER], MAX_PATH, "resource\\container.xml");
	strcpy_s(comicGenTemplate[tt_MIMETYPE], MAX_PATH, path);
	strcat_s(comicGenTemplate[tt_MIMETYPE], MAX_PATH, "resource\\mimetype");
	strcpy_s(comicGenTemplate[tt_TOC], MAX_PATH, path);
	strcat_s(comicGenTemplate[tt_TOC], MAX_PATH, "resource\\toc.xhtml");
	strcpy_s(comicGenTemplate[tt_OPF], MAX_PATH, path);
	strcat_s(comicGenTemplate[tt_OPF], MAX_PATH, "resource\\package.opf");
	strcpy_s(comicGenTemplate[tt_CSS], MAX_PATH, path);
	strcat_s(comicGenTemplate[tt_CSS], MAX_PATH, "resource\\default.css");
	return 0;
}

int get_uuid(char* uuid_str) {
	UUID uuid;
	RPC_CWSTR StringUuid;
	
	if (UuidCreate(&uuid) != RPC_S_OK) {
		puts("Failed to generate uuid.");
		return -1;
	}
	if (UuidToString(&uuid, &StringUuid) != RPC_S_OK) {
		puts("Translate uuid to string failed.");
		return -1;
	}
	WideCharToMultiByte(CP_ACP, 0, StringUuid, -1, uuid_str, 37, NULL, NULL);
	RpcStringFree(&StringUuid);
	return 0;
}

int get_time_str(char* time_str) {
	time_t cur;
	struct tm tms;
	time(&cur);
	localtime_s(&tms, &cur);
	strftime(time_str, 21, "%Y-%m-%dT%H:%M:%SZ", &tms);
	return 0;
}

Flist* get_list(TCHAR* path) {
	WIN32_FIND_DATAW ffd;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	TCHAR szDir[MAX_PATH];
	TCHAR file_path[MAX_PATH];
	char name[MAX_PATH];
	StringCchCopy(szDir, MAX_PATH, path);
	StringCchCat(szDir, MAX_PATH, TEXT("\\*"));

	hFind = FindFirstFile(szDir, &ffd);
	Flist* flist = new_flist();
	if (hFind == INVALID_HANDLE_VALUE) {
		puts("error: not found.");
		return NULL;
	}
	do {
		WideCharToMultiByte(CP_ACP, 0, ffd.cFileName, -1, name, MAX_PATH, NULL, NULL);
		
		if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			flist_append_node(flist, name, ffd.cFileName, FT_DIR);
		}
		else {
			StringCchCopy(file_path, MAX_PATH, path);
			StringCchCat(file_path, MAX_PATH, TEXT("\\"));
			StringCchCat(file_path, MAX_PATH, ffd.cFileName);
			flist_append_node(flist, name, file_path, get_type(name));
		}
		memset(file_path, 0, MAX_PATH);
	} while (FindNextFile(hFind, &ffd) != 0);
	FindClose(hFind);
	return flist;
}

xmlNodePtr getFirstChildByName(xmlNodePtr node, char* name) {
	xmlNodePtr cursor = NULL;
	cursor = node->children;
	while (strcmp(cursor->name, name) != 0) {
		cursor = cursor->next;
		if (cursor == NULL) {
			printf("Can't find node %s.\n", name);
			break;
		}
	}
	return cursor;
}
xmlNodePtr getNextNodeByName(xmlNodePtr node, char* name) {
	xmlNodePtr cursor = NULL;
	cursor = node->next;
	while (strcmp(cursor->name, name) != 0) {
		cursor = cursor->next;
		if (cursor == NULL) {
			printf("Can't find node %s.\n", name);
			break;
		}
	}
	return cursor;
}

int genTextFile(char* title, char* src, char* fname) {
	xmlKeepBlanksDefault(0);
	xmlIndentTreeOutput = 1;
	xmlDocPtr doc = NULL;
	xmlNodePtr node_root = NULL, node_title = NULL, node_img = NULL, cursor = NULL;
	doc = xmlParseFile(comicGenTemplate[tt_PAGE]);
	char buffer[MAX_PATH];
	if (!doc) {
		strerror_s(buffer, MAX_PATH, errno);
		printf("%s\n", buffer);
	}
	node_root = xmlDocGetRootElement(doc);
	cursor = getFirstChildByName(node_root, "head");
	node_title = getFirstChildByName(cursor, "title");
	
	TCHAR buf[MAX_PATH];
	char uniTitle[MAX_PATH];
	MultiByteToWideChar(CP_THREAD_ACP, 0, title, -1, buf, MAX_PATH);
	WideCharToMultiByte(CP_UTF8, 0, buf, -1, uniTitle, MAX_PATH, NULL, NULL);

	xmlNodeSetContent(node_title, BAD_CAST uniTitle);

	cursor = getFirstChildByName(node_root, "body");
	node_img = getFirstChildByName(cursor, "img");
	xmlSetProp(node_img, BAD_CAST "src", src);

	xmlSaveFormatFileEnc(fname, doc, "UTF-8", 1);
	xmlFreeDoc(doc);
	return 0;
}
int gen_toc(Flist* flist, char* path) {
	xmlKeepBlanksDefault(0);
	xmlIndentTreeOutput = 1;
	int ord = 1;
	char* sord[10];
	xmlDocPtr doc = NULL;
	xmlNodePtr node_html = NULL;
	xmlNodePtr node_li = NULL, node_ol = NULL, node_ol2 = NULL, node_a = NULL, node_nav = NULL, node_nav2 = NULL;
	xmlNodePtr cursor = NULL;
	FLNode * head = NULL, * tmp = NULL;
	head = flist->first_node;
	while (head->type != FT_XML)
		head = head->next;
	tmp = head;

	char fname[MAX_PATH];
	TCHAR buf[MAX_PATH];
	char uniTitle[MAX_PATH];

	char* title = strrchr(path, '\\') + 1;

	MultiByteToWideChar(CP_THREAD_ACP, 0, title, -1, buf, MAX_PATH);
	WideCharToMultiByte(CP_UTF8, 0, buf, -1, uniTitle, MAX_PATH, NULL, NULL);

	strcpy_s(fname, MAX_PATH, path);
	strcat_s(fname, MAX_PATH, "/toc.xhtml");

	doc = xmlParseFile(comicGenTemplate[tt_TOC]);
	node_html = xmlDocGetRootElement(doc);
	cursor = getFirstChildByName(node_html, "head");
	cursor = getFirstChildByName(cursor, "title");
	xmlNodeSetContent(cursor, BAD_CAST uniTitle);

	cursor = getFirstChildByName(node_html, "body");
	cursor = getFirstChildByName(cursor, "header");
	cursor = getFirstChildByName(cursor, "h1");
	xmlNodeSetContent(cursor, BAD_CAST "catalog");

	cursor = getFirstChildByName(node_html, "body");
	node_nav = getFirstChildByName(cursor, "nav");
	node_nav2 = getNextNodeByName(node_nav, "nav");

	node_ol = xmlNewChild(node_nav,NULL, BAD_CAST "ol", NULL);
	node_li = xmlNewChild(node_ol, NULL, BAD_CAST "li", NULL);
	xmlNewProp(node_li, BAD_CAST "epub:type", BAD_CAST "chapter");
	node_a = xmlNewChild(node_li, NULL, BAD_CAST "a", BAD_CAST uniTitle);
	xmlNewProp(node_a, BAD_CAST "href", BAD_CAST head->name);
	
	node_ol2 = xmlNewChild(node_nav2, NULL, BAD_CAST "ol", NULL);
	while (tmp != NULL && tmp->type == FT_XML) {
		node_li = xmlNewChild(node_ol2, NULL, BAD_CAST "li", NULL);
		node_a = xmlNewChild(node_li, NULL, BAD_CAST "a", NULL);
		xmlNewProp(node_a, BAD_CAST "href", BAD_CAST tmp->name);
		sprintf_s(sord, 10, "%d", ord);
		xmlNodeSetContent(node_a, BAD_CAST sord);
		tmp = tmp->next;
		ord++;
	}

	xmlSaveFormatFileEnc(fname, doc, "UTF-8", 1);
	xmlFreeDoc(doc);
	return 0;
}
int gen_opf(Flist* flist, char* path) {

	xmlKeepBlanksDefault(0);
	xmlIndentTreeOutput = 1;

	xmlDocPtr doc = NULL;
	char fname[MAX_PATH], href[MAX_PATH], fid[10], uuid[37], id_content[46], time_str[21];

	TCHAR buf[MAX_PATH];
	char uniTitle[MAX_PATH];
	char* title = strrchr(path, '\\') + 1;
	MultiByteToWideChar(CP_THREAD_ACP, 0, title, -1, buf, MAX_PATH);
	WideCharToMultiByte(CP_UTF8, 0, buf, -1, uniTitle, MAX_PATH, NULL, NULL);
	
	//printf("opf  %s.\n", title);
	int jorder = 1, porder = 1;
	
	strcpy_s(fname, MAX_PATH, path);
	strcat_s(fname, MAX_PATH, "/content.opf");

	get_uuid(uuid);
	sprintf_s(id_content, 46, "urn:uuid:%s", uuid);

	get_time_str(time_str);

	xmlNodePtr node_pkg = NULL, node_mf = NULL, node_sp = NULL, node_tmp = NULL, node_tmp2 = NULL;
	xmlNodePtr cursor = NULL;
	
	doc = xmlParseFile(comicGenTemplate[tt_OPF]);
	
	node_pkg = xmlDocGetRootElement(doc);
	
	cursor = getFirstChildByName(node_pkg, "metadata");

	cursor = getFirstChildByName(cursor, "identifier");
	
	xmlNodeSetContent(cursor, id_content);

	cursor = getNextNodeByName(cursor, "title");
	xmlNodeSetContent(cursor, BAD_CAST uniTitle);

	cursor = getNextNodeByName(cursor, "language");
	xmlNodeSetContent(cursor, "zh_CN");

	cursor = getNextNodeByName(cursor, "meta");
	xmlNodeSetContent(cursor, time_str);

	node_mf = getFirstChildByName(node_pkg, "manifest");
	node_sp = getFirstChildByName(node_pkg, "spine");

	FLNode* head = flist->first_node;
	while (head) {
		switch (head->type) {
		case FT_TOC:
			node_tmp = xmlNewChild(node_mf, NULL, "item", NULL);
			sprintf_s(fid, 10, "%s", "toc");
			xmlNewProp(node_tmp, "id", fid);
			strcpy_s(href, MAX_PATH, "xhtml/");
			strcat_s(href, MAX_PATH, head->name);
			xmlNewProp(node_tmp, "href", href);
			xmlNewProp(node_tmp, "media-type", "application/xhtml+xml");
			break;
		case FT_XML:
			node_tmp = xmlNewChild(node_mf, NULL, "item", NULL);
			sprintf_s(fid, 10, "p_%d", porder++);
			xmlNewProp(node_tmp, "id", fid);
			node_tmp2 = xmlNewChild(node_sp, NULL, "itemref", NULL);
			xmlNewProp(node_tmp2, "idref", fid);
			strcpy_s(href, MAX_PATH, "xhtml/");
			strcat_s(href, MAX_PATH, head->name);
			xmlNewProp(node_tmp, "href", href);
			xmlNewProp(node_tmp, "media-type", "application/xhtml+xml");
			break;
		case FT_IMAGE:
			node_tmp = xmlNewChild(node_mf, NULL, "item", NULL);
			sprintf_s(fid, 10, "j_%d", jorder++);
			xmlNewProp(node_tmp, "id", fid);
			strcpy_s(href, MAX_PATH, "images/");
			strcat_s(href, MAX_PATH, head->name);
			xmlNewProp(node_tmp, "href", href);
			xmlNewProp(node_tmp, "media-type", "image/jpeg");
			break;
		}
		
		head = head->next;
	}

	xmlSaveFormatFileEnc(fname, doc, "UTF-8", 1);
	xmlFreeDoc(doc);
	return 0;
}