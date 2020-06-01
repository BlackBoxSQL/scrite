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

#include "formatting.h"
#include "application.h"
#include "scritedocument.h"
#include "qobjectserializer.h"
#include "qobjectserializer.h"

#include <QPointer>
#include <QMarginsF>
#include <QMetaEnum>
#include <QPdfWriter>
#include <QTextCursor>
#include <QPageLayout>
#include <QTextBlockUserData>

SceneElementFormat::SceneElementFormat(SceneElement::Type type, ScreenplayFormat *parent)
                   : QObject(parent),
                     m_font(parent->defaultFont()),
                     m_format(parent),
                     m_elementType(type)
{        
    QObject::connect(this, &SceneElementFormat::elementFormatChanged, [this]() {
        this->markAsModified();
    });
    QObject::connect(this, &SceneElementFormat::fontChanged, this, &SceneElementFormat::font2Changed);
    QObject::connect(m_format, &ScreenplayFormat::defaultFontChanged, this, &SceneElementFormat::font2Changed);
}

SceneElementFormat::~SceneElementFormat()
{

}

void SceneElementFormat::setFont(const QFont &val)
{
    if(m_font == val)
        return;

    m_font = val;
    emit fontChanged();
    emit elementFormatChanged();
}

QFont SceneElementFormat::font2() const
{
    QFont font = m_font;
    font.setPointSize(font.pointSize() + m_format->fontPointSizeDelta());
    return font;
}

void SceneElementFormat::setFontFamily(const QString &val)
{
    if(m_font.family() == val)
        return;

    m_font.setFamily(val);
    emit fontChanged();
    emit elementFormatChanged();
}

void SceneElementFormat::setFontBold(bool val)
{
    if(m_font.bold() == val)
        return;

    m_font.setBold(val);
    emit fontChanged();
    emit elementFormatChanged();
}

void SceneElementFormat::setFontItalics(bool val)
{
    if(m_font.italic() == val)
        return;

    m_font.setItalic(val);
    emit fontChanged();
    emit elementFormatChanged();
}

void SceneElementFormat::setFontUnderline(bool val)
{
    if(m_font.underline() == val)
        return;

    m_font.setUnderline(val);
    emit fontChanged();
    emit elementFormatChanged();
}

void SceneElementFormat::setFontPointSize(int val)
{
    if(m_font.pointSize() == val)
        return;

    m_font.setPointSize(val);
    emit fontChanged();
    emit elementFormatChanged();
}

void SceneElementFormat::setTextColor(const QColor &val)
{
    if(m_textColor == val)
        return;

    m_textColor = val;
    emit textColorChanged();
    emit elementFormatChanged();
}

void SceneElementFormat::setBackgroundColor(const QColor &val)
{
    if(m_backgroundColor == val)
        return;

    QColor val2 = val;
    val2.setAlphaF(1);
    if(val2 == Qt::black || val2 == Qt::white)
        val2 = Qt::transparent;
    else
        val2.setAlphaF(0.25);

    m_backgroundColor = val2;
    emit backgroundColorChanged();
    emit elementFormatChanged();
}

void SceneElementFormat::setTextAlignment(Qt::Alignment val)
{
    if(m_textAlignment == val)
        return;

    m_textAlignment = val;
    emit textAlignmentChanged();
    emit elementFormatChanged();
}

void SceneElementFormat::setBlockWidth(qreal val)
{
    val = qBound(0.1, val, 1.0);
    if( qFuzzyCompare(m_blockWidth, val) )
        return;

    m_blockWidth = val;
    emit blockWidthChanged();
    emit elementFormatChanged();
}

void SceneElementFormat::setBlockAlignment(Qt::Alignment val)
{
    if(m_blockAlignment == val)
        return;

    m_blockAlignment = val;
    emit blockAlignmentChanged();
    emit elementFormatChanged();
}

void SceneElementFormat::setTopMargin(qreal val)
{
    if( qFuzzyCompare(m_topMargin, val) )
        return;

    m_topMargin = val;
    emit topMarginChanged();
    emit elementFormatChanged();
}

void SceneElementFormat::setBottomMargin(qreal val)
{
    if( qFuzzyCompare(m_bottomMargin, val) )
        return;

    m_bottomMargin = val;
    emit bottomMarginChanged();
    emit elementFormatChanged();
}

void SceneElementFormat::setLineHeight(qreal val)
{
    if( qFuzzyCompare(m_lineHeight, val) )
        return;

    m_lineHeight = val;
    emit lineHeightChanged();
    emit elementFormatChanged();
}

QTextBlockFormat SceneElementFormat::createBlockFormat(const qreal *givenPageWidth) const
{
    const qreal pageWidth = givenPageWidth ? *givenPageWidth : m_format->pageLayout()->contentWidth();
    const qreal blockWidthInPixels = pageWidth * m_blockWidth;
    const qreal fullMargin = (pageWidth - blockWidthInPixels);
    const qreal halfMargin = fullMargin*0.5;
    const qreal leftMargin = m_blockAlignment.testFlag(Qt::AlignLeft) ? 0 : (m_blockAlignment.testFlag(Qt::AlignHCenter) ? halfMargin : fullMargin);
    const qreal rightMargin = pageWidth - blockWidthInPixels - leftMargin;

    QTextBlockFormat format;
    format.setLeftMargin(leftMargin);
    format.setRightMargin(rightMargin);
    format.setTopMargin(m_topMargin);
    format.setBottomMargin(m_bottomMargin);
    format.setLineHeight(m_lineHeight*100, QTextBlockFormat::ProportionalHeight);
    format.setAlignment(m_textAlignment);
    format.setBackground(QBrush(m_backgroundColor));
    format.setForeground(QBrush(m_textColor));

    return format;
}

