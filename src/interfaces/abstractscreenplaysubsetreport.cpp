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

#include "abstractscreenplaysubsetreport.h"
#include "qtextdocumentpagedprinter.h"
#include "screenplaytextdocument.h"
#include "application.h"
#include "scene.h"

AbstractScreenplaySubsetReport::AbstractScreenplaySubsetReport(QObject *parent)
    : AbstractReportGenerator(parent)
{

}

AbstractScreenplaySubsetReport::~AbstractScreenplaySubsetReport()
{

}

void AbstractScreenplaySubsetReport::setGenerateTitlePage(bool val)
{
    if(m_generateTitlePage == val)
        return;

    m_generateTitlePage = val;
    emit generateTitlePageChanged();
}

void AbstractScreenplaySubsetReport::setListSceneCharacters(bool val)
{
    if(m_listSceneCharacters == val)
        return;

    m_listSceneCharacters = val;
    emit listSceneCharactersChanged();
}

void AbstractScreenplaySubsetReport::setIncludeSceneSynopsis(bool val)
{
    if(m_includeSceneSynopsis == val)
        return;

    m_includeSceneSynopsis = val;
    emit includeSceneSynopsisChanged();
}

void AbstractScreenplaySubsetReport::setIncludeSceneContents(bool val)
{
    if(m_includeSceneContents == val)
        return;

    m_includeSceneContents = val;
    emit includeSceneContentsChanged();
}

void AbstractScreenplaySubsetReport::setIncludeSceneNumbers(bool val)
{
    if(m_includeSceneNumbers == val)
        return;

    m_includeSceneNumbers = val;
    emit includeSceneNumbersChanged();
}

void AbstractScreenplaySubsetReport::setIncludeSceneIcons(bool val)
{
    if(m_includeSceneIcons == val)
        return;

    m_includeSceneIcons = val;
    emit includeSceneIconsChanged();
}

void AbstractScreenplaySubsetReport::setPrintEachSceneOnANewPage(bool val)
{
    if(m_printEachSceneOnANewPage == val)
        return;

    m_printEachSceneOnANewPage = val;
    emit printEachSceneOnANewPageChanged();
}

void AbstractScreenplaySubsetReport::setEpisodeNumbers(const QList<int> &val)
{
    if(m_episodeNumbers == val)
        return;

    m_episodeNumbers = val;
    emit episodeNumbersChanged();
}

void AbstractScreenplaySubsetReport::setTags(const QStringList &val)
{
    if(m_tags == val)
        return;

    m_tags = val;
    emit tagsChanged();
}

