/****************************************************************************
**
** Copyright (C) TERIFLIX Entertainment Spaces Pvt. Ltd. Bengaluru
** Author: Prashanth N Udupa (prashanth.udupa@teriflix.com)
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "pdfexporter.h"
#include "application.h"
#include "imageprinter.h"
#include "qtextdocumentpagedprinter.h"

#include <QDir>
#include <QPrinter>
#include <QSettings>
#include <QFileInfo>
#include <QPdfWriter>
#include <QTextCursor>
#include <QFontMetrics>
#include <QTextDocument>
#include <QObjectCleanupHandler>
#include <QAbstractTextDocumentLayout>

PdfExporter::PdfExporter(QObject *parent)
            : AbstractTextDocumentExporter(parent)
{

}

PdfExporter::~PdfExporter()
{

}

void PdfExporter::setIncludeSceneNumbers(bool val)
{
    if(m_includeSceneNumbers == val)
        return;

    m_includeSceneNumbers = val;
    emit includeSceneNumbersChanged();
}

void PdfExporter::setIncludeSceneIcons(bool val)
{
    if(m_includeSceneIcons == val)
        return;

    m_includeSceneIcons = val;
    emit includeSceneIconsChanged();
}

void PdfExporter::setPrintEachSceneOnANewPage(bool val)
{
    if(m_printEachSceneOnANewPage == val)
        return;

    m_printEachSceneOnANewPage = val;
    emit printEachSceneOnANewPageChanged();
}

void PdfExporter::setUsePageBreaks(bool val)
{
    if(m_usePageBreaks == val)
        return;

    m_usePageBreaks = val;
    emit usePageBreaksChanged();
}

void PdfExporter::setWatermark(const QString &val)
{
    if(m_watermark == val)
        return;

    m_watermark = val;
    emit watermarkChanged();
}

void PdfExporter::setComment(const QString &val)
{
    if(m_comment == val)
        return;

    m_comment = val;
    emit commentChanged();
}

bool PdfExporter::doExport(QIODevice *device)
{
    Screenplay *screenplay = this->document()->screenplay();
    ScreenplayFormat *format = this->document()->printFormat();

    const bool usePdfWriter = Application::instance()->settings()->value(QStringLiteral("PdfExport/usePdfDriver"), true).toBool();

    QScopedPointer<QPdfWriter> qpdfWriter;
    QScopedPointer<QPrinter> qprinter;
    QPagedPaintDevice *pdfDevice = nullptr;

    if(usePdfWriter)
    {
        qpdfWriter.reset(new QPdfWriter(device));
        qpdfWriter->setTitle(screenplay->title());
        qpdfWriter->setCreator(qApp->applicationName() + QStringLiteral(" ") + qApp->applicationVersion() + QStringLiteral(" PdfWriter"));
        format->pageLayout()->configure(qpdfWriter.data());
        qpdfWriter->setPageMargins(QMarginsF(0.2,0.1,0.2,0.1), QPageLayout::Inch);
        qpdfWriter->setPdfVersion(QPagedPaintDevice::PdfVersion_1_6);

        pdfDevice = qpdfWriter.data();
    }
    else
    {
        const QString pdfFileName = QDir::tempPath() + QStringLiteral("/scrite-pdfexporter-") + QString::number(QDateTime::currentSecsSinceEpoch()) + QStringLiteral(".pdf");

        qprinter.reset(new QPrinter);
        qprinter->setOutputFormat(QPrinter::PdfFormat);
        qprinter->setOutputFileName(pdfFileName);

        qprinter->setPdfVersion(QPagedPaintDevice::PdfVersion_1_6);
        qprinter->setDocName(screenplay->title());
        qprinter->setCreator(qApp->applicationName() + QStringLiteral(" ") + qApp->applicationVersion() + QStringLiteral(" Printer"));
        format->pageLayout()->configure(qprinter.data());
        qprinter->setPageMargins(QMarginsF(0.2,0.1,0.2,0.1), QPageLayout::Inch);

        pdfDevice = qprinter.data();
    }

    const qreal pageWidth = pdfDevice->width();
    QTextDocument textDocument;
    this->AbstractTextDocumentExporter::generate(&textDocument, pageWidth);    
    textDocument.setProperty("#comment", m_comment);
    textDocument.setProperty("#watermark", m_watermark);

    QTextDocumentPagedPrinter printer;
    printer.header()->setVisibleFromPageOne(false);
    printer.footer()->setVisibleFromPageOne(false);
    printer.watermark()->setVisibleFromPageOne(false);
    bool success = printer.print(&textDocument, pdfDevice);

    if(!qprinter.isNull())
    {
        const QString pdfFileName = qprinter->outputFileName();
        if(success)
        {
            QFile pdfFile(pdfFileName);
            pdfFile.open(QFile::ReadOnly);

            const int bufferSize = 65535;
            while(1)
            {
                const QByteArray bytes = pdfFile.read(bufferSize);
                if(bytes.isEmpty())
                    break;
                device->write(bytes);
            }
        }

        QFile::remove(pdfFileName);
    }

    return success;
}

QString PdfExporter::polishFileName(const QString &fileName) const
{
    QFileInfo fi(fileName);
    if(fi.suffix().toLower() != "pdf")
        return fileName + ".pdf";
    return fileName;
}
