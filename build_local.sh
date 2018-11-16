#!/bin/bash

CURRENT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )"

echo ""
echo "Setting environment variables: "
    # green text color
    tput setaf 2;
    export LENTIL_PATH=$CURRENT_DIR/../polynomial-optics/
    echo -e "\t LENTIL_PATH: " $LENTIL_PATH
    #export LENTIL_ARNOLD_SDKS=$LENTIL_PATH/../../
    #echo -e "\t LENTIL_ARNOLD_SDKS: " $LENTIL_ARNOLD_SDKS
echo ""

# reset text formatting
tput sgr0

make user_build_folder=.