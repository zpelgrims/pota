#!/bin/bash

# needs to be passed:
#   username
#   lens list

# create folder for user build
DATE="$( date +"%y%m%d-%H%M" )"
USER="zpelgrims"
USER_BUILD_FOLDER=$DATE-$USER
mkdir -p $LENTIL_BUILD_HOME/builds/$USER_BUILD_FOLDER/bin


CURRENT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )"

echo ""
echo "Setting environment variables: "
    # green text color
    tput setaf 2;
    export LENTIL_PATH=$CURRENT_DIR/../polynomial-optics/
    echo -e "\t LENTIL_PATH: " $LENTIL_PATH
echo ""

# reset text formatting
tput sgr0

# build the plugin
# will need to pass the lens list
# will need to build to custom location
make user_build_folder=$LENTIL_BUILD_HOME/builds/$USER_BUILD_FOLDER


# collect files into directories
rsync -ah --progress $LENTIL_BUILD_HOME/lentil/pota/maya $LENTIL_BUILD_HOME/builds/$USER_BUILD_FOLDER/

# zip it up
zip -r9 $USER_BUILD_FOLDER.zip $USER_BUILD_FOLDER

# sync file to server hosting website
# rsync something something something