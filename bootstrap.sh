#!/bin/bash

# https://stackoverflow.com/questions/192249/how-do-i-parse-command-line-arguments-in-bash

HOME_PATH=`pwd`

function usage_func {
   cat <<_ACEOF

Usage: $0

Options:
  --build=[debug/release]
  --disable-dependency           do not build dependency

_ACEOF
}

for i in "$@"
do
   case $i in
      --build=*)
         BUILD_TAG="${i#*=}"
         shift # past argument=value
         ;;

      --disable-dependency)
         DISABLE_DEPENDENCY="yes"
         shift # past argument=value
         ;;

      *)
         # unknown option
         ;;
   esac
done

if [[ x${BUILD_TAG} == x ]]; then
   usage_func
   exit 1
fi

if [[ x${BUILD_TAG} == xdebug ]]; then
   ENABLE_DEBUG="--enable-debug"
fi


# ==============================================================================
# 환경 정보 설정
# ------------------------------------------------------------------------------
# Project 버전은 configure.ac에서 직접 수정해야함.
# ------------------------------------------------------------------------------
REPOSITORY_REVISION=`git log --abbrev-commit --pretty=format:"%h,%ad" --date=short -1`

HOME_DIR=${PWD}
BIN_DIR=${HOME_DIR}/bin
LIB_DIR=${HOME_DIR}/lib
echo "HOME_DIR=${HOME_DIR}" > buildenv.am
echo "BIN_DIR=${BIN_DIR}" >> buildenv.am
echo "LIB_DIR=${LIB_DIR}" >> buildenv.am
echo "SYSLIBS=-lssl -lcrypto -ltbb -ldl -lz -lm -lstdc++" >> buildenv.am
echo "COMM_LD=-L${LIB_DIR} -L\$(LIB_DIR)" >> buildenv.am
echo "COMM_INC=-std=c++11 -W -Wall -Wextra -I${LIB_DIR} -D_POSIX_PTHREAD_SEMANTICS -D_REENTRANT " >> buildenv.am


# ==============================================================================
# submodule update
# ------------------------------------------------------------------------------
if [[ x${DISABLE_DEPENDENCY} == x ]]; then
   git submodule init
   git submodule update

   cp ${HOME_DIR}
   cp -f rlwrap-0.43.tar.gz ${LIB_DIR}

   cd ${LIB_DIR}
   tar -xvzf rlwrap-0.43.tar.gz
   cd rlwrap-0.43 && ./configure && make -j 8
   COPYFILE=src/rlwrap
   if test -f "${COPYFILE}"; then
      cp -f ${COPYFILE} ${BIN_DIR}
   fi
   cd ${LIB_DIR}
   rm -rf rlwrap-0.43

   # copy tbb
   cd ${LIB_DIR}
   cp -f libtbb.so.2 ${BIN_DIR}
fi


# ==============================================================================
# 컴파일
# ------------------------------------------------------------------------------
cd ${HOME_DIR}

export REPOSITORY_REVISION

aclocal
autoheader
autoconf
touch COPYING AUTHORS INSTALL NEWS README
automake -ac

./configure
make clean && make -j 8

unset REPOSITORY_REVISION
