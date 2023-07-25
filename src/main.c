#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HASHTABLE_SIZE 1000

typedef enum {
	Number,
	String,
	Array,
	Object
} Type;

typedef enum {
	Continue,
	End
} Status;

typedef enum {
	FindByIndex,
	FindByKey,
	Undefind
} Situation;

typedef struct JsonObject {//array or object
	Type type;//type
	char* key;//key

	union {//data
		char* text; //string
		int num; // number
		struct {//array or object
			int length;
			struct JsonObject* first;
			struct JsonObject* last;
		} Children;
	};
	struct JsonObject* next;//next
	struct JsonObject* parent;//parent
} JsonObject;

char* ScrapString() {
	char true = 1; char c; int i = 0; int size = 30;
	char* key = calloc(size, sizeof(char));
	c = getchar();
	while (c != '"') {// fix realloc if size not enough
		key[i] = c;
		c = getchar();
		i++;
	}
	key[i] = '\0';
	return key;
}

int ScrapInt(char c) {
	char str[21]; str[0] = c; int i = 1;
	while ((c = getchar()) >= '0' && c <= '9') {
		str[i] = c;
		i++;
	}
	str[i] = '\0';
	ungetc(c, stdin);
	return atoi(str);
}

int ScrapIndex(char c) {
	char str[21]; str[0] = c; int i = 1;
	while ((c = getchar()) >= '0' && c <= '9') {
		str[i] = c;
		i++;
	}
	str[i] = '\0';
	return atoi(str);
}

char* ScrapKey() {
	char true = 1; char c; char* key = NULL;
	while (true) {
		c = getchar();
		if (c == '"') {
			key = ScrapString();
			true = 0;
			getchar();
			getchar();//key: ...
		}
	}
	return key;
}

int Hash(char* key) {
	int hash = 0; char c;
	if (key == NULL) {
		return 0;
	}
	while ((c = *key++) != '\0') {
		hash += c * 2;
	}
	return hash;
}

void AddHashTable(JsonObject* js, JsonObject** hashTable) {
	char* key = js->key;
	int hash = Hash(key);
	while (hashTable[hash % HASHTABLE_SIZE] != NULL) {
		hash++;
	}
	hashTable[hash % HASHTABLE_SIZE] = js;
}

JsonObject* FindElementByHash(char* key, JsonObject** hashTable, JsonObject* json) {
	int hash = Hash(key); char true = 1;
	while (true) {
		if (strcmp(hashTable[hash % HASHTABLE_SIZE]->key, key) == 0) {
			if (hashTable[hash % HASHTABLE_SIZE]->parent == json) {
				break;
			}
		}
		hash++;
	}
	return hashTable[hash % HASHTABLE_SIZE];
}

JsonObject* FindElementByIndex(JsonObject* json, int index) {
	JsonObject* current = json;
	for (int i = 0; i < index; i++) {
		current = current->next;
	}
	return current;
}

JsonObject* FindElement(JsonObject* json, JsonObject** hashTable) {
	JsonObject* js = json;
	char c;
	char* key = NULL;
	int index;
	Situation situation = Undefind;
	while ((c = getchar()) != '\n') {
		if (c == '"') {
			key = ScrapString();
			situation = FindByKey;
		}
		else if ((c >= '0' && c <= '9')) {
			index = ScrapInt(c);
			situation = FindByIndex;
		}
		if (situation == FindByKey) {
			js = FindElementByHash(key, hashTable, js);
			situation = Undefind;
		}
		else if (situation == FindByIndex) {
			js = js->Children.first;
			js = FindElementByIndex(js, index);
			situation = Undefind;
		}
	}
	return js;
}

JsonObject* CreateJsonObject(Type type, char* key, JsonObject* parent) {
	JsonObject* js = calloc(1, sizeof(JsonObject));
	js->type = type;
	js->key = key;
	if (!parent->Children.last) {
		parent->Children.first = parent->Children.last = js;
	}
	else {
		parent->Children.last->next = js;
		parent->Children.last = js;
	}
	js->parent = parent;
	parent->Children.length++;
	return js;
}

Status JsonParse(JsonObject* json, char* key, JsonObject** hashTable) {
	char true = 1; char c;
	JsonObject* js;
	while (true) {
		c = getchar();
		if (c == -22) {//ú
			break;
		}
		else if (c == ',' || c == '\n' || c == ' ') {
			continue;
		}
		else if (c == '{') {
			//create object
			js = CreateJsonObject(Object, key, json);
			if (key != NULL) {
				AddHashTable(js, hashTable);
			}
			while (true) {
				char* new_key;
				new_key = ScrapKey();
				Status end = JsonParse(js, new_key, hashTable);
				/*end object NEED make*/
				if ((c = getchar()) != ',') {
					if (c != '}') {
						while ((c = getchar()) != '}') {
							continue;
						}
					}
					return End;
				}
				else {
					continue;
				}
			}
		}
		else if (c == '[') {
			//create array
			js = CreateJsonObject(Array, key, json);
			if (key != NULL) {
				AddHashTable(js, hashTable);
			}
			while (true) {
				Status end = JsonParse(js, 0, hashTable);
				/*end object NEED make*/
				if ((c = getchar()) != ',') {
					if (c != ']') {
						while ((c = getchar()) != ']') {
							continue;
						}
					}
					return End;
				}
				else {
					continue;
				}
			}
		}
		else if (c == '"') {
			//string
			js = CreateJsonObject(String, key, json);
			if (key != NULL) {
				AddHashTable(js, hashTable);
			}
			js->text = ScrapString();
			return End;
		}
		else if ((c >= '0' && c <= '9') || c == '-') {
			//int
			js = CreateJsonObject(Number, key, json);
			if (key != NULL) {
				AddHashTable(js, hashTable);
			}
			js->num = ScrapInt(c);
			/*end object NEED make*/
			return End;

		}
	}
}

int PrintData(JsonObject* js) {
	if (js->type == String) {
		printf("%s\n", js->text);
	}
	else if (js->type == Number) {
		printf("%d\n", js->num);
	}
	else if (js->type == Object) {
		return;
	}
	else if (js->type == Array) {
		return;
	}
	return 0;
}

void PrintfTable(JsonObject** hashTable) {
	for (int i = 0; i < HASHTABLE_SIZE; i++) {
		if (hashTable[i]) {
			printf("%d: %s\n", i, hashTable[i]->key);
		}
		else {
			printf("%d: -1\n", i);
		}
	}
}

void PrintDataInFile(JsonObject* js) {
	FILE* mf;
	mf = fopen("C:/Users/dima/Desktop/projects/json_search_js/myAnswer.txt", "a");
	if (js->type == String) {
		fprintf(mf, "%s\n", js->text);
	}
	else if (js->type == Number) {
		fprintf(mf, "%d\n", js->num);
	}
	else if (js->type == Object) {
		return;
	}
	else if (js->type == Array) {
		return;
	}
	fclose(mf);
}

int main() {
	JsonObject** hashTable = calloc(HASHTABLE_SIZE, sizeof(JsonObject*));
	JsonObject json = { 0 };
	JsonParse(&json, 0, hashTable);
	JsonObject* trueJson = json.Children.first;

	//PrintfTable(hashTable);
	int N;
	scanf("%d\n", &N);
	for (int i = 0; i < N; i++) {
		JsonObject* js = FindElement(trueJson, hashTable);
		PrintData(js);

		//PrintDataInFile(js);
	}

	return 0;
}
