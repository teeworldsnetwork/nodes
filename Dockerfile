FROM alpine:3.16 AS builder
WORKDIR /nodes
COPY . .
RUN sed -i 's/dl-cdn.alpinelinux.org/mirrors.ustc.edu.cn/g' /etc/apk/repositories
RUN apk update && apk upgrade && apk add g++ cmake make python3 curl-dev
RUN cmake -DCLIENT=OFF -DCMAKE_BUILD_TYPE=Release . && make -j

FROM alpine:3.16
WORKDIR /srv
COPY --from=builder /nodes/twnodes_srv .
COPY --from=builder /nodes/data/maps/ maps
COPY --from=builder /usr/lib/libgcc_s.so.1 /usr/lib/libstdc++.so.6 /usr/lib/libcurl.so.4 \
                    /usr/lib/libnghttp2.so.14 /usr/lib/libbrotlidec.so.1 /usr/lib/libbrotlicommon.so.1  /usr/lib/
CMD ["./twnodes_srv"]
