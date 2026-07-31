import struct
import sys
import os

SCRATCH = os.path.dirname(os.path.abspath(__file__))
PATH = sys.argv[1] if len(sys.argv) > 1 else os.path.join(SCRATCH, "NPCScript_EN.inf")


def read_mstring(f):
    (length,) = struct.unpack("<i", f.read(4))
    if length == 0:
        return ""
    if length < 0 or length > 65536:
        raise ValueError(f"Bad string length {length} at offset {f.tell()}")
    data = f.read(length)
    if len(data) != length:
        raise EOFError(f"Expected {length} bytes, got {len(data)} at offset {f.tell()}")
    return data.decode("utf-8")


with open(PATH, "rb") as f:
    (total,) = struct.unpack("<i", f.read(4))
    print("Declared total scripts:", total)
    parsed = 0
    sample = []
    for i in range(total):
        (script_id,) = struct.unpack("<I", f.read(4))
        owner = read_mstring(f)
        (subj_count,) = struct.unpack("<i", f.read(4))
        subjects = [read_mstring(f) for _ in range(subj_count)]
        (cont_count,) = struct.unpack("<i", f.read(4))
        contents = [read_mstring(f) for _ in range(cont_count)]
        parsed += 1
        if i < 3:
            sample.append((script_id, owner, subjects, contents))

    trailing = f.read()
    print("Parsed scripts OK:", parsed)
    print("Trailing bytes after last script (should be 0):", len(trailing))

    for sid, owner, subjects, contents in sample:
        print("---")
        print("ScriptID:", sid, "Owner:", owner)
        for s in subjects:
            print("  Subject:", s[:120])
        for c in contents:
            print("  Content:", c[:120])
