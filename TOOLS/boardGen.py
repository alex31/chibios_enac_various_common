#!/usr/bin/env python3
"""Generate a ChibiOS board header from an STM32 board configuration.

This is the Python successor to boardGen.pl.  It uses only the Python standard
library and reads the XML database installed with STM32CubeMX (or STM32 Open
Pin Data).
"""

from __future__ import annotations

import argparse
import ast
from collections import defaultdict
from dataclasses import dataclass, field
from decimal import Decimal, InvalidOperation
from pathlib import Path
import os
import re
import sys
import xml.etree.ElementTree as ET


class BoardGenError(Exception):
    """An input or database error suitable for display to the user."""


FIELDS = (
    "pin", "name", "mode", "otype", "speed", "pupdr", "level", "af",
    "ascr", "lockr", "adc_channel", "voltage",
)

FIELD_NAMES = {
    "pin": "PIN", "name": "NAME", "mode": "MODE", "otype": "TYPE",
    "speed": "SPEED", "pupdr": "PUPDR", "level": "INITIAL LEVEL",
    "af": "ALTERNATE FUNCTION", "ascr": "ANALOG SWITCH",
    "lockr": "LOCK SWITCH", "adc_channel": "ADC CHANNEL",
    "voltage": "CONNECTED VOLTAGE",
}

RANKS = {
    "MODER": ("mode", "PIN_MODE"),
    "OTYPER": ("otype", "PIN_OTYPE"),
    "OSPEEDR": ("speed", "PIN_OSPEED"),
    "PUPDR": ("pupdr", "PIN_PUPDR"),
    "ODR": ("level", "PIN_ODR"),
    "ASCR": ("ascr", "PIN_ASCR"),
    "LOCKR": ("lockr", "PIN_LOCKR"),
}


def _profile(mode: str, otype: str = "PUSHPULL", speed: str = "SPEED_HIGH",
             pupdr: str = "FLOATING", level: str = "LEVEL_HIGH",
             ascr: str = "ASCR_DISABLED", lockr: str = "LOCKR_DISABLED",
             af: str | None = None, adc_channel: int | None = 0) -> dict[str, object]:
    values: dict[str, object] = {
        "mode": mode, "otype": otype, "speed": speed, "pupdr": pupdr,
        "level": level, "ascr": ascr, "lockr": lockr,
    }
    if af is not None:
        values["af"] = af
    if adc_channel is not None:
        values["adc_channel"] = adc_channel
    return values


def make_parameter_profiles(input_voltage: float) -> dict[str, dict[str, object]]:
    profiles = {
        "SYS": _profile("ALTERNATE", af="0"),
        "OSC": _profile("INPUT", af="0"),
        "ADC": _profile("ANALOG", speed="SPEED_VERYLOW", level="LEVEL_LOW",
                        ascr="ASCR_ENABLED", af="0", adc_channel=None),
        "COMPIN": _profile("ANALOG", speed="SPEED_VERYLOW", level="LEVEL_LOW",
                           ascr="ASCR_ENABLED", af="0", adc_channel=None),
        "COMPOUT": _profile("ALTERNATE", level="LEVEL_LOW", adc_channel=0),
        "OPAMP": _profile("ANALOG", speed="SPEED_VERYLOW", level="LEVEL_LOW",
                          ascr="ASCR_ENABLED", af="0", adc_channel=None),
        "DAC": _profile("ANALOG", speed="SPEED_VERYLOW", level="LEVEL_LOW",
                        ascr="ASCR_ENABLED", af="0", adc_channel=None),
        "TIM": _profile("ALTERNATE", level="LEVEL_LOW", adc_channel=0),
        "GPT": _profile("ALTERNATE", level="LEVEL_LOW", adc_channel=0),
        "PWM": _profile("ALTERNATE", level="LEVEL_LOW", adc_channel=0),
        "ICU": _profile("ALTERNATE", adc_channel=0),
        "I2C": _profile("ALTERNATE", otype="OPENDRAIN", adc_channel=0),
        "SPI": _profile("ALTERNATE", adc_channel=0),
        "UART": _profile("ALTERNATE", adc_channel=0),
        "USART": _profile("ALTERNATE", adc_channel=0),
        "OTG": _profile("ALTERNATE", adc_channel=0),
        "ULPI": _profile("ALTERNATE", speed="SPEED_MEDIUM", level="LEVEL_LOW",
                         adc_channel=0),
        "ETH": _profile("ALTERNATE", adc_channel=0),
        "FSMC": _profile("ALTERNATE", adc_channel=0),
        "LTDC": _profile("ALTERNATE", adc_channel=0),
        "SDIO": _profile("ALTERNATE", pupdr="PULLUP", adc_channel=0),
        "SDIOCK": _profile("ALTERNATE", adc_channel=0),
        "SWDIO": _profile("ALTERNATE", pupdr="PULLUP", adc_channel=0),
        "SWCLK": _profile("ALTERNATE", pupdr="PULLDOWN", adc_channel=0),
        "CAN": _profile("ALTERNATE", adc_channel=0),
        "DCMI": _profile("ALTERNATE", adc_channel=0),
        "LED": _profile("OUTPUT", speed="SPEED_VERYLOW", level="LEVEL_LOW", af="0"),
        "PASSIVE": _profile("INPUT", otype="OPENDRAIN", speed="SPEED_VERYLOW",
                            pupdr="PULLDOWN", af="0"),
        "DEFAULT": _profile("INPUT", otype="OPENDRAIN", speed="SPEED_VERYLOW",
                            pupdr="PULLDOWN", af="0"),
        "ALTERNATE": {"mode": "ALTERNATE", "level": "LEVEL_HIGH",
                      "ascr": "ASCR_DISABLED", "lockr": "LOCKR_DISABLED",
                      "adc_channel": 0},
        "OUTPUT": {"mode": "OUTPUT", "speed": "SPEED_VERYLOW", "pupdr": "FLOATING",
                   "level": "LEVEL_LOW", "af": "0", "ascr": "ASCR_DISABLED",
                   "lockr": "LOCKR_DISABLED", "adc_channel": 0},
        "INPUT": {"mode": "INPUT", "otype": "OPENDRAIN", "speed": "SPEED_VERYLOW",
                  "level": "LEVEL_LOW", "af": "0", "ascr": "ASCR_DISABLED",
                  "lockr": "LOCKR_DISABLED", "adc_channel": 0},
        "ANALOG": {"mode": "ANALOG", "otype": "PUSHPULL", "speed": "SPEED_VERYLOW",
                   "pupdr": "FLOATING", "level": "LEVEL_LOW", "af": "0",
                   "ascr": "ASCR_ENABLED", "lockr": "LOCKR_DISABLED"},
        "PUSHPULL": {"otype": "PUSHPULL", "pupdr": "FLOATING"},
        "OPENDRAIN": {"otype": "OPENDRAIN", "pupdr": "PULLUP"},
        "SPEED_VERYLOW": {"speed": "SPEED_VERYLOW"},
        "SPEED_LOW": {"speed": "SPEED_LOW"},
        "SPEED_MEDIUM": {"speed": "SPEED_MEDIUM"},
        "SPEED_HIGH": {"speed": "SPEED_HIGH"},
        "FLOATING": {"pupdr": "FLOATING"},
        "PULLUP": {"pupdr": "PULLUP"},
        "PULLDOWN": {"pupdr": "PULLDOWN"},
        "LEVEL_HIGH": {"level": "LEVEL_HIGH"},
        "LEVEL_LOW": {"level": "LEVEL_LOW"},
        "ASCR_ENABLED": {"ascr": "ASCR_ENABLED"},
        "ASCR_DISABLED": {"ascr": "ASCR_DISABLED", "adc_channel": 0},
        "LOCKR_ENABLED": {"lockr": "LOCKR_ENABLED"},
        "LOCKR_DISABLED": {"lockr": "LOCKR_DISABLED"},
    }
    for profile in profiles.values():
        profile["voltage"] = input_voltage
    return profiles


