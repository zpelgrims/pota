SET WEBSERVER="root@78.141.196.104"
SET LENTIL_BUILD_HOME="C:\lentil-build"

SET LENSES=%1
SET USER=%2
SET USER_BUILD_DIR=%3
SET DOWNLOAD_DIR=%4

mkdir %LENTIL_BUILD_HOME%/builds/%USER_BUILD_DIR%/bin

setx LENTIL_PATH "C:\lentil-build\lentil\polynomial-optics" /M

:: need to run a git pull on master, currently only supporting latest release
cd %LENTIL_BUILD_HOME%/lentil
git pull --recurse-submodules

:: build the plugin
cd pota/build/server
make user_build_folder=%LENTIL_BUILD_HOME%/builds/%USER_BUILD_DIR% lens_list=%LENSES%
:: if this fails i need to be sent an urgent email/notification..!

:: collect files into directories
copy %LENTIL_BUILD_HOME%/lentil/pota/maya %LENTIL_BUILD_HOME%/builds/%USER_BUILD_DIR%/

:: zip it up
cd %LENTIL_BUILD_HOME%/builds
7z %USER_BUILD_DIR%.zip %USER_BUILD_DIR%

:: sync .zip to website server
C:\pscp %USER_BUILD_DIR%.zip %WEBSERVER%:%DOWNLOAD_DIR%