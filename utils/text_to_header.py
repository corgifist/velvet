import sys

if len(sys.argv) != 4:
    print("usage: python3 text_to_header.py IN OUT NAME")
    exit(1)

input = sys.argv[1]
output = sys.argv[2]
name = sys.argv[3]

lines = open(input, "r+").readlines()

template = \
f"""#ifndef TEXT_TO_HEADER_{name}
#define TEXT_TO_HEADER_{name}
const char *s_{name} = \\
INSERT
;
#endif
"""

insert = ""
double_quote = "\""
backslash = "\\"
new_line = "\n"
for line in lines:
    insert += f'"{line.replace(backslash, backslash + backslash).replace(double_quote, backslash + double_quote).replace(new_line, "")}\\n"\n'

template = template.replace("INSERT", insert)
open(output, "w+").write(template)
print("generated", output)