# Laporan Praktikum Modul 3

## Nama Anggota

| Nama                        | NRP        |
| --------------------------- | ---------- |
| Syifa Nurul Alfiah          | 5027241019 |
| Alnico Virendra Kitaro Diaz | 5027241081 |
| Hafiz Ramadhan              | 5027241096 |

## Soal No 1

## Soal No 2

## Soal No 3

# AntiNK FUSE Filesystem

### Deskripsi
Nafis dan Kimcun merupakan dua mahasiswa anomali😱 yang paling tidak tahu sopan santun dan sangat berbahaya di antara angkatan 24. Maka dari itu, Pujo sebagai komting yang baik hati dan penyayang😍, memutuskan untuk membuat sebuah sistem pendeteksi kenakalan bernama Anti Napis Kimcun (AntiNK) untuk melindungi file-file penting milik angkatan 24.

### Fitur Utama:
- **Proteksi File Berbahaya**: Mendeteksi dan menyamarkan file yang mengandung kata "nafis" atau "kimcun"
- **Enkripsi ROT13**: Mengenkripsi konten file .txt secara otomatis
- **Logging Komprehensif**: Mencatat semua aktivitas filesystem dalam log
- **Containerized**: Berjalan dalam environment Docker yang terisolasi
- **Real-time Monitoring**: Monitoring log secara real-time

### Struktur File

```
antink-fuse/
├── Dockerfile              
├── docker-compose.yml      
├── antink.c               
├── it24_host/           
├── antink_mount/          
└── antink-logs/            
```

### Fungsi Utama

#### 1. Deteksi File Berbahaya

```c
bool is_dangerous_file(const char *filename) {
    char *lower = strdup(filename);
    for (int i = 0; lower[i]; i++) lower[i] = tolower(lower[i]);
    
    bool dangerous = (strstr(lower, "nafis") || strstr(lower, "kimcun"));
    free(lower);
    return dangerous;
}
```

Fungsi ini mengecek apakah nama file mengandung kata-kata berbahaya ("nafis" atau "kimcun") dengan cara:
- Mengkonversi nama file ke lowercase
- Mencari substring berbahaya
- Mengembalikan true jika ditemukan

#### 2. Reverse Filename

```c
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
```

Fungsi untuk membalik nama file secara karakter demi karakter sebagai cara penyamaran file berbahaya.

#### 3. ROT13 Encryption

```c
void rot13(char *str) {
    for (int i = 0; str[i]; i++) {
        if (isalpha(str[i])) {
            if ((tolower(str[i]) - 'a') < 13) str[i] += 13;
            else str[i] -= 13;
        }
    }
}
```

Implementasi algoritma ROT13 untuk mengenkripsi konten file dengan menggeser setiap huruf sebanyak 13 posisi dalam alfabet.

#### 4. Logging System

```c
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
```

Sistem logging yang mencatat setiap operasi dengan timestamp dan informasi operasi yang dilakukan.

#### 5. FUSE Operations

##### Get Attributes

```c
static int antink_getattr(const char *path, struct stat *stbuf) {
    char fullpath[PATH_MAX];
    snprintf(fullpath, PATH_MAX, "%s%s", host_dir, path);
    int res = lstat(fullpath, stbuf);
    write_log("GETATTR", path);
    return res == -1 ? -errno : 0;
}
```

Mengambil atribut file/direktori dan mencatat operasi ke log.

##### Read Directory

```c
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
```

Membaca isi direktori dengan melakukan penyamaran nama file berbahaya dan logging.

##### Open File

```c
static int antink_open(const char *path, struct fuse_file_info *fi) {
    char fullpath[PATH_MAX];
    snprintf(fullpath, PATH_MAX, "%s%s", host_dir, path);
    int res = open(fullpath, fi->flags);
    write_log("OPEN", path);
    return res == -1 ? -errno : 0;
}
```

Membuka file dan mencatat operasi pembukaan.

##### Read File

```c
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
```

Membaca konten file dengan enkripsi ROT13 untuk file .txt yang tidak berbahaya.

#### 6. Main Function

```c
int main(int argc, char *argv[]) {
    FILE *log = fopen(log_path, "a");
    if (log) {
        fprintf(log, "=== AntiNK Started ===\n");
        fclose(log);
    }
    
    return fuse_main(argc, argv, &antink_oper, NULL);
}
```

Fungsi utama yang menginisialisasi log dan menjalankan FUSE filesystem.

### Cara Penggunaan

#### Build dan Jalankan Sistem

Di terminal (posisi direktori sudah di dalam `soal_3/`), jalankan perintah ini:

```bash
docker-compose up --build
```

**Apa yang terjadi?**

* Docker akan **membangun container** dari `Dockerfile`.
* `docker-compose.yml` akan menjalankan dua container:

  * `server`: untuk menjalankan FUSE dan monitoring file.
  * `client`: untuk mengakses dan tes fitur AntiNK.
* Container `logger` akan **memantau log** secara real-time.

---

#### 4. Tes Deteksi File Anomali

Kita bisa cek apakah file “nafis” dan “kimcun” terdeteksi.

```bash
docker exec antink-server ls /antink_mount
```

**Contoh Output:**

```
txt.nucmik  test.txt  sifan.vsc
```

File “kimcun.txt” otomatis diubah jadi `txt.nucmik`, karena mengandung kata terlarang.

---

#### 5. Cek Isi File dan Enkripsi

```bash
docker exec antink-server cat /antink_mount/test.txt
```

Kalau file **normal**, maka isinya akan dienkripsi pakai ROT13.
Kalau file **berbahaya**, maka tampil apa adanya (tidak dienkripsi).

---

#### 6. Pantau Log Kenakalan

```bash
docker exec antink-logger tail -f /var/log/it24.log
```

Kalau ada file aneh, log akan mencatat:

```
[WARNING] Detected forbidden name: nafis.txt
```

### Client Operations

#### Deteksi File Berbahaya
- File dengan nama mengandung "nafis" atau "kimcun" akan disamarkan
- Nama file akan ditampilkan terbalik di listing direktori
- Operasi dicatat dalam log dengan tag "DANGER_DETECTED"

#### Enkripsi Konten
- File .txt yang tidak berbahaya akan dienkripsi dengan ROT13
- Enkripsi dilakukan saat pembacaan file
- File asli tidak berubah, hanya output yang dienkripsi

#### Monitoring dan Logging
- Semua operasi dicatat dengan timestamp
- Log tersimpan di `/var/log/it24.log`
- Container terpisah untuk monitoring real-time

### Keamanan dan Fitur

#### Proteksi Tingkat Container
- Menggunakan `privileged: true` untuk akses FUSE
- `SYS_ADMIN` capability untuk operasi filesystem
- AppArmor unconfined untuk fleksibilitas

#### Volume Management
- Source directory di-mount read-only
- Mount point dengan sharing antar container
- Log directory dengan akses write

#### Network Isolation
- Custom bridge network untuk komunikasi antar container
- Isolasi dari host network untuk keamanan

### Troubleshooting

#### FUSE Mount Issues
```bash
ls -la /dev/fuse

docker inspect antink-server | grep -i priv
```

#### Log Issues
```bash
docker exec antink-server ls -la /var/log/

docker exec antink-server cat /var/log/it24.log
```

#### Container Communication
```bash
docker network ls
docker network inspect antink-fuse_antink-network
```

### Revisi
Tidak ada.

### Kendala
Bukan kendala tapi lebih baik jangan lupa untuk 
```bash
docker-compose down
```
untuk menghapus container.

### Dokumentasi

## Soal No 4
