#!/bin/bash

# Autobuild Script

dateformat="+%d.%m.%Y, %H:%M:%S"


function getGitBranch {
    if [ -z $CI ]; then
	    git rev-parse --abbrev-ref HEAD
	else
	    echo $CI_COMMIT_REF_NAME
	fi
}

function getGitCommit {
	git rev-parse --short HEAD
}

function getGitDate {
	timestamp=$(git show -s --format="%ct")
	date --date="@${timestamp}" "$dateformat"
}

function getGitAuthor {
	git show -s --format=%cn
}

function getBuilder {
	echo "GitLab CI"
}

function getBuildDate {
	date "$dateformat"
}

function upload {
	filename=$1
	heiboxPath=$2
	curl -T $filename -u ${HEIBOX_USER}:${HEIBOX_PASSWORD} https://heibox.uni-heidelberg.de/seafdav/$heiboxPath
}

function prepBuild {
	bash prebuild.sh
}

function buildPDF {
	texFile=$1
	export "TZ=Europe/Berlin"
	texPrefix=$(basename $texFile .tex)
	currDate=$(date "+%Y-%m-%d")
	pdfNameDraft=${PDF_PREFIX}"_Draft_${currDate}.pdf"
	pdfNameFinal=${PDF_PREFIX}".pdf"
	
	prepBuild
	
	# Build Final 
	latexmk -pdf $texFile
	mv ${texPrefix}.pdf $pdfNameFinal
	mv $texFile "_${texFile}"
	rm -f "${texPrefix}*" 	# Clear all building data
	mv "_${texFile}" $texFile 
		
	# Manipulate the autobuild-Flag in the Header-File
	replacementAutobuild="s|setbool{isautobuild}{false}|setbool{isautobuild}{true}|g;"
	sed -e $replacementAutobuild < header.tex > header1.tex
	cp header.tex header_orig.tex
	mv header1.tex header.tex
	
	# Manipulate Titlepage to show Build-Information
	buildDate=$(getBuildDate)
	commitId=$(getGitCommit)
	commitDate=$(getGitDate)
	
	replacementTitlepage="s|++DATE++|$buildDate|g;s|++CID++|$commitId|g;s|++CDATE++|$commitDate|g;"
	sed  -e "$replacementTitlepage" < florianTitlepage.sty > florianTitlepage1.sty
	cp florianTitlepage.sty florianTitlepage_orig.sty
	mv florianTitlepage1.sty florianTitlepage.sty
	
	# Build the Draft Version
	
	latexmk -pdf $texFile
	mv ${texPrefix}.pdf $pdfNameDraft
	
	upload $pdfNameDraft $HEIBOX_PATH
	upload $pdfNameFinal $HEIBOX_PATH
	echo "LatexMK run finished";
}
