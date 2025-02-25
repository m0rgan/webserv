# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: gabrielfernandezleroux <gabrielfernande    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/09/06 20:19:18 by gabrielfern       #+#    #+#              #
#    Updated: 2024/09/06 20:19:18 by gabrielfern      ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

all:
	docker-compose -p inception -f srcs/docker-compose.yml up --build

up:
	docker-compose -p inception -f srcs/docker-compose.yml up

down:
	docker-compose -p inception -f srcs/docker-compose.yml down

clean:
	docker volume rm inception_wp-db-vol || true
	docker volume rm inception_wp-web-vol || true
	docker volume rm inception_bonus-web-vol || true
	docker volume rm inception_bonus-redis-vol || true
	docker image prune -a -f || true

re: down clean all

.PHONY: all clean re up down