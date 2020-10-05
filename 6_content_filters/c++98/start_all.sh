#!/bin/sh

filename=$0
arch=$1
script_dir=`dirname $filename`
executable_name="ingredient_application"
bin_dir=${script_dir}

if [ -f $bin_dir/$executable_name ]
then
    export LD_LIBRARY_PATH=$LD_LIBRARY_PATH
    shift
    gnome-terminal -x bash -c "./$executable_name -k COCOA_BUTTER_CONTROLLER $*"
    gnome-terminal -x bash -c "./$executable_name -k SUGAR_CONTROLLER $*"
    gnome-terminal -x bash -c "./$executable_name -k MILK_CONTROLLER $*"
    gnome-terminal -x bash -c "./$executable_name -k VANILLA_CONTROLLER $*"
    gnome-terminal -x bash -c "./tempering_application $*"
    gnome-terminal -x bash -c "./monitoring_ctrl_application $*"
else
    echo "***************************************************************"
    echo $executable_name executable does not exist in:
    echo $bin_dir
    echo ""
    echo Please, try to recompile the application using the command:
    echo " $ make -f make/Makefile.${arch}"
    echo "***************************************************************"
fi
