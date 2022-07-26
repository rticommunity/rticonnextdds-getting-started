@echo off
setlocal

set dir=%~dp0

for /f "tokens=1,* delims= " %%a in ("%*") do set ALL_BUT_FIRST=%%b

start python ingredient_application.py --station-kind COCOA_BUTTER_CONTROLLER %ALL_BUT_FIRST%
start python ingredient_application.py --station-kind SUGAR_CONTROLLER %ALL_BUT_FIRST%
start python ingredient_application.py --station-kind MILK_CONTROLLER %ALL_BUT_FIRST%
start python ingredient_application.py --station-kind VANILLA_CONTROLLER %ALL_BUT_FIRST%
start python tempering_application.py  %ALL_BUT_FIRST%
start python monitoring_ctrl_application.py  %ALL_BUT_FIRST%