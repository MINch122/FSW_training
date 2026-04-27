"""Load and decode telemetry definitions from mission-local JSON files.

Telemetry JSON format:

    {
      "packets": [
        {
          "app": "PAYUEL_CAM",
          "name": "Beacon",
          "mid": "0x089C",
          "description": "Optional UI description",
          "payload_offset": 16,
          "fields": [
            { "name": "BootCount", "type": "uint16" },
            { "name": "ReturnValue", "type": "bytes", "length": 512,
              "display_length_from": "ReturnDataSize" }
          ]
        }
      ]
    }

Supported field types match payload.py:
  - scalar integers/floats
  - string
  - bytes
  - enum

Optional field display helpers:
  - format: "hex"
  - hex_digits: integer width for hex formatting
  - units: suffix string
  - scale: numeric multiplier for display only
  - display_length_from: use a previously decoded field to shorten the shown
    portion of a fixed-size bytes/string field (the packet still advances by
    the full storage length)
"""

import json
import struct
from dataclasses import dataclass, field
from pathlib import Path
from typing import Dict, List, Optional, Tuple

from .loader import load_mission_defs, resolve_int
from .payload import SCALAR_TYPES

TELEMETRY_DEFS_FILENAME = 'telemetry.json'
DEFAULT_PAYLOAD_OFFSET = 16


@dataclass
class TelemetryDef:
    name: str
    app: str
    mid: int
    description: str = ''
    payload_offset: int = DEFAULT_PAYLOAD_OFFSET
    fields: List[dict] = field(default_factory=list)

    def __str__(self) -> str:
        return f"{self.app}/{self.name} MID=0x{self.mid:04X}"


def load_telemetry_defs(directory: str) -> Dict[int, TelemetryDef]:
    """Load telemetry definitions from *directory*/telemetry.json."""
    path = Path(directory)
    if not path.is_dir():
        return {}

    defs = load_mission_defs(path)
    tlm_path = path / TELEMETRY_DEFS_FILENAME
    if not tlm_path.exists():
        return {}

    try:
        with tlm_path.open(encoding='utf-8') as fh:
            raw = json.load(fh)
    except Exception as exc:
        print(f"[telemetry] Warning: could not parse {TELEMETRY_DEFS_FILENAME}: {exc}")
        return {}

    packets_raw = raw.get('packets', raw) if isinstance(raw, dict) else raw
    if not isinstance(packets_raw, list):
        print(f"[telemetry] Warning: {TELEMETRY_DEFS_FILENAME} must contain a list or a dict with 'packets'")
        return {}

    packets: Dict[int, TelemetryDef] = {}
    for packet_raw in packets_raw:
        try:
            packet = _parse_packet(packet_raw, defs)
        except Exception as exc:
            print(f"[telemetry] Warning: skipping packet {packet_raw!r}: {exc}")
            continue

        if packet.mid in packets:
            print(f"[telemetry] Warning: MID 0x{packet.mid:04X} redefined by {packet}")
        packets[packet.mid] = packet

    return packets


def load_telemetry_file(directory: str) -> List[dict]:
    """Load the raw telemetry.json packet list from *directory*."""
    tlm_path = Path(directory) / TELEMETRY_DEFS_FILENAME
    if not tlm_path.exists():
        return []

    with tlm_path.open(encoding='utf-8') as fh:
        raw = json.load(fh)

    packets = raw.get('packets', raw) if isinstance(raw, dict) else raw
    if not isinstance(packets, list):
        raise ValueError(f"{TELEMETRY_DEFS_FILENAME} must contain a list or a dict with 'packets'")

    return list(packets)


def save_telemetry_file(directory: str, packets: List[dict]) -> Path:
    """Write *packets* to *directory*/telemetry.json."""
    path = Path(directory)
    path.mkdir(parents=True, exist_ok=True)
    tlm_path = path / TELEMETRY_DEFS_FILENAME
    payload = {'packets': packets}
    with tlm_path.open('w', encoding='utf-8') as fh:
        json.dump(payload, fh, indent=2)
    return tlm_path


def decode_telemetry_packet(data: bytes, packet_def: TelemetryDef,
                            default_endian: str = '<') -> Tuple[List[Tuple[str, str]], Optional[str]]:
    """Decode *data* according to *packet_def*.

    Returns a list of ``(field_name, rendered_value)`` tuples plus an optional
    decode error string.  Partial results are returned if decoding fails partway
    through a packet.
    """
    cursor = int(packet_def.payload_offset)
    context: Dict[str, object] = {}
    decoded: List[Tuple[str, str]] = []

    if cursor > len(data):
        return decoded, f"payload offset {cursor} exceeds packet size {len(data)}"

    for field_def in packet_def.fields:
        label = str(field_def.get('label', field_def.get('name', '?')))
        try:
            raw_value, rendered_value, cursor = _decode_field(
                data, cursor, field_def, context, default_endian
            )
        except Exception as exc:
            return decoded, f"{label}: {exc}"

        context[str(field_def.get('name', label))] = raw_value
        if not field_def.get('hidden', False):
            decoded.append((label, rendered_value))

    return decoded, None


def _parse_packet(raw: dict, defs: Dict[str, int]) -> TelemetryDef:
    if not isinstance(raw, dict):
        raise TypeError("packet definition must be a dict")

    mid_raw = raw['mid']
    if isinstance(mid_raw, str):
        mid = int(mid_raw, 16) if mid_raw.startswith(('0x', '0X')) else int(mid_raw)
    else:
        mid = int(mid_raw)

    payload_offset = resolve_int(raw.get('payload_offset', DEFAULT_PAYLOAD_OFFSET), defs)
    fields = _resolve_fields(list(raw.get('fields', [])), defs)

    return TelemetryDef(
        name=str(raw['name']),
        app=str(raw.get('app', 'Unknown')),
        mid=mid,
        description=str(raw.get('description', '')),
        payload_offset=payload_offset,
        fields=fields,
    )


