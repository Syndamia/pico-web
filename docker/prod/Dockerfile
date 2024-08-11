FROM alpine:latest

RUN apk update && apk upgrade
RUN apk add musl-dev gcc make

COPY ./src /usr/src
RUN make -C /usr/src prod-server

EXPOSE 8080
ENTRYPOINT [ "/usr/build/server", "0.0.0.0,8080" ]