def local_name(element: ET.Element) -> str:
    return element.tag.rsplit("}", 1)[-1]


def descendants(element: ET.Element, name: str) -> list[ET.Element]:
    return [item for item in element.iter() if local_name(item) == name]


def child_elements(element: ET.Element, name: str) -> list[ET.Element]:
    return [item for item in element if local_name(item) == name]


def element_text(element: ET.Element) -> str:
    return "".join(element.itertext()).strip()


@dataclass
class CubeDatabase:
    root: Path
    ports: list[str] = field(default_factory=list)
    gpio_af: dict[str, dict[str, int]] = field(default_factory=lambda: defaultdict(dict))
    signals_by_pin: dict[str, list[str]] = field(default_factory=dict)
    non_af_pins: dict[str, set[str]] = field(default_factory=lambda: defaultdict(set))
    dma_by_function: dict[str, list[tuple[int, int, int, str]]] = field(
        default_factory=lambda: defaultdict(list)
    )
    mcu_dom: ET.Element | None = None

    def load_mcu(self, mcu: str) -> None:
        path = self.root / "mcu" / f"{mcu}.xml"
        if not path.is_file():
            raise BoardGenError(f"incorrect MCU name {mcu!r}; expected {path}")
        try:
            self.mcu_dom = ET.parse(path).getroot()
        except (ET.ParseError, OSError) as error:
            raise BoardGenError(f"cannot read MCU database {path}: {error}") from error
        self._load_gpio(self.mcu_dom)
        self._load_signals(self.mcu_dom)
        self._load_dma(self.mcu_dom)

    def _load_gpio(self, dom: ET.Element) -> None:
        gpio_path = None
        for ip in descendants(dom, "IP"):
            if ip.get("ConfigFile", "").startswith("GPIO-"):
                gpio_path = self.root / "mcu" / "IP" / f"{ip.get('Name')}-{ip.get('Version')}_Modes.xml"
                break
        if gpio_path is None or not gpio_path.is_file():
            raise BoardGenError("cannot locate the GPIO modes XML in the selected database")
        try:
            gpio_dom = ET.parse(gpio_path).getroot()
        except (ET.ParseError, OSError) as error:
            raise BoardGenError(f"cannot read GPIO database {gpio_path}: {error}") from error

        for parameter in descendants(gpio_dom, "RefParameter"):
            if parameter.get("Name") == "GPIOx":
                for value in descendants(parameter, "PossibleValue"):
                    match = re.fullmatch(r"GPIO([A-P])", value.get("Value", ""))
                    if match and match.group(1) not in self.ports:
                        self.ports.append(match.group(1))
                break
        for port in descendants(gpio_dom, "GPIO_Port"):
            match = re.fullmatch(r"P([A-P])", port.get("Name", ""))
            if match and match.group(1) not in self.ports:
                self.ports.append(match.group(1))

        parents = {child: parent for parent in gpio_dom.iter() for child in parent}
        for value in descendants(gpio_dom, "PossibleValue"):
            match = re.match(r"GPIO_AF(\d{1,2})_\w+", element_text(value))
            if not match:
                continue
            try:
                signal_node = parents[parents[value]]
                pin_node = parents[signal_node]
            except KeyError:
                continue
            signal = signal_node.get("Name")
            pin = normalize_pin(pin_node.get("Name", ""))
            if signal and pin:
                self.gpio_af[signal][pin] = int(match.group(1))

    def _load_signals(self, dom: ET.Element) -> None:
        for pin_node in descendants(dom, "Pin"):
            pin = normalize_pin(pin_node.get("Name", ""))
            if not pin:
                continue
            signals = [item.get("Name", "") for item in child_elements(pin_node, "Signal")]
            signals = [signal for signal in signals if signal and not signal.startswith("GPIO")]
            self.signals_by_pin[pin] = signals
            for signal in signals:
                if pin not in self.gpio_af.get(signal, {}):
                    self.non_af_pins[signal].add(pin)

    def _load_dma(self, dom: ET.Element) -> None:
        dma_path = None
        version = ""
        for ip in descendants(dom, "IP"):
            if re.search(r"[BD]?DMA$", ip.get("InstanceName", "")):
                version = ip.get("Version", "")
                dma_path = self.root / "mcu" / "IP" / f"{ip.get('Name')}-{version}_Modes.xml"
        if dma_path is None or not dma_path.is_file():
            return
        if "dma_v1" in version:
            self._load_dma_modes(dma_path, request_word="REQUEST", unit_word="Channel", kind="R")
        elif "dma_v2_0" in version:
            self._load_dma_modes(dma_path, request_word="CHANNEL", unit_word="Stream", kind="C")
        else:
            print(f"warning: DMA version {version!r} is not supported", file=sys.stderr)

    def _load_dma_modes(self, path: Path, *, request_word: str,
                        unit_word: str, kind: str) -> None:
        dom = ET.parse(path).getroot()
        default: dict[str, int] = {}
        marker = re.compile(rf"DMA_{request_word}_(\d+)")
        for mode in descendants(dom, "RefMode"):
            name = mode.get("Name", "")
            if not mode.get("BaseMode", "").startswith("DMA_Request") or ":" in name:
                continue
            default[name] = 0
            for value in descendants(mode, "PossibleValue"):
                match = marker.search(element_text(value))
                if match:
                    default[name] = int(match.group(1))
                    break
        instance = re.compile(rf"DMA(\d)_{unit_word}(\d+)")
        explicit = re.compile(rf"(\S+):DMA_{request_word}_(\d+)")
        for mode in descendants(dom, "Mode"):
            match = instance.match(mode.get("Name", ""))
            if not match:
                continue
            controller, stream = map(int, match.groups())
            for child in descendants(mode, "Mode"):
                if child is mode:
                    continue
                assignment = child.get("Name", "")
                direct = explicit.search(assignment)
                if direct:
                    peripheral, channel = direct.group(1), int(direct.group(2))
                elif ":Conflict:" not in assignment:
                    peripheral = assignment
                    if peripheral not in default:
                        print(f"warning: no DMA {request_word.lower()} for {peripheral}", file=sys.stderr)
                        continue
                    channel = default[peripheral]
                else:
                    continue
                self.dma_by_function[peripheral].append((controller, stream, channel, kind))

    def matching_mcus(self, pattern: str) -> list[str]:
        pattern = re.sub(r".*M32", "", pattern)
        try:
            matcher = re.compile(pattern)
        except re.error as error:
            raise BoardGenError(f"invalid MCU regular expression: {error}") from error
        directory = self.root / "mcu"
        return sorted(
            path.stem for path in directory.glob("STM32*.xml")
            if "STM32MP" not in path.name and matcher.search(path.name)
        )


