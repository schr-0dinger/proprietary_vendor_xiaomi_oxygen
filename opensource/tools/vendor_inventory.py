#!/usr/bin/env python3
"""Build a machine-readable inventory for oxygen and mithorium-common vendor blobs."""

from __future__ import annotations

import argparse
import json
import re
import sys
import xml.etree.ElementTree as ET
from collections import Counter, defaultdict
from pathlib import Path
from typing import Any


ROOT = Path(__file__).resolve().parents[2]

MODULE_TYPES = {
    "android_app_import",
    "cc_prebuilt_library_shared",
    "dex_import",
    "prebuilt_etc_xml",
}


def normalize_path(path: Path) -> str:
    try:
        return path.relative_to(ROOT).as_posix()
    except ValueError:
        return path.as_posix()


def read_logical_make_lines(path: Path) -> list[str]:
    logical_lines: list[str] = []
    current = ""
    for raw_line in path.read_text().splitlines():
        line = raw_line.split("#", 1)[0].rstrip()
        if not line:
            continue
        if line.endswith("\\"):
            current += line[:-1] + " "
            continue
        logical_lines.append((current + line).strip())
        current = ""
    if current:
        logical_lines.append(current.strip())
    return logical_lines


def split_make_tokens(value: str) -> list[str]:
    return [token for token in value.split() if token]


def collect_bp_blocks(path: Path) -> dict[str, dict[str, Any]]:
    blocks: dict[str, dict[str, Any]] = {}
    lines = path.read_text().splitlines()
    i = 0
    while i < len(lines):
        stripped = lines[i].strip()
        block_type = stripped[:-1].strip()
        if stripped.endswith("{") and block_type in MODULE_TYPES:
            block_lines = [lines[i]]
            depth = stripped.count("{") - stripped.count("}")
            i += 1
            while i < len(lines) and depth > 0:
                block_lines.append(lines[i])
                depth += lines[i].count("{") - lines[i].count("}")
                i += 1
            text = "\n".join(block_lines)
            name = extract_first_string(text, "name")
            if name:
                blocks[name] = parse_bp_block(block_type, text)
            continue
        i += 1
    return blocks


def extract_first_string(text: str, key: str) -> str | None:
    match = re.search(rf"{re.escape(key)}\s*:\s*\"([^\"]+)\"", text)
    return match.group(1) if match else None


def extract_string_list(text: str, key: str) -> list[str]:
    match = re.search(rf"{re.escape(key)}\s*:\s*\[([^\]]*)\]", text, re.S)
    if not match:
        return []
    return re.findall(r"\"([^\"]+)\"", match.group(1))


def parse_bp_block(block_type: str, text: str) -> dict[str, Any]:
    item: dict[str, Any] = {
        "block_type": block_type,
        "name": extract_first_string(text, "name"),
        "partition": "system",
        "source_paths": [],
        "install_paths": [],
        "architectures": [],
    }

    if "product_specific: true" in text:
        item["partition"] = "product"
    elif "system_ext_specific: true" in text:
        item["partition"] = "system_ext"
    elif "soc_specific: true" in text or "vendor: true" in text or "proprietary: true" in text:
        item["partition"] = "vendor"

    if block_type == "android_app_import":
        apk = extract_first_string(text, "apk")
        privileged = "privileged: true" in text
        app_dir = "priv-app" if privileged else "app"
        if apk:
            apk_name = Path(apk).name
            item["source_paths"] = [apk]
            item["install_paths"] = [f"{item['partition']}/{app_dir}/{item['name']}/{apk_name}"]
    elif block_type == "dex_import":
        jars = extract_string_list(text, "jars")
        item["source_paths"] = jars
        item["install_paths"] = [f"{item['partition']}/framework/{Path(jar).name}" for jar in jars]
    elif block_type == "prebuilt_etc_xml":
        src = extract_first_string(text, "src")
        sub_dir = extract_first_string(text, "sub_dir") or "etc"
        if src:
            item["source_paths"] = [src]
            item["install_paths"] = [f"{item['partition']}/etc/{sub_dir}/{Path(src).name}"]
    elif block_type == "cc_prebuilt_library_shared":
        arm_sources = extract_target_srcs(text, "android_arm")
        arm64_sources = extract_target_srcs(text, "android_arm64")
        install_paths: list[str] = []
        source_paths: list[str] = []
        architectures: list[str] = []
        if arm_sources:
            architectures.append("arm")
            source_paths.extend(arm_sources)
            install_paths.extend(f"{item['partition']}/lib/{Path(src).name}" for src in arm_sources)
        if arm64_sources:
            architectures.append("arm64")
            source_paths.extend(arm64_sources)
            install_paths.extend(f"{item['partition']}/lib64/{Path(src).name}" for src in arm64_sources)
        compile_multilib = extract_first_string(text, "compile_multilib")
        if not architectures and compile_multilib == "32":
            architectures.append("arm")
        item["source_paths"] = source_paths
        item["install_paths"] = install_paths
        item["architectures"] = architectures
    return item


