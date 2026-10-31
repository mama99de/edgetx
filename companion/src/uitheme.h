/*
 * Copyright (C) EdgeTX
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#pragma once

#include <QString>

class QApplication;

// Application wide look: the "classic" platform look or a flat, modern,
// functional theme (light / dark / follow the system).
namespace UiTheme {

  enum Style {
    STYLE_CLASSIC = 0,
    STYLE_MODERN_SYSTEM,
    STYLE_MODERN_LIGHT,
    STYLE_MODERN_DARK,
    STYLE_COUNT
  };

  // must be called once, before the first apply(), to remember the platform style
  void init(QApplication * app);

  // apply the given style to the whole application (can be called at runtime)
  void apply(int style);

  // true when the modern theme is active and currently dark
  bool isDark();

  QString styleName(int style);
}