bool AbstractScreenplaySubsetReport::doGenerate(QTextDocument *textDocument)
{
    ScriteDocument *document = this->document();
    Screenplay *screenplay = document->screenplay();

    if(m_screenplaySubset)
        delete m_screenplaySubset;

    const bool hasEpisodes = screenplay->episodeCount() > 0;

    QString subtitle = this->screenplaySubtitle();
    if(hasEpisodes)
    {
        if(m_episodeNumbers.isEmpty())
        {
            if(screenplay->episodeCount() > 1)
                subtitle += QStringLiteral(" [All %1 Episodes]").arg(screenplay->episodeCount());
            else
                subtitle += QStringLiteral(" [Episode 1]");
        }
        else if(m_episodeNumbers.size() == 1)
            subtitle += QStringLiteral(" [Episode ") + QString::number(m_episodeNumbers.first()) + QStringLiteral("]");
        else
        {
            QStringList epNrs;
            epNrs.reserve(m_episodeNumbers.size());
            for(int nr : qAsConst(m_episodeNumbers))
                epNrs << QString::number(nr);
            subtitle += QStringLiteral(" [Episode ") + epNrs.join(", ") + QStringLiteral("]");
        }
    }

    m_screenplaySubset = new Screenplay(this);
    m_screenplaySubset->setEmail(screenplay->email());
    m_screenplaySubset->setTitle(screenplay->title());
    m_screenplaySubset->setAuthor(screenplay->author());
    m_screenplaySubset->setAddress(screenplay->address());
    m_screenplaySubset->setBasedOn(screenplay->basedOn());
    m_screenplaySubset->setContact(screenplay->contact());
    m_screenplaySubset->setVersion(screenplay->version());
    m_screenplaySubset->setSubtitle(subtitle);
    m_screenplaySubset->setPhoneNumber(screenplay->phoneNumber());
    m_screenplaySubset->setProperty("#useDocumentScreenplayForCoverPagePhoto", true);

    int episodeNr = 0; // Episode number is 1+episodeIndex
    for(int i=0; i<screenplay->elementCount(); i++)
    {
        ScreenplayElement *element = screenplay->elementAt(i);
        if(hasEpisodes && !m_episodeNumbers.isEmpty())
        {
            if(element->elementType() == ScreenplayElement::BreakElementType && element->breakType() == Screenplay::Episode)
                ++episodeNr;
            else if(i==0)
                ++episodeNr;

            if(!m_episodeNumbers.contains(episodeNr))
                continue;
        }

        if(!m_tags.isEmpty() && element->elementType() == ScreenplayElement::SceneElementType && element->scene() != nullptr)
        {
            Scene *scene = element->scene();

            const QStringList sceneTags = scene->groups();
            if(sceneTags.isEmpty())
                continue;

            QStringList tags;
            std::copy_if(sceneTags.begin(), sceneTags.end(), std::back_inserter(tags),
                         [=](const QString &sceneTag) {
                return tags.isEmpty() ? m_tags.contains(sceneTag) : false;
            });

            if(tags.isEmpty())
                continue;
        }

        if( (element->elementType() == ScreenplayElement::BreakElementType && element->breakType() == Screenplay::Episode) ||
            (element->scene() != nullptr && this->includeScreenplayElement(element)) )
        {
            ScreenplayElement *element2 = new ScreenplayElement(m_screenplaySubset);
            element2->setElementType(element->elementType());
            if(element->elementType() == ScreenplayElement::BreakElementType)
            {
                element2->setBreakType(element->breakType());
                element2->setBreakTitle(element->breakTitle());

                ScreenplayElement* lastElement = m_screenplaySubset->elementAt(m_screenplaySubset->elementCount()-1);
                if(lastElement &&
                   lastElement->elementType() == ScreenplayElement::BreakElementType &&
                   lastElement->breakType() == Screenplay::Episode)
                    m_screenplaySubset->removeElement(lastElement);
            }
            else
            {                
                element2->setScene(element->scene());
                element2->setProperty("#sceneNumber", element->sceneNumber());
                element2->setUserSceneNumber(element->userSceneNumber());
            }

            m_screenplaySubset->addElement(element2);
        }
    }

    ScreenplayElement* lastElement = m_screenplaySubset->elementAt(m_screenplaySubset->elementCount()-1);
    if(lastElement &&
       lastElement->elementType() == ScreenplayElement::BreakElementType &&
       lastElement->breakType() == Screenplay::Episode)
        m_screenplaySubset->removeElement(lastElement);

    ScreenplayTextDocument stDoc;
    stDoc.setTitlePage(this->format() == AdobePDF ? m_generateTitlePage : false);
    stDoc.setSceneNumbers(m_includeSceneNumbers);
    stDoc.setSceneIcons(this->format() == AdobePDF ? m_includeSceneIcons : false);
    stDoc.setListSceneCharacters(m_listSceneCharacters);
    stDoc.setPrintEachSceneOnANewPage(this->format() == AdobePDF ? m_printEachSceneOnANewPage : false);
    stDoc.setSyncEnabled(false);
    if(this->format() == AdobePDF)
        stDoc.setPurpose(ScreenplayTextDocument::ForPrinting);
    else
        stDoc.setPurpose(ScreenplayTextDocument::ForDisplay);
    stDoc.setScreenplay(m_screenplaySubset);
    stDoc.setFormatting(document->printFormat());
    stDoc.setTextDocument(textDocument);
    stDoc.setIncludeSceneSynopsis(m_includeSceneSynopsis);
    stDoc.setInjection(this);
    this->configureScreenplayTextDocument(stDoc);
    stDoc.syncNow();

    return true;
}

void AbstractScreenplaySubsetReport::configureTextDocumentPrinter(QTextDocumentPagedPrinter *printer, const QTextDocument *)
{
    printer->header()->setVisibleFromPageOne(!m_generateTitlePage);
    printer->footer()->setVisibleFromPageOne(!m_generateTitlePage);
    printer->watermark()->setVisibleFromPageOne(!m_generateTitlePage);
}

void AbstractScreenplaySubsetReport::inject(QTextCursor &, AbstractScreenplayTextDocumentInjectionInterface::InjectLocation)
{
    // Incase we need to plug something in.
}

bool AbstractScreenplaySubsetReport::filterSceneElement() const
{
    return !m_includeSceneContents;
}
