from __future__ import annotations

import argparse
import json
import sqlite3
from dataclasses import dataclass
from pathlib import Path
from typing import Any, Iterable


DEFAULT_LOG_PATH = Path(
    r"C:\Users\kerst\Desktop\DFMOD\dfhack\plugins\FortressChronicle\DF_Chronicle_The Ageless Universes_Rocksboulder_.log"
)


class EventType:
    JOB_COMPLETED = 0
    UNIT_DEATH = 1
    ITEM_CREATED = 2
    INVASION = 3
    MONTHLY_CITIZEN_LOG = 4
    MONTHLY_ANIMAL_LOG = 5
    MONTHLY_OTHER_LOG = 6
    NEW_CITIZEN = 7
    PETITION = 8
    SIEGE_START = 9
    SIEGE_END = 10
    ANNOUNCEMENT = 100
    INVASION_COUNT_CHANGED = 101


TABLE_DEFINITIONS = {
    "event_records": [
        "day INTEGER",
        "month INTEGER",
        "year INTEGER",
        "tick INTEGER",
        "event_type INTEGER",
        "text TEXT",
    ],
    "job_records": [
        "event_id INTEGER",
        "unit_tid INTEGER",
        "job_name TEXT",
        "job_type INTEGER",
        "material_type INTEGER",
        "material_index INTEGER",
        "material TEXT",
    ],
    "unit_records": [
        "event_id INTEGER",
        "name TEXT",
        "name_english TEXT",
        "age INTEGER",
        "sex TEXT",
        "race TEXT",
        "profession TEXT",
        "fatherId INTEGER",
        "motherId INTEGER",
        "spouseId INTEGER",
        "hostile BOOLEAN",
        "unit_id INTEGER",
        "isAnimal BOOLEAN",
        "isCitizen BOOLEAN",
        "isGuest BOOLEAN",
        "isMerchant BOOLEAN",
        "isPet BOOLEAN",
        "isResident BOOLEAN",
        "caged BOOLEAN",
        "butchered BOOLEAN",
        "isTame BOOLEAN",
        "petOwner INTEGER",
        "invasion_role INTEGER",
    ],
    "item_records": [
        "event_id INTEGER",
        "item_id INTEGER",
        "item_name TEXT",
        "item_type INTEGER",
        "item_type_str TEXT",
        "item_descr TEXT",
        "is_artifact BOOLEAN",
        "mat_index INTEGER",
        "mat_type INTEGER",
        "quality INTEGER",
        "value INTEGER",
        "book_title TEXT",
    ],
    "death_records": [
        "event_id INTEGER",
        "death_cause TEXT",
        "victim_id INTEGER",
        "killer_id INTEGER",
    ],
    "petition_records": [
        "event_id INTEGER",
        "agreement_id INTEGER",
        "year INTEGER",
        "ticks INTEGER",
        "details_type_str TEXT",
        "reason INTEGER",
        "reason_str TEXT",
        "applicant_party INTEGER",
        "government_party INTEGER",
        "site_id INTEGER",
        "end_year INTEGER",
        "end_season_tick INTEGER",
        "location_type INTEGER",
        "location_type_str TEXT",
        "location_tier INTEGER",
        "location_profession INTEGER",
        "location_profession_str TEXT",
        "location_deity_type INTEGER",
        "location_deity_type_str TEXT",
        "location_deity_data INTEGER",
        "location_warned_is_ready BOOLEAN",
        "service_requesting_party INTEGER",
        "service_serving_party INTEGER",
        "service_served_entity INTEGER",
    ],
    "siege_records": [
        "event_id INTEGER",
        "siegeId INTEGER",
        "year INTEGER",
        "mission INTEGER",
        "missionStr TEXT",
        "civID INTEGER",
        "wantsParley BOOLEAN",
        "undead BOOLEAN",
        "planless BOOLEAN",
        "handed_over_artifact BOOLEAN",
    ],
}


@dataclass
class ImportStats:
    records_seen: int = 0
    records_imported: int = 0
    unknown_records: int = 0
    malformed_lines: int = 0


def parse_args() -> argparse.Namespace:
    script_dir = Path(__file__).resolve().parent

    parser = argparse.ArgumentParser(
        description="Import FortressChronicle JSON log lines into the SQLite schema used by DataLogging/*.hpp."
    )
    parser.add_argument(
        "input_file",
        nargs="?",
        default=DEFAULT_LOG_PATH,
        type=Path,
        help="Path to the JSONL log file.",
    )
    parser.add_argument(
        "output_db",
        nargs="?",
        default=script_dir / "imported_fortress_chronicle.db",
        type=Path,
        help="Path to the SQLite database file to create or update.",
    )
    parser.add_argument(
        "--replace",
        action="store_true",
        help="Delete the destination database before importing.",
    )
    parser.add_argument(
        "--strict",
        action="store_true",
        help="Stop on the first malformed JSON line instead of skipping it.",
    )
    return parser.parse_args()


