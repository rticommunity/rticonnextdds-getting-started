#!/bin/sh

filename=$0
arch=$1
script_dir=`dirname $filename`
executable_name="ingredient_application"
bin_dir=${script_dir}


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

        export LD_LIBRARY_PATH=$LD_LIBRARY_PATH
        if [ -f $bin_dir/$executable_name ]
        then
            if [ ${terminal} != 'gnome-terminal' ]
            then
                ${terminal} ${param} ./$executable_name -k COCOA_BUTTER_CONTROLLER $* &
                ${terminal} ${param} ./$executable_name -k SUGAR_CONTROLLER $* &
                ${terminal} ${param} ./$executable_name -k MILK_CONTROLLER $* &
                ${terminal} ${param} ./$executable_name -k VANILLA_CONTROLLER $* &
                ${terminal} ${param} ./tempering_application $* &
                ${terminal} ${param} ./monitoring_ctrl_application $* &
            else
                ${terminal} ${param} ./$executable_name -k COCOA_BUTTER_CONTROLLER $*
                ${terminal} ${param} ./$executable_name -k SUGAR_CONTROLLER $*
                ${terminal} ${param} ./$executable_name -k MILK_CONTROLLER $*
                ${terminal} ${param} ./$executable_name -k VANILLA_CONTROLLER $*
                ${terminal} ${param} ./tempering_application $*
                ${terminal} ${param} ./monitoring_ctrl_application $*
            fi
        else
            echo "***************************************************************"
            echo $executable_name executable does not exist in:
            echo $bin_dir
            echo ""
            echo "***************************************************************"
        fi
    ;;

    Darwin*)
        # No clear way to pass all additional arguments to OSX, pass manually
        osascript -e 'tell application "Terminal" to do script "cd '$(pwd)';./ingredient_application -k COCOA_BUTTER_CONTROLLER '$1' '$2' '$3' '$4' '$5' '$6' '$7'"'
        osascript -e 'tell application "Terminal" to do script "cd '$(pwd)';./ingredient_application -k SUGAR_CONTROLLER '$1' '$2' '$3' '$4' '$5' '$6' '$7'"'
        osascript -e 'tell application "Terminal" to do script "cd '$(pwd)';./ingredient_application -k MILK_CONTROLLER '$1' '$2' '$3' '$4' '$5' '$6' '$7'"'
        osascript -e 'tell application "Terminal" to do script "cd '$(pwd)';./ingredient_application -k VANILLA_CONTROLLER '$1' '$2' '$3' '$4' '$5' '$6' '$7'"'
        osascript -e 'tell application "Terminal" to do script "cd '$(pwd)';./tempering_application '$1' '$2' '$3' '$4' '$5' '$6' '$7'"'
        osascript -e 'tell application "Terminal" to do script "cd '$(pwd)';./monitoring_ctrl_application '$1' '$2' '$3' '$4' '$5' '$6' '$7'"'
    ;;

esac

