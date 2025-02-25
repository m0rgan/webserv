FROM alpine:3.19

ARG DOMAIN_NAME
ARG DOMAIN_BONUS

RUN apk update && apk upgrade && apk add nginx openssl

RUN chown -R root:root /var/lib/nginx && \
	chown -R root:root /var/log/nginx && \
	chown -R root:root /etc/nginx && \
	chown -R root:root /run/nginx

COPY --chown=root:root /conf/default.conf /etc/nginx/http.d/default.conf
COPY --chown=root:root /conf/cert.pem /etc/nginx/ssl/nginx_cert.pem
COPY --chown=root:root /conf/key.pem /etc/nginx/ssl/nginx_key.pem

RUN sed -i 's/\$DOMAIN_NAME/'"$DOMAIN_NAME"'/g' /etc/nginx/http.d/default.conf && \
    sed -i 's/\$DOMAIN_BONUS/'"$DOMAIN_BONUS"'/g' /etc/nginx/http.d/default.conf

ENTRYPOINT ["nginx", "-g", "daemon off;"]