QTextCharFormat SceneElementFormat::createCharFormat(const qreal *givenPageWidth) const
{
    Q_UNUSED(givenPageWidth)

    QTextCharFormat format;

    // It turns out that format.setFont()
    // doesnt actually do all of the below.
    // So, we will have to do it explicitly
    format.setFontFamily(m_font.family());
    format.setFontItalic(m_font.italic());
    format.setFontWeight(m_font.weight());
    // format.setFontKerning(m_font.kerning());
    format.setFontStretch(m_font.stretch());
    format.setFontOverline(m_font.overline());
    format.setFontPointSize(m_font.pointSize());
    format.setFontStrikeOut(m_font.strikeOut());
    // format.setFontStyleHint(m_font.styleHint());
    // format.setFontStyleName(m_font.styleName());
    format.setFontUnderline(m_font.underline());
    // format.setFontFixedPitch(m_font.fixedPitch());
    format.setFontWordSpacing(m_font.wordSpacing());
    format.setFontLetterSpacing(m_font.letterSpacing());
    // format.setFontStyleStrategy(m_font.styleStrategy());
    format.setFontCapitalization(m_font.capitalization());
    // format.setFontHintingPreference(m_font.hintingPreference());
    format.setFontLetterSpacingType(m_font.letterSpacingType());

    format.setBackground(QBrush(m_backgroundColor));
    format.setForeground(QBrush(m_textColor));

    return format;
}

void SceneElementFormat::applyToAll(SceneElementFormat::Properties properties)
{
    m_format->applyToAll(this, properties);
}

///////////////////////////////////////////////////////////////////////////////

ScreenplayPageLayout::ScreenplayPageLayout(QObject *parent)
    : QObject(parent)
{
    m_padding[0] = 0; // just to get rid of the unused private variable warning.

    this->evaluateRects();
}

ScreenplayPageLayout::~ScreenplayPageLayout()
{

}

void ScreenplayPageLayout::setPaperSize(ScreenplayPageLayout::PaperSize val)
{
    if(m_paperSize == val)
        return;

    m_paperSize = val;
    this->evaluateRects();

    emit paperSizeChanged();
}

void ScreenplayPageLayout::setResolution(qreal val)
{
    if( qFuzzyCompare(m_resolution,val) )
        return;

    m_resolution = val;
    this->evaluateRects();

    emit resolutionChanged();
}

void ScreenplayPageLayout::configure(QTextDocument *document)
{
    const bool stdResolution = qFuzzyCompare(m_resolution,72);
    const QMarginsF pixelMargins = stdResolution ? m_margins : m_pageLayout.marginsPixels(72);
    const QSizeF pageSize = stdResolution ? m_paperRect.size() : m_pageLayout.pageSize().sizePixels(72);

    document->setPageSize(pageSize);

    QTextFrameFormat format;
    format.setTopMargin(pixelMargins.top());
    format.setBottomMargin(pixelMargins.bottom());
    format.setLeftMargin(pixelMargins.left());
    format.setRightMargin(pixelMargins.right());
    document->rootFrame()->setFrameFormat(format);
}

void ScreenplayPageLayout::configure(QPagedPaintDevice *printer)
{
    printer->setPageLayout(m_pageLayout);
}

void ScreenplayPageLayout::evaluateRects()
{
    // Page margins
    static const qreal leftMargin = 1.5; // inches
    static const qreal topMargin = 1.5; // inches
    static const qreal bottomMargin = 1.5; // inches
    static const qreal contentWidth = 6.0; // inches

    const QPageSize pageSize(m_paperSize == A4 ? QPageSize::A4 : QPageSize::Letter);
    const QRectF paperRectIn = pageSize.rect(QPageSize::Inch);
    const qreal rightMargin = paperRectIn.width() - contentWidth - leftMargin;

    const QMarginsF margins(leftMargin, topMargin, rightMargin, bottomMargin);

    QPageLayout pageLayout(pageSize, QPageLayout::Portrait, margins, QPageLayout::Inch);
    pageLayout.setMode(QPageLayout::StandardMode);

    m_paperRect = pageLayout.fullRectPixels(int(m_resolution));
    m_paintRect = pageLayout.paintRectPixels(int(m_resolution));
    m_margins = pageLayout.marginsPixels(int(m_resolution));

    m_headerRect = m_paperRect;
    m_headerRect.setLeft(m_paintRect.left());
    m_headerRect.setRight(m_paintRect.right());
    m_headerRect.setBottom(m_paintRect.top());

    m_footerRect = m_paperRect;
    m_footerRect.setLeft(m_paintRect.left());
    m_footerRect.setRight(m_paintRect.right());
    m_footerRect.setTop(m_paintRect.bottom());

    m_pageLayout = pageLayout;

    emit rectsChanged();
}

///////////////////////////////////////////////////////////////////////////////

ScreenplayFormat::ScreenplayFormat(QObject *parent)
    : QAbstractListModel(parent),
      m_pageWidth(750),
      m_scriteDocument(qobject_cast<ScriteDocument*>(parent))
{
    m_padding[0] = 0; // just to get rid of the unused private variable warning.

    for(int i=SceneElement::Min; i<=SceneElement::Max; i++)
    {
        SceneElementFormat *elementFormat = new SceneElementFormat(SceneElement::Type(i), this);
        connect(elementFormat, &SceneElementFormat::elementFormatChanged, this, &ScreenplayFormat::formatChanged);
        m_elementFormats.append(elementFormat);
    }

    QObject::connect(this, &ScreenplayFormat::formatChanged, [this]() {
        this->markAsModified();
    });

    this->resetToDefaults();
}

ScreenplayFormat::~ScreenplayFormat()
{

}

void ScreenplayFormat::setScreen(QScreen *val)
{
    if(m_screen == val || m_screen != nullptr)
        return;

    m_screen = val;
    emit screenChanged();

    m_pageLayout->setResolution(int(val->logicalDotsPerInch()));
}

