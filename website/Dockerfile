FROM alpine:3.19

RUN apk update && apk upgrade

RUN addgroup -S gabrifer && adduser -S gabrifer -G gabrifer

USER gabrifer

COPY content/* /var/www/bonus-website/.

WORKDIR /var/www/bonus-website/
