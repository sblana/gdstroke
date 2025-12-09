#ifdef DEFINE_EMBEDDED_DATA_TYPE
	#ifndef EMBEDDED_DATA_TYPE_DEFINED
		#define EMBEDDED_DATA_TYPE_DEFINED

		struct EmbeddedData {
			uint32_t length;
			uint8_t data[];
		};

	#endif
#else

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
int embed(char const *input_file, char const * output_file, char const * variable_name) {

	int input_size;
	char *input_data = read_entire_file(input_file, &input_size);
	FILE *fp = fopen(output_file, "wb");
	assert(fp);

	fprintf(fp, "#ifndef EMBEDDED_DATA_TYPE_DEFINED\n");
	fprintf(fp, "\t#define EMBEDDED_DATA_TYPE_DEFINED\n");
	fprintf(fp, "\n");
	fprintf(fp, "\tstruct EmbeddedData {\n");
	fprintf(fp, "\t\tuint32_t length;\n");
	fprintf(fp, "\t\tuint8_t data[];\n");
	fprintf(fp, "\t};\n");
	fprintf(fp, "#endif\n");
	fprintf(fp, "\n");
	fprintf(fp, "struct EmbeddedData %s = {\n", variable_name);
	fprintf(fp, "\t/*.length*/ %d,\n", input_size);
	fprintf(fp, "\t/*.data*/ {");
	for (int i = 0; i < input_size; ++i) {
		fprintf(fp, "%s0x%02hhx,", ( (i % 32) ? (" ") : ("\n\t\t") ), input_data[i]);
	}
	fprintf(fp, "\n\t}\n};");

	fclose(fp);
	return 0;
}

#ifdef EMBED_MAIN

int main(int argc, char **argv) {
	assert(argc == 4);
	char const *   input_file = argv[1];
	char const *  output_file = argv[2];
	char const *variable_name = argv[3];
	return embed(input_file, output_file, variable_name);
}

#endif

#endif