def extract_target_srcs(text: str, target_name: str) -> list[str]:
    match = re.search(rf"{re.escape(target_name)}\s*:\s*\{{(.*?)\}}", text, re.S)
    if not match:
        return []
    return re.findall(r"srcs\s*:\s*\[([^\]]+)\]", match.group(1), re.S)[0:1] and re.findall(
        r"\"([^\"]+)\"", re.findall(r"srcs\s*:\s*\[([^\]]+)\]", match.group(1), re.S)[0]
    ) or []


def infer_partition_from_copy_dest(dest: str) -> str:
    if dest.startswith("$(TARGET_COPY_OUT_PRODUCT)"):
        return "product"
    if dest.startswith("$(TARGET_COPY_OUT_SYSTEM_EXT)"):
        return "system_ext"
    if dest.startswith("$(TARGET_COPY_OUT_VENDOR)"):
        return "vendor"
    return "system"


def infer_arch_from_path(path: str) -> str | None:
    if "/lib64/" in path or path.startswith("lib64/"):
        return "arm64"
    if "/lib/" in path or path.startswith("lib/"):
        return "arm"
    return None


def infer_layer(source_path: str) -> str:
    if "vendor/xiaomi/mithorium-common/" in source_path:
        return "mithorium-common"
    return "oxygen"


def infer_kind(path: str) -> str:
    basename = Path(path).name
    suffix = Path(path).suffix
    if suffix == ".so":
        return "shared_library"
    if suffix == ".apk":
        return "apk"
    if suffix == ".jar":
        return "jar"
    if suffix == ".rc":
        return "init_rc"
    if suffix == ".xml":
        return "xml"
    if suffix == ".sql":
        return "sql"
    if "/firmware/" in path:
        return "firmware"
    if "." not in basename:
        return "binary"
    return suffix.lstrip(".") or "file"


def infer_subsystem(identifier: str) -> str:
    value = identifier.lower()
    if "consumerir" in value or "infrared" in value:
        return "consumerir"
    if (
        "camera/" in value
        or "mmcamera" in value
        or "chromatix" in value
        or "actuator_" in value
        or "_eeprom" in value
        or "qcamera" in value
    ):
        return "camera"
    if "fingerprint" in value or "libgf_" in value or "lib_fpc" in value:
        return "fingerprint"
    if "sensors" in value or "/sensor" in value:
        return "sensors"
    if any(token in value for token in ("thermal", "perf", "alarm", "hvdcp", "power_off_alarm", "time_daemon")):
        return "perf_thermal_power"
    if any(token in value for token in ("audio", "soundfx", "acdb", "audiosphere")):
        return "audio"
    if any(token in value for token in ("ims", "qcril", "iwlan", "radio", "cne", "uim", "lpa", "dpm", "netmgr")):
        return "radio_ims_data"
    if any(token in value for token in ("gnss", "loc_", "izat", "lowi", "xtra", "xtwifi", "slim")):
        return "location"
    if any(token in value for token in ("bluetooth", "btconfig", "wcnss", "ant@", "fm@")):
        return "connectivity"
    if any(token in value for token in ("adreno", "opencl", "vulkan", "egl", "omx", "llvm", "c2d", "gralloc")):
        return "graphics_media"
    if any(token in value for token in ("widevine", "drm", "qsee", "keystore", "gatekeeper", "tui_comm")):
        return "security"
    return "other"