def normalize_pin(value: str) -> str | None:
    match = re.match(r"(P[A-P])(\d{1,2})", value)
    return f"{match.group(1)}{int(match.group(2)):02d}" if match else None


@dataclass
class GroupDefinition:
    name: str
    expression: str
    line_number: int


@dataclass
class Config:
    parameters: dict[str, str] = field(default_factory=dict)
    header: list[str] = field(default_factory=list)
    pins: dict[str, dict[str, object]] = field(default_factory=dict)
    groups: list[GroupDefinition] = field(default_factory=list)
    ordered_groups: list[tuple[str, list[str], int]] = field(default_factory=list)
    passive_macros: list[str] = field(default_factory=list)


class GroupExpression:
    """Parser for the deliberately small, backwards-compatible GROUP language."""

    TOKEN = re.compile(
        r"\s*(?:(%NAME|%AF|%MODE|%ALIAS)|(eq|ne|=~|!~|&&|\|\||!)|"
        r"([()])|(/(?:\\.|[^/])*/[imsx]*)|('(?:\\.|[^'])*'|\"(?:\\.|[^\"])*\")|"
        r"([A-Za-z0-9_.:-]+))"
    )

    def __init__(self, expression: str, values: dict[str, str]):
        self.expression = expression
        self.values = values
        self.tokens = self._tokenize(expression)
        self.index = 0

    def evaluate(self) -> bool:
        result = self._or_expression()
        if self.index != len(self.tokens):
            raise BoardGenError(f"unexpected token {self.tokens[self.index][1]!r}")
        return bool(result)

    def _tokenize(self, expression: str) -> list[tuple[str, object]]:
        tokens: list[tuple[str, object]] = []
        position = 0
        while position < len(expression):
            match = self.TOKEN.match(expression, position)
            if not match:
                raise BoardGenError(f"unsupported syntax near {expression[position:]!r}")
            variable, operator, paren, regex_token, string, bare = match.groups()
            if variable:
                tokens.append(("value", self.values.get(variable[1:], "")))
            elif operator:
                tokens.append(("operator", operator))
            elif paren:
                tokens.append((paren, paren))
            elif regex_token:
                end = regex_token.rfind("/")
                pattern, flags_text = regex_token[1:end], regex_token[end + 1:]
                flags = sum({"i": re.I, "m": re.M, "s": re.S, "x": re.X}[flag]
                            for flag in flags_text)
                try:
                    tokens.append(("regex", re.compile(pattern, flags)))
                except re.error as error:
                    raise BoardGenError(f"invalid GROUP regular expression: {error}") from error
            elif string:
                try:
                    tokens.append(("value", ast.literal_eval(string)))
                except (SyntaxError, ValueError) as error:
                    raise BoardGenError(f"invalid quoted GROUP value {string}") from error
            else:
                tokens.append(("value", bare))
            position = match.end()
        return tokens

    def _accept(self, kind: str, value: object | None = None) -> object | None:
        if self.index >= len(self.tokens):
            return None
        token_kind, token_value = self.tokens[self.index]
        if token_kind == kind and (value is None or token_value == value):
            self.index += 1
            return token_value
        return None

    def _or_expression(self) -> bool:
        result = self._and_expression()
        while self._accept("operator", "||") is not None:
            right = self._and_expression()
            result = bool(result or right)
        return bool(result)

    def _and_expression(self) -> bool:
        result = self._unary()
        while self._accept("operator", "&&") is not None:
            right = self._unary()
            result = bool(result and right)
        return bool(result)

    def _unary(self) -> bool:
        if self._accept("operator", "!") is not None:
            return not self._unary()
        if self._accept("(") is not None:
            result = self._or_expression()
            if self._accept(")") is None:
                raise BoardGenError("missing ')' in GROUP expression")
            return result
        left = self._accept("value")
        if left is None:
            raise BoardGenError("expected a value in GROUP expression")
        operator = self._accept("operator")
        if operator is None:
            return bool(left)
        if operator in ("=~", "!~"):
            regex = self._accept("regex")
            if regex is None:
                raise BoardGenError(f"{operator} must be followed by /regex/")
            matched = bool(regex.search(str(left)))
            return matched if operator == "=~" else not matched
        if operator in ("eq", "ne"):
            right = self._accept("value")
            if right is None:
                raise BoardGenError(f"{operator} must be followed by a value")
            equal = str(left) == str(right)
            return equal if operator == "eq" else not equal
        raise BoardGenError(f"unexpected operator {operator!r}")


