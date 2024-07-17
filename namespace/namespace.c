#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* namespace_name(char* clj_content) {
    char* namespace_start = strstr(clj_content, "(ns ");
    if (namespace_start != NULL) {
        namespace_start += 4; // Skip "(ns "
        /* char* namespace_end = strchr(namespace_start, ')'); */
        char* namespace_end = namespace_start;
        while (*namespace_end != ' ' && *namespace_end != ')' && *namespace_end != '\n' && *namespace_end != '\r'  && *namespace_end != '\0') {
            namespace_end++;
        }
        if (namespace_end != NULL) {
            size_t length = namespace_end - namespace_start;// -1;
            char* namespace_name = (char*)malloc(length + 1);
            if (namespace_name != NULL) {
                strncpy(namespace_name, namespace_start, length);
                namespace_name[length] = '\0';
                return namespace_name;
            }
        }
    }
    return NULL; // Namespace not found or memory allocation failed
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <input.clj> [<input2.clj> ...]\n", argv[0]);
        return 1;
    }
    long buffer_size = 0;
    char *clj_content = NULL;
    for (int file_index = 1; file_index < argc; file_index++) {
        FILE *file = fopen(argv[file_index], "r");
        if (file == NULL) {
            perror("Error opening file");
            return 1;
        }

        // Find the file size
        fseek(file, 0, SEEK_END);
        long file_size = ftell(file);
        fseek(file, 0, SEEK_SET);

        // Allocate memory for the entire file
        if (file_size > buffer_size){
            clj_content = (char *)realloc(clj_content, file_size + 1);
        }
        if (clj_content == NULL) {
            perror("Memory allocation failed");
            fclose(file);
            return 1;
        }

        // Read the entire file into clj_content
        size_t bytes_read = fread(clj_content, 1, file_size, file);
        if (bytes_read != (size_t)file_size) {
            perror("Error reading file");
            fclose(file);
            free(clj_content);
            return 1;
        }

        // Null-terminate the string
        clj_content[file_size] = '\0';

        char* extracted_namespace = namespace_name(clj_content);
        if (extracted_namespace != NULL) {
            printf("%s\n", extracted_namespace);
            free(extracted_namespace);
        } else {
            printf("Namespace not found or memory allocation failed.\n");
        }
        fclose(file);

    }
    free(clj_content);
    return 0;
}
