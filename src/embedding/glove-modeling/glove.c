#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <wchar.h>
#include <locale.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include "glove.h"


#define MAX_STRING_LENGTH 1000

int use_unk_vec = 1; //0 or 1
int num_threads = 1;
int num_epochs = 50;
int vector_size = 50;
int vocab_size = 0;
int save_gradsq_and_deltasq = 0;
double eta = 0.05;
double gamma_mm = 0.9;
double alpha = 0.75, x_max = 100;
double* W, *gradsq, *deltasq, *cost;
long long num_lines, *lines_per_thread;
char *vocab_file = "vocab.bin";
char *input_file = "shuffle.bin";
char *save_W_file ="weight";
char *save_gradsq_file = "gradsq";
char *save_deltasq_file = "deltasq";
double epsilon = 1e-6;

void initialize_parameters(){
	long long a, b, ret;
	vector_size++;

	ret = posix_memalign((void**)&W, 128, 2*vocab_size*vector_size*sizeof(double));

	if(W == NULL){
		fprintf(stderr, "Error allocating memory for W\n");
		exit(1);
	}

	ret = posix_memalign((void**)&gradsq, 128, 2*vocab_size*vector_size*sizeof(double));

	if(gradsq == NULL){
		fprintf(stderr, "Error allocating memory for gradsq\n");
		exit(1);
	}

	ret = posix_memalign((void**)&deltasq, 128, 2*vocab_size*vector_size*sizeof(double));

	if(gradsq == NULL){
		fprintf(stderr, "Error allocating memory for gradsq\n");
		exit(1);
	}

	for(b = 0; b < vector_size; b++)
		for(a = 0; a < 2*vocab_size; a++){
			W[a * vector_size + b] = (rand() / (double)RAND_MAX - 0.5) / vector_size;
		}


	for(b = 0; b < vector_size; b++){
		for(a = 0; a < 2*vocab_size; a++){
			gradsq[a*vector_size + b] = 0.0;
			deltasq[a*vector_size + b] = 0.0;
		}
	}
	vector_size--;
}

void *glove_thread(void* vid){
	long long i, k, l1, l2;
	long long tid = (long long)vid;
	double diff, fdiff, delta1, delta2;
	COOC_REC cr;
	FILE* fs = fopen(input_file, "rb");
	fseek(fs, (num_lines/num_threads * tid)*sizeof(COOC_REC), SEEK_SET);
	cost[tid] = 0;

	for(i = 0; i < lines_per_thread[tid]; i++){
		fread(&cr, sizeof(COOC_REC), 1, fs);

		l1 = (cr.word1 - 1LL) * (vector_size + 1);
		l2 = ((cr.word2 - 1LL) + vocab_size)*(vector_size + 1);

		/* Caculate cost, save diff for gradients */
		diff = 0;
		for(k = 0; k < vector_size; k++) diff += W[k + l1] * W[k + l2];
		diff += W[vector_size + l1] + W[vector_size + l2] - log(cr.value);
		fdiff = (cr.value > x_max) ? diff: pow(cr.value / x_max, alpha) * diff;
		cost[tid] += 0.5 * fdiff * diff;

		/* Adadelta updates. Updating weight */
		for(k = 0; k < vector_size; k++){
			// update g[t]
			gradsq[l1 + k] = gamma_mm*gradsq[l1 + k] + (1 - gamma_mm)*(fdiff*W[l1 + k])*(fdiff*W[l1 + k]);
			gradsq[l2 + k] = gamma_mm*gradsq[l2 + k] + (1 - gamma_mm)*(fdiff*W[l2 + k])*(fdiff*W[l2 + k]);

			delta1 = -fdiff/sqrt(gradsq[l1 + k] + epsilon)*W[l1 + k];
			delta2 = -fdiff/sqrt(gradsq[l2 + k] + epsilon)*W[l2 + k];

			// w = w - rms_delta[t-1]/rms_grad[t]*w
			W[l1 + k] += sqrt(deltasq[l1 + k] + epsilon)*delta1;
			W[l2 + k] += sqrt(deltasq[l2 + k] + epsilon)*delta2;

			// delta[t]
			deltasq[l1 + k] = gamma_mm*deltasq[l1 + k] + (1 - gamma_mm)*(eta*delta1)*(eta*delta1);
			deltasq[l2 + k] = gamma_mm*deltasq[l2 + k] + (1 - gamma_mm)*(eta*delta2)*(eta*delta2);
		}

		/* Updating bias */
		gradsq[l1 + vector_size] = gamma_mm*gradsq[l1 + vector_size] + (1 - gamma_mm)*fdiff*fdiff;
		gradsq[l2 + vector_size] = gamma_mm*gradsq[l2 + vector_size] + (1 - gamma_mm)*fdiff*fdiff;

		delta1 = -fdiff/sqrt(gradsq[l1 + vector_size] + epsilon);
		delta2 = -fdiff/sqrt(gradsq[l2 + vector_size] + epsilon);

		W[l1 + vector_size] += sqrt(deltasq[l1 + vector_size] + epsilon)*delta1;
		W[l2 + vector_size] += sqrt(deltasq[l2 + vector_size] + epsilon)*delta2;

		deltasq[l1 + vector_size] = gamma_mm*deltasq[l1 + vector_size] + (1 - gamma_mm)*(eta*delta1)*(eta*delta1);
		deltasq[l2 + vector_size] = gamma_mm*deltasq[l2 + vector_size] + (1 - gamma_mm)*(eta*delta2)*(eta*delta2);
	}

	fclose(fs);
	pthread_exit(NULL);

}


