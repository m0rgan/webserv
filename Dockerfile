FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get upgrade -y && apt-get install -y \
    g++ make cmake openssl libstdc++6 libc6 netbase lsof net-tools \
	php-cgi python3 valgrind

RUN valgrind --version


WORKDIR /app
COPY . /app

RUN make re


# ENTRYPOINT ["valgrind", "--help"]
ENTRYPOINT ["./webserv", "webserv.conf"]
# ENTRYPOINT ["valgrind", "--leak-check=full", "--show-leak-kinds=definite", "--trace-children=yes", "--track-origins=yes", "--track-fds=all", "./webserv", "webserv.conf"]

# ENTRYPOINT ["valgrind", "--trace-children=yes", "--track-fds=all", "./webserv", "webserv.conf"]
