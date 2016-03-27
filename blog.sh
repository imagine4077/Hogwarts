#!/bin/bash

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
