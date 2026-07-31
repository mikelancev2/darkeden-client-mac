import json
import struct
import sys
import os

SCRATCH = os.path.dirname(os.path.abspath(__file__))
EXTRACT_PATH = os.path.join(SCRATCH, "npcscript_extract.json")
OWNER_GLOSSARY_PATH = os.path.join(SCRATCH, "owner_glossary.json")
TRANSLATION_CHUNKS = [os.path.join(SCRATCH, f"translations_{i}.json") for i in range(6)]
OUT_PATH = os.path.join(SCRATCH, "NPCScript_EN.inf")


def load_json(path):
    with open(path, encoding="utf-8-sig") as f:
        return json.load(f)


def main():
    scripts = load_json(EXTRACT_PATH)
    owner_glossary = load_json(OWNER_GLOSSARY_PATH)

    dialogue_map = {}
    missing_chunks = []
    for path in TRANSLATION_CHUNKS:
        if not os.path.exists(path):
            missing_chunks.append(path)
            continue
        chunk = load_json(path)
        dialogue_map.update(chunk)

    if missing_chunks:
        print("WARNING: missing translation chunk files (will pass through Korean for those strings):")
        for m in missing_chunks:
            print("  -", m)

    def translate_owner(s):
        return owner_glossary.get(s, s)

    def translate_dialogue(s):
        if s == "":
            return s
        return dialogue_map.get(s, s)

    total_scripts = len(scripts)
    missing_translation_count = 0
    total_dialogue_strings = 0

    with open(OUT_PATH, "wb") as out:
        out.write(struct.pack("<i", total_scripts))
        for sc in scripts:
            script_id = sc["ScriptID"]
            owner_id = translate_owner(sc["OwnerID"]) if sc["OwnerID"] else sc["OwnerID"]

            subjects = []
            for s in sc["Subjects"]:
                total_dialogue_strings += 1
                t = translate_dialogue(s)
                if s != "" and t == s and s not in dialogue_map:
                    missing_translation_count += 1
                subjects.append(t)

            contents = []
            for c in sc["Contents"]:
                total_dialogue_strings += 1
                t = translate_dialogue(c)
                if c != "" and t == c and c not in dialogue_map:
                    missing_translation_count += 1
                contents.append(t)

            out.write(struct.pack("<I", script_id))

            owner_bytes = owner_id.encode("utf-8")
            out.write(struct.pack("<i", len(owner_bytes)))
            out.write(owner_bytes)

            out.write(struct.pack("<i", len(subjects)))
            for s in subjects:
                b = s.encode("utf-8")
                out.write(struct.pack("<i", len(b)))
                out.write(b)

            out.write(struct.pack("<i", len(contents)))
            for c in contents:
                b = c.encode("utf-8")
                out.write(struct.pack("<i", len(b)))
                out.write(b)

    print(f"Wrote {OUT_PATH}")
    print(f"Total scripts: {total_scripts}")
    print(f"Total dialogue strings (subjects+contents): {total_dialogue_strings}")
    print(f"Strings with NO translation found (left in Korean): {missing_translation_count}")
    print(f"Unique dialogue strings translated: {len(dialogue_map)}")


if __name__ == "__main__":
    main()