def _resolve_fields(fields: List[dict], defs: Dict[str, int]) -> List[dict]:
    resolved: List[dict] = []
    for field_def in fields:
        item = dict(field_def)
        for key in ('length', 'count'):
            if key in item and isinstance(item[key], str):
                try:
                    item[key] = resolve_int(item[key], defs)
                except (TypeError, ValueError):
                    pass
        resolved.append(item)
    return resolved


def _decode_field(data: bytes, offset: int, field_def: dict, context: Dict[str, object],
                  endian: str) -> Tuple[object, str, int]:
    ptype = field_def['type']
    count = int(field_def.get('count', 1))

    if ptype in SCALAR_TYPES:
        raw, offset, elem_size = _decode_scalar_values(data, offset, ptype, count, endian)
        return raw, _format_scalar_values(raw, field_def, elem_size), offset

    if ptype == 'enum':
        storage = str(field_def.get('storage', 'uint16'))
        raw, offset, elem_size = _decode_scalar_values(data, offset, storage, count, endian)
        return raw, _format_enum_values(raw, field_def, elem_size), offset

    if ptype == 'string':
        length = int(field_def.get('length', 0))
        raw_bytes = _slice_bytes(data, offset, length)
        offset += length
        shown_len = _display_length(field_def, context, length)
        shown = raw_bytes[:shown_len].split(b'\x00', 1)[0].decode('ascii', errors='replace')
        return shown, f'"{shown}"', offset

    if ptype == 'bytes':
        length = int(field_def.get('length', 0))
        raw_bytes = _slice_bytes(data, offset, length)
        offset += length
        shown_len = _display_length(field_def, context, length)
        return raw_bytes, _format_bytes(raw_bytes, shown_len), offset

    raise ValueError(f"unknown field type {ptype!r}")


def _decode_scalar_values(data: bytes, offset: int, ptype: str, count: int,
                          endian: str) -> Tuple[object, int, int]:
    fmt_char, elem_size = SCALAR_TYPES[ptype]
    values: List[object] = []
    for _ in range(count):
        if offset + elem_size > len(data):
            raise ValueError(f"needs {elem_size} bytes at offset {offset}, packet size is {len(data)}")
        values.append(struct.unpack_from(endian + fmt_char, data, offset)[0])
        offset += elem_size
    if count == 1:
        return values[0], offset, elem_size
    return values, offset, elem_size


def _slice_bytes(data: bytes, offset: int, length: int) -> bytes:
    if offset + length > len(data):
        raise ValueError(f"needs {length} bytes at offset {offset}, packet size is {len(data)}")
    return data[offset:offset + length]


def _display_length(field_def: dict, context: Dict[str, object], default_length: int) -> int:
    ref = field_def.get('display_length_from')
    if ref is None:
        return default_length

    if isinstance(ref, int):
        return max(0, min(default_length, ref))

    if isinstance(ref, str):
        value = context.get(ref)
        if isinstance(value, list):
            value = value[0] if value else 0
        try:
            return max(0, min(default_length, int(value)))
        except (TypeError, ValueError):
            return default_length

    return default_length


def _format_scalar_values(raw_value: object, field_def: dict, elem_size: int) -> str:
    if isinstance(raw_value, list):
        return '[' + ', '.join(_format_scalar_value(v, field_def, elem_size) for v in raw_value) + ']'
    return _format_scalar_value(raw_value, field_def, elem_size)


def _format_scalar_value(raw_value: object, field_def: dict, elem_size: int) -> str:
    fmt = str(field_def.get('format', '')).lower()
    units = str(field_def.get('units', '')).strip()
    scale = field_def.get('scale')

    if fmt == 'hex':
        digits = int(field_def.get('hex_digits', elem_size * 2))
        mask = (1 << (elem_size * 8)) - 1
        text = f"0x{(int(raw_value) & mask):0{digits}X}"
    elif scale is not None:
        text = f"{float(raw_value) * float(scale):g}"
    else:
        text = str(raw_value)

    if units:
        text = f"{text} {units}"
    return text


def _format_enum_values(raw_value: object, field_def: dict, elem_size: int) -> str:
    values = field_def.get('values', {})
    reverse = {int(v): str(k) for k, v in values.items()}

    if isinstance(raw_value, list):
        return '[' + ', '.join(_format_enum_value(v, reverse, field_def, elem_size) for v in raw_value) + ']'
    return _format_enum_value(raw_value, reverse, field_def, elem_size)


def _format_enum_value(raw_value: object, labels: Dict[int, str], field_def: dict, elem_size: int) -> str:
    label = labels.get(int(raw_value))
    numeric = _format_scalar_value(raw_value, field_def, elem_size)
    if label:
        return f"{label} ({numeric})"
    return numeric


def _format_bytes(raw_bytes: bytes, shown_len: int) -> str:
    shown_len = max(0, min(len(raw_bytes), shown_len))
    if shown_len == 0:
        return "(empty)"

    preview = raw_bytes[:shown_len]
    text = ' '.join(f'{b:02X}' for b in preview)

    if shown_len != len(raw_bytes):
        return f"{text} ({shown_len}/{len(raw_bytes)}B shown)"
    return f"{text} ({shown_len}B)"
