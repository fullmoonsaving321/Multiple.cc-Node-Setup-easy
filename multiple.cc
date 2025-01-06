#!/bin/bash

# Check if the script is run as sudo
if [ "$EUID" -ne 0 ]; then
    echo "Please run this script as root using sudo."
    exit 1
fi

# Ask the user for PIN and Identifier
read -p "Enter your unique account identifier: " IDENTIFIER
read -p "Enter your PIN code: " PIN

if [ -z "$IDENTIFIER" ] || [ -z "$PIN" ]; then
    echo "Identifier and PIN cannot be empty. Exiting."
    exit 1
fi

# Ask the user for optional parameters with defaults
read -p "Enter bandwidth download limit (default: 100): " BANDWIDTH_DOWNLOAD
BANDWIDTH_DOWNLOAD=${BANDWIDTH_DOWNLOAD:-100}

read -p "Enter bandwidth upload limit (default: 100): " BANDWIDTH_UPLOAD
BANDWIDTH_UPLOAD=${BANDWIDTH_UPLOAD:-100}

read -p "Enter storage limit (default: 200): " STORAGE
STORAGE=${STORAGE:-200}

# Step 1: Download the appropriate client based on architecture
ARCH=$(uname -m)
if [ "$ARCH" == "x86_64" ]; then
    echo "Detected architecture: X64"
    wget https://cdn.app.multiple.cc/client/linux/x64/multipleforlinux.tar -O multipleforlinux.tar
elif [ "$ARCH" == "aarch64" ]; then
    echo "Detected architecture: ARM64"
    wget https://cdn.app.multiple.cc/client/linux/arm64/multipleforlinux.tar -O multipleforlinux.tar
else
    echo "Unsupported architecture: $ARCH"
    exit 1
fi

# Step 2: Extract the installation package
if [ -f "multipleforlinux.tar" ]; then
    echo "Extracting the installation package..."
    tar -xvf multipleforlinux.tar && rm -rf multipleforlinux.tar
    cd multipleforlinux || { echo "Extraction failed."; exit 1; }
else
    echo "Download failed. Exiting."
    exit 1
fi

# Step 3: Grant the required permissions
chmod +x ./multiple-cli
chmod +x ./multiple-node

# Step 4: Configure environment variables
EXTRACTED_DIR=$(pwd)
echo "Configuring environment variables..."
PROFILE_FILE=/etc/profile
if [ -w "$PROFILE_FILE" ]; then
    echo "PATH=\$PATH:$EXTRACTED_DIR" | sudo tee -a $PROFILE_FILE
    source $PROFILE_FILE
else
    echo "Cannot write to $PROFILE_FILE. Please run the script with sufficient privileges."
    exit 1
fi

# Step 5: Modify permissions for the extracted directory
chmod -R 777 $EXTRACTED_DIR

# Step 6: Start the multiple-node
nohup ./multiple-node > output.log 2>&1 &

# Step 7: Bind unique account identifier
./multiple-cli bind \
    --bandwidth-download $BANDWIDTH_DOWNLOAD \
    --identifier $IDENTIFIER \
    --pin $PIN \
    --storage $STORAGE \
    --bandwidth-upload $BANDWIDTH_UPLOAD

# Step 8: Verify installation
if multiple-cli --version; then
    echo "Installation successful!"
else
    echo "Installation failed. Check logs for more details."
fi
