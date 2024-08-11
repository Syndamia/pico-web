FROM alpine:latest

RUN apk update && apk upgrade
RUN apk add bash musl-dev gcc make gdb

COPY ./build /usr/build

EXPOSE 8080
WORKDIR /usr/build
CMD /bin/bash
