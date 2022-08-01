#pragma once
#include "ch.h"

typedef struct { // Font data stored PER GLYPH
  uint16_t bitmapOffset; // Pointer into GFXfont->bitmap
  uint8_t width;         // Bitmap dimensions in pixels
  uint8_t height;        // Bitmap dimensions in pixels
  uint8_t xAdvance;      // Distance to advance cursor (x axis)
  int8_t xOffset;        // X dist from cursor pos to UL corner
  int8_t yOffset;        // Y dist from cursor pos to UL corner
} GFXglyph;

typedef struct { // Data stored for FONT AS A WHOLE
  uint8_t *bitmap;  // Glyph bitmaps, concatenated
  GFXglyph *glyph;  // Glyph array
  uint16_t first;   // ASCII extents (first char)
  uint16_t last;    // ASCII extents (last char)
  uint8_t yAdvance; // Newline distance (y axis)
} GFXfont;

/***
 *        _           _ _      _    _       ___        _     _    _    _
 *       /_\__ ____ _(_) |__ _| |__| |___  | __|__ _ _| |_  | |  (_)__| |_
 *      / _ \ V / _` | | / _` | '_ \ / -_) | _/ _ \ ' \  _| | |__| (_-<  _|
 *     /_/ \_\_/\__,_|_|_\__,_|_.__/_\___| |_|\___/_||_\__| |____|_/__/\__|
 *
 */
/*
COMMENT CREER/INTEGRER UNE NOUVELLE FONTE
=========================================
* generer la fonte au format Adafruit GFX avec un des outils suivants (ou autre)
	-https://oleddisplay.squix.ch/#/home
	-https://rop.nl/truetype2gfx/
* ou en recuperer une existante
* copier la definition de la fonte dans un fichier <nom_de_la_nouvelle_fonte>.h
* dans ce fichier (avec un outils de recherche/remplacement) :
 	- supprimer les "PROGMEM"
 	- remplacer les "const" per des "static const"
 * dans ssd1306_fonts.h (ci-dessous), ajouter une occurence #include <nom_de_la_nouvelle_fonte>.h
 * mettre ce fichier dans le sous repertoire /GFXfonts du driver
*/
// Fixed
#include "Font4x7Fixed.h"
#include "Font5x5Fixed.h"
#include "Font5x7Fixed.h"
#include "Font5x7FixedMono.h"
// Lato Heavy
#include "Lato_Heavy_12.h"
#include "Lato_Heavy_14.h"
#include "Lato_Heavy_16.h"
#include "Lato_Heavy_18.h"
#include "Lato_Heavy_20.h"
#include "Lato_Heavy_22.h"
#include "Lato_Heavy_24.h"
// Lato Medium
#include "Lato_Medium_12.h"
#include "Lato_Medium_14.h"
#include "Lato_Medium_16.h"
#include "Lato_Medium_18.h"
#include "Lato_Medium_20.h"
#include "Lato_Medium_22.h"
#include "Lato_Medium_24.h"
// Lato thin
#include "Lato_Thin_12.h"
#include "Lato_Thin_14.h"
#include "Lato_Thin_16.h"
#include "Lato_Thin_18.h"
#include "Lato_Thin_20.h"
#include "Lato_Thin_22.h"
#include "Lato_Thin_24.h"
// Mono
#include "Mono9.h"
#include "Mono12.h"
#include "Mono18.h"
#include "Mono24.h"
// Monos
#include "Monos10.h"
// Sans
#include "Sans9.h"
#include "Sans12.h"
#include "Sans18.h"
#include "Sans24.h"

