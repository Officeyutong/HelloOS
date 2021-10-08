from elftools.elf.elffile import ELFFile
import sys


def main():
    ifile, ofile = sys.argv[1:]
    with open(ifile, "rb") as f:
        file = ELFFile(f)
        symtab_section = file.get_section_by_name(".symtab")
        # print(symtab_section)
        mymain_entry = symtab_section.get_symbol_by_name("MyMain")[0]
        offset: int = mymain_entry["st_value"]
        text_data = file.get_section_by_name(".text").data()
    with open(ofile, "wb") as f:
        f.write(offset.to_bytes(4, "little"))
        f.write(text_data)


if __name__ == "__main__":
    main()
