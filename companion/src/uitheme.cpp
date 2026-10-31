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

#include "uitheme.h"

#include <QApplication>
#include <QColor>
#include <QCoreApplication>
#include <QMap>
#include <QPalette>
#include <QStyle>
#include <QStyleFactory>
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
#include <QStyleHints>
#endif

namespace {

  QApplication * s_app = nullptr;
  QString s_platformStyle;
  QPalette s_platformPalette;
  bool s_dark = false;
  bool s_modern = false;

  struct Tokens {
    QString window;       // window / dialog background
    QString surface;      // inputs, tables, tab pages
    QString surfaceAlt;   // alternate rows, headers, toolbars
    QString hover;        // hover background
    QString pressed;      // pressed background
    QString border;       // default hairline
    QString borderStrong; // control borders
    QString text;
    QString textMuted;
    QString disabled;
    QString accent;
    QString accentText;   // text on accent fill
    QString selection;    // row / item selection background
    QString selectionText;
  };

  const Tokens LIGHT = {
    "#f4f4f2", "#ffffff", "#ecebe8", "#e6e5e1", "#dad8d3",
    "#d9d7d2", "#b9b6b0",
    "#1f1e1c", "#5f5d58", "#a3a19c",
    "#2563c9", "#ffffff",
    "#d6e4fa", "#11336b"
  };

  const Tokens DARK = {
    "#1f2023", "#28292d", "#2d2f33", "#34363b", "#3d4045",
    "#393b40", "#53565c",
    "#e8e7e4", "#a9a8a4", "#6d6c69",
    "#4b8cf0", "#ffffff",
    "#253b61", "#dbe7ff"
  };

  bool systemPrefersDark()
  {
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    return QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark;
#else
    const QPalette p = QGuiApplication::palette();
    return p.color(QPalette::Window).lightness() < 128;
#endif
  }

  QPalette makePalette(const Tokens & t)
  {
    QPalette p;
    const QColor text(t.text), disabled(t.disabled);

    p.setColor(QPalette::Window, QColor(t.window));
    p.setColor(QPalette::WindowText, text);
    p.setColor(QPalette::Base, QColor(t.surface));
    p.setColor(QPalette::AlternateBase, QColor(t.surfaceAlt));
    p.setColor(QPalette::ToolTipBase, QColor(t.surface));
    p.setColor(QPalette::ToolTipText, text);
    p.setColor(QPalette::PlaceholderText, QColor(t.textMuted));
    p.setColor(QPalette::Text, text);
    p.setColor(QPalette::Button, QColor(t.surface));
    p.setColor(QPalette::ButtonText, text);
    p.setColor(QPalette::BrightText, QColor("#e5484d"));
    p.setColor(QPalette::Light, QColor(t.surface));
    p.setColor(QPalette::Midlight, QColor(t.border));
    p.setColor(QPalette::Mid, QColor(t.borderStrong));
    p.setColor(QPalette::Dark, QColor(t.borderStrong));
    p.setColor(QPalette::Shadow, QColor(0, 0, 0, 60));
    p.setColor(QPalette::Highlight, QColor(t.accent));
    p.setColor(QPalette::HighlightedText, QColor(t.accentText));
    p.setColor(QPalette::Link, QColor(t.accent));
    p.setColor(QPalette::LinkVisited, QColor(t.accent));

    for (auto role : { QPalette::WindowText, QPalette::Text, QPalette::ButtonText }) {
      p.setColor(QPalette::Disabled, role, disabled);
    }
    p.setColor(QPalette::Disabled, QPalette::Base, QColor(t.window));
    p.setColor(QPalette::Disabled, QPalette::Button, QColor(t.window));
    p.setColor(QPalette::Disabled, QPalette::Highlight, QColor(t.borderStrong));
    return p;
  }

