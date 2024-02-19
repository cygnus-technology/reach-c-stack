#!/bin/bash

install_pyenv() {
    # Prompt user if they want to install pyenv
    read -p "pyenv is not installed. Would you like to install it? (Y/N) " installPyenv
    if [[ "$installPyenv" != "Y" ]]; then
        echo "Exiting..."
        exit 1
    fi

    # Prompt user for installation directory (skipped for simplicity, uses default pyenv installation path)
    
    # Download and install pyenv
    curl -L https://pyenv.run | bash

    # Update environment variables - Assumes .bashrc or equivalent is used
    echo 'export PATH="$HOME/.pyenv/bin:$PATH"' >> ~/.bashrc
    echo 'eval "$(pyenv init --path)"' >> ~/.bashrc
    echo 'eval "$(pyenv virtualenv-init -)"' >> ~/.bashrc
    source ~/.bashrc

    echo "pyenv installed successfully."
}

set_pyenv_local_version() {
    # Check if .python-version file exists
    if [ -f ".python-version" ]; then
        pythonVersion=$(cat .python-version)
        # Normalize the output of pyenv versions to ensure correct matching
        installedVersions=$(pyenv versions --bare)
        
        # Check if the specified Python version is installed
        if ! echo "$installedVersions" | grep -Fxq "$pythonVersion"; then
            echo "Python version $pythonVersion is not installed. Installing now..."
            pyenv install "$pythonVersion"
        else
            echo "Python version $pythonVersion is already installed."
        fi
        
        # Set the local Python version
        pyenv local "$pythonVersion"
        echo "Set local Python version to $pythonVersion."
    else
        echo ".python-version file not found. Exiting."
        exit 1
    fi
}

ensure_package_versions() {
    while IFS= read -r line || [[ -n "$line" ]]; do
        package=$(echo "$line" | cut -d'=' -f1)
        version=$(echo "$line" | cut -d'=' -f2)
        if ! pip list --format=freeze | grep -q "^$package==$version$"; then
            echo "Version mismatch or package not found for $package. Installing required version."
            pip install "$package==$version"
        fi
    done < "requirements.txt"

    echo "Package verification and installation/update process complete."
}

# Get the current directory
currentDirectory=$(pwd)

# Check if the current directory is the "docs" directory
if [[ "$(basename "$currentDirectory")" != "docs" ]]; then
    echo "Error: This script must be run from the 'docs' directory."
    exit 1
fi

# Check if pyenv is installed
if ! command -v pyenv &> /dev/null; then
    install_pyenv
fi

# After installing pyenv, set the local Python version as per .python-version file
set_pyenv_local_version

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

# Generate the Wiki html
sphinx-build -b html ./__source ./__build
