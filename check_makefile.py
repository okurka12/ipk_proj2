#
# checks that makefile contents are ok with what depend.sh outputs
#
# note: expects that every .c file has a corresponding .o target in makefile
#
import re
import os

RED = "\033[31m"
CYAN = "\033[96m"
YELLOW = "\033[93m"
RESET = "\033[m"

deps = os.popen("./depend.sh").read()
deps = re.findall(r"(?:\S+\.c):\s*(?:[\w\.-]*[ \t]*)*[\w\.-]", deps)

with open("Makefile", "r") as f:
    makefile = f.read()
modules = re.findall(r"\S+\.o\s*:\s*(?:(?:[\w\.-]+\s)+\\?\s*)*", makefile)

# error flag
all_ok = True

# go through all the *.c files and their dependencies
for dep in deps:
    src = dep.split(":")[0]
    assert src.count(".c") == 1
    target = src.replace(".c", ".o")

    # find the correct makefile target
    for module in modules:
        if module.split(":")[0] == target:
            break

    # check that all dependencies are listed
    for depfile in dep.split(":")[1].split():
        if depfile not in module.split(":")[1]:
            print(f"{RED}WARNING{RESET}: {YELLOW}{depfile}{RESET} not listed as a dependency of {CYAN}{target}{RESET}")
            all_ok = False

if all_ok:
    print("all ok")
