#define FUSE_USE_VERSION 31

#include <fuse3/fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <time.h>
#include <stddef.h>
#include <limits.h>
#include <ctype.h>
#include <stdbool.h>

#define CHUNK_SIZE 1024

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#ifndef NAME_MAX
#define NAME_MAX 255
#endif

// ————————————————————————————————————————————————————
// Config & option parsing
// ————————————————————————————————————————————————————
struct baymax_config {
    const char *relics_dir;
    const char *log_path;
} baymax_cfg;

#define OPTION(t, p) { t, offsetof(struct baymax_config, p), 1 }

static struct fuse_opt baymax_opts[] = {
    OPTION("relics=%s", relics_dir),
    OPTION("logfile=%s", log_path),
    FUSE_OPT_END
};

// ————————————————————————————————————————————————————
// Utility: logging ke activity.log
// ————————————————————————————————————————————————————
static void log_activity(const char *fmt, ...)
{
    va_list ap;
    FILE *logf = fopen(baymax_cfg.log_path, "a");
    if (!logf) return;
    time_t now = time(NULL);
    struct tm tm;
    localtime_r(&now, &tm);
    char buf[64];
    strftime(buf, sizeof(buf), "[%Y-%m-%d %H:%M:%S]", &tm);
    fprintf(logf, "%s ", buf);

    va_start(ap, fmt);
    vfprintf(logf, fmt, ap);
    va_end(ap);

    fprintf(logf, "\n");
    fclose(logf);
}

// ————————————————————————————————————————————————————
// FUSE callbacks (stubs)
// ————————————————————————————————————————————————————
static void *baymax_init(struct fuse_conn_info *conn,
                         struct fuse_config *cfg)
{
    (void) conn;
    // misal: enable caching
    cfg->kernel_cache = 1;
    return NULL;
}

static off_t get_virtual_size(const char *name)
{
    char path[PATH_MAX];
    struct stat st;
    off_t total = 0;
    for (int idx = 0; ; idx++) {
        snprintf(path, sizeof(path), "%s/%s.%03d",
                 baymax_cfg.relics_dir, name, idx);
        if (stat(path, &st) < 0)
            break;
        total += st.st_size;
    }
    return total;
}

// ————————————————————————————————————————————————————
// FUSE callback: getattr
// ————————————————————————————————————————————————————
static int baymax_getattr(const char *path, struct stat *st,
                          struct fuse_file_info *fi)
{
    (void) fi;
    // root directory
    if (strcmp(path, "/") == 0) {
        st->st_mode  = S_IFDIR | 0755;
        st->st_nlink = 2;
        return 0;
    }

    // virtual filename (skip leading '/')
    const char *fname = path + 1;

    // cek apakah ada fragment relics_dir/fname.000
    char frag0[PATH_MAX];
    snprintf(frag0, sizeof(frag0), "%s/%s.000",
             baymax_cfg.relics_dir, fname);
    if (access(frag0, F_OK) == 0) {
        off_t size = get_virtual_size(fname);
        st->st_mode  = S_IFREG | 0644;
        st->st_nlink = 1;
        st->st_size  = size;
        return 0;
    }

    return -ENOENT;
}

// ————————————————————————————————————————————————————
// FUSE callback: readdir
// ————————————————————————————————————————————————————
static int baymax_readdir(const char *path, void *buf,
                          fuse_fill_dir_t filler,
                          off_t offset,
                          struct fuse_file_info *fi,
                          enum fuse_readdir_flags flags)
{
    (void) offset; (void) fi; (void) flags;

    if (strcmp(path, "/") != 0)
        return -ENOENT;

    // “.” dan “..”
    filler(buf, ".",  NULL, 0, 0);
    filler(buf, "..", NULL, 0, 0);

    // scan relics_dir untuk cari semua prefix unik
    DIR *dp = opendir(baymax_cfg.relics_dir);
    if (!dp)
        return -errno;

    struct dirent *de;
    char names[256][NAME_MAX+1];
    int  n_names = 0;

    while ((de = readdir(dp)) != NULL) {
        // cari entry yang berformat nama.XXX (XXX digit)
        char *dot = strrchr(de->d_name, '.');
        if (!dot || strlen(dot+1) != 3) continue;
        if (!isdigit(dot[1]) || !isdigit(dot[2]) || !isdigit(dot[3]) || dot[4] != '\0') continue;

        size_t base_len = dot - de->d_name;
        // copy prefix
        char base[NAME_MAX+1];
        strncpy(base, de->d_name, base_len);
        base[base_len] = '\0';

        // periksa duplikat
        bool seen = false;
        for (int i = 0; i < n_names; i++) {
            if (strcmp(names[i], base) == 0) {
                seen = true;
                break;
            }
        }
        if (!seen && n_names < 256) {
            strncpy(names[n_names++], base, NAME_MAX);
        }
    }
    closedir(dp);

