# Python script to increment the patch version of the Reach-C-stack
import re

def update_patch_version(file_path):
    try:
        # Read the content of the file
        with open(file_path, 'r') as file:
            content = file.read()

        # Define the pattern to search for PATCH_VERSION = <integer>
        pattern = r'(REACH_C_PATCH_VERSION\s*)(\d+)'

        # Find the integer value of REACH_C_PATCH_VERSION
        match = re.search(pattern, content)
        if match:
            old_version = int(match.group(2))
            new_version = old_version + 1
            updated_content = re.sub(pattern, r'\g<1>' + str(new_version), content)

            # Write the updated content back to the file
            with open(file_path, 'w') as file:
                file.write(updated_content)

            print(f'REACH_C_PATCH_VERSION updated from {old_version} to {new_version}.')
        else:
            print('REACH_C_PATCH_VERSION not found in the file.')
    except FileNotFoundError:
        print(f'File {file_path} not found.')


# Example usage:
update_patch_version('include/reach_version.h')

