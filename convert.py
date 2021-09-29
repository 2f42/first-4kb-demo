out = []
with open("frag_.glsl") as f:
    for line in f.readlines():
        line = line.strip()
        if line == "" or line == "\n":
            line = ""
        elif line[0] == "#":
            #line += "\\n"
            pass
        out.append(line)
out[-1] += "\\0"
with open("frag_.h","w") as f:
    f.write("const char *frag_source = ");
    for line in out:
        f.write("\"" + line + "\\n\"\n")
    f.write(";")
