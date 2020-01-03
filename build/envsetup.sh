#!/bin/bash
#
#  Copyright (C) 2018, Wang Qiuwei <qiuwei.wang@ingenic.com / panddio@163.com>
#
#  Ingenic ms800 Project
#
#  This program is free software; you can redistribute it and/or modify it
#  under  the terms of the GNU General  Public License as published by the
#  Free Software Foundation;  either version 2 of the License, or (at your
#  option) any later version.
#
#  You should have received a copy of the GNU General Public License along
#  with this program; if not, write to the Free Software Foundation, Inc.,
#  675 Mass Ave, Cambridge, MA 02139, USA.
#

SUCCESS=0
FAILURE=-1

function echoc()
{
    echo -e "\e[0;91m$1\e[0m"
}

unset LUNCH_MENU_CHOICES
function add_lunch_combo()
{
    local new_combo=$1
    local c
    for c in ${LUNCH_MENU_CHOICES[@]} ; do
    if [ "$new_combo" = "$c" ] ; then
        return
    fi
    done
    LUNCH_MENU_CHOICES=(${LUNCH_MENU_CHOICES[@]} $new_combo)
}

function print_lunch_menu()
{
    echo ""
    echo "You're building on $(uname)"
    echoc "Lunch menu... pick a combo:"

    local i=1
    local choice
    for choice in ${LUNCH_MENU_CHOICES[@]}
    do
        echo "     $i. $choice"
        i=$(($i+1))
    done

    echo
}

function envsetup()
{
    SRCDIR=$(pwd)

    while true; do
        if [ -f $SRCDIR/build/envsetup.sh ]; then
            break
        fi

        if [ x$SRCDIR == x ]; then
            echoc "Failed to set environment..."
            return $FAILURE;
        fi
        SRCDIR=${SRCDIR%/*}
    done

    TOOLCHAINS=$SRCDIR/tools/toolchains/mips-gcc472-glibc216/bin
    if [ -d $TOOLCHAINS ]; then
        export PATH=$TOOLCHAINS:$PATH
    else
        echoc "TOOLCHAINS is not exist!!"
        return $FAILURE
    fi

    if [ -d $SRCDIR/tools/host ]; then
        export PATH=$SRCDIR/tools/host:$PATH
    fi

    # Execute the contents of any vendorsetup.sh files we can find.
    for f in `test -d $SRCDIR/device && find $SRCDIR/device -maxdepth 3 -name 'vendorsetup.sh' 2> /dev/null`
    do
        echo "including $f"
        . $f
    done

    echo "OK!!! You need to execute the 'lunch' ..."
    return $SUCCESS
}
declare -f croot > /dev/null || envsetup

function lunch()
{
    local answer=

    if [ "$1" ]; then
        answer=$1
    else
        print_lunch_menu
        read -p "Which would you like? [ms800-v10-norflash-eng] " answer
        echo ""
    fi

    local selection=

    if [ -z "$answer" ]
    then
        selection=ms800-v10-norflash-eng
    elif (echo -n $answer | grep -q -e "^[0-9][0-9]*$")
    then
        if [ $answer -le ${#LUNCH_MENU_CHOICES[@]} ]
        then
            selection=${LUNCH_MENU_CHOICES[$(($answer-1))]}
        fi
    elif (echo -n $answer | grep -q -e "^[^\-]*-[^\_]*$")
    then
        selection=$answer
    fi

    if [ -z "$selection" ]
    then
        echo
        echoc "Invalid lunch combo: $answer"
        return $FAILURE
    fi

    TARGET_DEVICE=$(echo $selection | awk -F "-" '{print $1}')
    TARGET_STORAGE_MEDIUM=$(echo $selection | awk -F "-" '{print $2}')

    if [ x"$TARGET_STORAGE_MEDIUM" = x"norflash" -o \
         x"$TARGET_STORAGE_MEDIUM" = x"spinand" ]; then
        TARGET_DEVICE_VERSION=""
    else
        TARGET_DEVICE_VERSION=$(echo $selection | awk -F "-" '{print $2}')
        TARGET_STORAGE_MEDIUM=$(echo $selection | awk -F "-" '{print $3}')
    fi

    if [ -z "$TARGET_DEVICE" -o \
         -z "$TARGET_STORAGE_MEDIUM" ]; then
         echoc "Invalid lunch select: $selection"
         return $FAILURE
     fi

    export TARGET_DEVICE TARGET_DEVICE_VERSION TARGET_STORAGE_MEDIUM
    cp $SRCDIR/build/Makefile $SRCDIR

    return $SUCCESS
}

function get_make_command()
{
    echo command make
}

function make()
{
    if [ x"$1" = x"help" ]; then
        $(get_make_command) "$@"
        return $?
    fi

    if [ x"$1" = x ]; then
        str1="completed"
    else
        str1=$1
    fi

    if [[ "$1" =~ "clean" ]]; then
        str2="clean"
    else
        str2="build"
    fi

    local start_time=$(date +"%s")
    $(get_make_command) "$@"
    local ret=$?
    local end_time=$(date +"%s")
    local tdiff=$(($end_time-$start_time))
    local hours=$(($tdiff / 3600 ))
    local mins=$((($tdiff % 3600) / 60))
    local secs=$(($tdiff % 60))
    local ncolors=$(tput colors 2>/dev/null)

    if [ -n "$ncolors" ] && [ $ncolors -ge 8 ]; then
        color_failed="\e[0;31m"
        color_success="\e[0;32m"
        color_reset="\e[00m"
    else
        color_failed=""
        color_success=""
        color_reset=""
    fi
    echo
    if [ $ret -eq 0 ] ; then
        echo -n -e "${color_success}#### make $str1 successfully "
    else
        if [ x"$str1" = x"completed" ]; then
            str1=
        fi
        echo -n -e "${color_failed}#### make $str1 failed to $str2 some targets "
    fi
    if [ $hours -gt 0 ] ; then
        printf "(%02g:%02g:%02g (hh:mm:ss))" $hours $mins $secs
    elif [ $mins -gt 0 ] ; then
        printf "(%02g:%02g (mm:ss))" $mins $secs
    elif [ $secs -gt 0 ] ; then
        printf "(%s seconds)" $secs
    fi
    echo -e " ####${color_reset}"
    echo
    return $ret
}
