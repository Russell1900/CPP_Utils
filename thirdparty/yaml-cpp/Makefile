OUT?=${CURDIR}/out
OUT:=$(abspath ${OUT})
url="https://github.com/jbeder/yaml-cpp/archive/refs/tags/0.8.0.zip"
zip_name=0.8.0.zip
folder_name=yaml-cpp-0.8.0

${OUT}: ${folder_name}
	if [ -e ${OUT} ]; then \
		rm -rf ${OUT}; \
	fi
	mkdir -p ${OUT}
	cd ${folder_name} && if [ -e build ]; then rm -rf build; fi && mkdir build && cd build && cmake -DCMAKE_INSTALL_PREFIX=${OUT} .. && make -j8 && make install

${folder_name}: ${zip_name}
	if [ -e ${OUT} ]; then \
		rm -rf ${folder_name}; \
	fi
	unzip ${zip_name}
	touch ${folder_name}

${zip_name}:
	if [ ! -e ${zip_name} ]; then \
		wget ${url}; \
	fi


