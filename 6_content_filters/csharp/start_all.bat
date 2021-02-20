@echo off
setlocal

set dir=%~dp0

for /f "tokens=1,* delims= " %%a in ("%*") do set ALL_BUT_FIRST=%%b

start dotnet run -p IngredientApplication -- --station-kind COCOA_BUTTER_CONTROLLER %ALL_BUT_FIRST%
start dotnet run -p IngredientApplication -- --station-kind SUGAR_CONTROLLER %ALL_BUT_FIRST%
start dotnet run -p IngredientApplication -- --station-kind MILK_CONTROLLER %ALL_BUT_FIRST%
start dotnet run -p IngredientApplication -- --station-kind VANILLA_CONTROLLER %ALL_BUT_FIRST%
start dotnet run -p TemperingApplication %ALL_BUT_FIRST%
start dotnet run -p MonitoringCtrlApplication %ALL_BUT_FIRST%