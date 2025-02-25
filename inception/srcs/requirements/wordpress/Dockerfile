FROM alpine:3.19

ENV MYSQL_USER=gabriel_sql
ENV MYSQL_DATABASE=wordpress
ENV MYSQL_HOST=mariadb
ENV WP_ADMIN=gabriel_wp
ENV WP_ADMIN_EMAIL=gabrifer@42.fr
ENV WP_USER=octavio_wp
ENV WP_USER_EMAIL=octavio@42.fr
ENV MYSQL_PASSWORD=gabriel
ENV MYSQL_ROOT_PASSWORD=gabriel
ENV WP_ADMIN_PASSWORD=gabriel
ENV WP_USER_PASSWORD=octavio

RUN apk update && apk upgrade && \
	apk add --no-cache php php-fpm php-mysqli php-phar php-mbstring php-tokenizer curl && \
	mkdir -p /var/www/html && \
	chown -R root:root /var/www/html /etc/php82/php-fpm.d /var/log/php82

RUN sed -i 's/listen = 127.0.0.1:9000/listen = wordpress:9000/' /etc/php82/php-fpm.d/www.conf

WORKDIR /var/www/html

COPY --chown=root:root /tools/entrypoint.sh /etc/entrypoint.sh

RUN chmod +x /etc/entrypoint.sh


ENTRYPOINT ["sh", "/etc/entrypoint.sh"]