  // Flat, quiet and dense: hairline borders, small radii, one accent colour.
  // Sub-controls (arrows, check marks) are left to the Fusion style.
  const char * STYLESHEET = R"QSS(
QMainWindow, QDialog { background: {{window}}; }
QMainWindow::separator { background: {{border}}; width: 1px; height: 1px; }

QToolTip { background: {{surface}}; color: {{text}}; border: 1px solid {{borderStrong}}; padding: 4px 6px; }

/* ---- menus ---- */
QMenuBar { background: {{window}}; border-bottom: 1px solid {{border}}; padding: 1px 2px; }
QMenuBar::item { background: transparent; padding: 4px 9px; border-radius: 4px; }
QMenuBar::item:selected { background: {{hover}}; }
QMenuBar::item:pressed { background: {{pressed}}; }
QMenu { background: {{surface}}; border: 1px solid {{borderStrong}}; padding: 4px; }
QMenu::item { padding: 5px 24px 5px 24px; border-radius: 4px; }
QMenu::item:selected { background: {{selection}}; color: {{selectionText}}; }
QMenu::item:disabled { color: {{disabled}}; }
QMenu::separator { height: 1px; background: {{border}}; margin: 4px 6px; }

/* ---- tool bars ---- */
QToolBar { background: {{window}}; border: none; border-bottom: 1px solid {{border}}; spacing: 2px; padding: 3px 4px; }
QToolBar::separator { background: {{border}}; width: 1px; margin: 4px 6px; }
QToolBar::handle { width: 0px; image: none; }
QToolButton { background: transparent; border: 1px solid transparent; border-radius: 4px; padding: 3px; }
QToolButton:hover { background: {{hover}}; }
QToolButton:pressed, QToolButton:checked { background: {{pressed}}; }
QToolButton:disabled { color: {{disabled}}; }
QToolButton[popupMode="1"] { padding-right: 14px; }

/* ---- status bar ---- */
QStatusBar { background: {{surfaceAlt}}; border-top: 1px solid {{border}}; color: {{textMuted}}; }
QStatusBar::item { border: none; }

/* ---- tabs ---- */
QTabWidget::pane { background: {{surface}}; border: 1px solid {{border}}; border-radius: 0px; top: -1px; }
QTabBar { qproperty-drawBase: 0; }
QTabBar::tab { background: transparent; color: {{textMuted}}; border: none; border-bottom: 2px solid transparent; padding: 6px 12px; margin-right: 2px; }
QTabBar::tab:hover { color: {{text}}; background: {{hover}}; }
QTabBar::tab:selected { color: {{text}}; border-bottom: 2px solid {{accent}}; }
QTabBar::tab:disabled { color: {{disabled}}; }
QMdiArea QTabBar::tab { padding: 6px 14px; }

/* ---- group boxes ---- */
QGroupBox { border: 1px solid {{border}}; border-radius: 6px; margin-top: 14px; padding: 8px 6px 6px 6px; }
QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top left; left: 8px; padding: 0 4px; color: {{textMuted}}; }

/* ---- buttons ---- */
QPushButton { background: {{surface}}; border: 1px solid {{borderStrong}}; border-radius: 4px; padding: 4px 14px; min-height: 18px; }
QPushButton:hover { background: {{hover}}; }
QPushButton:pressed { background: {{pressed}}; }
QPushButton:default { border: 1px solid {{accent}}; }
QPushButton:disabled { color: {{disabled}}; border-color: {{border}}; background: {{window}}; }
QPushButton:checked { background: {{selection}}; color: {{selectionText}}; border-color: {{accent}}; }

/* ---- inputs ----
   Frames and arrows of line edits, spin boxes and combo boxes are drawn by Fusion
   from the palette; styling their borders here would drop the arrow sub-controls. */
QPlainTextEdit, QTextEdit { background: {{surface}}; border: 1px solid {{borderStrong}}; border-radius: 4px; }
QPlainTextEdit:focus, QTextEdit:focus { border: 1px solid {{accent}}; }
QComboBox QAbstractItemView { background: {{surface}}; border: 1px solid {{borderStrong}}; selection-background-color: {{selection}}; selection-color: {{selectionText}}; outline: 0; }

/* ---- lists, tables, trees ---- */
QAbstractItemView { background: {{surface}}; alternate-background-color: {{surfaceAlt}}; border: 1px solid {{border}};
  selection-background-color: {{selection}}; selection-color: {{selectionText}}; outline: 0; }
QTableView, QTableWidget { gridline-color: {{border}}; }
QTreeView::item, QListView::item { padding: 2px 0px; }
QTreeView::item:hover, QListView::item:hover, QTableView::item:hover { background: {{hover}}; }
QTreeView::item:selected, QListView::item:selected, QTableView::item:selected { background: {{selection}}; color: {{selectionText}}; }
QHeaderView { background: {{surfaceAlt}}; border: none; }
QHeaderView::section { background: {{surfaceAlt}}; color: {{textMuted}}; border: none; border-right: 1px solid {{border}};
  border-bottom: 1px solid {{border}}; padding: 4px 6px; }
