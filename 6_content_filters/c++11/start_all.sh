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
        elif hash konsole &> /dev/null; then 
            terminal=konsole
        elif hash xterm &> /dev/null; then
            terminal=xterm
        fi

        if [ -f $bin_dir/$executable_name ]
        then
            export LD_LIBRARY_PATH=$LD_LIBRARY_PATH
            ${terminal} -- ./$executable_name -k COCOA_BUTTER_CONTROLLER $*
            ${terminal} -- ./$executable_name -k SUGAR_CONTROLLER $*
            ${terminal} -- ./$executable_name -k MILK_CONTROLLER $*
            ${terminal} -- ./$executable_name -k VANILLA_CONTROLLER $*
            ${terminal} -- ./tempering_application $*
            ${terminal} -- ./monitoring_ctrl_application $*
        else
            echo "***************************************************************"
            echo $executable_name executable does not exist in:
            echo $bin_dir
            echo ""
            echo "***************************************************************"
        fi
    ;;

    Darwin*)
        osascript -e 'tell application "Terminal" to do script "cd '$(pwd)';./ingredient_application -k COCOA_BUTTR_CONTROLLER"'
        osascript -e 'tell application "Terminal" to do script "cd '$(pwd)';./ingredient_application -k SUGAR_CONTROLLER"'
        osascript -e 'tell application "Terminal" to do script "cd '$(pwd)';./ingredient_application -k MILK_CONTROLLER"'
        osascript -e 'tell application "Terminal" to do script "cd '$(pwd)';./ingredient_application -k VANILLA_CONTROLLER"'
        osascript -e 'tell application "Terminal" to do script "cd '$(pwd)';./tempering_application"'
        osascript -e 'tell application "Terminal" to do script "cd '$(pwd)';./monitoring_ctrl_application"'
    ;;

esac

