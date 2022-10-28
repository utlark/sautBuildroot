#!/bin/sh

while :
do
    sleep 5
    updateFile="$(find /media -name "appUpdate.zip")"

    if [ -f "$updateFile" ]; then
        if [ "$(cat /opt/dat/hash 2>&- || echo "")" != "$(sha256sum $updateFile)" ]; then
            rm -rf /opt/bin
            sha256sum $updateFile > /opt/dat/hash
            unzip $updateFile -d /opt/bin > /dev/null
            reboot
        fi
    fi
done