class BoardGenerator:
    def __init__(self, database: CubeDatabase, *, input_voltage: float,
                 pin_prefix: bool, line_prefix: bool):
        self.db = database
        self.profiles = make_parameter_profiles(input_voltage)
        self.pin_prefix = pin_prefix
        self.line_prefix = line_prefix
        self.config = Config()
        self.pin_names: dict[str, str] = {}
        self.line_names: dict[str, str] = {}

    def parse(self, path: Path) -> None:
        try:
            lines = path.read_text(encoding="utf-8").splitlines()
        except OSError as error:
            raise BoardGenError(f"cannot read {path}: {error}") from error
        state = "parameters"
        seen: set[str] = set()
        for number, original in enumerate(lines, 1):
            line = original.rstrip()
            if state == "parameters":
                match = re.search(r"(\w+)\s?=\s?(\S+)", line)
                if match:
                    self.config.parameters[match.group(1)] = match.group(2)
                elif line.startswith("HEADER"):
                    state = "header"
                continue
            if state == "header":
                if line.startswith("CONFIG"):
                    state = "config"
                else:
                    self.config.header.append(line + "\n")
                continue
            line = re.sub(r"#.*", "", line)
            line = re.sub(r"//.*", "", line).strip()
            if not line:
                continue
            if line.startswith("GROUP "):
                match = re.fullmatch(r"GROUP\s+(\S+)\s+(.+)", line)
                if not match:
                    raise BoardGenError(f"{path}:{number}: malformed GROUP")
                self.config.groups.append(GroupDefinition(match.group(1), match.group(2), number))
                continue
            if line.startswith("ORDERED_GROUP "):
                match = re.fullmatch(r"ORDERED_GROUP\s+(\S+)\s+(.+)", line)
                if not match:
                    raise BoardGenError(f"{path}:{number}: malformed ORDERED_GROUP")
                names = [item for item in re.sub(r"\s+", "", match.group(2)).split(",") if item]
                self.config.ordered_groups.append((match.group(1), names, number))
                continue
            line = re.sub(r"\s+(?=[^()]*\))", "", line)
            words = re.split(r"\s+", line)
            if len(words) < 3:
                raise BoardGenError(f"{path}:{number}: malformed line: {line}")
            pin = words.pop(0)
            if pin != "DEFAULT":
                match = re.fullmatch(r"P?([A-P]\d{2})", pin)
                if not match:
                    raise BoardGenError(f"{path}:{number}: bad pin name {pin!r}")
                pin = match.group(1)
                name = words.pop(0)
            else:
                name = pin
            values = dict.fromkeys(FIELDS)
            values.update(pin=pin, name=name)
            current_function = ""
            for word in words:
                if word in self.profiles:
                    values["alias"] = word
                    values.update(self.profiles[word])
                    continue
                passive = re.fullmatch(r"\((.*)\)", word)
                if passive:
                    possible = passive.group(1) or current_function
                    if not possible:
                        raise BoardGenError(f"{path}:{number}: passive list must follow a function")
                    self._parse_passive(line, pin, name, possible)
                    continue
                af = self._af_by_name(pin, word)
                if af is not None:
                    current_function = word
                    values["symbolic_af"] = word
                    values["af"] = str(af)
                    continue
                if self._is_non_af(pin, word):
                    values["adc_channel"] = 1
                    current_function = word
                    continue
                voltage = re.match(r"([0-9.]*)[^\s]*volt", word, re.I)
                if voltage:
                    try:
                        values["voltage"] = float(voltage.group(1))
                    except ValueError as error:
                        raise BoardGenError(f"{path}:{number}: invalid voltage {word!r}") from error
                    continue
                raise BoardGenError(f"{path}:{number}: function {word!r} is not routable on {pin}")
            missing = [FIELD_NAMES[key] for key in FIELDS if values.get(key) is None]
            if missing:
                raise BoardGenError(f"{path}:{number}: missing {', '.join(missing)}: {line}")
            if pin in seen or name in seen:
                raise BoardGenError(f"{path}:{number}: duplicate pin or name {pin!r}/{name!r}")
            if values["mode"] == "OUTPUT" and values["otype"] == "PUSHPULL" \
                    and values["pupdr"] != "FLOATING":
                raise BoardGenError(f"{path}:{number}: push-pull output {pin} uses a pull resistor")
            self.config.pins[pin] = values
            seen.update((pin, name))
        if state != "config":
            raise BoardGenError(f"{path}: missing HEADER or CONFIG section")
        if "DEFAULT" not in self.config.pins:
            default = dict.fromkeys(FIELDS)
            default.update(self.profiles["DEFAULT"], pin="DEFAULT", name="DEFAULT")
            self.config.pins["DEFAULT"] = default

    def _af_by_name(self, pin: str, signal: str) -> int | None:
        match = re.match(r"^AF(\d+)", signal) or re.fullmatch(r"(\d+)", signal)
        if match:
            af = int(match.group(1))
        elif signal.startswith("AF:"):
            af = self.db.gpio_af.get(signal[3:], {}).get("P" + pin)
            if af is None:
                return None
        else:
            return None
        # AF15 support is intentionally settled at final review; STM32 defines 0..15.
        if af not in range(16):
            raise BoardGenError(f"incorrect alternate-function number {af}")
        return af

    def _is_non_af(self, pin: str, signal: str) -> bool:
        return not signal.startswith("AF") and "P" + pin in self.db.non_af_pins.get(signal, set())

    def _parse_passive(self, line: str, pin: str, name: str, functions: str) -> None:
        seen_types: set[str] = set()
        for function in functions.split(","):
            type_match = re.search(r"([A-Z]+)\d", function)
            if type_match and type_match.group(1) in seen_types:
                raise BoardGenError(f"{line}: more than one passive function of type {type_match.group(1)}")
            if type_match:
                seen_types.add(type_match.group(1))
            af = self._af_by_name(pin, function)
            if af is None and not self._is_non_af(pin, function):
                raise BoardGenError(f"{line}: function {function} cannot be associated with {pin}")
            self._add_passive_macros(name, function, af)

    def _add_passive_macros(self, name: str, function: str, af: int | None) -> None:
        function = function.replace("AF:", "")
        match = re.search(r"([A-Z]+\d?[A-Z]+)(\d*)_([A-Z]+)(\d*)", function)
        if not match:
            raise BoardGenError(f"cannot create passive macros from function {function!r}")
        peripheral_type, number, operation, operation_number = match.groups()
        self.config.passive_macros.extend((
            f"#define {name}_{peripheral_type}\t {number}\n",
            f"#define {name}_{peripheral_type}_FN\t {operation}\n",
        ))
        if operation_number:
            self.config.passive_macros.append(
                f"#define {name}_{peripheral_type}_{operation}\t {operation_number}\n"
            )
        if af is not None:
            self.config.passive_macros.append(f"#define {name}_{peripheral_type}_AF\t {af}\n")

    def validate(self) -> None:
        parameters = self.config.parameters
        mcu = parameters.get("MCU_MODEL")
        if not mcu:
            raise BoardGenError("MCU_MODEL is mandatory")
        try:
            version = Decimal(parameters.get("CHIBIOS_VERSION", ""))
        except InvalidOperation as error:
            raise BoardGenError("CHIBIOS_VERSION must be a number") from error
        if not version.is_finite():
            raise BoardGenError("CHIBIOS_VERSION must be a finite number")
        if version < 3:
            raise BoardGenError("CHIBIOS_VERSION below 3 is not supported")
        if not self.db.ports:
            raise BoardGenError("GPIO ports list is missing from the database")
        l4_registers = mcu.startswith("STM32L4")
        for pin, values in self.config.pins.items():
            if pin == "DEFAULT":
                continue
            database_pin = "P" + pin
            if values["mode"] == "ANALOG" and l4_registers and values["ascr"] == "ASCR_ENABLED":
                if not any(re.match(r"ADC\d_IN[PN]?\d+", signal)
                           for signal in self.db.signals_by_pin.get(database_pin, [])):
                    raise BoardGenError(f"{database_pin} has no ADC capability but ASCR is enabled")
            if values["mode"] in ("INPUT", "ALTERNATE") or (
                values["mode"] == "OUTPUT" and values["otype"] == "OPENDRAIN"
            ):
                is_adc = any(re.match(r"ADC\d_IN[PN]?\d+", signal)
                             for signal in self.db.signals_by_pin.get(database_pin, []))
                if float(values["voltage"]) > 3.33 and is_adc:
                    raise BoardGenError(
                        f"forbidden {values['voltage']} V on non-5V-tolerant ADC pin "
                        f"{database_pin} [{values['name']}]"
                    )
            af = int(values["af"])
            if af:
                if values["mode"] != "ALTERNATE":
                    raise BoardGenError(f"{database_pin} has AF{af} but is not in ALTERNATE mode")
                if database_pin not in self.db.gpio_af.get(str(values.get("symbolic_af", ""))[3:], {}) \
                        and af not in self._afs_for_pin(database_pin):
                    raise BoardGenError(f"AF{af} cannot be assigned to {database_pin} [{values['name']}]")
        # Validate every expression even when a group happens to have no matching pins.
        for group in self.config.groups:
            try:
                GroupExpression(group.expression, {
                    "NAME": "", "AF": "", "MODE": "", "ALIAS": ""
                }).evaluate()
            except BoardGenError as error:
                raise BoardGenError(f"GROUP {group.name} at line {group.line_number}: {error}") from error

    def _afs_for_pin(self, pin: str) -> set[int]:
        return {mapping[pin] for mapping in self.db.gpio_af.values() if pin in mapping}

    def generate(self) -> str:
        output = [HEADER, *self.config.header, "/*\n * IO pins assignments.\n */\n"]
        for port in self.db.ports:
            for number in range(16):
                key = f"{port}{number:02d}"
                name = f"P{key}"
                if key in self.config.pins:
                    configured = str(self.config.pins[key]["name"])
                    name = f"{name}_{configured}" if self.pin_prefix else configured
                self.pin_names[key] = name
                output.append(f"#define\t{name:<30} {number}U\n")
            output.append("\n")
        output.append("/*\n * IO lines assignments.\n */\n")
        for port in self.db.ports:
            emitted = False
            for number in range(16):
                key = f"{port}{number:02d}"
                if key not in self.config.pins:
                    continue
                base = f"LINE_{key}" if self.line_prefix else "LINE"
                line_name = f"{base}_{self.config.pins[key]['name']}"
                self.line_names[str(self.config.pins[key]["name"])] = line_name
                output.append(f"#define\t{line_name:<30} PAL_LINE(GPIO{port}, {number}U)\n")
                emitted = True
            if emitted:
                output.append("\n")
        output.append(MACROS)
        l4_registers = self.config.parameters["MCU_MODEL"].startswith("STM32L4")
        if l4_registers:
            output.append(LOCK_ASCR_MACROS)
        for port in self.db.ports:
            for register in ("MODER", "OTYPER", "OSPEEDR", "PUPDR", "ODR"):
                output.append(self._register_macro(port, register, 0, 15) + "\n")
            output.append(self._af_register_macro(port, "AFRL", 0, 7) + "\n")
            output.append(self._af_register_macro(port, "AFRH", 8, 15) + "\n")
            if l4_registers:
                output.append(self._register_macro(port, "ASCR", 0, 15) + "\n")
                output.append(self._register_macro(port, "LOCKR", 0, 15) + "\n")
        output.extend(self._af_defines())
        output.extend(self.config.passive_macros)
        output.append("\n")
        output.extend(self._group_macros())
        output.append(FOOTER)
        return "".join(output)

    def _pin_value(self, port: str, number: int, field: str) -> object:
        return self.config.pins.get(f"{port}{number:02d}", self.config.pins["DEFAULT"])[field]

    def _register_macro(self, port: str, register: str, start: int, end: int) -> str:
        field, prefix = RANKS[register]
        parts = [f"{prefix}_{self._pin_value(port, number, field)}({self.pin_names[f'{port}{number:02d}']})"
                 for number in range(start, end + 1)]
        return self._multiline_macro(f"VAL_GPIO{port}_{register}", parts)

    def _af_register_macro(self, port: str, register: str, start: int, end: int) -> str:
        parts = [f"PIN_AFIO_AF({self.pin_names[f'{port}{number:02d}']}, "
                 f"{self._pin_value(port, number, 'af')})" for number in range(start, end + 1)]
        return self._multiline_macro(f"VAL_GPIO{port}_{register}", parts)

    @staticmethod
    def _multiline_macro(name: str, parts: list[str]) -> str:
        indentation = " " * 41
        return f"#define {name:<32}(" + (" | \\\n" + indentation).join(parts) + ")\n"

    def _af_defines(self) -> list[str]:
        result: list[str] = []
        for database_pin in sorted(self.db.signals_by_pin):
            key = database_pin[1:]
            if key not in self.config.pins or self.config.pins[key]["mode"] != "ALTERNATE":
                continue
            values = self.config.pins[key]
            pin_name = f"_{database_pin}" if self.pin_prefix else ""
            line_key = f"_{key}" if self.line_prefix else ""
            result.append(f"{'#define AF' + pin_name + '_' + str(values['name']):<40} {values['af']}U\n".replace("-", "_"))
            result.append(f"{'#define AF_LINE' + line_key + '_' + str(values['name']):<40} {values['af']}U\n".replace("-", "_"))
        result.append("\n\n")
        return result

    def _group_macros(self) -> list[str]:
        result = [GROUP_HELPERS]
        for group in self.config.groups:
            members: list[str] = []
            configured_keys = (
                f"{port}{number:02d}"
                for port in self.db.ports
                for number in range(16)
                if f"{port}{number:02d}" in self.config.pins
            )
            for key in configured_keys:
                values = self.config.pins[key]
                context = {
                    "NAME": str(values["name"]), "AF": str(values.get("symbolic_af", "")),
                    "MODE": str(values["mode"]), "ALIAS": str(values.get("alias", "")),
                }
                try:
                    if GroupExpression(group.expression, context).evaluate():
                        members.append(self.line_names[str(values["name"])])
                except BoardGenError as error:
                    raise BoardGenError(f"GROUP {group.name} at line {group.line_number}: {error}") from error
            result.extend(self._one_group(group.name, members))
        for name, pin_names, line_number in self.config.ordered_groups:
            missing = [pin for pin in pin_names if pin not in self.line_names]
            if missing:
                raise BoardGenError(f"ORDERED_GROUP {name} at line {line_number}: undefined pin(s): {', '.join(missing)}")
            result.extend(self._one_group(name, [self.line_names[pin] for pin in pin_names]))
        return result

    @staticmethod
    def _one_group(name: str, members: list[str]) -> list[str]:
        if members:
            definition = f"#define {name} \\\n\t" + ", \\\n\t".join(members) + "\n"
        else:
            definition = f"#define {name} \n"
            print(f"warning: group {name} has no members", file=sys.stderr)
        return [definition, f"#define {name}_SIZE \t {len(members)}\n\n"]