def create_tables(connection: sqlite3.Connection) -> None:
    for table_name, columns in TABLE_DEFINITIONS.items():
        sql = (
            f"CREATE TABLE IF NOT EXISTS {table_name} "
            f"(id INTEGER PRIMARY KEY AUTOINCREMENT,{','.join(columns)});"
        )
        connection.execute(sql)


def ensure_parent(path: Path) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)


def normalize_date(record: dict[str, Any]) -> tuple[int, int, int, int]:
    date = record.get("date") or {}
    return (
        int(date.get("day", -1)),
        int(date.get("month", -1)),
        int(date.get("year", -1)),
        int(date.get("ticks", -1)),
    )


def bool_value(value: Any) -> int:
    return 1 if bool(value) else 0


def int_value(value: Any, default: int = -1) -> int:
    if value is None:
        return default
    if isinstance(value, bool):
        return int(value)
    if isinstance(value, (int, float)):
        return int(value)
    try:
        return int(value)
    except (TypeError, ValueError):
        return default


def float_value(value: Any, default: float = -1.0) -> float:
    if value is None:
        return default
    if isinstance(value, (int, float)):
        return float(value)
    try:
        return float(value)
    except (TypeError, ValueError):
        return default


def text_value(value: Any, default: str = "") -> str:
    if value is None:
        return default
    return str(value)


def insert_row(
    connection: sqlite3.Connection,
    table_name: str,
    columns: list[str],
    values: Iterable[Any],
) -> int:
    placeholders = ",".join("?" for _ in columns)
    sql = f"INSERT INTO {table_name} ({','.join(columns)}) VALUES ({placeholders})"
    cursor = connection.execute(sql, tuple(values))
    return int(cursor.lastrowid)


def insert_event(
    connection: sqlite3.Connection,
    record: dict[str, Any],
    event_type: int,
    text: str,
) -> int:
    day, month, year, tick = normalize_date(record)
    return insert_row(
        connection,
        "event_records",
        ["day", "month", "year", "tick", "event_type", "text"],
        [day, month, year, tick, event_type, text],
    )


def unit_row(event_id: int, unit: dict[str, Any]) -> list[Any]:
    return [
        event_id,
        text_value(unit.get("name")),
        text_value(unit.get("name_english")),
        float_value(unit.get("age")),
        normalize_sex(unit.get("sex")),
        text_value(unit.get("race")),
        text_value(unit.get("profession")),
        int_value(unit.get("fatherId")),
        int_value(unit.get("motherId")),
        int_value(unit.get("spouseId")),
        bool_value(unit.get("hostile")),
        int_value(unit.get("id")),
        bool_value(unit.get("isAnimal")),
        bool_value(unit.get("isCitizen")),
        bool_value(unit.get("isGuest")),
        bool_value(unit.get("isMerchant")),
        bool_value(unit.get("isPet")),
        bool_value(unit.get("isResident")),
        bool_value(unit.get("caged")),
        bool_value(unit.get("butchered")),
        bool_value(unit.get("isTame")),
        int_value(unit.get("petOwner")),
        int_value(unit.get("invasion_role")),
    ]


def normalize_sex(value: Any) -> str:
    if value is None:
        return "Unknown"
    normalized = str(value).strip().lower()
    if normalized in {"0", "female", "f"}:
        return "Female"
    if normalized in {"1", "male", "m"}:
        return "Male"
    return str(value)


def insert_unit(connection: sqlite3.Connection, event_id: int, unit: dict[str, Any]) -> int:
    return insert_row(
        connection,
        "unit_records",
        [
            "event_id",
            "name",
            "name_english",
            "age",
            "sex",
            "race",
            "profession",
            "fatherId",
            "motherId",
            "spouseId",
            "hostile",
            "unit_id",
            "isAnimal",
            "isCitizen",
            "isGuest",
            "isMerchant",
            "isPet",
            "isResident",
            "caged",
            "butchered",
            "isTame",
            "petOwner",
            "invasion_role",
        ],
        unit_row(event_id, unit),
    )


