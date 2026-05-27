#/bin/bash

set -e

docker run --rm -w "/opt" montage_sdk tar c montage | sudo tar xvC "/opt"
docker run --rm -w "/usr/local" montage_sdk tar c crosstool-ng codesourcery | sudo tar xvC "/usr/local"
