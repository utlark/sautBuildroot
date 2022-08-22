#!/bin/sh

rtcTime=""
rtcDate=""
status=false
port="/dev/ttyUSB0"

function init () {
   while [ ! -c $port ]
   do
      sleep 2
   done
   echo "RTC found: $(ls $port)"

   stty -F $port 9600
   cat $port> /tmp/rtc &
   sleep 5
}

function push () {
    cat /dev/null > /tmp/rtc
    printf "!$1\r" > $port
    sleep 1
}

function getStatus () {
   ans="$(cat /tmp/rtc)"
   ans="${ans//[!A-Za-z]/}"

   if [ "$ans" = "N" ]; then
      status=false
   else
      status=true
   fi
}

function getTime () {
   push "GetT"
   rtcTime="$(cat /tmp/rtc)"
   rtcTime="${rtcTime//[!0-9]/}"
   rtcTime="${rtcTime::-2}:${rtcTime:2}:00"
}

function getDate () {
   push "GetD"
   rtcDate="$(cat /tmp/rtc)"
   rtcDate="${rtcDate//[!0-9]/}"
   rtcDate="20${rtcDate:4}-${rtcDate:2:-2}-${rtcDate::-4}"
}

function getAll () {
   push "GetA"
   all="$(cat /tmp/rtc)"
   all="${all//[!0-9]/}"

   rtcTime="${all::-12}:${all:2:-10}:00"
   rtcDate="20${all:8:-4}-${all:6:-6}-${all:4:-8}"
}

function setRTC () {
   if [ "$#" -ne 1 ]; then
      echo "Illegal number of parameters"
      exit 2
   fi

   newDate="${1::-9}"
   newDate="${newDate:8}.${newDate:5:-3}.${newDate:2:-6}"

   newTime="${1:11}"
   newTime=${newTime::-3}

   init
   echo "Try set time to RTC: $1"
   push "SetA"$newTime""$newDate"26" 
   killall cat; getStatus

   if [ $status = true ]; then
      echo 'Set RTC: success'
   else
      echo 'Set RTC: fail'
      setRTC "$1"
   fi
}

function setLinux () {
   init; getAll; killall cat
   echo "Set time from RTC: $rtcDate $rtcTime"
   date -u --set="$rtcDate $rtcTime" > /dev/null
}

"$@"
