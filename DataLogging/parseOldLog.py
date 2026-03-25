from __future__ import annotations

import argparse
import json
from pathlib import Path


class JsonLineError(ValueError):
	def __init__(self, line_number: int, message: str) -> None:
		super().__init__(f"Invalid JSON on line {line_number}: {message}")
		self.line_number = line_number
		self.message = message


class KeyNode:
	def __init__(self) -> None:
		self.children: dict[str, KeyNode] = {}
		self.value_types: set[str] = set()

	def child(self, name: str) -> "KeyNode":
		if name not in self.children:
			self.children[name] = KeyNode()
		return self.children[name]

	def add_type(self, type_name: str) -> None:
		self.value_types.add(type_name)


def format_type_set(type_names: set[str]) -> str:
	if not type_names:
		return "unknown"

	sorted_types = sorted(type_names)
	if len(sorted_types) == 1:
		return sorted_types[0]

	return f"std::variant<{', '.join(sorted_types)}>"


def infer_cpp_type(value: object) -> str:
	if value is None:
		return "std::nullptr_t"
	if isinstance(value, bool):
		return "bool"
	if isinstance(value, int):
		return "int64_t"
	if isinstance(value, float):
		return "double"
	if isinstance(value, str):
		return "std::string"
	if isinstance(value, dict):
		return "struct"
	if isinstance(value, list):
		if not value:
			return "std::vector<unknown>"
		element_types = {infer_cpp_type(item) for item in value}
		return f"std::vector<{format_type_set(element_types)}>"
	return "unknown"


def visit_value(value: object, node: KeyNode) -> None:
	node.add_type(infer_cpp_type(value))

	if isinstance(value, dict):
		for key, child_value in value.items():
			visit_value(child_value, node.child(key))
		return

	if isinstance(value, list):
		item_node = node.child("[]")
		for item in value:
			visit_value(item, item_node)


def load_records(path: Path, strict: bool = False) -> tuple[list[object], list[int]]:
	text = path.read_text(encoding="utf-8")
	stripped = text.strip()
	if not stripped:
		return [], []

	try:
		parsed = json.loads(stripped)
	except json.JSONDecodeError:
		return load_json_lines(path, strict=strict)

	if isinstance(parsed, list):
		return parsed, []

	return [parsed], []


def load_json_lines(path: Path, strict: bool = False) -> tuple[list[object], list[int]]:
	records: list[object] = []
	skipped_lines: list[int] = []
	with path.open("r", encoding="utf-8") as handle:
		for line_number, raw_line in enumerate(handle, start=1):
			line = raw_line.strip()
			if not line:
				continue
			try:
				records.append(json.loads(line))
			except json.JSONDecodeError as exc:
				if strict:
					raise JsonLineError(line_number, exc.msg) from exc
				skipped_lines.append(line_number)
	return records, skipped_lines


def build_key_tree(records: list[object]) -> KeyNode:
	root = KeyNode()
	for record in records:
		if isinstance(record, dict):
			type_name = record.get("type")
			if isinstance(type_name, str) and type_name:
				type_node = root.child(f"message type: {type_name}")
				type_node.add_type("struct")
				for key, value in record.items():
					if key == "type":
						continue
					visit_value(value, type_node.child(key))
				continue

		visit_value(record, root.child("<unknown>"))
	return root


def render_tree(node: KeyNode, indent: int = 0) -> list[str]:
	lines: list[str] = []
	for key in sorted(node.children):
		child_node = node.children[key]
		lines.append(
			f"{'  ' * indent}{format_type_set(child_node.value_types)} {key};"
		)
		lines.extend(render_tree(child_node, indent + 1))
	return lines


def parse_args() -> argparse.Namespace:
	script_dir = Path(__file__).resolve().parent
	default_input = script_dir / "OldLog.json"

	parser = argparse.ArgumentParser(
		description="List unique JSON keys from OldLog.json and show their nesting."
	)
	parser.add_argument(
		"input_file",
		nargs="?",
		default=default_input,
		type=Path,
		help="Path to the log file. Defaults to OldLog.json next to this script.",
	)
	parser.add_argument(
		"--strict",
		action="store_true",
		help="Stop on the first malformed JSON line instead of skipping bad lines.",
	)
	return parser.parse_args()


def main() -> int:
	args = parse_args()
	input_path = args.input_file.resolve()

	if not input_path.exists():
		raise FileNotFoundError(f"Input file not found: {input_path}")

	records, skipped_lines = load_records(input_path, strict=args.strict)
	key_tree = build_key_tree(records)
	lines = render_tree(key_tree)

	print(f"Scanned {len(records)} record(s) from {input_path}")
	if skipped_lines:
		preview = ", ".join(str(line_number) for line_number in skipped_lines[:10])
		suffix = "" if len(skipped_lines) <= 10 else ", ..."
		print(
			f"Skipped {len(skipped_lines)} malformed line(s): {preview}{suffix}"
		)
	if not lines:
		print("No keys found.")
		return 0

	print("Unique keys by record type:")
	for line in lines:
		print(line)
	return 0


if __name__ == "__main__":
	raise SystemExit(main())
