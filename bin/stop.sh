#!/bin/sh

ps -ef | grep comm_server | grep -v grep | awk '{print  $2}' | xargs -t -i kill -9 {}
