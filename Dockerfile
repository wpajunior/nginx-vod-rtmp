FROM alpine:3.4 AS builder  

RUN apk add --no-cache --virtual .build-deps \
  gcc \
  libc-dev \
  make \
  openssl-dev \
  pcre-dev \
  zlib-dev \
  linux-headers \
  curl \
  gnupg \
  libxslt-dev \
  gd-dev \
  geoip-dev \
  perl \
  git 

COPY . /tmp

WORKDIR /tmp/nginx-1.6.3

RUN ./configure --add-module=../nginx-rtmp-module-1.1.9/ \
  --add-module=../nginx-vod-module/ \
  --with-file-aio --with-cc-opt="-O3" \
  --with-http_stub_status_module  \
  --with-http_ssl_module \
  --conf-path=/etc/nginx/nginx.conf \
  --pid-path=/run/nginx.pid \
  --error-log-path=/dev/stderr \
  --http-log-path=/dev/stdout \
  --with-http_secure_link_module \
  --with-http_gzip_static_module \
  && make


FROM alpine:3.4

RUN addgroup -S -g 1001 nginx \
  && adduser -S  -G nginx -H -g "nginx user" -s /bin/false -u 1001 nginx \
  && apk --no-cache add pcre-dev \
  openssl \
  ca-certificates \
  && mkdir -p /usr/local/nginx/client_body_temp \
  /usr/local/nginx/fastcgi_temp \
  /usr/local/nginx/html \
  /usr/local/nginx/proxy_temp \
  /usr/local/nginx/sbin \
  /usr/local/nginx/scgi_temp \
  /usr/local/nginx/uwsgi_temp

COPY --from=builder /tmp/nginx-1.6.3/objs/nginx /usr/local/nginx/sbin/
COPY --from=builder /tmp/nginx-1.6.3/conf/ /etc/nginx/

EXPOSE 80 443

CMD ["/usr/local/nginx/sbin/nginx", "-g", "daemon off;"]
