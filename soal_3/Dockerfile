FROM ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    libfuse-dev \
    pkg-config \
    fuse \
    && rm -rf /var/lib/apt/lists/*

RUN mkdir -p /it24_host /antink_mount /var/log

COPY antink.c /root/antink.c

RUN gcc -o /usr/local/bin/antink /root/antink.c `pkg-config fuse --cflags --libs`

RUN touch /var/log/it24.log && chmod 666 /var/log/it24.log

CMD ["antink", "-f", "/antink_mount"]
