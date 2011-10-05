#!/bin/sh
git log --pretty=format:"%ad" --date=short | head -n 1 | sed "s/-//g"