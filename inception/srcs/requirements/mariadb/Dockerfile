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

RUN apk update && apk upgrade && apk add --no-cache mariadb mariadb-client mysql 

RUN mariadb-install-db --user=root --datadir=/var/lib/mysql --skip-test-db

RUN mkdir -p /run/mysqld && \
	chown -R root:root /run/mysqld && \
	chown -R root:root /var/lib/mysql

COPY --chown=root:root /tools/entrypoint.sh /etc/entrypoint.sh

RUN mysql_install_db --datadir=/var/lib/mysql

EXPOSE 3306

ENTRYPOINT ["sh", "/etc/entrypoint.sh"]