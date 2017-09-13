@echo off
cls
REM ADDITIONAL PATHS
SET ADDITIONAL_INCLUDE_PATH=/usr/local/include
SET ADDITIONAL_LIBRARY_PATH=/usr/local/lib/
SET PATH_TO_C_FILES=/mnt/local/work/lcdproc/server
SET PATH_TO_OUTPUT_FILE=/mnt/local/work/lcdproc/server
REM PATH TO PLINK EXECUTABLE
SET PLINK_EXEC=plink.exe
SET USERNAME=root
SET PASSWORD=password
SET IP_ADDR=192.168.1.3
REM END OF CONFIGURATION
REM ====================
REM SET BUILD_SCRIPT=gcc -Wall  -g -lwiringPi -lwiringPiDev $(mysql_config --cflags) $(mysql_config --libs) -L%ADDITIONAL_LIBRARY_PATH% -I%ADDITIONAL_INCLUDE_PATH% %PATH_TO_C_FILES%/main.c  %PATH_TO_C_FILES%/homey.c %PATH_TO_C_FILES%/out.c %PATH_TO_C_FILES%/shmem.c %PATH_TO_C_FILES%/db.c  -o %PATH_TO_OUTPUT_FILE%
@echo Building Started..
SET BUILD_SCRIPT=make -C %PATH_TO_C_FILES%
REM @echo %PLINK_EXEC% %USERNAME%@%IP_ADDR% -pw %PASSWORD% %BUILD_SCRIPT%
%PLINK_EXEC% %USERNAME%@%IP_ADDR% -pw %PASSWORD% %BUILD_SCRIPT%
@echo Building completed..