void ScreenplayFormat::setDefaultFont(const QFont &val)
{
    if(m_defaultFont == val)
        return;

    m_defaultFont = val;

    static const int minPixelSize = 21; // Fonts should be atleast 22pixels tall on screen.
    QFont font = m_defaultFont;
    QFontInfo fontInfo(font);
    while(fontInfo.pixelSize() < minPixelSize)
    {
        font.setPointSize(font.pointSize()+1);
        fontInfo = QFontInfo(font);
    }

    m_fontPointSizeDelta = qMax(fontInfo.pointSize()-m_defaultFont.pointSize(),0);

    emit defaultFontChanged();
}

QFont ScreenplayFormat::defaultFont2() const
{
    QFont font = m_defaultFont;
    font.setPointSize( font.pointSize()+m_fontPointSizeDelta );
    return font;
}

SceneElementFormat *ScreenplayFormat::elementFormat(SceneElement::Type type) const
{
    int itype = int(type);
    itype = itype%(SceneElement::Max+1);
    return m_elementFormats.at(itype);
}

SceneElementFormat *ScreenplayFormat::elementFormat(int type) const
{
    type = type%(SceneElement::Max+1);
    return this->elementFormat(SceneElement::Type(type));
}

QQmlListProperty<SceneElementFormat> ScreenplayFormat::elementFormats()
{
    return QQmlListProperty<SceneElementFormat>(
                reinterpret_cast<QObject*>(this),
                static_cast<void*>(this),
                &ScreenplayFormat::staticElementFormatCount,
                &ScreenplayFormat::staticElementFormatAt);
}

void ScreenplayFormat::applyToAll(const SceneElementFormat *from, SceneElementFormat::Properties properties)
{
    if(from == nullptr)
        return;

    Q_FOREACH(SceneElementFormat *format, m_elementFormats)
    {
        if(from == format)
            continue;

        switch(properties)
        {
        case SceneElementFormat::FontFamily:
            format->setFontFamily( from->font().family() );
            break;
        case SceneElementFormat::FontSize:
            format->setFontPointSize( from->font().pointSize() );
            break;
        case SceneElementFormat::FontStyle:
            format->setFontBold( from->font().bold() );
            format->setFontItalics( from->font().italic() );
            format->setFontUnderline( from->font().underline() );
            break;
        case SceneElementFormat::LineHeight:
            format->setLineHeight( from->lineHeight() );
            break;
        case SceneElementFormat::TextAndBackgroundColors:
            format->setTextColor( from->textColor() );
            format->setBackgroundColor( from->backgroundColor() );
            break;
        case SceneElementFormat::TextAlignment:
            format->setTextAlignment( from->textAlignment() );
            break;
        case SceneElementFormat::BlockWidth:
            format->setBlockWidth( from->blockWidth() );
            break;
        case SceneElementFormat::BlockAlignment:
            format->setBlockAlignment( from->blockAlignment() );
            break;
        case SceneElementFormat::Margins:
            format->setTopMargin( from->topMargin() );
            format->setBottomMargin( from->bottomMargin() );
            break;
        }
    }
}

int ScreenplayFormat::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_elementFormats.size();
}

QVariant ScreenplayFormat::data(const QModelIndex &index, int role) const
{
    if(role == SceneElementFomat && index.isValid())
        return QVariant::fromValue<QObject*>( qobject_cast<QObject*>(m_elementFormats.at(index.row())) );

    return QVariant();
}

QHash<int, QByteArray> ScreenplayFormat::roleNames() const
{
    QHash<int,QByteArray> roles;
    roles[SceneElementFomat] = "sceneElementFormat";
    return roles;
}

void ScreenplayFormat::resetToDefaults()
{
    // Standard font names, sizes, margins etc picked from the URL below.
    // https://johnaugust.com/2007/hollywood-standard
    this->setDefaultFont(QFont("Courier Prime", 12));

    for(int i=SceneElement::Min; i<=SceneElement::Max; i++)
        m_elementFormats.at(i)->setFont(m_defaultFont);

    m_elementFormats[SceneElement::Action]->setTextAlignment(Qt::AlignJustify);

    m_elementFormats[SceneElement::Character]->setBlockWidth(0.6);
    m_elementFormats[SceneElement::Character]->setTextAlignment(Qt::AlignHCenter);
    m_elementFormats[SceneElement::Character]->fontRef().setBold(true);
    m_elementFormats[SceneElement::Character]->fontRef().setCapitalization(QFont::AllUppercase);

    m_elementFormats[SceneElement::Dialogue]->setBlockWidth(0.6);
    m_elementFormats[SceneElement::Dialogue]->setTextAlignment(Qt::AlignJustify);
    m_elementFormats[SceneElement::Dialogue]->setTopMargin(0);

    m_elementFormats[SceneElement::Parenthetical]->setBlockWidth(0.5);
    m_elementFormats[SceneElement::Parenthetical]->setTextAlignment(Qt::AlignHCenter);
    m_elementFormats[SceneElement::Parenthetical]->fontRef().setItalic(true);
    m_elementFormats[SceneElement::Parenthetical]->setTopMargin(0);

    m_elementFormats[SceneElement::Shot]->setTextAlignment(Qt::AlignLeft);
    m_elementFormats[SceneElement::Shot]->setBlockWidth(0.85);
    m_elementFormats[SceneElement::Shot]->setBlockAlignment(Qt::AlignCenter);
    m_elementFormats[SceneElement::Shot]->fontRef().setCapitalization(QFont::AllUppercase);

    m_elementFormats[SceneElement::Transition]->setTextAlignment(Qt::AlignRight);
    m_elementFormats[SceneElement::Transition]->fontRef().setCapitalization(QFont::AllUppercase);

    m_elementFormats[SceneElement::Heading]->fontRef().setBold(true);
    m_elementFormats[SceneElement::Heading]->fontRef().setPointSize(m_defaultFont.pointSize()+2);
    m_elementFormats[SceneElement::Heading]->fontRef().setCapitalization(QFont::AllUppercase);
}

SceneElementFormat *ScreenplayFormat::staticElementFormatAt(QQmlListProperty<SceneElementFormat> *list, int index)
{
    index = index%(SceneElement::Max+1);
    return reinterpret_cast< ScreenplayFormat* >(list->data)->m_elementFormats.at(index);
}

