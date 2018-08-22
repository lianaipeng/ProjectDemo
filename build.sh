#!/bin/bash

helpFun() {
    echo "## please input: $1 [|build|rebuild|clean]"
}

buildFun() {
    # change root dir
    cd obj
    #echo "## change root dir:cd bin"
    echo "## current root dir:"`pwd`"" 
    
    cmake ..
    echo "## cmake .."

    make 
    echo "## make"
}

cleanFun() {
    #echo "## I will clean old project files"

    rm -rf bin/*
    echo "## rm -rf bin/*"
    rm -rf obj/*
    echo "## rm -rf obj/*"
    rm -rf logs/*
    echo "## rm -rf logs/*"
}

if [ $# == 0 ]
then 
    echo "########## Start build project ##########"
    buildFun
elif [ $# == 1 ]
then
    if [ $1 == "build" ]
    then
        echo "########## Start build project ##########"
        buildFun
    elif [ $1 == "rebuild" ]
    then
        echo "########## Start rebuild project ##########"
        cleanFun
        buildFun
    elif [ $1 == "clean" ]
    then
        echo "########## Start clean project ##########"
        cleanFun
    else 
        echo "########## error wrong argment ##########"
        helpfun $0
    fi
else
    echo "########## error wrong argment ##########"
    helpfun $0
fi  
echo "########## End execute command ##########"
