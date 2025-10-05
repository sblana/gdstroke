#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <malloc.h>


char *read_entire_file(char const *filename, int *out_length) {
	FILE *fp = fopen(filename, "rb");
	assert(fp);
	fseek(fp, 0, SEEK_END);
	int f_sz = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	char *file_content = (char*)malloc(f_sz + 1);
	size_t ret = fread(file_content, 1, f_sz, fp);
	assert(ret == f_sz);
	fclose(fp);
	*out_length = f_sz;
	return file_content;
}


// embed <input file> <output file> <variable name>
int main(int argc, char **argv) {
	assert(argc == 4);
	int input_size;
	char *input_data = read_entire_file(argv[1], &input_size);
	FILE *fp = fopen(argv[2], "wb");
	assert(fp);
	fprintf(fp, "int %s_LENGTH = %d;\n", argv[3], input_size);
	fprintf(fp, "char const *%s = \"", argv[3]);
	for (int i = 0; i < input_size; ++i) {
		fprintf(fp, "\\x%02hhx", input_data[i]);
	}
	fprintf(fp, "\";\n");
}