int ScreenplayFormat::staticElementFormatCount(QQmlListProperty<SceneElementFormat> *list)
{
    return reinterpret_cast< ScreenplayFormat* >(list->data)->m_elementFormats.size();
}

///////////////////////////////////////////////////////////////////////////////

class SceneDocumentBlockUserData : public QTextBlockUserData
{
public:
    SceneDocumentBlockUserData(SceneElement *element);
    ~SceneDocumentBlockUserData();

    SceneElement *sceneElement() const { return m_sceneElement; }

    void resetFormat() { m_formatMTime = -1; }
    bool shouldUpdateFromFormat(const SceneElementFormat *format) {
        return format->isModified(&m_formatMTime);
    }

    QTextBlockFormat blockFormat;
    QTextCharFormat charFormat;

    void setHighlightedText(const QString &text) {
        m_highlightedText = text;
    }
    QString highlightedText() const { return m_highlightedText; }

    void setTransliteratedSegment(int start, int end, TransliterationEngine::Language language) {
        m_transliterationStart = start;
        m_transliterationEnd = end;
        m_translitrationLanguage = language;
    }
    QPair<int,int> transliteratedSegment() {
        const QPair<int,int> ret = qMakePair(m_transliterationStart, m_transliterationEnd);
        m_transliterationStart = -1;
        m_transliterationEnd = -1;
        return ret;
    }
    TransliterationEngine::Language transliterationLanguage() {
        TransliterationEngine::Language ret = m_translitrationLanguage;
        m_translitrationLanguage = TransliterationEngine::English;
        return ret;
    }
    bool hasTransliteratedSegment() const {
        return m_transliterationEnd >= 0 && m_transliterationStart >= 0 && m_transliterationEnd > m_transliterationStart;
    }

    static SceneDocumentBlockUserData *get(const QTextBlock &block);
    static SceneDocumentBlockUserData *get(QTextBlockUserData *userData);

private:
    QPointer<SceneElement> m_sceneElement;
    QString m_highlightedText;
    int m_formatMTime = 0;
    int m_transliterationEnd = 0;
    int m_transliterationStart = 0;
    TransliterationEngine::Language m_translitrationLanguage = TransliterationEngine::English;
};

SceneDocumentBlockUserData::SceneDocumentBlockUserData(SceneElement *element)
    : m_sceneElement(element) { }
SceneDocumentBlockUserData::~SceneDocumentBlockUserData() { }

SceneDocumentBlockUserData *SceneDocumentBlockUserData::get(const QTextBlock &block)
{
    return get(block.userData());
}

SceneDocumentBlockUserData *SceneDocumentBlockUserData::get(QTextBlockUserData *userData)
{
    if(userData == nullptr)
        return nullptr;

    SceneDocumentBlockUserData *userData2 = reinterpret_cast<SceneDocumentBlockUserData*>(userData);
    return userData2;
}

SceneDocumentBinder::SceneDocumentBinder(QObject *parent)
    : QSyntaxHighlighter(parent),
      m_initializeDocumentTimer("SceneDocumentBinder.m_initializeDocumentTimer")
{

}

SceneDocumentBinder::~SceneDocumentBinder()
{

}

void SceneDocumentBinder::setScreenplayFormat(ScreenplayFormat *val)
{
    if(m_screenplayFormat == val)
        return;

    if(m_screenplayFormat != nullptr)
        disconnect(m_screenplayFormat, &ScreenplayFormat::formatChanged,
                this, &QSyntaxHighlighter::rehighlight);

    m_screenplayFormat = val;
    if(m_screenplayFormat != nullptr)
    {
        connect(m_screenplayFormat, &ScreenplayFormat::formatChanged,
                this, &QSyntaxHighlighter::rehighlight);

        if( qFuzzyCompare(m_textWidth,0.0) )
            this->setTextWidth(m_screenplayFormat->pageLayout()->contentWidth());
    }

    emit screenplayFormatChanged();

    this->initializeDocument();
}

void SceneDocumentBinder::setScene(Scene *val)
{
    if(m_scene == val)
        return;

    if(m_scene != nullptr)
    {
        disconnect(m_scene, &Scene::sceneElementChanged,
                   this, &SceneDocumentBinder::onSceneElementChanged);
        disconnect(m_scene, &Scene::sceneAboutToReset,
                   this, &SceneDocumentBinder::onSceneAboutToReset);
        disconnect(m_scene, &Scene::sceneReset,
                   this, &SceneDocumentBinder::onSceneReset);
    }

    m_scene = val;

    if(m_scene != nullptr)
    {
        connect(m_scene, &Scene::sceneElementChanged,
                this, &SceneDocumentBinder::onSceneElementChanged);
        connect(m_scene, &Scene::sceneAboutToReset,
                this, &SceneDocumentBinder::onSceneAboutToReset);
        connect(m_scene, &Scene::sceneReset,
                this, &SceneDocumentBinder::onSceneReset);
    }

    emit sceneChanged();

    this->initializeDocument();
}

