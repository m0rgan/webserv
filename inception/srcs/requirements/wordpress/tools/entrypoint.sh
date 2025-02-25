#!/bin/bash

curl -O https://raw.githubusercontent.com/wp-cli/builds/gh-pages/phar/wp-cli.phar

chmod +x wp-cli.phar

php -d memory_limit=256M ./wp-cli.phar core download

php -d memory_limit=256M ./wp-cli.phar config create --dbname=$MYSQL_DATABASE --dbuser=$MYSQL_USER --dbpass=$MYSQL_PASSWORD --dbhost=$MYSQL_HOST

php -d memory_limit=256M ./wp-cli.phar core install --url=localhost --title=inception --admin_user=$WP_ADMIN --admin_password=$WP_ADMIN_PASSWORD --admin_email=$WP_ADMIN_EMAIL

php -d memory_limit=256M ./wp-cli.phar user create $WP_USER $WP_USER_EMAIL --role=editor --user_pass=$WP_USER_PASSWORD

php-fpm82 -F

