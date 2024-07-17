#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <limits.h>

size_t resource_name_len;
const char* namespace;
const char* resource_name;

char* root_resource(const char* lib) {
    if (lib == NULL) return NULL;

    size_t len = strlen(lib);
    char* result = malloc(len + 1);  // +1 null terminator
    if (result == NULL) return NULL;

    size_t j = 0;

    for (size_t i = 0; i < len; i++) {
        if (lib[i] == '-') {
            result[j++] = '_';
        } else if (lib[i] == '.') {
            result[j++] = '/';
        } else {
            result[j++] = lib[i];
        }
    }

    result[j] = '\0';
    return result;
}

char* namespace_name(char* clj_content) {
    char* namespace_start = strstr(clj_content, "(ns ");
    if (namespace_start != NULL) {
        namespace_start += 4; // Skip "(ns "
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


int verify (char* full_path) {
    long buffer_size = 0;
    char *clj_content = NULL;

    FILE *file = fopen(full_path, "r");
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
        if (strcmp(namespace, extracted_namespace) == 0) {
            printf("%s\n", full_path);
        }

        free(extracted_namespace);
    } else {
        printf("Namespace not found or memory allocation failed.\n");
    }
    fclose(file);

    free(clj_content);
    return 0;
}

void search_files(const char* path) {
    DIR* dir;
    struct dirent* entry;
    char full_path[PATH_MAX];

    dir = opendir(path);
    if (dir == NULL) {
        printf("Cannot open directory: %s\n", path);
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);

        if (entry->d_type == DT_DIR) {
            search_files(full_path);
        } else if (entry->d_type == DT_REG || entry->d_type == DT_UNKNOWN) {
            size_t full_path_len = strlen(full_path);
            if ((full_path_len > resource_name_len + 4 &&
                    strncmp(full_path + full_path_len - resource_name_len - 4, resource_name, resource_name_len) == 0 &&
                    strncmp(full_path + full_path_len - 4, ".clj", 4) == 0) ||
                (full_path_len > resource_name_len + 5 &&
                    strncmp(full_path + full_path_len - resource_name_len - 5, resource_name, resource_name_len) == 0 &&
                    (strncmp(full_path + full_path_len - 5, ".cljc", 5) == 0||
                     strncmp(full_path + full_path_len - 5, ".cljs", 5) == 0))) {
                verify(full_path);
            }
        }
    }

    closedir(dir);
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Usage: %s <resource> <path>\n", argv[0]);
        return 1;
    }

    namespace = argv[1];
    const char* path = argv[2];

    resource_name = root_resource(namespace);
    if (resource_name == NULL) {
        printf("Failed to get root resource\n");
        return 1;
    }
    resource_name_len = strlen(resource_name);

    search_files(path);

    return 0;
}
