FROM ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive

# ✅ Install dependensi yang diperlukan
RUN apt-get update && apt-get install -y \
    build-essential \
    libfuse-dev \
    pkg-config \
    fuse \
    && rm -rf /var/lib/apt/lists/*

# ✅ Buat direktori untuk mount point dan direktori host
RUN mkdir -p /it24_host /antink_mount /var/log

# ✅ Salin file FUSE ke dalam container
COPY antink.c /root/antink.c

# ✅ Kompilasi program FUSE
RUN gcc -o /usr/local/bin/antink /root/antink.c `pkg-config fuse --cflags --libs`

# ✅ Buat file log
RUN touch /var/log/it24.log && chmod 666 /var/log/it24.log

# (optional) kalau kamu perlu mount FUSE, pastikan container dijalankan pakai `--privileged`
# 🟡 Ekspos port hanya diperlukan jika program pakai network (FUSE gak perlu ini)
# EXPOSE 9999

# ✅ Jalankan antink saat container start
CMD ["antink", "-f", "/antink_mount"]