HEADER = """/*
    ChibiOS - Copyright (C) 2006..2015 Giovanni Di Sirio

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#pragma once

"""

MACROS = """
/*
 * I/O ports initial setup, this configuration is established soon after reset
 * in the initialization code.
 * Please refer to the STM32 Reference Manual for details.
 */
#define PIN_MODE_INPUT(n)           (0U << ((n) * 2U))
#define PIN_MODE_OUTPUT(n)          (1U << ((n) * 2U))
#define PIN_MODE_ALTERNATE(n)       (2U << ((n) * 2U))
#define PIN_MODE_ANALOG(n)          (3U << ((n) * 2U))
#define PIN_ODR_LEVEL_LOW(n)        (0U << (n))
#define PIN_ODR_LEVEL_HIGH(n)       (1U << (n))
#define PIN_OTYPE_PUSHPULL(n)       (0U << (n))
#define PIN_OTYPE_OPENDRAIN(n)      (1U << (n))
#define PIN_OSPEED_SPEED_VERYLOW(n) (0U << ((n) * 2U))
#define PIN_OSPEED_SPEED_LOW(n)     (1U << ((n) * 2U))
#define PIN_OSPEED_SPEED_MEDIUM(n)  (2U << ((n) * 2U))
#define PIN_OSPEED_SPEED_HIGH(n)    (3U << ((n) * 2U))
#define PIN_PUPDR_FLOATING(n)       (0U << ((n) * 2U))
#define PIN_PUPDR_PULLUP(n)         (1U << ((n) * 2U))
#define PIN_PUPDR_PULLDOWN(n)       (2U << ((n) * 2U))
#define PIN_AFIO_AF(n, v)           ((v) << (((n) % 8U) * 4U))

"""

