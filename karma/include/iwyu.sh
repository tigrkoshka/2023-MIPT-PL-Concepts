#!/bin/zsh

# this is a linter for header includes utilizing the include-what-you-use util
# (see https://github.com/include-what-you-use/include-what-you-use)

CPP_FILES=("${(f)"$(find . -name "*.cpp" -not -path "*/*build*/*")"}")

for FILE in "${CPP_FILES[@]}"
do
  echo "${FILE}: "

  include-what-you-use -std=c++20 -I. "${FILE}"

  printf "\n\n\n\n"
done

