import os
import sys


def encode_line(text: str) -> bytes:
    assert len(text) == 8
    ret = sum(2**i for i, v in enumerate(text) if v == "*")
    return ret.to_bytes(1, "little")


def main():
    input_file, output_file = sys.argv[1:]
    data = [b"" for i in range(256)]
    with open(input_file, "r", encoding="shiftjis") as ifile:
        while True:
            line = ifile.readline()
            while ifile and not line.startswith("char"):
                line = ifile.readline()
            char_ord = int(line.strip().split()[1], 16)
            if not ifile:
                break
            next_lines = [ifile.readline().strip() for _ in range(16)]
            curr = b"".join(encode_line(x) for x in next_lines)
            data[char_ord] = curr
            if char_ord == 255:
                break
    with open(output_file, "wb") as f:
        for x in data:
            f.write(x)


if __name__ == "__main__":
    main()
