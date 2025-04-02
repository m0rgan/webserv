FROM alpine:3.19

RUN apk update && apk upgrade && \
apk add --no-cache php-cgi python3

COPY content/* /var/www/bonus-website/.

WORKDIR /var/www/bonus-website/

RUN touch entries.txt

RUN mkdir -p sessions && chmod 777 sessions

RUN chmod +x index.php index.py