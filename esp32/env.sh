#!/bin/zsh
IDF_PATH=~/esp/esp-idf

if [ ! -f ~/.espressif/espidf.constraints.v6.1.txt ]; then
    "$IDF_PATH/install.sh" esp32c3
fi

source "$IDF_PATH/export.sh"