def merge_item(items_by_id: dict[str, dict[str, Any]], new_item: dict[str, Any]) -> dict[str, Any]:
    existing = items_by_id.get(new_item["id"])
    if existing is None:
        new_item["makefiles"] = [new_item.pop("makefile")]
        new_item["layers"] = [new_item["layer"]]
        items_by_id[new_item["id"]] = new_item
        return new_item

    existing["source_paths"] = sorted(set(existing["source_paths"]) | set(new_item["source_paths"]))
    existing["install_paths"] = sorted(set(existing["install_paths"]) | set(new_item["install_paths"]))
    existing["architectures"] = sorted(set(existing["architectures"]) | set(new_item["architectures"]))
    existing["makefiles"] = sorted(set(existing["makefiles"]) | {new_item["makefile"]})
    existing["layers"] = sorted(set(existing["layers"]) | {new_item["layer"]})
    existing["layer"] = existing["layers"][0] if len(existing["layers"]) == 1 else "mixed"
    return existing


def load_policy(path: Path | None) -> dict[str, Any]:
    if not path:
        return {"subsystem_defaults": {}, "path_overrides": {}, "module_overrides": {}}
    return json.loads(path.read_text())


def classify_item(item: dict[str, Any], policy: dict[str, Any]) -> str:
    for key in item.get("source_paths", []):
        if key in policy.get("path_overrides", {}):
            return policy["path_overrides"][key]
    module_name = item.get("module_name")
    if module_name and module_name in policy.get("module_overrides", {}):
        return policy["module_overrides"][module_name]
    return policy.get("subsystem_defaults", {}).get(item["subsystem"], "keep-binary")


def build_inventory(args: argparse.Namespace) -> dict[str, Any]:
    soong_modules = collect_bp_blocks(ROOT / "Android.bp")
    items_by_id: dict[str, dict[str, Any]] = {}
    items_by_source: dict[str, str] = {}
    items_by_install_base: dict[str, set[str]] = defaultdict(set)

    for mk_name in ("oxygen-vendor.mk", "mithorium-common-vendor.mk"):
        mk_path = ROOT / mk_name
        for line in read_logical_make_lines(mk_path):
            if line.startswith("PRODUCT_COPY_FILES +="):
                for entry in split_make_tokens(line.split("+=", 1)[1]):
                    if ":" not in entry:
                        continue
                    source_path, dest_path = entry.split(":", 1)
                    item_id = dest_path
                    item = {
                        "id": item_id,
                        "entry_type": "copy_file",
                        "module_name": None,
                        "makefile": mk_name,
                        "layer": infer_layer(source_path),
                        "kind": infer_kind(source_path),
                        "partition": infer_partition_from_copy_dest(dest_path),
                        "source_paths": [source_path],
                        "install_paths": [dest_path],
                        "architectures": [arch for arch in [infer_arch_from_path(dest_path)] if arch],
                        "subsystem": infer_subsystem(source_path),
                    }
                    merged_item = merge_item(items_by_id, item)
                    for source in merged_item["source_paths"]:
                        items_by_source[source] = merged_item["id"]
                    for install_path in merged_item["install_paths"]:
                        items_by_install_base[Path(install_path).name].add(merged_item["id"])
            elif line.startswith("PRODUCT_PACKAGES +="):
                for module_name in split_make_tokens(line.split("+=", 1)[1]):
                    module = soong_modules.get(module_name, {})
                    install_paths = module.get("install_paths", [f"module:{module_name}"])
                    item = {
                        "id": f"module:{module_name}",
                        "entry_type": "product_package",
                        "module_name": module_name,
                        "makefile": mk_name,
                        "layer": "oxygen",
                        "kind": module.get("block_type", "module"),
                        "partition": module.get("partition", "unknown"),
                        "source_paths": module.get("source_paths", []),
                        "install_paths": install_paths,
                        "architectures": module.get("architectures", []),
                        "subsystem": infer_subsystem(" ".join([module_name] + module.get("source_paths", []))),
                    }
                    merged_item = merge_item(items_by_id, item)
                    for source_path in merged_item["source_paths"]:
                        items_by_source[source_path] = merged_item["id"]
                    for install_path in merged_item["install_paths"]:
                        items_by_install_base[Path(install_path).name].add(merged_item["id"])

    policy = load_policy(ROOT / args.policy if args.policy else None)
    items = list(items_by_id.values())
    for item in items:
        item["policy"] = classify_item(item, policy)

    edges = build_edges(items_by_id, items_by_source, items_by_install_base)
    summary = {
        "total_items": len(items),
        "by_layer": dict(Counter(item["layer"] for item in items)),
        "by_partition": dict(Counter(item["partition"] for item in items)),
        "by_subsystem": dict(Counter(item["subsystem"] for item in items)),
        "by_policy": dict(Counter(item["policy"] for item in items)),
    }
    return {
        "metadata": {
            "root": normalize_path(ROOT),
            "policy_file": args.policy,
            "makefiles": ["oxygen-vendor.mk", "mithorium-common-vendor.mk"],
        },
        "summary": summary,
        "items": sorted(items, key=lambda item: item["id"]),
        "edges": sorted(edges, key=lambda edge: (edge["type"], edge["from"], edge["to"])),
    }


