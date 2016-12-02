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

typedef struct _COOC_REC{
	unsigned int word1;
	unsigned int word2;
	unsigned int cooc;
}COOC_REC;
