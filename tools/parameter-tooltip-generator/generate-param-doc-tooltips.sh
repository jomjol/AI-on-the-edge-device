#!/bin/bash

# Checkout the documentation reposo we can extract the parameter documentation
if [ -d "AI-on-the-edge-device-docs" ] ; then
    # Repo already checked out, pull it
    cd AI-on-the-edge-device-docs
    git checkout main
    git pull
    cd ..
else
    # Repos folde ris missing, clone it
    git clone https://github.com/jomjol/AI-on-the-edge-device-docs.git
fi

python generate-param-doc-tooltips.py
