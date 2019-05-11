#!/bin/bash

WEBSERVER=root@78.141.196.104
LENTIL_BUILD_HOME=/root/lentil-build

args=("$@")
LENSES=${args[0]}
USER=${args[1]}
USER_BUILD_FOLDER=${args[2]}
DOWNLOAD_DIR=${args[3]}

mkdir -p $LENTIL_BUILD_HOME/builds/$USER_BUILD_FOLDER/bin

echo "Setting environment variables: "
    export LENTIL_PATH=/root/lentil-build/lentil/polynomial-optics/
    echo -e "\t LENTIL_PATH: " $LENTIL_PATH
echo ""

# need to run a git pull on master, currently only supporting latest release
cd $LENTIL_BUILD_HOME/lentil
git pull --recurse-submodules

# build the plugin
cd pota/makefile_prod
make user_build_folder=$LENTIL_BUILD_HOME/builds/$USER_BUILD_FOLDER lens_list=$LENSES
# if this fails i need to be sent an urgent email/notification..!

# collect files into directories
rsync -ah --progress $LENTIL_BUILD_HOME/lentil/pota/maya $LENTIL_BUILD_HOME/builds/$USER_BUILD_FOLDER/

# zip it up
cd /root/lentil-build/builds
zip -r9 $USER_BUILD_FOLDER.zip $USER_BUILD_FOLDER

# sync .zip to website server
rsync -avz -e "ssh -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null" --progress /root/lentil-build/builds/$USER_BUILD_FOLDER/$USER_BUILD_FOLDER.zip $WEBSERVER:$DOWNLOAD_DIR