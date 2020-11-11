@echo off
setlocal

set dir=%~dp0
set executable_name=ingredient_application.exe
set tempering_name=tempering_application.exe
set monitoring_ctrl_name=monitoring_ctrl_application.exe
set arch=%1

if not exist  %dir%\Debug\%executable_name% ( 
   echo %dir%\Debug\%executable_name% does not exist
   echo Please try to compile the application using Visual Studio
   exit /b
)

for /f "tokens=1,* delims= " %%a in ("%*") do set ALL_BUT_FIRST=%%b

start %dir%\Debug\%executable_name% -k COCOA_BUTTER_CONTROLLER %ALL_BUT_FIRST%
start %dir%\Debug\%executable_name% -k SUGAR_CONTROLLER %ALL_BUT_FIRST%
start %dir%\Debug\%executable_name% -k MILK_CONTROLLER %ALL_BUT_FIRST%
start %dir%\Debug\%executable_name% -k VANILLA_CONTROLLER %ALL_BUT_FIRST%
start %dir%\Debug\%tempering_name% %ALL_BUT_FIRST%
start %dir%\Debug\%monitoring_ctrl_name% %ALL_BUT_FIRST%