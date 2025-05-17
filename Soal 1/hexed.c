#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>

#define ZIP_FILE "anomali.zip"
#define FILE_URL "https://drive.usercontent.google.com/u/0/uc?id=1hi_GDdP51Kn2JJMw02WmCOxuc3qrXzh5&export=download"
#define UNZIP_FOLDER "unzipped"
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

void log_conversion(const char *text_filename, const char *image_filename, struct tm *t) {
    FILE *log = fopen(LOG_FILE, "a");
    if (log) {
        fprintf(log,
                "[%04d-%02d-%02d][%02d:%02d:%02d]: Successfully converted hexadecimal text %s to %s.\n",
                t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
                t->tm_hour, t->tm_min, t->tm_sec,
                text_filename, image_filename);
        fclose(log);
    }
}

void process_hex_file(const char *filename) {
    char full_path[256];
    snprintf(full_path, sizeof(full_path), "%s/%s", UNZIP_FOLDER, filename);

    FILE *in = fopen(full_path, "r");
    if (!in) return;

    create_directory(IMAGE_DIR);

    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    char basename[100];
    strncpy(basename, filename, strrchr(filename, '.') - filename);
    basename[strrchr(filename, '.') - filename] = '\0';

    char output_path[512];
    generate_output_filename(output_path, sizeof(output_path), basename, "png", t);

    FILE *out = fopen(output_path, "wb");
    if (!out) {
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
    log_conversion(filename, strrchr(output_path, '/') + 1, t);
}

int main() {
    printf("[1] Mengunduh file zip...\n");
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "wget -q -O %s \"%s\"", ZIP_FILE, FILE_URL);
    if (system(cmd) != 0) {
        fprintf(stderr, "Gagal download.\n");
        return 1;
    }

    printf("[2] Mengekstrak file zip...\n");
    create_directory(UNZIP_FOLDER);
    snprintf(cmd, sizeof(cmd), "unzip -o %s -d %s > /dev/null", ZIP_FILE, UNZIP_FOLDER);
    if (system(cmd) != 0) {
        fprintf(stderr, "Gagal unzip.\n");
        return 1;
    }

    printf("[3] Menghapus file zip...\n");
    remove(ZIP_FILE);

    printf("[4] Konversi semua file hex...\n");
    DIR *dir = opendir(UNZIP_FOLDER);
    if (!dir) {
        perror("Gagal buka direktori unzip");
        return 1;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strstr(entry->d_name, ".txt")) {
            process_hex_file(entry->d_name);
        }
    }

    closedir(dir);

    printf("[✔] Semua file berhasil dikonversi dan dicatat di conversion.log.\n");
    return 0;
}
