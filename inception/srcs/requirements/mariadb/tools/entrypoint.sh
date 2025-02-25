#!/bin/sh

tempfile=$(mktemp)

echo "
ALTER USER 'root'@'localhost' IDENTIFIED BY '${MYSQL_ROOT_PASSWORD}';
CREATE DATABASE IF NOT EXISTS wordpress;
CREATE USER IF NOT EXISTS '${MYSQL_USER}'@'localhost' IDENTIFIED BY '${MYSQL_PASSWORD}';
CREATE USER IF NOT EXISTS '${MYSQL_USER}'@'%' IDENTIFIED BY '${MYSQL_PASSWORD}';
GRANT ALL PRIVILEGES ON wordpress.* TO '${MYSQL_USER}'@'localhost';
GRANT ALL PRIVILEGES ON wordpress.* TO '${MYSQL_USER}'@'%';
FLUSH PRIVILEGES;
" > $tempfile

# Debugging information
echo "Running initialization script:"
cat $tempfile

exec mariadbd --no-defaults --user=root --datadir=/var/lib/mysql --init-file=$tempfile

rm -f $tempfile