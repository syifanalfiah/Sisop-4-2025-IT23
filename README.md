# Laporan Praktikum Modul 3

## Nama Anggota

| Nama                        | NRP        |
| --------------------------- | ---------- |
| Syifa Nurul Alfiah          | 5027241019 |
| Alnico Virendra Kitaro Diaz | 5027241081 |
| Hafiz Ramadhan              | 5027241096 |

## Soal No 1

🧩 Konversi Hexadecimal ke Gambar PNG

Program ini ditulis dalam bahasa C dan berfungsi untuk:

- 📥 Mengunduh file ZIP dari Google Drive
- 📂 Mengekstrak file `.txt` yang berisi data string hexadecimal
- 🖼️ Mengonversi isi file `.txt` menjadi file gambar `.png`
- 📁 Menyimpan hasil gambar ke dalam folder `image/`
- 📝 Mencatat seluruh aktivitas konversi ke file log `conversion.log`

---

## 📁 Struktur Direktori

.
├── anomali.zip # File ZIP hasil unduhan
├── anomali/ # Folder hasil ekstraksi
│ ├── 1.txt
│ ├── 2.txt
│ └── ...
├── image/ # Hasil file PNG
│ ├── 1_image_2025-05-22_21:55:12.png
│ └── ...
├── conversion.log # Log hasil konversi
└── program.c # Source code program

yaml
Salin
Edit

---

## 🔍 Penjelasan Fungsi

### `create_directory(const char *name)`
Membuat folder jika belum ada (misal: `anomali/`, `image/`).

---

### `parse_byte(char h, char l)`
Menggabungkan 2 karakter hex (`h`, `l`) menjadi 1 byte.

Contoh:
```c
parse_byte('4', '1'); // menghasilkan 0x41
generate_filename(...)
Membuat nama file PNG dengan format:

css
Salin
Edit
[nama_file]_image_[YYYY-mm-dd]_[HH:MM:SS].png
log_conversion(...)
Mencatat log konversi ke file conversion.log dalam format:

css
Salin
Edit
[YYYY-MM-DD][HH:MM:SS]: Successfully converted hexadecimal text 1.txt to 1_image_2025-05-22_21:55:12.png.
convert_file(...)
Fungsi utama konversi:

Membaca file .txt karakter demi karakter

Mengubah setiap 2 karakter hex menjadi 1 byte

Menyimpan byte ke file .png

Menulis log konversi

🧠 Alur Program main()
🔽 Download ZIP

bash
Salin
Edit
wget -q -O anomali.zip "<URL>"
🗃️ Ekstrak File ZIP

bash
Salin
Edit
unzip -j -o anomali.zip -d anomali/
🧹 Hapus File ZIP

bash
Salin
Edit
remove("anomali.zip");
🔄 Proses Semua .txt

Buka direktori anomali/

Konversi semua file .txt menjadi .png

Simpan di image/

Catat ke conversion.log

🚀 Cara Menjalankan
1. Kompilasi Program
bash
Salin
Edit
gcc program.c -o hex_converter
2. Jalankan Program
bash
Salin
Edit
./hex_converter
3. Hasil Akhir
Gambar .png berada di folder image/

Log konversi tercatat di conversion.log

📝 Catatan Tambahan
Folder anomali/ dan image/ dibuat otomatis jika belum ada

Nama file gambar disesuaikan dengan timestamp agar unik

Program ini cocok untuk kebutuhan forensik, ekstraksi data, atau rekonstruksi file berbasis hex

```

## Soal No 2

## Soal No 3

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
![image](https://github.com/user-attachments/assets/60b1fea9-d16e-440a-b021-2bd038e516e5)
![image](https://github.com/user-attachments/assets/338143a6-50e9-4248-9ff7-022cad63bbec)
![image](https://github.com/user-attachments/assets/025b40fc-7a99-4e0c-ad06-ff38c2d15837)
![image](https://github.com/user-attachments/assets/4eb332c8-0b0c-482e-8161-ac6a98c690ca)
![image](https://github.com/user-attachments/assets/c8e7ee83-8022-479f-bf8b-4a6228bed8f2)
![image](https://github.com/user-attachments/assets/07c5bd2d-2b70-4fce-aee6-1505ed29e797)

## Soal No 4
