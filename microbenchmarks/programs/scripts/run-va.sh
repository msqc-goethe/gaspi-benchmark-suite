#!/bin/bash

export LD_LIBRARY_PATH=$1/src/.libs/;

"${@:2}"
