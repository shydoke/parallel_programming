@echo off
setlocal enabledelayedexpansion
chcp 65001 >nul

echo ------------------ Compilation -------------
cl /EHsc /openmp /O2 main.cpp 2>nul
if errorlevel 1 (
    echo ERROR compilation
    pause
    exit /b 1
)

echo.
echo ------------------ Experiments -------------

if exist results.csv del results.csv

echo size,threads,read_time_ms,mult_time_ms,total_time_ms > results.csv

set sizes=200 400 800 1200 1600 2000
set threads=1 2 4 8

for %%s in (%sizes%) do (
    for %%t in (%threads%) do (
        echo.
        echo Running: size=%%s threads=%%t
        
        set OMP_NUM_THREADS=%%t
        
        REM Запускаем и сохраняем вывод во временный файл
        main.exe mat_%%s_1.txt mat_%%s_2.txt result_%%s.txt > temp_output.txt 2>&1
        
        REM Ищем строки с временем
        set read_time=0
        set mult_time=0
        set total_time=0
        
        for /f "tokens=1,2,3 delims=: " %%a in ('findstr "Read time:" temp_output.txt') do (
            set read_time=%%c
        )
        for /f "tokens=1,2,3 delims=: " %%a in ('findstr "Multiplication time:" temp_output.txt') do (
            set mult_time=%%c
        )
        for /f "tokens=1,2,3 delims=: " %%a in ('findstr "Total time:" temp_output.txt') do (
            set total_time=%%c
        )
        
        set read_time=!read_time:ms=!
        set mult_time=!mult_time:ms=!
        set total_time=!total_time:ms=!
        
        set read_time=!read_time: =!
        set mult_time=!mult_time: =!
        set total_time=!total_time: =!
        
        echo %%s,%%t,!read_time!,!mult_time!,!total_time! >> results.csv
        
        echo Read: !read_time! ms, Mult: !mult_time! ms, Total: !total_time! ms
    )
)

del temp_output.txt 2>nul

echo.
echo ------------------ Results saved to results.csv
echo.
type results.csv

pause