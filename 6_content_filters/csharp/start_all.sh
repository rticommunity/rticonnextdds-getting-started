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
            ${terminal} ${param} dotnet run -p IngredientApplication -- --station-kind COCOA_BUTTER_CONTROLLER $* &
            ${terminal} ${param} dotnet run -p IngredientApplication -- --station-kind SUGAR_CONTROLLER $* &
            ${terminal} ${param} dotnet run -p IngredientApplication -- --station-kind MILK_CONTROLLER $* &
            ${terminal} ${param} dotnet run -p IngredientApplication -- --station-kind VANILLA_CONTROLLER $* &
            ${terminal} ${param} dotnet run -p TemperingApplication $* &
            ${terminal} ${param} dotnet run -p MonitoringCtrlApplication $* &
        else
            ${terminal} ${param} dotnet run -p IngredientApplication -- --station-kind COCOA_BUTTER_CONTROLLER $*
            ${terminal} ${param} dotnet run -p IngredientApplication -- --station-kind SUGAR_CONTROLLER $*
            ${terminal} ${param} dotnet run -p IngredientApplication -- --station-kind MILK_CONTROLLER $*
            ${terminal} ${param} dotnet run -p IngredientApplication -- --station-kind VANILLA_CONTROLLER $*
            ${terminal} ${param} dotnet run -p TemperingApplication $*
            ${terminal} ${param} dotnet run -p MonitoringCtrlApplication $*
        fi
    ;;

    Darwin*)
        # No clear way to pass all additional arguments to OSX, pass manually
        osascript -e 'tell application "Terminal" to do script "cd '$(pwd)';dotnet run -p IngredientApplication -- --station-kind COCOA_BUTTER_CONTROLLER '$1' '$2' '$3' '$4' '$5' '$6' '$7'"'
        osascript -e 'tell application "Terminal" to do script "cd '$(pwd)';dotnet run -p IngredientApplication -- --station-kind SUGAR_CONTROLLER '$1' '$2' '$3' '$4' '$5' '$6' '$7'"'
        osascript -e 'tell application "Terminal" to do script "cd '$(pwd)';dotnet run -p IngredientApplication -- --station-kind MILK_CONTROLLER '$1' '$2' '$3' '$4' '$5' '$6' '$7'"'
        osascript -e 'tell application "Terminal" to do script "cd '$(pwd)';dotnet run -p IngredientApplication -- --station-kind VANILLA_CONTROLLER '$1' '$2' '$3' '$4' '$5' '$6' '$7'"'
        osascript -e 'tell application "Terminal" to do script "cd '$(pwd)';dotnet run -p TemperingApplication '$1' '$2' '$3' '$4' '$5' '$6' '$7'"'
        osascript -e 'tell application "Terminal" to do script "cd '$(pwd)';dotnet run -p MonitoringCtrlApplication '$1' '$2' '$3' '$4' '$5' '$6' '$7'"'
    ;;

esac