void SceneDocumentBinder::setTextDocument(QQuickTextDocument *val)
{
    if(m_textDocument == val)
        return;

    if(this->document() != nullptr)
    {
        this->document()->setUndoRedoEnabled(true);
        this->document()->removeEventFilter(this);

        disconnect( this->document(), &QTextDocument::contentsChange,
                    this, &SceneDocumentBinder::onContentsChange);
        disconnect( this->document(), &QTextDocument::blockCountChanged,
                    this, &SceneDocumentBinder::syncSceneFromDocument);

        if(m_scene != nullptr)
            disconnect(m_scene, &Scene::sceneElementChanged,
                       this, &SceneDocumentBinder::onSceneElementChanged);

        this->setCurrentElement(nullptr);
        this->setCursorPosition(-1);
    }

    m_textDocument = val;
    if(m_textDocument != nullptr)
        this->QSyntaxHighlighter::setDocument(m_textDocument->textDocument());
    else
        this->QSyntaxHighlighter::setDocument(nullptr);

    this->evaluateAutoCompleteHints();

    emit textDocumentChanged();

    this->initializeDocument();

    if(m_textDocument != nullptr)
    {
        this->document()->setUndoRedoEnabled(false);
        this->document()->installEventFilter(this);

        connect(this->document(), &QTextDocument::contentsChange,
                this, &SceneDocumentBinder::onContentsChange);
        connect(this->document(), &QTextDocument::blockCountChanged,
                    this, &SceneDocumentBinder::syncSceneFromDocument);

        if(m_scene != nullptr)
            connect(m_scene, &Scene::sceneElementChanged,
                    this, &SceneDocumentBinder::onSceneElementChanged);

#if 0 // At the moment, this seems to be causing more trouble than help.
        this->document()->setTextWidth(m_textWidth);
#endif

        this->setCursorPosition(0);
    }
    else
        this->setCursorPosition(-1);
}

void SceneDocumentBinder::setTextWidth(qreal val)
{
    if( qFuzzyCompare(m_textWidth, val) )
        return;

    m_textWidth = val;
#if 0 // At the moment, this seems to be causing more trouble than help.
    if(this->document() != nullptr)
        this->document()->setTextWidth(m_textWidth);
#endif

    emit textWidthChanged();
}

void SceneDocumentBinder::setCursorPosition(int val)
{
    if(m_initializingDocument)
        return;

    if(m_cursorPosition >= 0 && (m_textDocument == nullptr || this->document() == nullptr))
    {
        m_cursorPosition = -1;
        m_currentElementCursorPosition = -1;
        emit cursorPositionChanged();
    }

    if(m_cursorPosition == val)
        return;

    m_cursorPosition = val;
    m_currentElementCursorPosition = -1;
    if(m_scene != nullptr)
        m_scene->setCursorPosition(m_cursorPosition);

    if(m_cursorPosition < 0)
    {
        emit cursorPositionChanged();
        return;
    }

    if(this->document()->isEmpty() || m_cursorPosition > this->document()->characterCount())
    {
        emit cursorPositionChanged();
        return;
    }

    QTextCursor cursor(this->document());
    cursor.setPosition(val);

    QTextBlock block = cursor.block();
    if(!block.isValid())
    {
        qDebug("[%d] There is no block at the cursor position %d.", __LINE__, val);
        emit cursorPositionChanged();
        return;
    }

    SceneDocumentBlockUserData *userData = SceneDocumentBlockUserData::get(block);
    if(userData == nullptr)
    {
        this->syncSceneFromDocument();
        userData = SceneDocumentBlockUserData::get(block);
    }

    if(userData == nullptr)
    {
        this->setCurrentElement(nullptr);
        qWarning("[%d] TextDocument has a block at %d that isnt backed by a SceneElement!!", __LINE__, val);
    }
    else
    {
        this->setCurrentElement(userData->sceneElement());
        if(!m_autoCompleteHints.isEmpty())
            this->setCompletionPrefix(block.text());
    }

    m_currentElementCursorPosition = m_cursorPosition - block.position();
    emit cursorPositionChanged();
}

void SceneDocumentBinder::setCharacterNames(const QStringList &val)
{
    if(m_characterNames == val)
        return;

    m_characterNames = val;
    emit characterNamesChanged();
}

void SceneDocumentBinder::setForceSyncDocument(bool val)
{
    if(m_forceSyncDocument == val)
        return;

    m_forceSyncDocument = val;
    emit forceSyncDocumentChanged();
}

void SceneDocumentBinder::tab()
{
    if(m_cursorPosition < 0 || m_textDocument == nullptr || m_currentElement == nullptr || this->document() == nullptr)
        return;

    const int elementNr = m_scene->indexOfElement(m_currentElement);
    if(elementNr < 0)
        return;

    switch(m_currentElement->type())
    {
    case SceneElement::Action:
        m_currentElement->setType(SceneElement::Character);
        break;
    case SceneElement::Character:
        if(m_tabHistory.isEmpty())
            m_currentElement->setType(SceneElement::Action);
        else
            m_currentElement->setType(SceneElement::Transition);
        break;
    case SceneElement::Dialogue:
        m_currentElement->setType(SceneElement::Parenthetical);
        break;
    case SceneElement::Parenthetical:
        m_currentElement->setType(SceneElement::Dialogue);
        break;
    case SceneElement::Shot:
        m_currentElement->setType(SceneElement::Transition);
        break;
    case SceneElement::Transition:
        m_currentElement->setType(SceneElement::Action);
        break;
    default:
        break;
    }

    m_tabHistory.append(m_currentElement->type());
}

void SceneDocumentBinder::backtab()
{
    // Do nothing. It doesnt work anyway!
}

bool SceneDocumentBinder::canGoUp()
{
    if(m_cursorPosition < 0 || this->document() == nullptr)
        return false;

    QTextCursor cursor(this->document());
    cursor.setPosition(qMax(m_cursorPosition,0));
    return cursor.movePosition(QTextCursor::Up);
}

bool SceneDocumentBinder::canGoDown()
{
    if(m_cursorPosition < 0 || this->document() == nullptr)
        return false;

    QTextCursor cursor(this->document());
    cursor.setPosition(qMax(m_cursorPosition,0));
    return cursor.movePosition(QTextCursor::Down);
}

void SceneDocumentBinder::refresh()
{
    if(this->document())
    {
        QTextBlock block = this->document()->firstBlock();
        while(block.isValid())
        {
            SceneDocumentBlockUserData *userData = SceneDocumentBlockUserData::get(block);
            if(userData)
                userData->resetFormat();
            block = block.next();
        }

        this->rehighlight();
    }
}

int SceneDocumentBinder::lastCursorPosition() const
{
    if(m_cursorPosition < 0 || this->document() == nullptr)
        return 0;

    QTextCursor cursor(this->document());
    cursor.setPosition(qMax(m_cursorPosition,0));
    cursor.movePosition(QTextCursor::End);
    return cursor.position();
}

