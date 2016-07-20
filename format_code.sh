#!/bin/bash

#Format all files which are under version control, ignoring untracked files and files in submoudules
git ls-files | xargs -I{} find '{}' -maxdepth 1 -type f -and  \( -name \*.h  -or -name \*.cpp \) -print | xargs astyle --options=style.astylerc -Q #--dry-run

