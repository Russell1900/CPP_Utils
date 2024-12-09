TAG=v3.1
GIT_URL="git@github.com:p-ranav/argparse.git"
VERSION=3.1
PROJECT=argparse
ZIP_URL="https://github.com/p-ranav/argparse/archive/refs/tags/v${VERSION}.zip"
ZIP_NAME=v${VERSION}.zip
FOLDER=${PROJECT}-${VERSION}
OUT?=${CURDIR}/out
OUT:=$(abspath ${OUT})
hpp=include/argparse/argparse.hpp

${OUT}:${FOLDER}
	if [ -e ${OUT} ]; then rm -rf ${OUT}; fi 
	mkdir -p ${OUT}
	cp ${FOLDER}/include ${OUT} -r

${FOLDER}: ${ZIP_NAME}
	if [ -e ${FOLDER} ]; then \
		rm -rf ${FOLDER}; \
	fi
	unzip ${ZIP_NAME}
	touch ${FOLDER}

${ZIP_NAME}:
	if [ ! -e ${ZIP_NAME} ]; then \
		wget ${ZIP_URL}; \
	fi
