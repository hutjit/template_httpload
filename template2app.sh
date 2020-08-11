#!/bin/bash

HOME_PATH=`pwd`

function usage_func {
   cat <<_ACEOF

Usage: $0

Options:
  --app=[process-name]

_ACEOF
}

for i in "$@"
do
   case $i in
      --app=*)
         PROCESS_NAME="${i#*=}"
         shift # past argument=value
         ;;

      *)
         # unknown option
         ;;
   esac
done

if [[ x${PROCESS_NAME} == x ]]; then
   usage_func
   exit 1
fi

UPPER_APP_NAME=`echo ${PROCESS_NAME} | awk '{print toupper($0)}'`
LOWER_APP_NAME=`echo ${PROCESS_NAME} | awk '{print tolower($0)}'`


cp -R template ${LOWER_APP_NAME}
find ./${LOWER_APP_NAME} -name "Makefile.am" -exec sed -i "s/template/${LOWER_APP_NAME}/g" {} \;
find ./${LOWER_APP_NAME} -name "*.h" -exec sed -i "s/template\//${LOWER_APP_NAME}\//g" {} \;
find ./${LOWER_APP_NAME} -name "*.hxx" -exec sed -i "s/template\//${LOWER_APP_NAME}\//g" {} \;
find ./${LOWER_APP_NAME} -name "*.cpp" -exec sed -i "s/template\//${LOWER_APP_NAME}\//g" {} \;
find ./${LOWER_APP_NAME} -name "*.cxx" -exec sed -i "s/template\//${LOWER_APP_NAME}\//g" {} \;
find ./${LOWER_APP_NAME} -name "*.h" -exec sed -i "s/TEMPLATE_/${UPPER_APP_NAME}_/g" {} \;
find ./${LOWER_APP_NAME} -name "*.hxx" -exec sed -i "s/TEMPLATE_/${UPPER_APP_NAME}_/g" {} \;
find ./${LOWER_APP_NAME} -name "main.cpp" -exec sed -i "s/template/${LOWER_APP_NAME}/g" {} \;
sed -i "s/template\//${LOWER_APP_NAME}\//g" configure.ac
sed -i "s/template/${LOWER_APP_NAME}/g" Makefile.am
sed -i "s/template/${LOWER_APP_NAME}/g" .gitignore
cp bin/config.template.cfg bin/config.${LOWER_APP_NAME}.cfg
cp bin/cli.template.sh bin/cli.${LOWER_APP_NAME}.sh
cp bin/ctl.template.sh bin/ctl.${LOWER_APP_NAME}.sh
