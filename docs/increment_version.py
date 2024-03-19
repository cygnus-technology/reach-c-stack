import re
import sys
 
def find_version(filename):
    try:
        f = open(f"{filename}", "r")
    except Exception as e:
        print(f"ERROR: Failed to access {filename}.  Exception: {e}")
        sys.exit(-1)
    test_text = f.read()
    pattern = re.compile(r"BUILD_VERSION(\s+)(\d+)")
    matches = re.findall(pattern, test_text)
    if len(matches) > 1:
        print(f"ERROR: Found more than one match for the build version in {filename}.")
        sys.exit(-2)
    if (int(matches[0][1]) >= 255):
        print(f"ERROR: Cannot increment build version beyond 255.")
        sys.exit(-3)
    new_text = re.sub(r"BUILD_VERSION\s+(\d+)", f"BUILD_VERSION{matches[0][0]}{int(matches[0][1]) + 1}", test_text)
    try:
        f = open(f"{filename}", "w")
    except Exception as e:
        print(f"ERROR: Failed to access {filename}.  Exception: {e}")
        sys.exit(-4)
    f.write(new_text)
 
if (len(sys.argv) < 2):
    print(f"ERROR: 'versioning.py' needs an argument of the file to be parsed")
    sys.exit(-1)
find_version(sys.argv[1])