def insert_item(
    connection: sqlite3.Connection,
    event_id: int,
    data: dict[str, Any],
    *,
    book_title: str = "",
) -> int:
    return insert_row(
        connection,
        "item_records",
        [
            "event_id",
            "item_id",
            "item_name",
            "item_type",
            "item_type_str",
            "item_descr",
            "is_artifact",
            "mat_index",
            "mat_type",
            "quality",
            "value",
            "book_title",
        ],
        [
            event_id,
            int_value(data.get("item_id")),
            text_value(data.get("item_name"), book_title),
            int_value(data.get("item_type")),
            text_value(data.get("item_type_str")),
            text_value(data.get("item_descr"), book_title),
            bool_value(data.get("is_artifact")),
            int_value(data.get("mat_index")),
            int_value(data.get("mat_type")),
            int_value(data.get("quality")),
            int_value(data.get("value")),
            book_title,
        ],
    )


def insert_job(
    connection: sqlite3.Connection,
    event_id: int,
    unit_row_id: int,
    data: dict[str, Any],
) -> int:
    job_name = text_value(data.get("job_type")) or text_value(data.get("job_name"))
    return insert_row(
        connection,
        "job_records",
        [
            "event_id",
            "unit_tid",
            "job_name",
            "job_type",
            "material_type",
            "material_index",
            "material",
        ],
        [
            event_id,
            unit_row_id,
            job_name,
            int_value(data.get("job_type_code")),
            int_value(data.get("mat_type")),
            int_value(data.get("mat_index")),
            text_value(data.get("material"), "unknown material"),
        ],
    )


def insert_death(
    connection: sqlite3.Connection,
    event_id: int,
    death_cause: str,
    victim_row_id: int,
    killer_row_id: int,
) -> int:
    return insert_row(
        connection,
        "death_records",
        ["event_id", "death_cause", "victim_id", "killer_id"],
        [event_id, death_cause, victim_row_id, killer_row_id],
    )


def insert_petition(connection: sqlite3.Connection, event_id: int, record: dict[str, Any]) -> int:
    data = record.get("data") or {}
    _, _, year, tick = normalize_date(record)
    petition_type = text_value(data.get("PetitionType")) or text_value(record.get("type"))

    details_type_str = "Residency" if record.get("type") == "PetitionResidency" else "Location"
    reason_text = text_value(data.get("reason"))
    guild_name = text_value(data.get("guild"))

    applicant_party = -1
    if isinstance(data.get("unit"), dict):
        applicant_party = int_value(data["unit"].get("id"))
    elif data.get("id") is not None:
        applicant_party = int_value(data.get("id"))

    location_type_str = guild_name or petition_type
    location_profession_str = text_value(data.get("profession"))

    return insert_row(
        connection,
        "petition_records",
        [
            "event_id",
            "agreement_id",
            "year",
            "ticks",
            "details_type_str",
            "reason",
            "reason_str",
            "applicant_party",
            "government_party",
            "site_id",
            "end_year",
            "end_season_tick",
            "location_type",
            "location_type_str",
            "location_tier",
            "location_profession",
            "location_profession_str",
            "location_deity_type",
            "location_deity_type_str",
            "location_deity_data",
            "location_warned_is_ready",
            "service_requesting_party",
            "service_serving_party",
            "service_served_entity",
        ],
        [
            event_id,
            int_value(data.get("id")),
            year,
            tick,
            details_type_str,
            -1,
            reason_text,
            applicant_party,
            -1,
            -1,
            -1,
            -1,
            -1,
            location_type_str,
            int_value(data.get("tier")),
            -1,
            location_profession_str,
            -1,
            "",
            -1,
            0,
            -1,
            -1,
            -1,
        ],
    )


def insert_siege(connection: sqlite3.Connection, event_id: int, data: dict[str, Any]) -> int:
    flags = data.get("flags") or {}
    army_controller = data.get("army_controller") or {}
    siege_id = int_value(army_controller.get("id"))
    if siege_id == -1:
        siege_id = int_value(data.get("id"))

    return insert_row(
        connection,
        "siege_records",
        [
            "event_id",
            "siegeId",
            "year",
            "mission",
            "missionStr",
            "civID",
            "wantsParley",
            "undead",
            "planless",
            "handed_over_artifact",
        ],
        [
            event_id,
            siege_id,
            int_value(data.get("year")),
            int_value(data.get("mission_code")),
            text_value(data.get("mission")),
            int_value(data.get("civ_id")),
            bool_value(flags.get("wants_parley")),
            bool_value(flags.get("undead")),
            bool_value(flags.get("planless")),
            bool_value(flags.get("handed_over_artifact")),
        ],
    )


