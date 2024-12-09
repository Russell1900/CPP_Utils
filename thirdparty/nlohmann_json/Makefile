LOCAL=include
hpp=${LOCAL}/json.hpp
url="https://github.com/nlohmann/json/releases/download/v3.11.3/json.hpp"
OUT?=${CURDIR}/out
OUT:=$(abspath ${OUT})

${OUT}/${hpp}: 
	@mkdir -p ${OUT}
	@rm ${LOCAL} -rf
	@mkdir ${LOCAL} && wget ${url} -O ${hpp}
	@mv ${LOCAL} ${OUT}
	@echo "nlohmann installed in ${OUT}"

