#!/bin/sh

os=`uname -s`


case $os in
    Linux*)
        if hash gnome-terminal &> /dev/null; then
            terminal=gnome-terminal
            param=--
        elif hash konsole &> /dev/null; then
            terminal=konsole
            param=-e
        elif hash xterm &> /dev/null; then
            terminal=xterm
            param=-e
        fi

        if [ ${terminal} != 'gnome-terminal' ]
        then
            ${terminal} ${param} python ingredient_application.py --station-kind COCOA_BUTTER_CONTROLLER $* &
            ${terminal} ${param} python ingredient_application.py --station-kind SUGAR_CONTROLLER $* &
            ${terminal} ${param} python ingredient_application.py --station-kind MILK_CONTROLLER $* &
            ${terminal} ${param} python ingredient_application.py --station-kind VANILLA_CONTROLLER $* &
            ${terminal} ${param} python tempering_application.py  $* &
            ${terminal} ${param} python monitoring_ctrl_application.py  $* &
        else
            ${terminal} ${param} python ingredient_application.py --station-kind COCOA_BUTTER_CONTROLLER $*
            ${terminal} ${param} python ingredient_application.py --station-kind SUGAR_CONTROLLER $*
            ${terminal} ${param} python ingredient_application.py --station-kind MILK_CONTROLLER $*
            ${terminal} ${param} python ingredient_application.py --station-kind VANILLA_CONTROLLER $*
            ${terminal} ${param} python tempering_application.py  $*
            ${terminal} ${param} python monitoring_ctrl_application.py  $*
        fi
    ;;

    Darwin*)
        # No clear way to pass all additional arguments to OSX, pass manually
        osascript -e 'tell application "Terminal" to do script "cd '$(pwd)';python ingredient_application.py --station-kind COCOA_BUTTER_CONTROLLER '$1' '$2' '$3' '$4' '$5' '$6' '$7'"'
        osascript -e 'tell application "Terminal" to do script "cd '$(pwd)';python ingredient_application.py --station-kind SUGAR_CONTROLLER '$1' '$2' '$3' '$4' '$5' '$6' '$7'"'
        osascript -e 'tell application "Terminal" to do script "cd '$(pwd)';python ingredient_application.py --station-kind MILK_CONTROLLER '$1' '$2' '$3' '$4' '$5' '$6' '$7'"'
        osascript -e 'tell application "Terminal" to do script "cd '$(pwd)';python ingredient_application.py --station-kind VANILLA_CONTROLLER '$1' '$2' '$3' '$4' '$5' '$6' '$7'"'
        osascript -e 'tell application "Terminal" to do script "cd '$(pwd)';python tempering_application.py  '$1' '$2' '$3' '$4' '$5' '$6' '$7'"'
        osascript -e 'tell application "Terminal" to do script "cd '$(pwd)';python monitoring_ctrl_application.py  '$1' '$2' '$3' '$4' '$5' '$6' '$7'"'
    ;;

esac

