#!/bin/sh

filename=$0
arch=$1
script_dir=`pwd`
executable_name="ingredient_application"
bin_dir=${script_dir}

if [ -f $bin_dir/$executable_name ]
then
    export LD_LIBRARY_PATH=$LD_LIBRARY_PATH

    osascript -e 'tell application "Terminal" to do script "cd '$(pwd)';./ingredient_application -k SUGAR_CONTROLLER"'
    osascript -e 'tell application "Terminal" to do script "cd '$(pwd)';./ingredient_application -k COCOA_BUTTER_CONTROLLER"'
    osascript -e 'tell application "Terminal" to do script "cd '$(pwd)';./ingredient_application -k MILK_CONTROLLER"'
    osascript -e 'tell application "Terminal" to do script "cd '$(pwd)';./ingredient_application -k VANILLA_CONTROLLER"'
    osascript -e 'tell application "Terminal" to do script "cd '$(pwd)';./tempering_application"'
    osascript -e 'tell application "Terminal" to do script "cd '$(pwd)';./monitoring_ctrl_application"'
else
    echo "***************************************************************"
    echo $executable_name executable does not exist in:
    echo $bin_dir
    echo ""
    echo Please, try to recompile the application using the command:
    echo " $ make -f make/Makefile.${arch}"
    echo "***************************************************************"
fi