int SceneDocumentBinder::cursorPositionAtBlock(int blockNumber) const
{
    if(this->document() != nullptr)
    {
        const QTextBlock block = this->document()->findBlockByNumber(blockNumber);
        if( m_cursorPosition >= block.position() && m_cursorPosition < block.position()+block.length() )
            return m_cursorPosition;

        return block.position()+block.length()-1;
    }

    return -1;
}

QFont SceneDocumentBinder::currentFont() const
{
    if(this->document() == nullptr)
        return QFont();

    QTextCursor cursor(this->document());
    cursor.setPosition(qMax(m_cursorPosition,0));

    QTextCharFormat format = cursor.charFormat();
    return format.font();
}

void SceneDocumentBinder::highlightBlock(const QString &text)
{
    if(m_initializingDocument)
        return;

    if(m_screenplayFormat == nullptr)
        return;

    QTextBlock block = this->QSyntaxHighlighter::currentBlock();
    SceneDocumentBlockUserData *userData = SceneDocumentBlockUserData::get(block);
    if(userData == nullptr)
    {
        this->syncSceneFromDocument();
        userData = SceneDocumentBlockUserData::get(block);
    }

    if(userData == nullptr)
    {
        qWarning("[%d] TextDocument has a block that isnt backed by a SceneElement!!", __LINE__);
        return;
    }

    SceneElement *element = userData->sceneElement();
    if(element == nullptr)
    {
        qWarning("[%d] TextDocument has a block that isnt backed by a SceneElement!!", __LINE__);
        return;
    }

    const SceneElementFormat *format = m_screenplayFormat->elementFormat(element->type());
    const bool updateFromFormat = userData->shouldUpdateFromFormat(format) || userData->highlightedText().isEmpty();
    QTextCursor cursor(block);

    if(updateFromFormat)
    {
        userData->blockFormat = format->createBlockFormat();
        userData->charFormat = format->createCharFormat();
        userData->charFormat.setFontPointSize(format->font().pointSize()+m_screenplayFormat->fontPointSizeDelta());

        cursor.setPosition(block.position(), QTextCursor::MoveAnchor);
        cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
        cursor.setCharFormat(userData->charFormat);
        cursor.setBlockFormat(userData->blockFormat);
        cursor.clearSelection();
    }

    if(userData->hasTransliteratedSegment())
    {
        const QPair<int,int> range = userData->transliteratedSegment();
        TransliterationEngine::Language language = userData->transliterationLanguage();
        const QFont font = TransliterationEngine::instance()->languageFont(language);

        cursor.setPosition(range.first + block.position());
        cursor.setPosition(range.second + block.position(), QTextCursor::KeepAnchor);
        QTextCharFormat format;
        format.setFontFamily(font.family());
        cursor.mergeCharFormat(format);
        cursor.clearSelection();

        if(m_currentElement == element)
            emit currentFontChanged();

        return;
    }

    auto applyFormatChanges = [](QTextCursor &cursor, QChar::Script script) {
        if(cursor.hasSelection()) {
            TransliterationEngine::Language language = TransliterationEngine::languageForScript(script);
            const QFont font = TransliterationEngine::instance()->languageFont(language);

            if(cursor.charFormat().fontFamily() != font.family())
            {
                QTextCharFormat format;
                format.setFontFamily(font.family());
                // format.setForeground(script == QChar::Script_Latin ? Qt::black : Qt::red);
                cursor.mergeCharFormat(format);
            }
            cursor.clearSelection();
        }
    };

    auto isEnglishChar = [](const QChar &ch) {
        return ch.isSpace() || ch.isDigit() || ch.isPunct() || ch.category() == QChar::Separator_Line || ch.script() == QChar::Script_Latin;
    };

    const int charsAdded = updateFromFormat ? text.length() : (m_cursorPosition >= 0 ? qMax(text.length() - userData->highlightedText().length(), 0) : 0);
    const int charsRemoved = updateFromFormat ? 0 : (m_cursorPosition >= 0 ? qMax(userData->highlightedText().length() - text.length(), 0) : 0);
    const int cursorPositon = updateFromFormat ? 0 : (m_cursorPosition >= 0 ? qMax(m_cursorPosition - block.position(), 0) : 0);
    const int from = charsAdded > 0 ? qMax(cursorPositon-1,0) : (charsRemoved > 0 ? cursorPositon : 0);

    userData->setHighlightedText(text);
    cursor.setPosition(block.position() + from);

    QChar::Script script = QChar::Script_Unknown;
    while(!cursor.atBlockEnd())
    {
        const int index = cursor.position()-block.position();
        const QChar ch = index < 0 || index >= text.length() ? QChar() : text.at(index);
        if(ch.isNull())
            break;

        const QChar::Script chScript = isEnglishChar(ch) ? QChar::Script_Latin : ch.script();
        if(script != chScript)
        {
            applyFormatChanges(cursor, script);
            script = chScript;
        }

        cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
    }

    applyFormatChanges(cursor, script);

    if(m_currentElement == element)
        emit currentFontChanged();
}

void SceneDocumentBinder::timerEvent(QTimerEvent *te)
{
    if(te->timerId() == m_initializeDocumentTimer.timerId())
    {
        m_initializeDocumentTimer.stop();
        this->initializeDocument();
    }
}