def normalize_sequence(value: Any) -> list[dict[str, Any]]:
    if isinstance(value, list):
        return [item for item in value if isinstance(item, dict)]
    if isinstance(value, dict):
        if not value:
            return []
        return [value]
    return []


def import_all_citizens(connection: sqlite3.Connection, record: dict[str, Any]) -> None:
    event_id = insert_event(
        connection,
        record,
        EventType.MONTHLY_CITIZEN_LOG,
        "Log of all active citizens",
    )
    for unit in normalize_sequence(record.get("data")):
        insert_unit(connection, event_id, unit)


def import_visitors_and_others(connection: sqlite3.Connection, record: dict[str, Any]) -> None:
    data = record.get("data") or {}
    all_units: list[dict[str, Any]] = []
    for key in ("ghosts", "guests", "lifestock", "merchants", "pets"):
        all_units.extend(normalize_sequence(data.get(key)))

    animal_units = [unit for unit in all_units if bool(unit.get("isAnimal"))]
    other_units = [unit for unit in all_units if not bool(unit.get("isAnimal"))]

    animal_event_id = insert_event(
        connection,
        record,
        EventType.MONTHLY_ANIMAL_LOG,
        "Log of all active animals",
    )
    for unit in animal_units:
        insert_unit(connection, animal_event_id, unit)

    other_event_id = insert_event(
        connection,
        record,
        EventType.MONTHLY_OTHER_LOG,
        "Log of other active units",
    )
    for unit in other_units:
        insert_unit(connection, other_event_id, unit)


def import_job_completed(connection: sqlite3.Connection, record: dict[str, Any]) -> None:
    data = record.get("data") or {}
    job_type_text = text_value(data.get("job_type")) or text_value(data.get("job_name"))
    event_id = insert_event(
        connection,
        record,
        EventType.JOB_COMPLETED,
        f"JobCompleted: {job_type_text}",
    )
    unit_row_id = -1
    if isinstance(data.get("job_unit"), dict):
        unit_row_id = insert_unit(connection, event_id, data["job_unit"])
    insert_job(connection, event_id, unit_row_id, data)


def import_unit_death(connection: sqlite3.Connection, record: dict[str, Any]) -> None:
    data = record.get("data") or {}
    event_id = insert_event(connection, record, EventType.UNIT_DEATH, "Unit death")
    victim_row_id = -1
    killer_row_id = -1
    if isinstance(data.get("victim"), dict):
        victim_row_id = insert_unit(connection, event_id, data["victim"])
    if isinstance(data.get("killer"), dict):
        killer_row_id = insert_unit(connection, event_id, data["killer"])
    insert_death(
        connection,
        event_id,
        text_value(data.get("death_cause")),
        victim_row_id,
        killer_row_id,
    )


def import_item_created(connection: sqlite3.Connection, record: dict[str, Any]) -> None:
    data = record.get("data") or {}
    item_description = text_value(data.get("item_descr")) or text_value(data.get("item_name"))
    event_id = insert_event(
        connection,
        record,
        EventType.ITEM_CREATED,
        f"Item created: {item_description}",
    )
    insert_item(connection, event_id, data)


def import_book_creation(connection: sqlite3.Connection, record: dict[str, Any]) -> None:
    data = record.get("data") or {}
    title = text_value(data.get("title"))
    event_id = insert_event(
        connection,
        record,
        EventType.ITEM_CREATED,
        f"Book created: {title}",
    )
    insert_item(
        connection,
        event_id,
        {
            "item_id": -1,
            "item_name": title,
            "item_type": -1,
            "item_type_str": "BOOK",
            "item_descr": title,
            "is_artifact": 0,
            "mat_index": -1,
            "mat_type": -1,
            "quality": data.get("quality"),
            "value": -1,
        },
        book_title=title,
    )


def import_citizen(connection: sqlite3.Connection, record: dict[str, Any]) -> None:
    data = record.get("data") or {}
    transition = text_value(data.get("type"))
    event_text = "New Citizens detected"
    if transition:
        event_text = f"Citizen transition: {transition}"
    event_id = insert_event(connection, record, EventType.NEW_CITIZEN, event_text)
    citizen = data.get("citizen")
    if isinstance(citizen, dict):
        insert_unit(connection, event_id, citizen)


