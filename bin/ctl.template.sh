#!/bin/bash

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:./

PROCESS_NAME="templated"
PROCESS_CONFIG="./config.template.json"
PROCESS_LOGPATH="./"
PROCESS_PIDFILE="${PROCESS_LOGPATH}${PROCESS_NAME}.pid"
PROCESS_STATUS=""
PROCESS_PID=""
START_COMMAND="./${PROCESS_NAME} -c ${PROCESS_CONFIG} -l ${PROCESS_LOGPATH}"
STATUS_FILTER=$START_COMMAND
ERROR=0
ARG=$1


get_pid() {
   PID=""
   PIDFILE=$1
   if [ -f "$PIDFILE" ] ; then
      PID=`cat $PIDFILE`
   fi
}

get_process_pid() {
   get_pid $PROCESS_PIDFILE
   if [ ! "$PID" ]; then
      return
   fi
   if [ "$PID" -gt 0 ]; then
      PROCESS_PID=$PID
   else
      PROCESS_PID=""
   fi
}

is_service_running() {
   PID=$1
   if [ "x$PID" != "x" ] && kill -0 $PID 2>/dev/null ; then
      RUNNING=1
   else
      RUNNING=0
   fi
   return $RUNNING
}

is_process_running() {
   get_process_pid
   is_service_running $PROCESS_PID
   RUNNING=$?
   if [ $RUNNING -eq 0 ]; then
      PROCESS_STATUS="$PROCESS_NAME not running"
   else
      PROCESS_STATUS="$PROCESS_NAME (pid $PROCESS_PID) already running"
   fi
   return $RUNNING
}

start_process() {
   is_process_running
   RUNNING=$?
   if [ $RUNNING -eq 1 ]; then
      echo "$0 $ARG: $PROCESS_NAME (pid $PROCESS_PID) already running"
      exit
   fi
   $START_COMMAND >> /dev/null 2>&1 &
   COUNTER=5
   while [ $RUNNING -eq 0 ] && [ $COUNTER -ne 0 ]; do
      COUNTER=`expr $COUNTER - 1`
      sleep 1
      is_process_running
      RUNNING=$?
   done
   if [ $RUNNING -eq 0 ]; then
      ERROR=1
   fi
   if [ $ERROR -eq 0 ]; then
      echo "$0 $ARG: $PROCESS_NAME started"
   else
      echo "$0 $ARG: $PROCESS_NAME could not be started"
      ERROR=3
   fi
}

stop_process() {
   NO_EXIT_ON_ERROR=$1
   is_process_running
   RUNNING=$?
   if [ $RUNNING -eq 0 ]; then
      echo "$ARG: $PROCESS_STATUS"
      if [ "x$NO_EXIT_ON_ERROR" != "xno_exit" ]; then
         exit
      else
         return
      fi
   fi

   kill $PROCESS_PID

   COUNTER=10
   while [ $RUNNING -eq 1 ] && [ $COUNTER -ne 0 ]; do
      COUNTER=`expr $COUNTER - 1`
      sleep 1
      is_process_running
      RUNNING=$?
   done

   is_process_running
   RUNNING=$?
   if [ $RUNNING -eq 0 ]; then
      echo "$0 $ARG: $PROCESS_NAME stopped"
   else
      echo "$0 $ARG: $PROCESS_NAME could not be stopped"
      ERROR=4
   fi
}

if [ "x$1" = "xstart" ]; then
   start_process
elif [ "x$1" = "xstop" ]; then
   stop_process
elif [ "x$1" = "xstatus" ]; then
   is_process_running
   echo "$ARG: $PROCESS_STATUS"
else
   echo "usage) $0 (start|stop|status)"
   cat <<EOF

help       - this screen
start      - start the service
stop       - stop  the service
status     - show the status of the service

EOF
fi

exit $ERROR

