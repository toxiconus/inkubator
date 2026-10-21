from elftools.elf.elffile import ELFFile
from pathlib import Path
addr = 0x40057067
elf_path = Path('.pio') / 'build' / 'esp32s3' / 'firmware.elf'
with elf_path.open('rb') as f:
    elffile = ELFFile(f)
    dwarf = elffile.get_dwarf_info()
    prev_entry = None
    found = False
    for cu in dwarf.iter_CUs():
        lineprog = dwarf.line_program_for_CU(cu)
        if not lineprog:
            continue
        files = lineprog.header.file_entry
        for entry in lineprog.get_entries():
            if entry.state is None:
                continue
            if entry.state.address == addr:
                file_entry = files[entry.state.file - 1]
                fname = file_entry.name.decode('utf-8','replace') if isinstance(file_entry.name, bytes) else file_entry.name
                print('EXACT', fname, entry.state.line, hex(entry.state.address))
                found = True
                break
            if entry.state.address > addr and prev_entry is not None:
                file_entry = files[prev_entry.state.file - 1]
                fname = file_entry.name.decode('utf-8','replace') if isinstance(file_entry.name, bytes) else file_entry.name
                print('NEAREST', fname, prev_entry.state.line, hex(prev_entry.state.address), 'next', hex(entry.state.address))
                found = True
                break
            prev_entry = entry
        if found:
            break
    if not found:
        print('NOT FOUND', hex(addr))
