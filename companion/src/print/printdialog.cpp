/*
 * Copyright (C) EdgeTX
 *
 * Based on code named
 *   opentx - https://github.com/opentx/opentx
 *   th9x - http://code.google.com/p/th9x
 *   er9x - http://code.google.com/p/er9x
 *   gruvin9x - http://code.google.com/p/gruvin9x
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

#include "printdialog.h"
#include "ui_printdialog.h"
#include "helpers.h"
#include "styleeditdialog.h"
#include <QtPrintSupport/QPrinter>
#include <QtPrintSupport/QPrintDialog>
#include <QTextDocument>
#include "version.h"
#include "boards.h"

PrintDialog::PrintDialog(QWidget *parent, Firmware * firmware, GeneralSettings & generalSettings, ModelData & model, const QString & filename) :
  QDialog(parent),
  firmware(firmware),
  generalSettings(generalSettings),
  model(model),
  printfilename(filename),
  ui(new Ui::PrintDialog),
  multiModelPrinter(firmware)
{
  ui->setupUi(this);
  setWindowIcon(CompanionIcon("print.png"));
  setWindowTitle(model.name);
  multiModelPrinter.setModel(0, &model, &generalSettings);
  ui->textEdit->setHtml(multiModelPrinter.print(ui->textEdit->document()));
  if (!printfilename.isEmpty()) {
    printToFile();
    QTimer::singleShot(0, this, SLOT(autoClose()));
  }
}

void PrintDialog::closeEvent(QCloseEvent *event)
{
}

PrintDialog::~PrintDialog()
{
  delete ui;
}

void PrintDialog::on_printButton_clicked()
{
  QPrinter printer;
  printer.setPageMargins(QMarginsF(10.0, 10.0, 10.0, 10.0), QPageLayout::Millimeter);
  QPrintDialog *dialog = new QPrintDialog(&printer, this);
  dialog->setWindowTitle(tr("Print Document"));
  if (dialog->exec() != QDialog::Accepted)
    return;
  ui->textEdit->print(&printer);
}

void PrintDialog::on_printFileButton_clicked()
{
  QString fn = QFileDialog::getSaveFileName(this,tr("Select output file"),QString(),tr("PDF files(*.pdf);;HTML files (*.htm *.html);;All files (*)"));
  if (fn.isEmpty())
    return;
  if (! (fn.endsWith(".pdf", Qt::CaseInsensitive) || fn.endsWith(".htm", Qt::CaseInsensitive) || fn.endsWith(".html", Qt::CaseInsensitive)) )
    fn += ".pdf"; // default
  if (fn.endsWith(".pdf", Qt::CaseInsensitive)) {
    QPrinter printer;
    printer.setPageMargins(QMarginsF(10.0, 10.0, 10.0, 10.0), QPageLayout::Millimeter);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setColorMode(QPrinter::Color);
    printer.setOutputFileName(fn);
    ui->textEdit->print(&printer);
  }
  else {
    QTextDocumentWriter writer(fn);
     writer.write(ui->textEdit->document());
  }
}

void PrintDialog::printToFile()
{
  if (printfilename.isEmpty())
    return;
  if (! (printfilename.endsWith(".pdf", Qt::CaseInsensitive) || printfilename.endsWith(".htm", Qt::CaseInsensitive) || printfilename.endsWith(".html", Qt::CaseInsensitive)) )
    printfilename += ".pdf"; // default
  if (printfilename.endsWith(".pdf", Qt::CaseInsensitive)) {
    QPrinter printer;
    printer.setPageMargins(QMarginsF(10.0, 10.0, 10.0, 10.0), QPageLayout::Millimeter);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setColorMode(QPrinter::Color);
    printer.setOutputFileName(printfilename);
    ui->textEdit->print(&printer);
  }
  else {
    QTextDocumentWriter writer(printfilename);
    writer.write(ui->textEdit->document());
  }
}

void PrintDialog::autoClose()
{
  this->close();
}

void PrintDialog::on_styleButton_clicked()
{
  StyleEditDialog *g = new StyleEditDialog(this, MODEL_PRINT_CSS);
  if (g->exec() == QDialog::Accepted)
    ui->textEdit->setHtml(multiModelPrinter.print(ui->textEdit->document()));
}


// ---------------------------------------------------------------------------
// PDF export of a single model ("Save as PDF..." in the model context menu)
// ---------------------------------------------------------------------------

// Own, print friendly style for the PDF. The user's print style (modelprt.css)
// stays in effect for the print dialog.
static const char * PDF_CSS = R"CSS(
body, p, table, td, th { font-family: "Segoe UI", "Helvetica Neue", Arial, sans-serif; font-size: 8.5pt; color: #1f1e1c; }
td, th { padding: 2px 4px; }
b { color: #1f1e1c; }
.mpc-section-title { color: #ffffff; background-color: #2563c9; font-size: 11pt; font-weight: bold; padding: 4px 6px; }
.mpc-diff1, .mpc-diff2 { color: #1f1e1c; }
.pdf-title { font-size: 18pt; font-weight: bold; color: #1f1e1c; }
.pdf-sub { font-size: 9pt; color: #5f5d58; }
.pdf-key { color: #5f5d58; }
.pdf-foot { font-size: 7.5pt; color: #8a8883; }
)CSS";

QString PrintDialog::saveModelPdf(Firmware * firmware, GeneralSettings & generalSettings, ModelData & model,
                                  const QString & pdfFileName, const QString & sourceFileName)
{
  if (pdfFileName.isEmpty())
    return tr("No file name given.");

  MultiModelPrinter printer(firmware);
  printer.setModel(0, &model, &generalSettings);

  QTextDocument doc;
  const QString body = printer.print(&doc);   // also loads the user print style ...
  doc.setDefaultStyleSheet(PDF_CSS);           // ... which we replace by the PDF style

  const QString modelName = QString(model.name).trimmed().isEmpty() ? tr("(no name)") : QString(model.name).trimmed().toHtmlEscaped();
  const QString labels = QString(model.labels).split(',', Qt::SkipEmptyParts).join(", ").toHtmlEscaped();
  const QString created = QLocale().toString(QDateTime::currentDateTime(), QLocale::ShortFormat);

  QString head;
  head.append(QString("<p class='pdf-title'>%1</p>").arg(modelName));
  head.append(QString("<p class='pdf-sub'>%1</p>").arg(tr("Model settings")));
  head.append("<table cellspacing='0' cellpadding='2' width='100%' border='0'>");
  auto row = [&head](const QString & key, const QString & value) {
    if (!value.isEmpty())
      head.append(QString("<tr><td class='pdf-key' width='22%'>%1</td><td>%2</td></tr>").arg(key, value));
  };
  row(tr("Radio"), Boards::getBoardName(firmware->getBoard()).toHtmlEscaped());
  row(tr("Firmware"), firmware->getName().toHtmlEscaped());
  row(tr("Labels"), labels);
  row(tr("File"), QFileInfo(sourceFileName).fileName().toHtmlEscaped());
  row(tr("Created"), created);
  head.append("</table><br/>");

  const QString foot = QString("<br/><p class='pdf-foot'>EdgeTX Companion %1 (%2) - %3</p>")
                         .arg(VERSION, __DATE__, SPECIAL_BUILD);

  doc.setHtml("<html><body>" + head + body + foot + "</body></html>");

  QPrinter pdf(QPrinter::HighResolution);
  pdf.setOutputFormat(QPrinter::PdfFormat);
  pdf.setColorMode(QPrinter::Color);
  pdf.setPageSize(QPageSize(QPageSize::A4));
  pdf.setPageMargins(QMarginsF(12.0, 12.0, 12.0, 12.0), QPageLayout::Millimeter);
  pdf.setDocName(QString(model.name).trimmed());
  pdf.setCreator(QString("EdgeTX Companion %1 - %2").arg(VERSION, SPECIAL_BUILD));
  pdf.setOutputFileName(pdfFileName);

  doc.print(&pdf);

  QFileInfo fi(pdfFileName);
  if (!fi.exists() || fi.size() == 0)
    return tr("The PDF file could not be written: %1").arg(QDir::toNativeSeparators(pdfFileName));

  return QString();
}
