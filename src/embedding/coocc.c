#include <stdio.h>
#include <wchar.h>
#include <stdlib.h>
#include <locale.h>
#include <string.h>


typedef struct _HASHMAP_ENTRY{
	unsigned int hash;
	unsigned int index;
	struct _HASHMAP_ENTRY* next;
}HASHMAP_ENTRY;

typedef struct _HASHMAP_TABLE{
	int buckets;
	HASHMAP_ENTRY** table;
}HASHMAP_TABLE;

#define BUCKET_LENGTH 100
#define WINDOW_SIZE 4

wchar_t** g_vocab = NULL;
int g_vocab_len = 0;
HASHMAP_TABLE g_map = {};

typedef struct _COOC_REC{
	unsigned int word1;
	unsigned int word2;
	unsigned int cooc;
}COOC_REC;

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

	if(wch == (wchar_t)WEOF)
		*eof = (wchar_t)WEOF;

	if(line_length == 0)
		return NULL;


	line = (wchar_t*)malloc((line_length+1)*sizeof(wchar_t));
	if (line == NULL)
		return NULL;

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
const char* unicode_to_utf8(wchar_t c){
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

void free_tokens(wchar_t** tokens, int length){
	int i = 0;

	for(;i < length; i++){
		free(tokens[i]);
	}

	free(tokens);

	return;
}

int count_chars(wchar_t ch, wchar_t* line){
    if(line == NULL)
        return 0;

    int len = (int)vnslen(line);
    int i = 0;
    int count = 0;

    for(i = 0; i < len; i++){
    	if(line[i] == ch)
    		count++;
    }

    return count;
}

int count_tokens(wchar_t* line){
	int token_len = count_chars(L' ',line) + 1;

	return token_len;
}

wchar_t** line2tokens(wchar_t* line, int* length){
    if(line == NULL)
        return NULL;

    *length = count_tokens(line);
    wchar_t** tokens = (wchar_t**)malloc((*length) * sizeof(wchar_t*));

    wchar_t* tk = NULL;
    tk = vntok(line, L' ');
    int i = 0;

    if(tk == NULL){
    	*length = 0;
    	return NULL;
    }

    tokens[0] = (wchar_t*)malloc((vnslen(tk)+1)*sizeof(wchar_t));
    memcpy(tokens[0], tk, (vnslen(tk)+1)*sizeof(wchar_t));

    for(i = 1; i < *length; i++){
    	tk = vntok(NULL, L' ');
    	tokens[i] = (wchar_t*)malloc((vnslen(tk)+1)*sizeof(wchar_t));
    	memcpy(tokens[i], tk, (vnslen(tk)+1)*sizeof(wchar_t));
    }

    return tokens;
}

wchar_t* UNKOWN_WORD = L"@UNK";

wchar_t** read_vocab(char* filename, int* vocab_len){
	wchar_t wch = 0;
	wchar_t** vocab = NULL;
	int num_lines = 0;
	FILE* fs = fopen(filename, "r");
	wchar_t* line = NULL;

	if(fs == NULL)
		return NULL;

	while(wch != (wchar_t)WEOF){
		line = readline(fs, &wch);
		num_lines++;
		free(line);
	}

	rewind(fs);

	vocab = (wchar_t**)malloc(num_lines * sizeof(wchar_t*));
	int i = 0;
	for(; i < (num_lines-1); i++){
		line = readline(fs, &wch);
		vocab[i] = line;
	}

	vocab[i] = (wchar_t*)malloc((vnslen(UNKOWN_WORD)+1) * sizeof(wchar_t));
	memcpy(vocab[i], UNKOWN_WORD, (vnslen(UNKOWN_WORD)+1) * sizeof(wchar_t));

	*vocab_len = num_lines;
	return vocab;
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

unsigned int vocab2hash(wchar_t* vocab){
	int length = vnslen(vocab);

	return jenkins_hash((char*)vocab, length*sizeof(wchar_t));
}


HASHMAP_TABLE* alloc_table(int buckets, HASHMAP_TABLE* map){
	int i;

	map->table = (HASHMAP_ENTRY**)malloc(buckets * sizeof(HASHMAP_ENTRY*));
	map->buckets = buckets;

	for(i = 0; i < buckets; i++){
		map->table[i] = NULL;
	}

	return map;
}

HASHMAP_ENTRY* insert_entry(HASHMAP_TABLE* map, HASHMAP_ENTRY* entry){
	if(entry == NULL)
		return NULL;

	int bucket_num = entry->hash % map->buckets;
	HASHMAP_ENTRY* s = map->table[bucket_num];
	map->table[bucket_num] = entry;
	entry->next = s;

	return entry;
}

HASHMAP_ENTRY* insert_word(HASHMAP_TABLE* map, wchar_t* word, unsigned int index){
	HASHMAP_ENTRY* entry = (HASHMAP_ENTRY*)malloc(sizeof(HASHMAP_ENTRY));

	entry->hash = jenkins_hash((char*)word, vnslen(word) * sizeof(wchar_t));
	entry->index = index;

	return insert_entry(map, entry);
}

unsigned int get_index(HASHMAP_TABLE* map, unsigned int hash){
	int bucket_num = hash % map->buckets;
	HASHMAP_ENTRY* s = map->table[bucket_num];

	while(s != NULL){
		if(s->hash == hash)
			return s->index;
		s = s->next;
	}

	return -1;
}

void free_map(HASHMAP_TABLE* map){
	int i;
	HASHMAP_ENTRY* s = NULL;
	HASHMAP_ENTRY* n = NULL;

	for(i = 0; i < map->buckets; i++){
		s = map->table[i];
		if(s == NULL)
			continue;

		while(s != NULL){
			n = s;
			s = s->next;
			free(n);
		}
	}

	free(map->table);

	return;
}

void dump_map(HASHMAP_TABLE* map, char* output_file){
	int i;
	FILE* fs = fopen(output_file, "wb");
	HASHMAP_ENTRY* s = NULL;

	if(fs == NULL)
		return;

	fwrite(&map->buckets, sizeof(int), 1, fs);

	for(i = 0; i < map->buckets; i++){
		s = map->table[i];

		while(s != NULL){
			fwrite(&s->hash, sizeof(unsigned int), 1, fs);
			fwrite(&s->index, sizeof(unsigned int), 1, fs);
			s = s->next;
		}
	}

	fclose(fs);
	return;

}

void load_map(HASHMAP_TABLE* map, char* output_file){
	int buckets;
	int result;
	int i;
	unsigned int hash;
	unsigned int index;
	HASHMAP_ENTRY* entry = NULL;
	FILE* fs = fopen(output_file, "r");

	result = fread(&buckets, sizeof(int),1, fs);
	alloc_table(buckets, map);

	for(i = 0; i < buckets; i++){
		entry = (HASHMAP_ENTRY*)malloc(sizeof(HASHMAP_ENTRY));

		fread(&hash, sizeof(unsigned int),1, fs);
		fread(&index, sizeof(unsigned int),1, fs);
		entry->hash = hash;
		entry->index = index;

		insert_entry(map, entry);
	}

	return;
}

int count_cooc(unsigned int* tokens, int length, unsigned int A, unsigned int B){
	unsigned int* i,*j, *end, *begin;
	int count = 0;

	if(A == B)
		return 0;

	for(i = tokens; i < (tokens+length); i++){
		if(*i == A){
			begin = i - WINDOW_SIZE - 1;
			if(begin <= tokens)
				begin = tokens;

			end = i + WINDOW_SIZE + 1;
			if(end >= (tokens + length))
				end = tokens + length;

			for(j = begin; j < end; j++){
				if(*j == B)
					count++;
			}
		}
	}

	return count;
}

int find_in_set(unsigned int* set, int current_length, unsigned int v){
	int travel = 0;

	while(travel < current_length){
		if(set[travel] == v)
			return 1;

		if(set[travel] > v)
			travel = travel*2 + 2;
		else
			travel = travel*2 + 1;
	}

	return 0;
}

int push_in_set(unsigned int* set, int* current_length, unsigned int v){
	set[++(*current_length) - 1] = v;
	int pos = (*current_length) - 1;

	int travel = pos;
	int* parent = &set[(travel-1)/2];
	int* current = &set[travel];
	int tmp;

	while(*current < *parent){
		tmp = *current;
		*current = *parent;
		*parent = tmp;
		travel = (travel - 1)/2;
		parent = &set[(travel-1)/2];
		current = &set[travel];
	}

	return travel;
}

unsigned int* tokens2set(wchar_t** tokens, int length, int* set_len){
	int i,j;
	unsigned int* set = (unsigned int*)malloc(length * sizeof(unsigned int));
	int count = 0;
	unsigned int hash;

	for(i = 0; i < length; i++){
		hash = jenkins_hash((char*)tokens[i], vnslen(tokens[i]) * sizeof(wchar_t));

		//hash to unknown word
		if(get_index(&g_map,hash) == -1){
			hash = jenkins_hash((char*)UNKOWN_WORD, vnslen(UNKOWN_WORD)* sizeof(wchar_t));
		}

		if(find_in_set(set, count, hash) == 1)
			continue;
		else
			push_in_set(set, &count, hash);
	}

	*set_len = count;
	return set;
}

unsigned int* convert_tokens(wchar_t** tokens, int length){
	int i;
	unsigned int* hash_tokens = (unsigned int*)malloc(length * sizeof(unsigned int));
	unsigned int hash;

	for(i = 0; i < length ; i++){
		hash = jenkins_hash((char*)tokens[i], vnslen(tokens[i]) * sizeof(wchar_t));

		if(get_index(&g_map,hash) == -1){
			hash = jenkins_hash((char*)UNKOWN_WORD, vnslen(UNKOWN_WORD) * sizeof(wchar_t));
		}

		hash_tokens[i] = hash;
	}

	return hash_tokens;
}

void sen2cocc(char* input_file, char* output_file){
	FILE* ifs = fopen(input_file, "r");
	FILE* ofs = fopen(output_file, "wb");
	wchar_t* line = NULL;
	wchar_t** tokens;
	int tokens_len;
	int set_len;
	wchar_t wch=0;
	unsigned int* set_words;
	unsigned int* hash_tokens;
	int i,j;
	COOC_REC crec;

	while(wch != (wchar_t)WEOF){
		line = readline(ifs, &wch);
		tokens = line2tokens(line, &tokens_len);
		hash_tokens = convert_tokens(tokens, tokens_len);
		set_words = tokens2set(tokens, tokens_len, &set_len);

		for(i = 0; i < set_len; i++){
			for(j = (i+1); j < set_len; j++){
				crec.word1 = set_words[i];
				crec.word2 = set_words[j];
				crec.cooc = count_cooc(hash_tokens, tokens_len, crec.word1, crec.word2);
				if(crec.cooc == 0)
					continue;

				fwrite(&crec, sizeof(COOC_REC), 1, ofs);

				crec.word1 = set_words[j];
				crec.word2 = set_words[i];
				fwrite(&crec, sizeof(COOC_REC), 1, ofs);
			}
		}

		free(line);
		free_tokens(tokens, tokens_len);
		free(hash_tokens);
		free(set_words);
	}

	fclose(ifs);
	fclose(ofs);
}



int main(){
	setlocale(LC_ALL, "en_US.UTF-8");
	int length=0;
	wchar_t** vocab = read_vocab("../data/vn-vocab.txt", &length);
	int i;

	g_vocab = vocab;
	g_vocab_len = length;

	alloc_table(BUCKET_LENGTH, &g_map);

	for(i = 0; i < length; i++){
		insert_word(&g_map, vocab[i], i);
	}

	dump_map(&g_map, "vocab.bin");
	sen2cocc("../data/vn-sentences.txt", "cooc.bin");

	free_tokens(vocab, length);

	return 0;
}
