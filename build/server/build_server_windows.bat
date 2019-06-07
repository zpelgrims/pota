:: initialize x64 developer command line environment:
call "C:\Program Files (x86)\Microsoft Visual Studio\2017\BuildTools\VC\Auxiliary\Build\vcvars64.bat"

SET WEBSERVER="root@78.141.196.104"
SET LENTIL_BUILD_HOME="C:\lentil-build"

SET LENSES=%1
echo %LENSES%
SET USER=%2
echo %USER%
SET USER_BUILD_DIR=%3
echo %USER_BUILD_DIR%
SET DOWNLOAD_DIR=%4
echo %DOWNLOAD_DIR%

mkdir %LENTIL_BUILD_HOME%/builds/%USER_BUILD_DIR%/bin

setx LENTIL_PATH "C:\lentil-build\lentil\polynomial-optics" /M

:: need to run a git pull on master, currently only supporting latest release
:: cd %LENTIL_BUILD_HOME%/lentil
:: git pull --recurse-submodules
:: need to switch to origin/dev for all repos here

:: build the plugin
CALL SET LENSES=%LENSES:.=-DLENS_ID_%
ECHO %LENSES%
cl /LD /I %LENTIL_BUILD_HOME%\arnold\Arnold-5.2.2.0-windows\include /I %LENTIL_BUILD_HOME%\lentil\Eigen\Eigen /I %LENTIL_BUILD_HOME%\lentil\fmt\include\fmt /I %LENTIL_BUILD_HOME%\lentil\polynomial-optics\src /EHsc /O2 %LENSES% -DFMT_HEADER_ONLY %LENTIL_BUILD_HOME%\lentil\pota\src\pota.cpp /link /LIBPATH:%LENTIL_BUILD_HOME%\arnold\Arnold-5.2.2.0-windows\lib ai.lib /OUT:%LENTIL_BUILD_HOME%\builds\%USER_BUILD_DIR%\lentil.dll
:: if this fails i need to be sent an urgent email/notification..!

:: collect files into directories
copy %LENTIL_BUILD_HOME%/lentil/pota/maya %LENTIL_BUILD_HOME%/builds/%USER_BUILD_DIR%/

:: zip it up
cd %LENTIL_BUILD_HOME%/builds
7z %USER_BUILD_DIR%.zip %USER_BUILD_DIR%

:: sync .zip to website server
scp %USER_BUILD_DIR%.zip %WEBSERVER%:%DOWNLOAD_DIR%