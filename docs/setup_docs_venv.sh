#!/bin/bash

# Initialize a variable to hold the auto-decision flag (-Y or -N), empty means no decision made
AUTO_DECISION=""
SILENT_MODE=""

check_prerequisites() {
    # List of required packages
    declare -a packages=("build-essential" "curl" "libssl-dev" "libbz2-dev" "libreadline-dev" \
                         "libsqlite3-dev" "libffi-dev" "zlib1g-dev" "liblzma-dev" "libncurses5-dev" \
                         "libncursesw5-dev" "xz-utils" "tk-dev" "libdb-dev" "libgdbm-dev" \
                         "libsqlite3-dev" "openssl" "libbz2-dev" "libreadline-dev")

    echo "Checking for required packages..."
    missing_packages=()

    for pkg in "${packages[@]}"; do
        if ! dpkg -l | grep -qw "$pkg"; then
            missing_packages+=("$pkg")
        fi
    done

    if [ ${#missing_packages[@]} -eq 0 ]; then
        echo "All required packages are already installed."
    else
        echo "The following required packages are missing:"
        printf ' - %s\n' "${missing_packages[@]}"
        echo "Please install them using the following command and try again:"
        echo "sudo apt-get install ${missing_packages[*]}"
    fi
}


check_doxygen_installation() {
    if ! command -v doxygen &> /dev/null; then
        echo "Doxygen is not installed."
        if [[ "$AUTO_DECISION" == "-Y" ]]; then
            echo "Automatically proceeding with Doxygen installation as per the provided flag."
        elif [[ "$AUTO_DECISION" == "-N" ]]; then
            echo "Doxygen installation is essential for this script to function. Exiting due to the no-installation flag."
            exit 1
        else
            read -p "Would you like to install Doxygen? (Y/N) " userDecision
            if [[ "$userDecision" != "Y" ]]; then
                echo "Doxygen installation is essential for this script to function. Exiting due to user choice."
                exit 1
            fi
        fi
        
        echo "Installing Doxygen..."
        sudo apt install -y doxygen
        if ! command -v doxygen &> /dev/null; then
            echo "Failed to install Doxygen."
            exit 1 # Exit the script due to installation failure
        else
            echo "Doxygen installed successfully."
        fi
    else
        echo "Doxygen is already installed."
    fi
}

install_pyenv() {
    if [[ -z "$AUTO_DECISION" ]]; then
        # AUTO_DECISION is empty, ask the user
        read -p "pyenv is not installed. Would you like to install it? (Y/N) " userDecision
        if [[ "$userDecision" != "Y" ]]; then
            echo "pyenv is essential for this script to function. Exiting due to user choice."
            exit 1
        fi
    elif [[ "$AUTO_DECISION" == "-N" ]]; then
        # AUTO_DECISION is -N, exit and explain why
        echo "pyenv is essential for this script to function. Exiting due to the no-installation flag."
        exit 1
    elif [[ "$AUTO_DECISION" == "-Y" ]]; then
        # AUTO_DECISION is -Y, automatically proceed with installation
        echo "Automatically proceeding with pyenv installation as per the provided flag."
    else
        # If AUTO_DECISION is anything other than -Y or -N
        echo "Invalid flag provided for automatic decision. Exiting."
        exit 1
    fi

    # Proceed with pyenv installation
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
    # Capture the output of pip list into a variable
    pip_list=$(pip list --format=freeze)

    while IFS== read -r package version; do
        normalizedPackage=$(echo "$package" | tr '[:upper:]' '[:lower:]')
        version=${version#=} # Ensure leading '=' is removed if present
        
        # Now, grep against the variable content
        if ! echo "$pip_list" | grep -iq "^$normalizedPackage==$version$"; then
            echo "Version mismatch or package not found for $package ($version). Installing required version."
            pip install "$package==$version"
        else
            echo "$package ($version) is already installed with the correct version."
        fi
    done < "requirements.txt"

    echo "Package verification and installation/update process complete."
}

# Process command-line arguments
while [[ "$#" -gt 0 ]]; do
    case $1 in
        -Y|-N) AUTO_DECISION="$1";;
        -s) SILENT_MODE="-qq";; # Set the silent mode flag for apt
        *) echo "Unknown option: $1"; exit 1;;
    esac
    shift
done

# Get the current directory
currentDirectory=$(pwd)

# Check if the current directory is the "docs" directory
if [[ "$(basename "$currentDirectory")" != "docs" ]]; then
    echo "Error: This script must be run from the 'docs' directory."
    exit 1
fi

# Check if the prerequisite packages are installed
check_prerequisites

# Check if Doxygen is installed
check_doxygen_installation

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