def build_edges(
    items_by_id: dict[str, dict[str, Any]],
    items_by_source: dict[str, str],
    items_by_install_base: dict[str, set[str]],
) -> list[dict[str, str]]:
    edge_set: set[tuple[str, str, str]] = set()

    for item in items_by_id.values():
        if item["kind"] != "init_rc":
            continue
        source_path = item["source_paths"][0]
        source_file = ROOT / source_path
        if not source_file.exists():
            continue
        for line in source_file.read_text().splitlines():
            stripped = line.strip()
            if not stripped.startswith("service "):
                continue
            parts = stripped.split()
            if len(parts) < 3:
                continue
            binary_basename = Path(parts[2]).name
            for target_id in items_by_install_base.get(binary_basename, set()):
                edge_set.add(("service-binary", item["id"], target_id))

    camera_config = ROOT / "proprietary/vendor/etc/camera/camera_config.xml"
    if camera_config.exists():
        tree = ET.parse(camera_config)
        root = tree.getroot()
        for module in root.findall("CameraModuleConfig"):
            sensor_name = text_or_empty(module, "SensorName")
            eeprom_name = text_or_empty(module, "EepromName")
            actuator_name = text_or_empty(module, "ActuatorName")
            chromatix_name = text_or_empty(module, "ChromatixName")
            sensor_id = items_by_source.get(
                f"vendor/xiaomi/oxygen/proprietary/vendor/lib/libmmcamera_{sensor_name}.so"
            )
            if sensor_id and chromatix_name:
                chromatix_id = items_by_source.get(
                    f"vendor/xiaomi/oxygen/proprietary/vendor/etc/camera/{chromatix_name}.xml"
                )
                if chromatix_id:
                    edge_set.add(("camera-chromatix", sensor_id, chromatix_id))
            if sensor_id and eeprom_name:
                eeprom_id = items_by_source.get(
                    f"vendor/xiaomi/oxygen/proprietary/vendor/lib/libmmcamera_{eeprom_name}_eeprom.so"
                )
                if eeprom_id:
                    edge_set.add(("camera-eeprom", sensor_id, eeprom_id))
            if sensor_id and actuator_name:
                actuator_id = items_by_source.get(
                    f"vendor/xiaomi/oxygen/proprietary/vendor/lib/libactuator_{actuator_name}.so"
                )
                if actuator_id:
                    edge_set.add(("camera-actuator", sensor_id, actuator_id))

    for item in items_by_id.values():
        if not item["kind"] == "xml":
            continue
        source_path = item["source_paths"][0] if item["source_paths"] else ""
        if "/vintf/manifest/" not in source_path:
            continue
        stem = Path(source_path).stem
        normalized_stem = stem.replace("manifest_", "")
        for target_id in items_by_id:
            if target_id == item["id"]:
                continue
            if normalized_stem in target_id:
                edge_set.add(("service-manifest", item["id"], target_id))

    return [{"type": edge_type, "from": source, "to": target} for edge_type, source, target in edge_set]


def text_or_empty(node: ET.Element, tag: str) -> str:
    child = node.find(tag)
    return child.text.strip() if child is not None and child.text else ""


def parse_args(argv: list[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--policy",
        default="opensource/policy/vendor_policy.json",
        help="JSON policy file relative to the repo root.",
    )
    parser.add_argument(
        "--summary",
        action="store_true",
        help="Print only the summary section instead of the full inventory.",
    )
    return parser.parse_args(argv)


def main(argv: list[str]) -> int:
    args = parse_args(argv)
    inventory = build_inventory(args)
    output = inventory["summary"] if args.summary else inventory
    json.dump(output, sys.stdout, indent=2, sort_keys=True)
    sys.stdout.write("\n")
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
