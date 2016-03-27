#!/bin/bash
if [ $# -ne 1 ]; then
	echo "FORMAT: ./blog.sh ARTICAL_NAME"
	exit
fi
cd ~/Documents/CODE/GIT_launch_platform/Hogwarts/_posts
time=$(date +%Y-%m-%d)
title=$time-$1.md
touch $title
cat > $title << EOF
---
layout: post
title: $1
date: $(date '+%Y-%m-%d %H:%M:%S')
tags: 
category: 
comments: true
---


EOF
mdcharm $title
ls
cd ..
git add .
git commit -m $title
#xdg-open https://github.com/imagine4077/Hogwarts/tree/gh-pages/_posts/$title
gnome-www-browser https://github.com/imagine4077/Hogwarts/tree/gh-pages/_posts/$title