def import_petition(connection: sqlite3.Connection, record: dict[str, Any]) -> None:
    details_type = "Residency" if record.get("type") == "PetitionResidency" else "Location"
    event_id = insert_event(
        connection,
        record,
        EventType.PETITION,
        f"New petition: {details_type}",
    )
    data = record.get("data") or {}
    if isinstance(data.get("unit"), dict):
        insert_unit(connection, event_id, data["unit"])
    insert_petition(connection, event_id, record)


def import_announcement(connection: sqlite3.Connection, record: dict[str, Any]) -> None:
    data = record.get("data") or {}
    insert_event(
        connection,
        record,
        EventType.ANNOUNCEMENT,
        text_value(data.get("text"), "Announcement"),
    )


def import_invasion(
    connection: sqlite3.Connection,
    record: dict[str, Any],
    active_siege: dict[str, Any] | None,
) -> dict[str, Any]:
    data = record.get("data") or {}
    event_id = insert_event(connection, record, EventType.SIEGE_START, "New Siege detected")
    insert_siege(connection, event_id, data)
    return data


def import_invasion_count_changed(
    connection: sqlite3.Connection,
    record: dict[str, Any],
    active_siege: dict[str, Any] | None,
) -> dict[str, Any] | None:
    data = record.get("data") or {}
    count = int_value(data.get("count"), 0)
    insert_event(
        connection,
        record,
        EventType.INVASION_COUNT_CHANGED,
        f"Invasion count changed: {count}",
    )

    if count == 0 and active_siege is not None:
        event_id = insert_event(connection, record, EventType.SIEGE_END, "Siege ended")
        insert_siege(connection, event_id, active_siege)
        return None
    return active_siege


def import_record(
    connection: sqlite3.Connection,
    record: dict[str, Any],
    active_siege: dict[str, Any] | None,
) -> tuple[bool, dict[str, Any] | None]:
    record_type = text_value(record.get("type"))

    if record_type == "AllCitizens":
        import_all_citizens(connection, record)
        return True, active_siege
    if record_type == "VisitorsAndOthers":
        import_visitors_and_others(connection, record)
        return True, active_siege
    if record_type == "JobCompleted":
        import_job_completed(connection, record)
        return True, active_siege
    if record_type == "UnitDeath":
        import_unit_death(connection, record)
        return True, active_siege
    if record_type == "ItemCreated":
        import_item_created(connection, record)
        return True, active_siege
    if record_type == "BookCreation":
        import_book_creation(connection, record)
        return True, active_siege
    if record_type == "Citizen":
        import_citizen(connection, record)
        return True, active_siege
    if record_type in {"PetitionResidency", "PetitionGuildhall"}:
        import_petition(connection, record)
        return True, active_siege
    if record_type == "Announcement":
        import_announcement(connection, record)
        return True, active_siege
    if record_type == "Invasion":
        return True, import_invasion(connection, record, active_siege)
    if record_type == "InvasionCountChanged":
        return True, import_invasion_count_changed(connection, record, active_siege)

    return False, active_siege


def import_log(
    input_path: Path,
    output_path: Path,
    *,
    replace: bool,
    strict: bool,
) -> ImportStats:
    if replace and output_path.exists():
        output_path.unlink()

    ensure_parent(output_path)
    stats = ImportStats()
    active_siege: dict[str, Any] | None = None

    with sqlite3.connect(output_path) as connection:
        create_tables(connection)

        with input_path.open("r", encoding="utf-8") as handle:
            for line_number, raw_line in enumerate(handle, start=1):
                line = raw_line.strip()
                if not line:
                    continue

                stats.records_seen += 1
                try:
                    record = json.loads(line)
                except json.JSONDecodeError as exc:
                    if strict:
                        raise ValueError(f"Invalid JSON on line {line_number}: {exc.msg}") from exc
                    stats.malformed_lines += 1
                    continue

                if not isinstance(record, dict):
                    stats.unknown_records += 1
                    continue

                imported, active_siege = import_record(connection, record, active_siege)
                if imported:
                    stats.records_imported += 1
                else:
                    stats.unknown_records += 1

        connection.commit()

    return stats


def main() -> int:
    args = parse_args()
    input_path = args.input_file.resolve()
    output_path = args.output_db.resolve()

    if not input_path.exists():
        raise FileNotFoundError(f"Input file not found: {input_path}")

    stats = import_log(
        input_path,
        output_path,
        replace=args.replace,
        strict=args.strict,
    )

    print(f"Imported {stats.records_imported} record(s) into {output_path}")
    print(f"Read {stats.records_seen} JSON line(s)")
    print(f"Skipped {stats.malformed_lines} malformed line(s)")
    print(f"Skipped {stats.unknown_records} unsupported record(s)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())