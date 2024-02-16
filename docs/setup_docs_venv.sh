#!/bin/bash

function install_pyenv() {
    # Prompt user if they want to install pyenv
    read -p "pyenv is not installed. Would you like to install it? (Y/N) " installPyenv
    if [ "$installPyenv" != "Y" ]; then
        echo "Exiting..."
        exit 1
    fi

    # Prompt user for installation directory
    read -p "Enter the full path to the directory where you want to install pyenv (press Enter for default): " installDirectory

    # If the user provided a path, validate it
    if [ -n "$installDirectory" ]; then
        while [ ! -d "$installDirectory" ]; do
            echo "Invalid path. Please enter a valid directory path."
            read -p "Enter the full path to the directory where you want to install pyenv (press Enter for default): " installDirectory
        done
    fi

    # Download and install pyenv
    installScriptUrl="https://github.com/pyenv/pyenv-installer/raw/master/bin/pyenv-installer"
    curl -L "$installScriptUrl" | bash

    # Assuming the installation path customization for pyenv is done via environment variables
    if [ -n "$installDirectory" ]; then
        echo 'export PYENV_ROOT="$installDirectory"' >> ~/.bashrc
        echo 'export PATH="$PYENV_ROOT/bin:$PATH"' >> ~/.bashrc
        source ~/.bashrc
    fi

    echo "pyenv installed successfully."
}

function ensure_package_versions() {
    # This is a simplified version since parsing and comparing versions directly in Bash is more complex
    while IFS= read -r line; do
        package=$(echo $line | cut -d'=' -f1)
        version=$(echo $line | cut -d'=' -f2)
        if ! pip list | grep -q "$package ($version)"; then
            echo "Version mismatch or package not found for $package. Installing required version."
            pip install "$package==$version"
        fi
    done < "requirements.txt"

    echo "Package verification and installation/update process complete."
}

# Check if the current directory is the "docs" directory
currentDirectory=$(pwd)
if [ "$(basename "$currentDirectory")" != "docs" ]; then
    echo "Error: This script must be run from the 'docs' directory."
    exit 1
fi

# Check if pyenv is installed
if ! command -v pyenv &> /dev/null; then
    install_pyenv
fi

# Check if __venv directory exists
if [ ! -d "__venv" ]; then
    # Create a virtual environment if it doesn't exist
    python -m venv __venv
fi

# Activate the virtual environment if activation script exists
if [ ! -f "__venv/bin/activate" ]; then
    echo "Error: Activation script for virtual environment not found."
    exit 1
else
    source __venv/bin/activate
fi

# Call ensure_package_versions to check and update packages
ensure_package_versions

# Generate the Doxygen XML
doxygen ./reach.doxyfile.in
