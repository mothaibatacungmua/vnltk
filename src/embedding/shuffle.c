#include <stdio.h>
#include <wchar.h>
#include <stdlib.h>
#include <locale.h>
#include <string.h>
#include <time.h>
#include "glove.h"

#define MAX_SIZE 20

static long rnd(long n) {
    return ((long)rand()) % n;
}

void shuffle_array(COOC_REC* records, long len){
	COOC_REC tmp;
	long i,j;

	for(i = 1; i < len; i++){
		j = rnd(i);
		tmp = records[i];
		records[i] = records[j];
		records[j] = tmp;
	}

	return;
}

void shuffle_file(char* inputfile, char* outputfile){
	FILE* ifs = fopen(inputfile, "r");
	FILE* ofs = fopen(outputfile, "wb");
	FILE* tfs = fopen("tmp.bin", "wb");
	long lsize, result;
	long num_chunks,i, j;
	long total;
	long read_len;
	long t;
	long check_end;

	COOC_REC* buffer = NULL;

	if(ifs == NULL)
		return;

	if(ofs == NULL)
		return;

	if(tfs == NULL)
		return;

	fseek(ifs, 0, SEEK_END);
	lsize = ftell(ifs);

	num_chunks = lsize / (sizeof(COOC_REC) * MAX_SIZE) + 1;
	buffer = (COOC_REC*)malloc(MAX_SIZE*sizeof(COOC_REC));
	rewind(ifs);
	for(i = 0; i < num_chunks; i++){
		result = fread(buffer, sizeof(COOC_REC), MAX_SIZE, ifs);
		shuffle_array(buffer, result);
		fwrite(buffer, sizeof(COOC_REC), result, tfs);
	}

	fclose(ifs);
	fflush(tfs);
	tfs = freopen("tmp.bin", "r", tfs);

	t = MAX_SIZE / num_chunks;
	i = 1;
	total = 0;
	while(total < (lsize/sizeof(COOC_REC))){
		result = 0;
		for(j = 0; j < num_chunks; j++){
			fseek(tfs, j*sizeof(COOC_REC) * MAX_SIZE + (i-1)*t*sizeof(COOC_REC), SEEK_SET);
			read_len = t;
			if(i*t > MAX_SIZE){
				read_len = MAX_SIZE % num_chunks;
			}

			check_end = fread(buffer+result, sizeof(COOC_REC), read_len, tfs);
			result += check_end;

		}

		shuffle_array(buffer, result);
		fwrite(buffer, sizeof(COOC_REC), result, ofs);
		total += result;
		i++;
	}

	fclose(tfs);
	fclose(ofs);
	return;

}

int main(){
	/* initialize random seed: */
	srand (time(NULL));
	shuffle_file("cooc.bin", "shuffle.bin");
	return 0;
}
