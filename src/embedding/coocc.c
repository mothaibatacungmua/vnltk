#include <stdio.h>
#include <wchar.h>
#include <stdlib.h>
#include <locale.h>
#include <string.h>

typedef struct DICT_HASH_ENTRY{
	int hashed;
	int index;
	struct DICT_HASH_ENTRY* next;
}DICT_HASH_ENTRY;

typedef struct DICT_HASH_TABLE{
	int num_dicts;
	DICT_HASH_ENTRY** table;
}DICT_HASH_TABLE;

typedef struct WW_HASH_ENTRY{
	int word1;
	int word2;
	int coocc;
	struct WW_HASH_ENTRY* next;
}WW_HASH_ENTRY;

typedef struct WW_HASH_TABLE{
	int num_ww;
	WW_HASH_ENTRY** table;
}WW_HASH_TABLE;

typedef struct COOCC_HASH_TABLE{
	int num_ww_tables;
	WW_HASH_TABLE** ww_tables;
}COOCC_HASH_TABLE;

DICT_HASH_TABLE hvocab;
DICT_HASH_TABLE* create_vocab_table(int num_dicts){
	int i = 0;
	hvocab->table = (DICT_HASH_ENTRY**)malloc(num_dicts*sizeof(DICT_HASH_ENTRY*));
	hvocab->num_dicts = num_dicts;

	for(;i < num_dicts; i++){
		hvocab->table[i] = NULL;
	}

	return hvocab;
}

COOCC_HASH_TABLE hcoocc;
COOCC_HASH_TABLE* create_coocc_table(int num_ww_tables, int num_ww){
	int i = 0;
	int j = 0;
	hcoocc->ww_tables = (WW_HASH_TABLE**)malloc(num_ww_tables*sizeof(WW_HASH_TABLE*));
	hcoocc->num_ww_tables = num_ww_tables;

	for(i = 0;i < num_ww_tables; i++){
		hcoocc->ww_tables[i]->table = (WW_HASH_ENTRY**)malloc(num_ww*sizeof(WW_HASH_ENTRY*));
		hcoocc->ww_tables[i]->num_ww = num_ww;
		for(j=0;j < num_ww; j++){
			hcoocc->ww_tables[i]->table[j] = NULL;
		}
	}

	return hcoocc;
}

void free_dict(){

}

void free_coocc(){

}

void dump_dict(){

}

void dump_coocc(){

}

int load_dict(){
	return -1;
}

int load_coocc(){
	return -1;
}

DICT_HASH_ENTRY* find_on_dict(){
	return NULL;
}

WW_HASH_ENTRY* find_on_coocc(){
	return NULL;
}


unsigned int jenkins_hash(char* key, size_t length){
	unsigned int hash, i;
	for(hash = i = 0; i < length; ++i){
		hash += key[i];
		hash += (hash << 10);
		hash ^= (hash >> 6);
	}

	hash += (hash << 3);
	hash ^= (hash >> 11);
	hash += (hash << 15);

	return hash;
}

wchar_t* readline(FILE* fs, wchar_t* eof){
	fpos_t pos;
	int line_length = 0;
	int ret;
	wchar_t wch;
	wchar_t* line = NULL;
	//get current position
	ret = fgetpos(fs, &pos);
	long int num = 0;
	*eof = 0;
	if (ret != 0) return NULL;

	wch = fgetwc(fs);

	while(wch != L'\n' && wch != (wchar_t)WEOF){
		line_length++;
		wch = fgetwc(fs);
	}

	if(line_length == 0) return NULL;
	if(wch == (wchar_t)WEOF) *eof = (wchar_t)WEOF;

	line = (wchar_t*)malloc((line_length+1)*sizeof(wchar_t));
	if (line == NULL) return NULL;

	fsetpos(fs, &pos);
	while(num < line_length){
		line[num] = fgetwc(fs);
		num++;
	}
	fgetwc(fs);
	line[line_length] = (wchar_t)0x0000;
	return line;
}

int vnslen(wchar_t* s){
	int i = 0;
	if(s == NULL) return 0;
	while(s[i++] != 0);
	return (i-1);
}

wchar_t* vntok(wchar_t* s, wchar_t delim){
	static wchar_t _token_s[100];
	static wchar_t* _s = NULL;
	int length = 0;
	int i = 0;

	if(s != NULL) _s = s;
	if(_s == NULL) return NULL;

	length = vnslen(_s);
	if(length == 0) return NULL;

	while((_s[i] != delim) && (i < 100) && (i < length)){
		_token_s[i] = _s[i];
		i++;
	}
	_token_s[i] = (wchar_t)0x0000;
	_s = &_s[i+1];
	if((i+1) > length) _s = NULL;

	return _token_s;
}

/* http://stackoverflow.com/questions/7695592/print-wchar-to-linux-console */
const char* unicode_to_utf8(wchar_t c)
{
    static unsigned char b_static[5];
    unsigned char* b = b_static;

    if (c<(1<<7))// 7 bit Unicode encoded as plain ascii
    {
        *b++ = (unsigned char)(c);
    }
    else if (c<(1<<11))// 11 bit Unicode encoded in 2 UTF-8 bytes
    {
        *b++ = (unsigned char)((c>>6)|0xC0);
        *b++ = (unsigned char)((c&0x3F)|0x80);
    }
    else if (c<(1<<16))// 16 bit Unicode encoded in 3 UTF-8 bytes
        {
        *b++ = (unsigned char)(((c>>12))|0xE0);
        *b++ =  (unsigned char)(((c>>6)&0x3F)|0x80);
        *b++ =  (unsigned char)((c&0x3F)|0x80);
    }

    else if (c<(1<<21))// 21 bit Unicode encoded in 4 UTF-8 bytes
    {
        *b++ = (unsigned char)(((c>>18))|0xF0);
        *b++ = (unsigned char)(((c>>12)&0x3F)|0x80);
        *b++ = (unsigned char)(((c>>6)&0x3F)|0x80);
        *b++ = (unsigned char)((c&0x3F)|0x80);
    }
    *b = '\0';
    return b_static;
}

char* encode_utf8(wchar_t* s){
	int length = 0;
	int i = 0,total_en = 0;
	char* en_s = NULL;

	length = vnslen(s);
	for(i=0; i<length;i++){
		total_en += strlen(unicode_to_utf8(s[i]));
	}
	en_s = (char*)malloc(total_en+1);
	memset(en_s, 0, total_en+1);

	for(i=0; i<length;i++){
		strcat(en_s, unicode_to_utf8(s[i]));
	}

	return en_s;
}

void print_vns(wchar_t* s){
	char* en_line = NULL;
	en_line = encode_utf8(s);
	if(en_line != NULL){
		printf("%s\n", en_line);
		free(en_line);
	}
}

int main(){
	setlocale(LC_ALL, "en_US.UTF-8");
	FILE* fs;
	wchar_t wch=0;
	fs = fopen("../data/vn-sentences.txt", "r");
	int i=0;
	unsigned short c;
	wchar_t* line = NULL;
	wchar_t* tk = NULL;

	printf("%d\n", (int)sizeof(wchar_t));
	printf("%s\n", unicode_to_utf8(L'ư'));

	while(wch != (wchar_t)WEOF){
		line = readline(fs, &wch);
		printf("%d\n", (int)vnslen(line));
		print_vns(line);
		tk = vntok(line, L' ');
		while(tk != NULL){
			print_vns(tk);
			tk = vntok(NULL, L' ');
		}
		free(line);
	}

	fclose(fs);
	return 0;
}
