import os
import subprocess
import sys
import shutil

def main():
    """ Run file as if it were invoked directly

    Runs a file forwarding in all the arguments to the file itself. The file chosen is inferred from the name of the
    python executable (i.e. sys.arv[0]). Exits forwarding back the return code.
    """
    binary_file = os.path.join(os.path.dirname(__file__), os.path.basename(sys.argv[0]))
    arguments = [f"{binary_file}"] + sys.argv[1:]
    process = subprocess.run(arguments)
    sys.exit(process.returncode)

if __name__ == "__main__":
    main()