    // isikan setiap virtual file
    for (int i = 0; i < n_names; i++) {
        filler(buf, names[i], NULL, 0, 0);
    }

    return 0;
}

// ————————————————————————————————————————————————————
// FUSE callback: open
// ————————————————————————————————————————————————————
static int baymax_open(const char *path, struct fuse_file_info *fi)
{
    // tidak boleh buka direktori
    if (strcmp(path, "/") == 0)
        return -EISDIR;

    // nama file tanpa leading “/”
    const char *fname = path + 1;

    // cek apakah fragmen pertama ada
    char frag0[PATH_MAX];
    snprintf(frag0, sizeof(frag0), "%s/%s.000",
             baymax_cfg.relics_dir, fname);
    if (access(frag0, F_OK) != 0)
        return -ENOENT;

    // log aktivitas READ
    log_activity("READ: %s", fname);
    return 0;
}

// ————————————————————————————————————————————————————
// FUSE callback: read
// ————————————————————————————————————————————————————
static int baymax_read(const char *path, char *buf,
                       size_t size, off_t offset,
                       struct fuse_file_info *fi)
{
    (void) fi;
    const char *fname = path + 1;

    // total panjang file virtual
    off_t total_size = get_virtual_size(fname);
    if (offset >= total_size)
        return 0;
    if (offset + size > total_size)
        size = total_size - offset;

    size_t bytes_read = 0;
    int idx = offset / CHUNK_SIZE;
    off_t off_in_frag = offset % CHUNK_SIZE;
    char frag_path[PATH_MAX];
    struct stat st;

    while (bytes_read < size) {
        // path fragmen ke-idx
        snprintf(frag_path, sizeof(frag_path), "%s/%s.%03d",
                 baymax_cfg.relics_dir, fname, idx);

        // kalau fragmen sudah tidak ada, selesai
        if (stat(frag_path, &st) < 0)
            break;

        // buka fragmen
        int fd = open(frag_path, O_RDONLY);
        if (fd < 0)
            break;

        // hitung berapa byte bisa dibaca dari fragmen ini
        size_t avail = st.st_size - off_in_frag;
        size_t to_copy = avail;
        if (to_copy > size - bytes_read)
            to_copy = size - bytes_read;

        // pread ke buffer akhir
        pread(fd, buf + bytes_read, to_copy, off_in_frag);
        close(fd);

        bytes_read   += to_copy;
        off_in_frag   = 0;      // setelah frag pertama, baca dari offset 0
        idx++;
    }

    return bytes_read;
}

static int baymax_create(const char *path, mode_t mode,
                         struct fuse_file_info *fi)
{
    return -EROFS;
}

static int baymax_write(const char *path, const char *buf, size_t size,
                        off_t offset, struct fuse_file_info *fi)
{
    return -EROFS;
}

static int baymax_truncate(const char *path, off_t size,
                           struct fuse_file_info *fi)
{
    return -EROFS;
}

static int baymax_release(const char *path, struct fuse_file_info *fi)
{
    return 0;  // optional: bisa dibiarkan
}

static int baymax_unlink(const char *path)
{
    return -EROFS;
}

// (opsional) flush, fsync, dll…

// ————————————————————————————————————————————————————
// FUSE operations struct
// ————————————————————————————————————————————————————
static struct fuse_operations baymax_oper = {
    .init       = baymax_init,
    .getattr    = baymax_getattr,
    .readdir    = baymax_readdir,
    .open       = baymax_open,
    .read       = baymax_read
    // .create, .write, .truncate, .release, .unlink tidak dipasang dulu
};

// ————————————————————————————————————————————————————
// Main
// ————————————————————————————————————————————————————
int main(int argc, char *argv[])
{
    struct fuse_args args = FUSE_ARGS_INIT(argc, argv);

    if (fuse_opt_parse(&args, &baymax_cfg, baymax_opts, NULL) == -1)
        return 1;

    if (!baymax_cfg.relics_dir || !baymax_cfg.log_path) {
        fprintf(stderr, "Usage: %s -orelics=<dir> -ologfile=<file> <mountpoint>\n",
                argv[0]);
        return 1;
    }

    return fuse_main(args.argc, args.argv, &baymax_oper, NULL);
}