QTableCornerButton::section { background: {{surfaceAlt}}; border: none; border-bottom: 1px solid {{border}}; }

/* ---- scroll bars (slim) ---- */
QScrollBar:vertical { background: transparent; width: 11px; margin: 0; }
QScrollBar:horizontal { background: transparent; height: 11px; margin: 0; }
QScrollBar::handle:vertical { background: {{borderStrong}}; border-radius: 4px; min-height: 28px; margin: 2px; }
QScrollBar::handle:horizontal { background: {{borderStrong}}; border-radius: 4px; min-width: 28px; margin: 2px; }
QScrollBar::handle:hover { background: {{textMuted}}; }
QScrollBar::add-line, QScrollBar::sub-line { width: 0px; height: 0px; border: none; background: none; }
QScrollBar::add-page, QScrollBar::sub-page { background: none; }
QScrollArea { border: none; }

/* ---- misc ---- */
QSplitter::handle { background: {{border}}; }
QSplitter::handle:horizontal { width: 1px; }
QSplitter::handle:vertical { height: 1px; }
QProgressBar { background: {{surfaceAlt}}; border: 1px solid {{border}}; border-radius: 4px; text-align: center; min-height: 14px; }
QProgressBar::chunk { background: {{accent}}; border-radius: 3px; }
QDockWidget::title { background: {{surfaceAlt}}; padding: 4px 6px; border-bottom: 1px solid {{border}}; }
QMdiArea { background: {{window}}; }
)QSS";

  QString buildStyleSheet(const Tokens & t)
  {
    const QMap<QString, QString> map = {
      { "window", t.window }, { "surface", t.surface }, { "surfaceAlt", t.surfaceAlt },
      { "hover", t.hover }, { "pressed", t.pressed }, { "border", t.border },
      { "borderStrong", t.borderStrong }, { "text", t.text }, { "textMuted", t.textMuted },
      { "disabled", t.disabled }, { "accent", t.accent }, { "accentText", t.accentText },
      { "selection", t.selection }, { "selectionText", t.selectionText },
    };

    QString qss = QString::fromUtf8(STYLESHEET);
    for (auto it = map.constBegin(); it != map.constEnd(); ++it)
      qss.replace("{{" + it.key() + "}}", it.value());
    return qss;
  }

}  // namespace

namespace UiTheme {

void init(QApplication * app)
{
  s_app = app;
  if (app->style())
    s_platformStyle = app->style()->name();
  s_platformPalette = app->palette();
}

void apply(int style)
{
  if (!s_app)
    return;

  if (style < STYLE_CLASSIC || style >= STYLE_COUNT)
    style = STYLE_MODERN_SYSTEM;

  if (style == STYLE_CLASSIC) {
    s_app->setStyleSheet(QString());
    if (s_modern) {
      // only touch the style when coming back from the modern theme, so that
      // custom (proxy) styles set at start up are kept for the classic look
      if (QStyle * st = QStyleFactory::create(s_platformStyle))
        s_app->setStyle(st);
      s_app->setPalette(s_platformPalette);
    }
    s_modern = false;
    s_dark = false;
    return;
  }

  bool dark = (style == STYLE_MODERN_DARK) || (style == STYLE_MODERN_SYSTEM && systemPrefersDark());
  const Tokens & t = dark ? DARK : LIGHT;

  if (QStyle * st = QStyleFactory::create("Fusion"))
    s_app->setStyle(st);
  s_app->setPalette(makePalette(t));
  s_app->setStyleSheet(buildStyleSheet(t));

  s_modern = true;
  s_dark = dark;
}

bool isDark()
{
  return s_modern && s_dark;
}

QString styleName(int style)
{
  switch (style) {
    case STYLE_CLASSIC:
      return QCoreApplication::translate("UiTheme", "Classic");
    case STYLE_MODERN_SYSTEM:
      return QCoreApplication::translate("UiTheme", "Modern (follow system)");
    case STYLE_MODERN_LIGHT:
      return QCoreApplication::translate("UiTheme", "Modern light");
    case STYLE_MODERN_DARK:
      return QCoreApplication::translate("UiTheme", "Modern dark");
    default:
      return QString();
  }
}

}  // namespace UiTheme