void SceneDocumentBinder::initializeDocument()
{
    if(m_textDocument == nullptr || m_scene == nullptr || m_screenplayFormat == nullptr)
        return;

    m_initializingDocument = true;

    m_tabHistory.clear();

    QFont defaultFont = m_screenplayFormat->defaultFont();
    defaultFont.setPointSize(defaultFont.pointSize()+m_screenplayFormat->fontPointSizeDelta());

    QTextDocument *document = m_textDocument->textDocument();
    document->blockSignals(true);
    document->clear();
    document->setDefaultFont(defaultFont);

    const int nrElements = m_scene->elementCount();

    QTextCursor cursor(document);
    for(int i=0; i<nrElements; i++)
    {
        SceneElement *element = m_scene->elementAt(i);
        if(i > 0)
            cursor.insertBlock();

        QTextBlock block = cursor.block();
        if(!block.isValid() && i == 0)
        {
            cursor.insertBlock();
            block = cursor.block();
        }
        block.setUserData(new SceneDocumentBlockUserData(element));
        cursor.insertText(element->text());
    }
    document->blockSignals(false);

    if(m_cursorPosition <= 0 && m_currentElement == nullptr && nrElements == 1)
        this->setCurrentElement(m_scene->elementAt(0));

    this->setDocumentLoadCount(m_documentLoadCount+1);
    m_initializingDocument = false;
    this->QSyntaxHighlighter::rehighlight();

    emit documentInitialized();
}

void SceneDocumentBinder::initializeDocumentLater()
{
    m_initializeDocumentTimer.start(100, this);
}

void SceneDocumentBinder::setDocumentLoadCount(int val)
{
    if(m_documentLoadCount == val)
        return;

    m_documentLoadCount = val;
    emit documentLoadCountChanged();
}

void SceneDocumentBinder::setCurrentElement(SceneElement *val)
{
    if(m_currentElement == val)
        return;

    m_currentElement = val;
    emit currentElementChanged();

    m_tabHistory.clear();
    this->evaluateAutoCompleteHints();

    emit currentFontChanged();
}

class ForceCursorPositionHack : public QObject
{
public:
    ForceCursorPositionHack(const QTextBlock &block, SceneDocumentBinder *binder);
    ~ForceCursorPositionHack();

    void timerEvent(QTimerEvent *event);

private:
    QTextBlock m_block;
    SimpleTimer m_timer;
    SceneDocumentBinder *m_binder = nullptr;
};

ForceCursorPositionHack::ForceCursorPositionHack(const QTextBlock &block, SceneDocumentBinder *binder)
    : QObject(const_cast<QTextDocument*>(block.document())),
      m_block(block),
      m_timer("ForceCursorPositionHack.m_timer"),
      m_binder(binder)
{
    if(!m_block.text().isEmpty()) {
        GarbageCollector::instance()->add(this);
        return;
    }

    QTextCursor cursor(m_block);
    cursor.insertText(QStringLiteral("("));
    m_timer.start(0, this);
}

ForceCursorPositionHack::~ForceCursorPositionHack() {  }

void ForceCursorPositionHack::timerEvent(QTimerEvent *event)
{
    if(event->timerId() == m_timer.timerId())
    {
        m_timer.stop();
        QTextCursor cursor(m_block);
        cursor.deleteChar();

        SceneDocumentBlockUserData *userData = SceneDocumentBlockUserData::get(m_block);
        if(userData && userData->sceneElement()->type() == SceneElement::Parenthetical)
        {
            if(m_block.text().isEmpty())
            {
                cursor.insertText(QStringLiteral("()"));
                cursor.movePosition(QTextCursor::Left);
            }
        }

        emit m_binder->requestCursorPosition(cursor.position());

        GarbageCollector::instance()->add(this);
    }
}

void SceneDocumentBinder::onSceneElementChanged(SceneElement *element, Scene::SceneElementChangeType type)
{
    if(m_initializingDocument)
        return;

    if(m_textDocument == nullptr || this->document() == nullptr || m_scene == nullptr || element->scene() != m_scene)
        return;

    if(m_forceSyncDocument)
        this->initializeDocumentLater();

    if(type != Scene::ElementTypeChange)
        return;

    this->evaluateAutoCompleteHints();

    auto updateBlock = [=](const QTextBlock &block) {
        SceneDocumentBlockUserData *userData = SceneDocumentBlockUserData::get(block);
        if(userData != nullptr && userData->sceneElement() == element) {
            // Text changes from scene element to block are not applied
            // Only element type changes can be applied.
            userData->resetFormat();
            this->rehighlightBlock(block);
            if(element->text().isEmpty())
                new ForceCursorPositionHack(block, this);
            return true;
        }
        return false;
    };

    const int elementNr = m_scene->indexOfElement(element);
    QTextBlock block;

    if(elementNr >= 0)
    {
        block = this->document()->findBlockByNumber(elementNr);
        if(updateBlock(block))
            return;
    }

    block = this->document()->firstBlock();
    while(block.isValid())
    {
        if( updateBlock(block) )
            return;

        block = block.next();
    }
}

void SceneDocumentBinder::onContentsChange(int from, int charsRemoved, int charsAdded)
{
    if(m_initializingDocument || m_sceneIsBeingReset)
        return;

    Q_UNUSED(charsRemoved)
    Q_UNUSED(charsAdded)

    if(m_textDocument == nullptr || m_scene == nullptr || this->document() == nullptr)
        return;

    QTextCursor cursor(this->document());
    cursor.setPosition(from);

    QTextBlock block = cursor.block();
    SceneDocumentBlockUserData *userData = SceneDocumentBlockUserData::get(block);
    if(userData == nullptr)
    {
        this->syncSceneFromDocument();
        return;
    }

    if(userData == nullptr)
    {
        qWarning("[%d] TextDocument has a block at %d that isnt backed by a SceneElement!!", __LINE__, from);
        return;
    }

    SceneElement *sceneElement = userData->sceneElement();
    if(sceneElement == nullptr)
    {
        qWarning("[%d] TextDocument has a block at %d that isnt backed by a SceneElement!!", __LINE__, from);
        return;
    }

    sceneElement->setText(block.text());
    m_tabHistory.clear();
}

