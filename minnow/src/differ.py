def read_asm_file(file_path):
    with open(file_path, 'r') as file:
        asm_code = file.read()
    return asm_code


def compare_asm_files(file1_path, file2_path):
    asm_code1 = read_asm_file(file1_path)
    asm_code2 = read_asm_file(file2_path)

    lines1 = asm_code1.splitlines()
    lines2 = asm_code2.splitlines()

    diff_lines = []
    for line_num, (line1, line2) in enumerate(zip(lines1, lines2), start=1):
        if line1 != line2:
            diff_lines.append(f"Line {line_num}:")
            diff_lines.append(f"{file1_path}: {line1}")
            diff_lines.append(f"{file2_path}: {line2}")
            diff_lines.append("")

    return "\n".join(diff_lines)


if __name__ == "__main__":
    file1_path = "src/0.s"
    file2_path = "src/1.s"

    diff_content = compare_asm_files(file1_path, file2_path)
    with open('diff.txt', 'w') as f:
        if diff_content:
            f.write("The following differences were found:\n")
            f.write(diff_content)
        else:
            f.write("No differences found between the two files.")
