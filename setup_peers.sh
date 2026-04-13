#!/bin/bash

# Ensure config files exist before running
if [[ ! -f "Common.cfg" || ! -f "PeerInfo.cfg" ]]; then
    echo "Error: Common.cfg or PeerInfo.cfg not found in the current directory."
    exit 1
fi

# Extract the target filename from Common.cfg
FILE_NAME=$(grep "FileName" Common.cfg | awk '{print $2}' | tr -d '\r')
echo "Target file from config: $FILE_NAME"

# Clean up old log files
rm -f log_peer_*.log

# Read PeerInfo.cfg line by line
while read -r peer_id host port has_file; do
    # Skip empty lines
    if [[ -z "$peer_id" ]]; then continue; fi
    
    # Strip hidden carriage returns (fixes issues if config was edited on Windows)
    peer_id=$(echo "$peer_id" | tr -d '\r')
    has_file=$(echo "$has_file" | tr -d '\r')

    DIR_NAME="peer_$peer_id"
    echo "Setting up $DIR_NAME..."
    
    # Remove old directory and recreate fresh
    rm -rf "$DIR_NAME"
    mkdir -p "$DIR_NAME"

    # If the peer is marked as having the file, copy it into their directory
    if [[ "$has_file" == "1" ]]; then
        if [[ -f "$FILE_NAME" ]]; then
            cp "$FILE_NAME" "$DIR_NAME/"
            echo "  -> Copied $FILE_NAME into $DIR_NAME"
        else
            echo "  -> WARNING: Cannot copy $FILE_NAME because it is not in the current directory!"
        fi
    fi
done < PeerInfo.cfg

echo "Setup complete! You can now start your peers."
