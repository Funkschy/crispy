#!/bin/bash
# Installs the crispy color scheme for vim

# creates the specified dir if it does not exist
function opt_create {
	if [ ! -d "$1" ]; then
		mkdir -p "$1"
	fi
}

opt_create ~/.vim/syntax
opt_create ~/.vim/ftdetect

cp ./syntax/crispy.vim ~/.vim/syntax
cp ./ftdetect/crispy.vim ~/.vim/ftdetect

