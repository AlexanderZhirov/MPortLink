#!/bin/sh

CMAKE="$(which cmake)"
MAKE="$(which make)"
CURRENT_PATH="$(dirname $(realpath ${0}))"

${CMAKE} -S "${CURRENT_PATH}" -B "${CURRENT_PATH}/build"
${MAKE} -C "${CURRENT_PATH}/build"
