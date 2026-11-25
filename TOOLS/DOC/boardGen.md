# boardGen.py: a ChibiOS `board.h` generator

## Prerequisites

This guide assumes some familiarity with the STM32 GPIO multiplexer and with
the way ChibiOS initializes the Port Abstraction Layer (PAL).

`boardGen.py` requires Python 3.10 or newer. It uses only the Python standard
library, so no Python packages need to be installed. It also needs one of
these pin databases:

- the database included with
  [STM32CubeMX](https://www.st.com/en/development-tools/stm32cubemx.html); or
- [STM32 Open Pin Data](https://github.com/STMicroelectronics/STM32_open_pin_data),
  a redistributable subset of the CubeMX database.

Open Pin Data is sufficient to generate `board.h` and inspect pin functions,
but does not contain DMA routing information.

## Why generate `board.h`?

An application calls `halInit()` early during startup. Among other hardware
initialization, ChibiOS configures every GPIO pin from macros in `board.h`.
Those macros describe each pin's:

- input, output, analog, or alternate-function mode;
- push-pull or open-drain output type;
- pull-up or pull-down resistor;
- output speed (slew rate);
- initial output level;
- alternate-function selection; and
- on supported MCUs, analog-switch and configuration-lock state.

The description includes every pin, not just the pins used by the application.
On a large MCU this produces thousands of repetitive C macros, making manual
editing tedious and error-prone.

The generated header is consumed by the board's `board.c`. That source file is
still the place for board-specific functions, such as reading an SD-card
presence switch. ChibiOS also has its own XML-based `board.chcfg` workflow;
`boardGen.py` is an alternative intended for a compact, text-based setup.

The generator is especially useful when wiring changes frequently or when
students need to experiment with pin configurations. A `board.cfg` declares
only the pins of interest; all other pins receive a default configuration. The
Cube database is used to find alternate-function numbers and reject functions
that cannot be routed to the selected pin.

## Database discovery

By default, the generator looks for STM32CubeMX in these locations:

- `~/STM32CubeMX`;
- `/usr/local/STMicroelectronics/STM32Cube/STM32CubeMX`;
- the usual `Program Files` locations on Windows; and
- STM32CubeMX plugins under `~/opt/CHIBISTUDIO/eclipse/plugins`.

Both an installation directory and its `db` directory are accepted. An
explicit location avoids relying on discovery:

```sh
TOOLS/boardGen.py --cubemx-root /opt/STM32CubeMX board.cfg board.h
```

To use Open Pin Data, clone it and pass its root directory:

```sh
git clone https://github.com/STMicroelectronics/STM32_open_pin_data.git \
  ext/STM32_open_pin_data
TOOLS/boardGen.py --open-pin-data ext/STM32_open_pin_data board.cfg board.h
```

Open Pin Data can also be included as a Git submodule, allowing a project to be
built without a separate CubeMX installation.

## Generating `board.h`

The basic invocation is:

```sh
TOOLS/boardGen.py [options] board.cfg board.h
```

Use `-` as the output path, or omit the output argument, to write the generated
header to standard output:

```sh
TOOLS/boardGen.py board.cfg -
```

The configuration is parsed, validated, and rendered completely before the
destination is opened. A configuration error therefore leaves an existing
`board.h` unchanged.

### Naming options

For a configured pin such as:

```text
PC00 LED1 LED
```

the pin macro includes the physical location, while the line macro is stable if
the pin assignment changes:

```c
#define PC00_LED1 0U
#define LINE_LED1 PAL_LINE(GPIOC, 0U)
```

Keeping the location in the pin macro is useful in teaching because the
selected pin remains visible in source code. For a project in which wiring may
change independently of application code, it can also be omitted:

```sh
TOOLS/boardGen.py --no-pin-prefix board.cfg board.h
```

The resulting pin name is `LED1`; the line name remains `LINE_LED1`.

Use `--line-prefix` when the physical location is also wanted in line macros:

```sh
TOOLS/boardGen.py --line-prefix board.cfg board.h
```

This produces `LINE_C00_LED1`.

### Input-voltage validation

Unspecified inputs are assumed to use 3.3 V. If unspecified inputs may receive
5 V, request stricter validation:

```sh
TOOLS/boardGen.py --input-voltage 5 board.cfg board.h
```

The generator then rejects ADC-capable pins used as inputs, alternate
functions, or open-drain outputs because those pins are treated as not 5 V
tolerant. A voltage written explicitly after the other profile tokens on a pin
line overrides the command-line default.

## Makefile integration

A typical rule regenerates the header whenever the configuration, generator,
or Makefile changes:

```make
BOARD_GEN := TOOLS/boardGen.py

$(OBJS): board.h

board.h: board.cfg $(BOARD_GEN) Makefile
	$(BOARD_GEN) --input-voltage 5 $< $@
```

Adjust `BOARD_GEN` for the location of the shared tools directory. Because the
script has a Python shebang and is executable, it can be invoked directly on
Unix-like systems. `python TOOLS/boardGen.py ...` can be used on Windows.

## `board.cfg` format

A configuration has three sections:

1. generator parameters;
2. literal C header content between `HEADER` and `CONFIG`; and
3. pin and group definitions after `CONFIG`.

Here is a small complete example:

```text
MCU_MODEL = STM32F401CCFx
CHIBIOS_VERSION = 3

HEADER
#define STM32F401xC
#define STM32_HSECLK 8000000U
#define STM32_VDD 330U

CONFIG
DEFAULT INPUT PUSHPULL SPEED_VERYLOW PULLDOWN LEVEL_LOW AF0

PA00 STATUS_LED LED LEVEL_HIGH
PA02 CONSOLE_TX UART AF:USART2_TX
PA03 CONSOLE_RX UART AF:USART2_RX
PA04 SPI1_CS OUTPUT PUSHPULL SPEED_HIGH FLOATING LEVEL_HIGH
PA05 SPI1_SCK SPI AF:SPI1_SCK

GROUP CONSOLE_LINES %NAME =~ /^CONSOLE_/
ORDERED_GROUP CONTROL_LINES STATUS_LED,SPI1_CS
```

Blank lines are allowed. In the configuration section, `#` and `//` introduce
comments. Keep preprocessor comments and directives in the header section.

### Parameters

`MCU_MODEL` is mandatory and must be the exact stem of an MCU XML filename in
the database's `mcu` directory. The MCU-query mode described below helps find
the correct value.

`CHIBIOS_VERSION` is mandatory and must be a number greater than or equal to
3. Values used by existing configurations, such as `3`, `3.0`, and `18.2`, are
accepted. Older ChibiOS GPIO definitions are not supported.

### Header section

Everything after `HEADER` and before `CONFIG` is copied verbatim into the
generated header. This section normally defines the board and MCU macros needed
by ChibiOS, for example:

- the MCU symbol expected by the ST headers, such as `STM32F407xx`;
- `STM32_LSECLK` when the low-speed external oscillator is used;
- `STM32_HSECLK` when the high-speed external oscillator is used; and
- `STM32_VDD`, in hundredths of a volt.

A ChibiOS example for the same MCU is often a useful starting point, but clock
and voltage values must match the actual board.

### Default pin configuration

`DEFAULT` controls every GPIO pin that has no explicit definition:

```text
DEFAULT INPUT PUSHPULL SPEED_VERYLOW PULLDOWN LEVEL_LOW AF0
```

If this line is omitted, the built-in default is an open-drain input with
pull-down, very-low speed, high output-data level, and AF0.

The custom `DEFAULT` applies only to pins that are not listed. An explicitly
listed pin must itself receive every required field, either through a shortcut
or through individual attributes.

### Pin definitions

The first two fields of an ordinary pin line have a fixed order:

```text
PC00 LED1 LED
```

- `PC00` selects port C, pin 0. An optional leading `P` is accepted, ports A
  through P are supported, and the pin number uses two digits.
- `LED1` is the symbolic name used in generated macros.

The remaining fields are profiles or attributes. They are processed from left
to right, and a later field wins if it assigns the same property. A shortcut
such as `LED`, `UART`, or `ADC` fills a complete or nearly complete profile;
individual attributes can then override it.

Avoid spaces inside a parenthesized passive-function list. The parser removes
spaces inside parentheses for compatibility, but comma-separated tokens are
the clearest form.

### Pin attributes

The basic mode is one of:

- `INPUT`
- `OUTPUT`
- `ALTERNATE`
- `ANALOG`

Other available attributes are:

| Property | Values |
| --- | --- |
| Output type | `PUSHPULL`, `OPENDRAIN` |
| Pull resistor | `FLOATING`, `PULLUP`, `PULLDOWN` |
| Output speed | `SPEED_VERYLOW`, `SPEED_LOW`, `SPEED_MEDIUM`, `SPEED_HIGH` |
| Initial output level | `LEVEL_LOW`, `LEVEL_HIGH` |
| Analog switch | `ASCR_DISABLED`, `ASCR_ENABLED` |
| Configuration lock | `LOCKR_DISABLED`, `LOCKR_ENABLED` |

`ASCR` and `LOCKR` macros are currently generated for STM32L4 MCUs, whose
GPIO implementation requires them.

A push-pull output cannot enable a pull-up or pull-down resistor; that
configuration is rejected.

### Alternate functions and analog signals

An alternate-function number can be written directly as `AF0` through `AF15`,
or as a bare number. Numeric selection is discouraged because it cannot prove
that the intended peripheral signal is available on the pin.

The recommended form names the Cube signal:

```text
PB08 I2C1_SCL I2C AF:I2C1_SCL
```

The generator looks up the AF number and verifies the pin assignment. This
example also produces reusable AF macros:

```c
#define AF_PB08_I2C1_SCL     4U
#define AF_LINE_I2C1_SCL     4U
```

With `--no-pin-prefix`, the pin-oriented name is `AF_I2C1_SCL`. With
`--line-prefix`, the line-oriented name is `AF_LINE_B08_I2C1_SCL`. A line can
then be returned to its configured alternate function at run time:

```c
palSetLineMode(LINE_I2C1_SCL,
               PAL_MODE_ALTERNATE(AF_LINE_I2C1_SCL));
```

Signals that do not use the alternate-function multiplexer, such as ADC input
channels, are written without `AF:`:

```text
PA00 CURRENT_SENSE ADC ADC1_IN0
```

Signal names use the spelling from the selected database. Use `--find` to
discover them.

### Per-pin voltage

The maximum external voltage can be stated with tokens such as `0_VOLT`,
`3.3_VOLTS`, or `5_VOLTS`:

```text
PA00 BUTTON INPUT FLOATING 3.3_VOLTS
```

The numeric prefix is used; the suffix is case-insensitive. Put the voltage
after mode and shortcut tokens so it is the final value assigned.

A voltage annotation is generally unnecessary for push-pull outputs or pins
known to be 5 V tolerant.

### Shortcuts

Shortcuts provide defaults appropriate for common peripheral roles:

```text
SYS OSC ADC COMPIN COMPOUT OPAMP DAC
TIM GPT PWM ICU
I2C SPI UART USART
OTG ULPI ETH FSMC LTDC
SDIO SDIOCK SWDIO SWCLK CAN DCMI
LED PASSIVE
```

For example, `I2C` selects alternate mode with open-drain output. It deliberately
does not enable the internal pull-up: an appropriate external I2C pull-up is
still required. `LED` selects a low-speed push-pull output. `ADC`, `DAC`,
`COMPIN`, and `OPAMP` select analog mode and enable the analog switch where
applicable.

Shortcuts can be followed by attributes to override individual choices. Since
fields are processed left to right, put the shortcut before its overrides:

```text
PC00 STATUS_LED LED LEVEL_HIGH
```

## Dynamic pin reconfiguration

A pin can have one initial configuration and a parenthesized list of additional
functions that may be selected at run time:

```text
PB00 AUX_B3 INPUT PULLDOWN (ADC1_IN8,AF:TIM3_CH3)
```

The generator checks that each additional function is available on PB00 and
emits macros describing the peripheral, operation, channel, and AF:

```c
#define AUX_B3_ADC       1
#define AUX_B3_ADC_FN    IN
#define AUX_B3_ADC_IN    8
#define AUX_B3_TIM       3
#define AUX_B3_TIM_FN    CH
#define AUX_B3_TIM_CH    3
#define AUX_B3_TIM_AF    2
```

Application code can use those macros to switch modes:

```c
palSetLineMode(LINE_AUX_B3,
               PAL_MODE_ALTERNATE(AUX_B3_TIM_AF));
palSetLineMode(LINE_AUX_B3, PAL_MODE_INPUT_ANALOG);
```

With `--line-prefix`, use the generated name containing the port and pin, such
as `LINE_B00_AUX_B3`, instead.

Only the function metadata is generated; the application remains responsible
for safe electrical transitions and peripheral configuration.

## Groups of lines

Groups generate comma-separated line macros and a `<GROUP>_SIZE` macro. They
are useful for applying the same PAL operation to several lines.

### Filtered groups

`GROUP` selects pins with a safe expression:

```text
GROUP SPI_CHIP_SELECTS %NAME =~ /SPI.*CS/
GROUP SERVO_LINES %NAME =~ /AUX_[AB][1-4]|SRV[AB][1-4]/
GROUP NON_SYSTEM_LINES %ALIAS !~ /SYS|SDIO|SPI/
GROUP ALT_OUTPUTS %MODE eq 'ALTERNATE' && %AF =~ /_TX$/
```

Available variables are:

- `%NAME`: the configured pin name;
- `%AF`: the symbolic alternate-function token, such as `AF:USART2_TX`;
- `%MODE`: `INPUT`, `OUTPUT`, `ANALOG`, or `ALTERNATE`; and
- `%ALIAS`: the last recognized profile/attribute keyword on the line.

The supported expression language consists of:

- string comparisons `eq` and `ne`;
- regular-expression matches `=~ /pattern/` and `!~ /pattern/`;
- Boolean operators `&&`, `||`, and `!`;
- parentheses; and
- regular-expression flags `i`, `m`, `s`, and `x`.

Arbitrary Python or Perl code is not accepted. Filtered groups follow the
configuration's pin order, which is normally port then pin number.

When `%ALIAS` is important, make the intended shortcut the last keyword that
appears on the pin line. Alternate-function and passive signal tokens do not
change it.

### Ordered groups

`ORDERED_GROUP` explicitly lists symbolic pin names and preserves their order:

```text
ORDERED_GROUP DATA_BUS D0,D1,D2,D3,D4,D5,D6,D7
```

Every listed name must have a pin definition. Whitespace around commas is
ignored.

### Iterating over a group

Four helper macros are generated. On the first use in a C scope, use a `DECL`
variant to declare the group's local array:

```c
BOARD_GROUP_DECLFOREACH(line, SERVO_LINES) {
  palSetLine(line);
  palSetLineMode(line, PAL_MODE_OUTPUT_PUSHPULL);
}

chThdSleepMilliseconds(10);

BOARD_GROUP_FOREACH(line, SERVO_LINES) {
  palClearLine(line);
}
```

Use the `FOR` variants when the index or array is needed:

```c
BOARD_GROUP_DECLFOR(line_array, index, SERVO_LINES) {
  palSetLine(line_array[index]);
  palSetLineMode(line_array[index], PAL_MODE_OUTPUT_PUSHPULL);
  DebugTrace("setting line index %u", index);
}

BOARD_GROUP_FOR(line_array, index, SERVO_LINES) {
  palClearLine(line_array[index]);
}
```

The non-`DECL` variant reuses the array declared by the first invocation in the
same C scope.

## Inspecting the MCU database

### Listing MCU names

`--mcu` accepts a regular expression and prints every matching MCU XML name:

```sh
TOOLS/boardGen.py --mcu STM32F4
```

When another operation needs MCU data, the expression must match exactly one
name. Parentheses and other regular-expression characters must be escaped when
they are intended literally.

### Finding pin functions

With a configuration file, the MCU comes from `MCU_MODEL`:

```sh
TOOLS/boardGen.py --find UART4 board.cfg
TOOLS/boardGen.py --find 'ADC._IN11' board.cfg
TOOLS/boardGen.py --find . board.cfg
```

The query is a comma-separated list of case-insensitive regular expressions;
all items must match. For example:

```sh
TOOLS/boardGen.py --find USART,TX board.cfg
```

Typical output is:

```text
USART2_TX on PA02 has AF7
USART1_TX on PA09 has AF7
```

A function such as an ADC input is reported as `is not alternate`. The output
can be filtered to inspect one pin:

```sh
TOOLS/boardGen.py --find . board.cfg | grep PA10
```

The same query can be made without a configuration when an MCU expression
selects exactly one database entry:

```sh
TOOLS/boardGen.py --mcu L452V --find 'COMP|OPAMP'
```

### Finding DMA routes

`--dma` prints matching DMA routes and definitions suitable for adapting into
`mcuconf.h`:

```sh
TOOLS/boardGen.py --dma USART1_TX board.cfg
TOOLS/boardGen.py --dma 'USART1_[TR]X' board.cfg
TOOLS/boardGen.py --mcu F401CCFx --dma USART2
```

Example output:

```c
USART1_TX : [stream(2, 7) C4]
#define STM32_USART1_TX_DMA_STREAM       STM32_DMA_STREAM_ID(2, 7)
#define STM32_USART1_TX_DMA_CHANNEL      4
#define STM32_USART1_TX_DMA_IRQ_PRIORITY 6
#define STM32_USART1_TX_DMA_PRIORITY     2
```

Some peripherals have several valid routes; select one appropriate route in
the application configuration. The DMA regular expression is case-sensitive.
DMA lookup cannot be combined with `--open-pin-data`.

### KiCad-oriented exports

Generate a pin table and a complete function list with:

```sh
TOOLS/boardGen.py --mcu G030J6Mx --kicad-symbol
```

The MCU expression must select one entry. Two files are written:

- `<MCU>.csv`, a schematic pin description suitable for the existing KiCad
  symbol workflow; and
- `<MCU>_allFunctions.txt`, listing every pin and its multiplexer functions.

With a configuration file, these files are written beside that file. Without
one, they are written in the current directory. If normal board generation is
also requested, pass the usual configuration and output arguments:

```sh
TOOLS/boardGen.py --kicad-symbol board.cfg board.h
```

## What changed from `boardGen.pl`?

- Python 3 and the standard library replace Perl and its external modules.
- Options were renamed: `--no-pp-pin` to `--no-pin-prefix`, `--no-adcp-in` to
  `--input-voltage 5`, `--opd` to `--open-pin-data`, and `--kikadsymb` to
  `--kicad-symbol`. The old `--no-pp-line` behavior is now the default;
  `--line-prefix` restores location-prefixed line names when desired.
- `--cubemx-root` provides an explicit CubeMX database location.
- `GROUP` accepts the documented safe expression subset instead of executing
  arbitrary Perl. The ineffective `--no-error` option was removed.
- Output is fully validated and rendered before the destination is opened.
- ChibiOS versions below 3 were dropped; AF15 and GPIO ports A through P are
  accepted; MCU regex searches report all matches rather than silently taking
  the first.

Run `TOOLS/boardGen.py --help` for the concise command-line reference.
