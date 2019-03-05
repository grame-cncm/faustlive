#
#
#

VERSION  := 1.00
BASE     := faustbase:$(VERSION)
NAME     := faustlive:latest

.phony: base

build: base Dockerfile
	@echo building docker image $(NAME)...
	docker build -t $(NAME) .

help:
	@echo "Makefile for docker image management."
	@echo "Image intended to check faust and faustlive compilation."
	@echo "Available targets are:"
	@echo "  build (default) : build the docker image"
	@echo "  base    : build the base image"
	@echo "  list    : list current docker images and containers"
	@echo "  run     : run the $(NAME) image"
	@echo "  rm      : remove the $(NAME) image"

list:
	docker images
	docker container ls

base: Dockerbase
	@echo building docker image $(BASE) ...
	docker build -t $(NAME) -f Dockerbase .


run:
	docker run -p 6000:6000 -t -i $(NAME) bash

rmstopped:
	@echo remove all stopped containers
	docker rm $(shell docker ps -a -q)

rm:
	docker rmi -f $(NAME)