LOCK_ASCR_MACROS = """#define PIN_ASCR_DISABLED(n)        (0U << (n))
#define PIN_ASCR_ENABLED(n)         (1U << (n))
#define PIN_LOCKR_DISABLED(n)       (0U << (n))
#define PIN_LOCKR_ENABLED(n)        (1U << (n))

"""

GROUP_HELPERS = """#define BOARD_GROUP_DECLFOREACH(line, group) \\
  static const ioline_t group ## _ARRAY[] = {group}; \\
  for (ioline_t i=0, line = group ## _ARRAY[i]; (i < group ## _SIZE) && (line = group ## _ARRAY[i]); i++)

#define BOARD_GROUP_FOREACH(line, group) \\
  for (ioline_t i=0, line = group ## _ARRAY[i]; (i < group ## _SIZE) && (line = group ## _ARRAY[i]); i++)

#define BOARD_GROUP_DECLFOR(array, index, group) \\
  static const ioline_t group ## _ARRAY[] = {group}; \\
  for (ioline_t index=0, *array = (ioline_t *) group ## _ARRAY; index < group ## _SIZE; index++)

#define BOARD_GROUP_FOR(array, index, group) \\
  for (ioline_t index=0, *array = (ioline_t *) group ## _ARRAY; index < group ## _SIZE; index++)

"""

