function Install-Pyenv {
    # Prompt user if they want to install pyenv
    $installPyenv = Read-Host "pyenv is not installed. Would you like to install it? (Y/N)"
    if ($installPyenv -ne "Y") {
        Write-Host "Exiting..."
        exit 1
    }

    # Prompt user for installation directory
    $installDirectory = Read-Host "Enter the full path to the directory where you want to install pyenv (press Enter for default)"

    # If the user provided a path, validate it
    if ($installDirectory -ne "") {
        while (-not (Test-Path -Path $installDirectory -PathType Container)) {
            Write-Host "Invalid path. Please enter a valid directory path."
            $installDirectory = Read-Host "Enter the full path to the directory where you want to install pyenv (press Enter for default)"
        }
    }

    # Download and install pyenv
    $installScriptUrl = "https://raw.githubusercontent.com/pyenv-win/pyenv-win/master/pyenv-win/install-pyenv-win.ps1"
    Invoke-WebRequest -UseBasicParsing -Uri $installScriptUrl -OutFile "~/Downloads/install-pyenv-win.ps1"

    # Modify install-pyenv-win.ps1 if user provided a custom install directory
    if ($installDirectory -ne "") {
        (Get-Content -Path "~/Downloads/install-pyenv-win.ps1") | ForEach-Object {
            $_ -replace '\$PyEnvDir = .*', "\$PyEnvDir = '$installDirectory'"
        } | Set-Content -Path "~/Downloads/install-pyenv-win.ps1"
    }

    # Install pyenv
    . ~/Downloads/install-pyenv-win.ps1

    Write-Host "pyenv installed successfully."
}

function Ensure-PackageVersions {
    # Get a hashtable of installed packages with their versions
    $installedPackagesWithVersions = @{}
    pip list | ForEach-Object {
        if ($_ -match '(\S+)\s+(\S+)') {
            $installedPackagesWithVersions[$matches[1]] = $matches[2]
        }
    }

    # Read the required packages and their versions from 'requirements.txt'
    $requiredPackagesWithVersions = @{}
    Get-Content "requirements.txt" | ForEach-Object {
        if ($_ -match '(\S+)==(\S+)') {
            $requiredPackagesWithVersions[$matches[1]] = $matches[2]
        }
    }

    # Check for missing packages or packages with version mismatches
    foreach ($package in $requiredPackagesWithVersions.Keys) {
        $requiredVersion = $requiredPackagesWithVersions[$package]
        if (-not $installedPackagesWithVersions.ContainsKey($package)) {
            Write-Host "Installing missing package: $package"
            pip install "$package==$requiredVersion"
        }
        elseif ($installedPackagesWithVersions[$package] -ne $requiredVersion) {
            Write-Host "Found package $package, but with a different version (installed: $($installedPackagesWithVersions[$package]), required: $requiredVersion). Updating..."
            pip install "$package==$requiredVersion"
        }
    }

    Write-Host "Package verification and installation/update process complete."
}

# Get the current directory
$currentDirectory = Get-Location

# Check if the current directory is the "docs" directory
if ((Split-Path -Path $currentDirectory -Leaf) -ne "docs") {
    Write-Host "Error: This script must be run from the 'docs' directory."
    exit 1
}

# Check if pyenv is installed
$pyenvPath = (Get-Command pyenv -ErrorAction SilentlyContinue).Path

if (-not $pyenvPath) {
    Install-Pyenv
}

# Check if __venv directory exists
if (-not (Test-Path "__venv")) {
    # Create a virtual environment if it doesn't exist
    python -m venv __venv
}

# Activate the virtual environment if activation script exists
if (-not (Test-Path "__venv\Scripts\Activate")) {
    Write-Host "Error: Activation script for virtual environment not found."
    exit 1
}
else {
    . .\__venv\Scripts\Activate
}

# Call Ensure-PackageVersions to check and update packages
Ensure-PackageVersions

# Generate the Doxygen XML
doxygen ./reach.doxyfile.in