int save_params(){
	long long a;
	char output_W_file[MAX_STRING_LENGTH],
		 output_gradsq_file[MAX_STRING_LENGTH],
		 output_deltasq_file[MAX_STRING_LENGTH];
	FILE *fW, *fgrad, *fdelta;

	sprintf(output_W_file,"%s.bin",save_W_file);
	fW = fopen(output_W_file, "wb");
	if(fW == NULL){
		fprintf(stderr,"Unable to open file %s.\n", save_W_file);
		return -1;
	}

	for(a = 0; a < 2 * (long long)vocab_size * (vector_size + 1); a++){
		fwrite(&W[a], sizeof(double),1,fW);
	}
	fclose(fW);

	if(save_gradsq_and_deltasq > 0){
		sprintf(output_gradsq_file, "%s.bin", save_gradsq_file);
		sprintf(output_deltasq_file, "%s.bin", save_deltasq_file);
		fgrad = fopen(output_gradsq_file, "wb");
		fdelta = fopen(output_deltasq_file, "wb");
		if(fgrad == NULL){
			fprintf(stderr,"Unable to open file %s.\n", save_gradsq_file);
			return -1;
		}

		if(fdelta == NULL){
			fprintf(stderr,"Unable to open file %s.\n", save_deltasq_file);
			fclose(fgrad);
			return -1;
		}

		for(a = 0; a < 2 * (long long)vocab_size * (vector_size + 1); a++){
			fwrite(&gradsq[a], sizeof(double),1,fgrad);
			fwrite(&deltasq[a], sizeof(double),1,fdelta);
		}

		fclose(fgrad);
		fclose(fdelta);
	}
	return 0;
}

int train_glove(){
	FILE* fs = fopen(input_file, "r");
	long i,k;
	double total_cost = 0.0;

	fseek(fs, 0, SEEK_END);
	num_lines = ftell(fs);
	num_lines /= sizeof(COOC_REC);
	fclose(fs);

	cost = (double*)malloc(num_threads * sizeof(double));
	memset(cost, 0, num_threads * sizeof(double));

	lines_per_thread = (long long*)malloc(num_threads * sizeof(long long));
	pthread_t *pt = (pthread_t*)malloc(num_threads * sizeof(pthread_t));

	for(i = 0 ; i < num_epochs; i++){
		total_cost = 0.0;
		for(k = 0; k < num_threads - 1; k++) lines_per_thread[k] = num_lines/num_threads;
		lines_per_thread[k] = num_lines / num_threads + num_lines % num_threads;

		for(k = 0; k < num_threads; k++) pthread_create(&pt[k], NULL, glove_thread, (void*)k);
		for(k = 0; k < num_threads; k++) pthread_join(pt[k], NULL);
		for(k = 0; k < num_threads; k++) total_cost += cost[k];

		fprintf(stderr, "epochs:%03d, cost:%lf\n", (int)i+1, total_cost/num_lines);
	}

	return save_params();
}

int main(){
	FILE* fvocab;
	long lsize, result;
	fvocab = fopen(vocab_file, "r");

	if(fvocab == NULL){
		fprintf(stderr,"Unable to open file %s.\n", vocab_file);
		return -1;
	}

	fseek(fvocab, 0, SEEK_END);
	lsize = ftell(fvocab);
	fclose(fvocab);
	vocab_size = ((int)lsize)/(sizeof(HASHMAP_ENTRY) - sizeof(HASHMAP_ENTRY*));

	initialize_parameters();
	return train_glove();
}