FOOTER = """#if !defined(_FROM_ASM_)
#ifdef __cplusplus
extern "C" {
#endif
  void boardInit(void);
#ifdef __cplusplus
}
#endif
#endif /* _FROM_ASM_ */

"""


def discover_database(explicit: Path | None, open_pin_data: Path | None) -> Path:
    if explicit and open_pin_data:
        raise BoardGenError("--cubemx-root and --open-pin-data are mutually exclusive")
    if explicit or open_pin_data:
        candidates = [explicit or open_pin_data]
    else:
        home = Path.home()
        candidates = [
            home / "STM32CubeMX",
            Path("/usr/local/STMicroelectronics/STM32Cube/STM32CubeMX"),
            Path(r"C:\Program Files (x86)\STMicroelectronics\STM32Cube\STM32CubeMX"),
            Path(r"C:\Program Files\STMicroelectronics\STM32Cube\STM32CubeMX"),
        ]
        plugins = home / "opt" / "CHIBISTUDIO" / "eclipse" / "plugins"
        if plugins.is_dir():
            candidates.extend(sorted(plugins.glob("com.st.microxplorer.rcp_*"), reverse=True))
    for candidate in candidates:
        if candidate is None:
            continue
        candidate = candidate / "db" if (candidate / "db").is_dir() else candidate
        if (candidate / "mcu").is_dir():
            return candidate
    raise BoardGenError("cannot find STM32CubeMX/Open Pin Data; use --cubemx-root")


def read_config_parameters(path: Path) -> dict[str, str]:
    """Read the leading key/value block needed to select the MCU database."""
    try:
        lines = path.read_text(encoding="utf-8").splitlines()
    except OSError as error:
        raise BoardGenError(f"cannot read {path}: {error}") from error
    parameters: dict[str, str] = {}
    for line in lines:
        if line.startswith("HEADER"):
            break
        match = re.search(r"(\w+)\s?=\s?(\S+)", line)
        if match:
            parameters[match.group(1)] = match.group(2)
    return parameters


def write_kicad_files(database: CubeDatabase, mcu: str, directory: Path) -> None:
    if database.mcu_dom is None:
        raise BoardGenError("MCU data has not been loaded")
    safe_name = re.sub(r"[()-]", "", mcu)
    rows: list[tuple[str, str, str, str]] = []
    for pin in descendants(database.mcu_dom, "Pin"):
        name = pin.get("Name", "")
        position = pin.get("Position", "")
        pin_type = pin.get("Type", "")
        if name.startswith("VSS"):
            side = "bottom"
        elif name.startswith("VDD"):
            side = "top"
        elif re.match(r"P[A-M]\d{1,2}", name):
            side = "right"
        else:
            side = "left"
        pin_type = re.sub(r"Reset|Boot", "in", pin_type)
        pin_type = pin_type.replace("I/O", "bi").replace("MonoIO", "un").replace("Power", "pwr")
        rows.append((position, name, pin_type, side))
    rows.sort(key=lambda row: re.sub(r"P(.)(\d)$", r"P\g<1>0\g<2>", row[1]))
    csv_text = f"{safe_name},,,\nPin,name,type,side\n" + "".join(",".join(row) + "\n" for row in rows)
    functions = []
    for pin in sorted(database.signals_by_pin):
        rendered = [f"{signal}:{database.gpio_af[signal][pin]}"
                    if pin in database.gpio_af.get(signal, {}) else signal
                    for signal in database.signals_by_pin[pin]]
        functions.append(f"{pin} : {' '.join(rendered)}\n")
    try:
        (directory / f"{safe_name}.csv").write_text(csv_text, encoding="utf-8")
        (directory / f"{safe_name}_allFunctions.txt").write_text("".join(functions), encoding="utf-8")
    except OSError as error:
        raise BoardGenError(f"cannot write KiCad symbol data: {error}") from error


