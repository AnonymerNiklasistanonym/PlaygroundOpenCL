#!/usr/bin/env bash

# Format all project related source files but exclude all build/vendor directories
find src/ -regex ".*\.\(cpp\|hpp\|h|\cl\)" \
     ! -path '*/vendor/*' ! -path '*/build*/*' \
     -print0 | xargs -n 1 -0 astyle --project=astyle_config.astylerc

