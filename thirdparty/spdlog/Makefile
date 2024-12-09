OUT?=${CURDIR}/out
OUT:=$(abspath ${OUT})
LIB=lib
INCLUDE=include
version="1.14.1"
spd_dir="spdlog-${version}"
tar="${spd_dir}.tar.gz"
url="https://github.com/gabime/spdlog/archive/refs/tags/v${version}.tar.gz"


${OUT}:
	@mkdir -p ${OUT}
	@rm ${LIB} -rf
	@rm ${INCLUDE} -rf
	@wget ${url} -O ${tar}
	@tar -zxf ${tar}
	@cd ${spd_dir} && mkdir build && cd build && cmake .. -DCMAKE_INSTALL_PREFIX=${OUT} && make -j8 && make install
	@echo "spdlog installed on ${OUT}"

