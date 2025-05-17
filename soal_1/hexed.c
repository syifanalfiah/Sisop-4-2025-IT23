#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>

#define ZIP_FILE "anomali.zip"
#define FILE_URL "https://drive.usercontent.google.com/u/0/uc?id=1hi_GDdP51Kn2JJMw02WmCOxuc3qrXzh5&export=download"
#define EXTRACT_FOLDER "anomali"
#define IMAGE_DIR "image"
#define LOG_FILE "conversion.log"

void create_directory(const char *dirname) {
    struct stat st = {0};
    if (stat(dirname, &st) == -1) {
        mkdir(dirname, 0700);
    }
}

unsigned char parse_byte(char high, char low) {
    char byte_str[3] = {high, low, '\0'};
    unsigned int byte_val;
    sscanf(byte_str, "%02x", &byte_val);
    return (unsigned char)byte_val;
}

void generate_output_filename(char *output_path, size_t size, const char *basename, const char *ext, struct tm *t) {
    snprintf(output_path, size, "%s/%s_image_%04d-%02d-%02d_%02d:%02d:%02d.%s",
             IMAGE_DIR, basename,
             t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
             t->tm_hour, t->tm_min, t->tm_sec,
             ext);
}

void log_conversion(const char *basename, const char *image_filename, struct tm *t) {
    FILE *log = fopen(LOG_FILE, "a");
    if (log) {
        fprintf(log, "[%04d-%02d-%02d][%02d:%02d:%02d]: Successfully converted hexadecimal text %s to %s.\n",
                t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
                t->tm_hour, t->tm_min, t->tm_sec,
                basename, image_filename);
        fclose(log);
    }
}

void convert_hex_file(const char *filepath, const char *basename) {
    FILE *in = fopen(filepath, "r");
    if (!in) {
        perror("Failed to open hex file");
        return;
    }

    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    char output_path[512];
    generate_output_filename(output_path, sizeof(output_path), basename, "png", t);

    FILE *out = fopen(output_path, "wb");
    if (!out) {
        perror("Failed to create image file");
        fclose(in);
        return;
    }

    char ch1, ch2;
    while ((ch1 = fgetc(in)) != EOF && (ch2 = fgetc(in)) != EOF) {
        unsigned char byte = parse_byte(ch1, ch2);
        fwrite(&byte, 1, 1, out);
    }

    fclose(in);
    fclose(out);

    log_conversion(basename, strrchr(output_path, '/') + 1, t);
    printf("[✔] Image saved as: %s\n", output_path);
}

int main() {
    printf("[1] Downloading zip file using wget...\n");
    char download_cmd[512];
    snprintf(download_cmd, sizeof(download_cmd), "wget -q -O %s \"%s\"", ZIP_FILE, FILE_URL);
    if (system(download_cmd) != 0) {
        fprintf(stderr, "Failed to download file.\n");
        return 1;
    }

    printf("[2] Extracting zip file to 'anomali/'...\n");
    create_directory(EXTRACT_FOLDER);
    char unzip_cmd[256];
    snprintf(unzip_cmd, sizeof(unzip_cmd), "unzip -j -o %s -d %s > /dev/null", ZIP_FILE, EXTRACT_FOLDER);
    if (system(unzip_cmd) != 0) {
        fprintf(stderr, "Failed to unzip file.\n");
        return 1;
    }

    printf("[3] Deleting zip file...\n");
    remove(ZIP_FILE);

    printf("[4] Converting all hex .txt files in 'anomali/' to images...\n");
    create_directory(IMAGE_DIR);

    DIR *dir = opendir(EXTRACT_FOLDER);
    if (!dir) {
        perror("Failed to open target folder");
        return 1;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG && strstr(entry->d_name, ".txt")) {
            char filepath[512];
            snprintf(filepath, sizeof(filepath), "%s/%s", EXTRACT_FOLDER, entry->d_name);

            char basename[256];
            strncpy(basename, entry->d_name, strcspn(entry->d_name, "."));
            basename[strcspn(entry->d_name, ".")] = '\0';

            printf("Converting %s...\n", entry->d_name);
            convert_hex_file(filepath, basename);
        }
    }

    closedir(dir);
    printf("[✔] All files converted successfully.\n");

    return 0;
}
