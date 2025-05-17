import json

def load_multiple_json_objects_from_lines(json_lines):
    """
    Parses each line (which is a stringified JSON object) into a real JSON object.
    """
    messages = []
    for line in json_lines.splitlines():
        line = line.strip()
        if line:  # skip empty lines
            try:
                obj = json.loads(line)
                messages.append(obj)
            except json.JSONDecodeError as e:
                print(f"⚠️ Skipping invalid JSON line: {line}\nError: {e}")
    return messages

def parse_and_write_json(input_file, output_file):
    with open(input_file, 'r') as infile:
        raw_data = infile.read()

    json_objects = load_multiple_json_objects_from_lines(raw_data)

    with open(output_file, 'w') as outfile:
        json.dump(json_objects, outfile, indent=2)

    print(f"✅ Parsed {len(json_objects)} JSON objects and wrote to '{output_file}'.")

# Usage
if __name__ == "__main__":
    parse_and_write_json("messages.json", "parsed_logs.json")
