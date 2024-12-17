from git import *
import re

filter_map = [
"<tExt>*",
"<PackID>",
"<RvdsMve>",
"<RvdsCdeCp>",
"<nBranchProt>",
"<uClangAs>",
"<ClangAsOpt>",
"<SFDFile>",
]

git_op = Repo("./").git

status = git_op.status()
status_line = status.split("\n", -1)
modify_keil = []

def filter_modify(params):
    if params == ['']:
        return 1
    for line in params:
        if (line[0] == '-' and line[1] != '-') or (line[0] == '+' and line[1] != '+'):
            match_result = 0
            for match in filter_map:
                r = re.match(r'(.*)'+match+r'(.*)', line, re.M|re.I)
                if r:
                    match_result = 1
            if match_result == 0:
                return 0
    return 1

# search modify document related keil
for line in status_line:
    r1 = re.match(r'(.*).uvoptx',line, re.M|re.I)
    r2 = re.match(r'(.*).uvprojx',line, re.M|re.I)
    if r1 or r2:
        line = line.split(" ", 1)
        modify_keil.append(line[1])

# check if or not revert
for doc in modify_keil:
    doc = doc[2:]
    diff = git_op.diff(doc)
    diff_line = diff.split("\n", -1)
    if filter_modify(diff_line) == 0:
        print("should modify")
    else:
        print("\nrevert doc")
        print(doc)
        git_op.checkout(doc)