def print_af_matches(database: CubeDatabase, query: str) -> None:
    try:
        tokens = [re.compile(token, re.I) for token in query.split(",")]
    except re.error as error:
        raise BoardGenError(f"invalid --find regular expression: {error}") from error
    for pin in sorted(database.signals_by_pin):
        for signal in database.signals_by_pin[pin]:
            if not all(token.search(signal) for token in tokens):
                continue
            af = database.gpio_af.get(signal, {}).get(pin)
            print(f"{signal} on {pin} has AF{af}" if af is not None
                  else f"{signal} on {pin} is not alternate")


def print_dma_matches(database: CubeDatabase, query: str) -> None:
    try:
        pattern = re.compile(query)
    except re.error as error:
        raise BoardGenError(f"invalid --dma regular expression: {error}") from error
    for function, routes in database.dma_by_function.items():
        display = re.sub(r"(\w+)_(\w+)/(\w+)", r"\1_\2 / \1_\3", function)
        if not pattern.search(display):
            continue
        macro = display.replace(" / ", " / ")
        for controller, stream, channel, kind in routes:
            print(f"{display} : [stream({controller}, {stream}) {kind}{channel}]")
            print(f"#define STM32_{macro}_DMA_STREAM\t\tSTM32_DMA_STREAM_ID({controller}, {stream})")
            label = "REQUEST" if kind == "R" else "CHANNEL"
            print(f"#define STM32_{macro}_DMA_{label}\t\t{channel}")
            print(f"#define STM32_{macro}_DMA_IRQ_PRIORITY\t\t6")
            print(f"#define STM32_{macro}_DMA_PRIORITY\t\t2\n")


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("config", nargs="?", type=Path, help="board configuration file")
    parser.add_argument("output", nargs="?", type=Path, help="output header (default: stdout)")
    parser.add_argument("--cubemx-root", type=Path, help="STM32CubeMX installation or database root")
    parser.add_argument("--open-pin-data", type=Path, help="STM32 Open Pin Data root")
    parser.add_argument("--mcu", metavar="REGEX", help="select/list an MCU without a config file")
    modes = parser.add_mutually_exclusive_group()
    modes.add_argument("--find", metavar="TOKENS", help="find signals matching comma-separated regexes")
    modes.add_argument("--dma", metavar="REGEX", help="show matching DMA definitions")
    parser.add_argument("--kicad-symbol", action="store_true",
                        help="write MCU pin CSV and function-list files")
    parser.add_argument("--no-pin-prefix", action="store_true",
                        help="omit the port/pin prefix from configured pin macros")
    parser.add_argument("--line-prefix", action="store_true",
                        help="include the port/pin prefix in line macros")
    parser.add_argument("--input-voltage", type=float, choices=(3.3, 5.0), default=3.3,
                        help="voltage assumed for unspecified inputs (default: 3.3); 5 rejects "
                             "use of non-5V-tolerant ADC-capable pins")
    return parser


def main(argv: list[str] | None = None) -> int:
    parser = build_parser()
    args = parser.parse_args(argv)
    try:
        root = discover_database(args.cubemx_root, args.open_pin_data)
        database = CubeDatabase(root)
        generator = BoardGenerator(database, input_voltage=args.input_voltage,
                                   pin_prefix=not args.no_pin_prefix,
                                   line_prefix=args.line_prefix)
        if args.config:
            leading_parameters = read_config_parameters(args.config)
            configured_mcu = leading_parameters.get("MCU_MODEL")
            if args.mcu and configured_mcu and args.mcu != configured_mcu:
                raise BoardGenError("do not combine a config MCU_MODEL with a different --mcu")
            mcu = configured_mcu or args.mcu
            if not mcu:
                raise BoardGenError("MCU_MODEL is mandatory")
            database.load_mcu(mcu)
            generator.parse(args.config)
        else:
            if not args.mcu:
                parser.error("CONFIG is required unless --mcu is supplied")
            matches = database.matching_mcus(args.mcu)
            if not (args.find or args.dma or args.kicad_symbol):
                for match in matches:
                    print(match)
                return 0
            if len(matches) != 1:
                raise BoardGenError(
                    f"--mcu {args.mcu!r} matches {len(matches)} MCUs; select exactly one"
                )
            mcu = matches[0]
            generator.config.parameters["MCU_MODEL"] = mcu
        if not mcu:
            raise BoardGenError("MCU_MODEL is mandatory")
        if database.mcu_dom is None:
            database.load_mcu(mcu)
        if args.find:
            print_af_matches(database, args.find)
            return 0
        if args.dma:
            if args.open_pin_data:
                raise BoardGenError("Open Pin Data does not contain DMA information")
            print_dma_matches(database, args.dma)
            return 0
        content = None
        if args.config:
            generator.validate()
            content = generator.generate()
        if args.kicad_symbol:
            directory = args.config.parent if args.config else Path.cwd()
            write_kicad_files(database, mcu, directory)
        if content is None:
            return 0
        if args.output is None or str(args.output) == "-":
            sys.stdout.write(content)
        else:
            # Validation and rendering are complete before the destination is opened.
            args.output.write_text(content, encoding="utf-8")
        return 0
    except BrokenPipeError:
        # Avoid a second BrokenPipeError while Python flushes stdout at exit.
        try:
            devnull = os.open(os.devnull, os.O_WRONLY)
            os.dup2(devnull, sys.stdout.fileno())
            os.close(devnull)
        except OSError:
            pass
        return 0
    except (BoardGenError, OSError) as error:
        print(f"boardGen.py: error: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
