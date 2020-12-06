#!/bin/sh

DIRNAME="ephe"
DOMAIN="https://www.astro.com/ftp/swisseph/ephe/archive_gzip/"

mkdir $DIRNAME
cd $DIRNAME
for i in swephm54.tar.gz \
	swephm48.tar.gz \
	swephm42.tar.gz \
	swephm36.tar.gz \
	swephm30.tar.gz \
	swephm24.tar.gz \
	swephm18.tar.gz \
	swephm12.tar.gz \
	swephm06.tar.gz \
	sweph_00.tar.gz \
	sweph_06.tar.gz \
	sweph_12.tar.gz \
	sweph_18.tar.gz \
	sweph_24.tar.gz \
	sweph_30.tar.gz \
	sweph_36.tar.gz \
	sweph_42.tar.gz \
	sweph_48.tar.gz; do
		wget $DOMAIN$i
		tar -xaf $i
done
# also dowload Eris -- all hail discordia!
wget https://www.astro.com/ftp/swisseph/ephe/ast136/s136199s.se1
