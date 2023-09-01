#!/bin/bash
get_this_script_dir () {
    FILE="${BASH_SOURCE[0]}"
    SCRIPT_DIR="$( cd -P "$( dirname "$FILE" )" && pwd )"
    echo $SCRIPT_DIR
}

source "$(get_this_script_dir)"/setup_sdk_env.source
export  HEXAGON_TOOLS_ROOT=$HEXAGON_SDK_ROOT/tools/HEXAGON_Tools/8.3.07
export  PATH=${HEXAGON_TOOLS_ROOT}/Tools/bin/:${HEXAGON_TOOLS_ROOT}/Tools/lib/iss:${PATH}
$HEXAGON_SDK_ROOT/tools/hexagon_ide/eclipse

