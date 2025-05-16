#define FUSE_USE_VERSION 30
#include <fuse.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <stdbool.h>
#include <ctype.h>
#include <limits.h>

static const char *host_dir = "/it24_host";
static const char *log_path = "/var/log/it24.log";

// Logging function
void write_log(const char *action, const char *path) {
    FILE *log_file = fopen(log_path, "a");
    if (log_file) {
        time_t now = time(NULL);
        struct tm *t = localtime(&now);
        char time_str[100];
        strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", t);
        fprintf(log_file, "[%s] %s: %s\n", time_str, action, path);
        fclose(log_file);
    }
}

// Check for dangerous files
bool is_dangerous_file(const char *filename) {
    char *lower = strdup(filename);
    for (int i = 0; lower[i]; i++) lower[i] = tolower(lower[i]);
    
    bool dangerous = (strstr(lower, "nafis") || strstr(lower, "kimcun"));
    free(lower);
    return dangerous;
}

// Reverse filename
char* reverse_filename(const char *filename) {
    char *reversed = strdup(filename);
    int len = strlen(reversed);
    for (int i = 0; i < len/2; i++) {
        char temp = reversed[i];
        reversed[i] = reversed[len-1-i];
        reversed[len-1-i] = temp;
    }
    return reversed;
}

// ROT13 encryption
void rot13(char *str) {
    for (int i = 0; str[i]; i++) {
        if (isalpha(str[i])) {
            if ((tolower(str[i]) - 'a') < 13) str[i] += 13;
            else str[i] -= 13;
        }
    }
}

// FUSE operations
static int antink_getattr(const char *path, struct stat *stbuf) {
    char fullpath[PATH_MAX];
    snprintf(fullpath, PATH_MAX, "%s%s", host_dir, path);
    int res = lstat(fullpath, stbuf);
    write_log("GETATTR", path);
    return res == -1 ? -errno : 0;
}

static int antink_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                         off_t offset, struct fuse_file_info *fi) {
    char fullpath[PATH_MAX];
    snprintf(fullpath, PATH_MAX, "%s%s", host_dir, path);
    
    DIR *dp = opendir(fullpath);
    if (!dp) return -errno;
    
    struct dirent *de;
    while ((de = readdir(dp))) {
        struct stat st;
        memset(&st, 0, sizeof(st));
        st.st_ino = de->d_ino;
        st.st_mode = de->d_type << 12;
        
        char *display_name;
        if (is_dangerous_file(de->d_name)) {
            display_name = reverse_filename(de->d_name);
            write_log("DANGER_DETECTED", de->d_name);
        } else {
            display_name = strdup(de->d_name);
        }
        
        filler(buf, display_name, &st, 0);
        free(display_name);
    }
    
    closedir(dp);
    return 0;
}

static int antink_open(const char *path, struct fuse_file_info *fi) {
    char fullpath[PATH_MAX];
    snprintf(fullpath, PATH_MAX, "%s%s", host_dir, path);
    int res = open(fullpath, fi->flags);
    write_log("OPEN", path);
    return res == -1 ? -errno : 0;
}

static int antink_read(const char *path, char *buf, size_t size, off_t offset,
                      struct fuse_file_info *fi) {
    char fullpath[PATH_MAX];
    snprintf(fullpath, PATH_MAX, "%s%s", host_dir, path);
    
    int fd = open(fullpath, O_RDONLY);
    if (fd == -1) return -errno;
    
    int res = pread(fd, buf, size, offset);
    if (res > 0 && !is_dangerous_file(path) && strstr(path, ".txt")) {
        rot13(buf);
    }
    
    write_log("READ", path);
    close(fd);
    return res == -1 ? -errno : res;
}

static struct fuse_operations antink_oper = {
    .getattr = antink_getattr,
    .readdir = antink_readdir,
    .open = antink_open,
    .read = antink_read,
};

int main(int argc, char *argv[]) {
    // Initialize log file
    FILE *log = fopen(log_path, "a");
    if (log) {
        fprintf(log, "=== AntiNK Started ===\n");
        fclose(log);
    }
    
    return fuse_main(argc, argv, &antink_oper, NULL);
}