void SceneDocumentBinder::syncSceneFromDocument(int nrBlocks)
{
    if(m_initializingDocument || m_sceneIsBeingReset)
        return;

    if(m_textDocument == nullptr || m_scene == nullptr)
        return;

    if(nrBlocks < 0)
        nrBlocks = this->document()->blockCount();

    /*
     * Ensure that blocks on the QTextDocument are in sync with
     * SceneElements in the Scene. I know that we are using a for loop
     * to make this happen, so we are (many-times) needlessly looping
     * over blocks that have already been touched, thereby making
     * this function slow. Still, I feel that this is better. A scene
     * would not have more than a few blocks, atbest 100 blocks.
     * So its better we sync it like this.
     */

    m_scene->beginUndoCapture();

    QList<SceneElement*> elementList;
    elementList.reserve(nrBlocks);

    QTextBlock block = this->document()->begin();
    QTextBlock previousBlock;
    while(block.isValid())
    {
        SceneDocumentBlockUserData *userData = SceneDocumentBlockUserData::get(block);
        if(userData == nullptr)
        {
            SceneElement *newElement = new SceneElement(m_scene);

            if(previousBlock.isValid())
            {
                SceneDocumentBlockUserData *prevUserData = SceneDocumentBlockUserData::get(previousBlock);
                SceneElement *prevElement = prevUserData->sceneElement();
                switch(prevElement->type())
                {
                case SceneElement::Action:
                    newElement->setType(SceneElement::Action);
                    break;
                case SceneElement::Character:
                    newElement->setType(SceneElement::Dialogue);
                    break;
                case SceneElement::Dialogue:
                    newElement->setType(SceneElement::Character);
                    break;
                case SceneElement::Parenthetical:
                    newElement->setType(SceneElement::Dialogue);
                    break;
                case SceneElement::Shot:
                    newElement->setType(SceneElement::Action);
                    break;
                case SceneElement::Transition:
                    newElement->setType(SceneElement::Action);
                    break;
                default:
                    newElement->setType(SceneElement::Action);
                    break;
                }

                m_scene->insertElementAfter(newElement, prevElement);
            }
            else
            {
                newElement->setType(SceneElement::Action);
                m_scene->insertElementAt(newElement, 0);
            }

            userData = new SceneDocumentBlockUserData(newElement);
            block.setUserData(userData);
        }

        elementList.append(userData->sceneElement());
        userData->sceneElement()->setText(block.text());

        previousBlock = block;
        block = block.next();
    }

    m_scene->setElementsList(elementList);
    m_scene->endUndoCapture();
}

bool SceneDocumentBinder::eventFilter(QObject *object, QEvent *event)
{
    if(object == this->document() && event->type() == TransliterationEvent::EventType())
    {
        TransliterationEvent *te = static_cast<TransliterationEvent*>(event);

        QTextCursor cursor(this->document());
        cursor.setPosition(te->start());

        QTextBlock block = cursor.block();
        SceneDocumentBlockUserData *userData = SceneDocumentBlockUserData::get(block);
        if(userData)
        {
            const int start = te->start()-block.position();
            const int end = te->end()-block.position();
            userData->setTransliteratedSegment(start, end, te->language());
            this->rehighlightBlock(block);
        }

        return true;
    }

    return false;
}

void SceneDocumentBinder::evaluateAutoCompleteHints()
{
    QStringList hints;

    if(m_currentElement == nullptr)
    {
        this->setAutoCompleteHints(hints);
        return;
    }

    static QStringList transitions = QStringList() <<
            QStringLiteral("CUT TO") <<
            QStringLiteral("DISSOLVE TO") <<
            QStringLiteral("FADE IN") <<
            QStringLiteral("FADE OUT") <<
            QStringLiteral("FADE TO") <<
            QStringLiteral("FLASH CUT TO") <<
            QStringLiteral("FREEZE FRAME") <<
            QStringLiteral("IRIS IN") <<
            QStringLiteral("IRIS OUT") <<
            QStringLiteral("JUMP CUT TO") <<
            QStringLiteral("MATCH CUT TO") <<
            QStringLiteral("MATCH DISSOLVE TO") <<
            QStringLiteral("SMASH CUT TO") <<
            QStringLiteral("STOCK SHOT") <<
            QStringLiteral("TIME CUT") <<
            QStringLiteral("WIPE TO");

    static QStringList shots = QStringList() <<
            QStringLiteral("AIR") <<
            QStringLiteral("CLOSE ON") <<
            QStringLiteral("CLOSER ON") <<
            QStringLiteral("CLOSEUP") <<
            QStringLiteral("ESTABLISHING") <<
            QStringLiteral("EXTREME CLOSEUP") <<
            QStringLiteral("INSERT") <<
            QStringLiteral("POV") <<
            QStringLiteral("SURFACE") <<
            QStringLiteral("THREE SHOT") <<
            QStringLiteral("TWO SHOT") <<
            QStringLiteral("UNDERWATER") <<
            QStringLiteral("WIDE") <<
            QStringLiteral("WIDE ON") <<
            QStringLiteral("WIDER ANGLE");

    switch(m_currentElement->type())
    {
    case SceneElement::Character:
        hints = m_characterNames;
        break;
    case SceneElement::Transition:
        hints = transitions;
        break;
    case SceneElement::Shot:
        hints = shots;
        break;
    default:
        break;
    }

    this->setAutoCompleteHints(hints);
}

void SceneDocumentBinder::setAutoCompleteHints(const QStringList &val)
{
    if(m_autoCompleteHints == val)
        return;

    m_autoCompleteHints = val;
    emit autoCompleteHintsChanged();
}

void SceneDocumentBinder::setCompletionPrefix(const QString &val)
{
    if(m_completionPrefix == val)
        return;

    m_completionPrefix = val;
    emit completionPrefixChanged();
}

void SceneDocumentBinder::onSceneAboutToReset()
{
    m_sceneIsBeingReset = true;
}

void SceneDocumentBinder::onSceneReset(int position)
{
    this->initializeDocument();

    if(position >= 0)
    {
        QTextCursor cursor(this->document());
        cursor.movePosition(QTextCursor::End);
        position = qBound(0, position, cursor.position());
        emit requestCursorPosition(position);
    }

    m_sceneIsBeingReset = false;
}

