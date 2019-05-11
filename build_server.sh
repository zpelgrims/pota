#!/bin/bash

WEBSERVER=root@78.141.196.104

args=("$@")
LENSES=${args[0]}
USER=${args[1]}
USER_BUILD_FOLDER=${args[2]}
DOWNLOAD_DIR=${args[3]}

mkdir -p $LENTIL_BUILD_HOME/builds/$USER_BUILD_FOLDER/bin


CURRENT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )"

echo ""
echo "Setting environment variables: "
    export LENTIL_PATH=$CURRENT_DIR/../polynomial-optics/
    echo -e "\t LENTIL_PATH: " $LENTIL_PATH
echo ""


# build the plugin
make user_build_folder=$LENTIL_BUILD_HOME/builds/$USER_BUILD_FOLDER lens_list=lenses

# collect files into directories
rsync -ah --progress $LENTIL_BUILD_HOME/lentil/pota/maya $LENTIL_BUILD_HOME/builds/$USER_BUILD_FOLDER/

# zip it up
zip -r9 $USER_BUILD_FOLDER.zip $USER_BUILD_FOLDER

# sync .zip to website server
rsync -avz -e "ssh -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null" --progress $USER_BUILD_FOLDER.zip WEBSERVER:$DOWNLOAD_DIR/