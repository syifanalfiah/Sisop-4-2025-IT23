#define FUSE_USE_VERSION 30
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <time.h>

#define MOUNT_DIR "ExAid"
#define RELICS_DIR "relics"
#define LOG_FILE "activity.log"
#define MAX_PIECES 14
#define PIECE_SIZE 1024

// Fungsi untuk mencatat aktivitas
void log_activity(const char *action, const char *detail) {
    FILE *log_file = fopen(LOG_FILE, "a");
    if (!log_file) return;

    time_t now = time(NULL);
    struct tm *timeinfo = localtime(&now);
    char time_str[20];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", timeinfo);
    
    fprintf(log_file, "[%s] %s: %s\n", time_str, action, detail);
    fclose(log_file);
}

// Fungsi untuk memeriksa keberadaan direktori relics
void check_relics_dir() {
    struct stat st;
    if (stat(RELICS_DIR, &st) == -1) {
        fprintf(stderr, "Folder '%s' tidak ditemukan. Pastikan folder tersebut sudah ada.\n", RELICS_DIR);
        exit(EXIT_FAILURE);
    }
}

// Fungsi untuk membaca file gabungan
static int baymax_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    if (strcmp(path, "/Baymax.jpeg") != 0) {
        return -ENOENT;
    }

    char piece_path[256];
    size_t bytes_read = 0;

    for (int i = 0; i < MAX_PIECES; i++) {
        snprintf(piece_path, sizeof(piece_path), "%s/Baymax.jpeg.%03d", RELICS_DIR, i);
        FILE *piece_file = fopen(piece_path, "rb");

        if (!piece_file) break;

        fseek(piece_file, 0, SEEK_END);
        size_t piece_size = ftell(piece_file);
        fseek(piece_file, 0, SEEK_SET);

        size_t read_size = (size < piece_size) ? size : piece_size;
        fread(buf + bytes_read, 1, read_size, piece_file);
        bytes_read += read_size;
        fclose(piece_file);

        if (bytes_read >= size) break;
    }

    log_activity("READ", "Baymax.jpeg");
    return bytes_read;
}

// Fungsi untuk menampilkan direktori
static int baymax_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {

    if (strcmp(path, "/") != 0) {
        return -ENOENT;
    }

    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);
    filler(buf, "Baymax.jpeg", NULL, 0);

    return 0;
}

// Fungsi untuk menulis file baru
static int baymax_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    char file_name[256];
    snprintf(file_name, sizeof(file_name), "%s", path + 1);

    if (strchr(file_name, '.')) {
        return -EACCES;
    }

    char detail[512];
    snprintf(detail, sizeof(detail), "WRITE: %s", file_name);
    log_activity("WRITE", detail);

    size_t written_bytes = 0;
    int piece_index = 0;

    while (written_bytes < size) {
        char piece_path[256];
        snprintf(piece_path, sizeof(piece_path), "%s/%s.%03d", RELICS_DIR, file_name, piece_index);

        FILE *piece_file = fopen(piece_path, "wb");
        if (!piece_file) return -EIO;

        size_t chunk_size = (size - written_bytes < PIECE_SIZE) ? size - written_bytes : PIECE_SIZE;
        fwrite(buf + written_bytes, 1, chunk_size, piece_file);
        fclose(piece_file);

        written_bytes += chunk_size;
        piece_index++;
    }

    return size;
}

// Fungsi untuk menghapus file
static int baymax_unlink(const char *path) {
    char file_name[256];
    snprintf(file_name, sizeof(file_name), "%s", path + 1);

    char detail[512];
    snprintf(detail, sizeof(detail), "DELETE: %s", file_name);
    log_activity("DELETE", detail);

    for (int i = 0; i < MAX_PIECES; i++) {
        char piece_path[256];
        snprintf(piece_path, sizeof(piece_path), "%s/%s.%03d", RELICS_DIR, file_name, i);

        if (remove(piece_path) == 0) {
            char log_detail[512];
            snprintf(log_detail, sizeof(log_detail), "DELETE: %s.%03d", file_name, i);
            log_activity("DELETE", log_detail);
        }
    }

    return 0;
}

// Struktur operasi FUSE
static struct fuse_operations baymax_oper = {
    .readdir = baymax_readdir,
    .read = baymax_read,
    .write = baymax_write,
    .unlink = baymax_unlink,
};

// Fungsi main FUSE
int main(int argc, char *argv[]) {
    check_relics_dir();
    return fuse_main(argc, argv, &baymax_oper, NULL);
}
