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

#ifndef SCREENPLAYTEXTDOCUMENT_H
#define SCREENPLAYTEXTDOCUMENT_H

#include <QTextDocument>
#include <QQmlParserStatus>
#include <QPagedPaintDevice>

#include "scene.h"
#include "formatting.h"
#include "screenplay.h"

class QQmlEngine;
class ScreenplayTextDocumentUpdate;
class ScreenplayTextDocument : public QObject,
                               public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)

public:
    ScreenplayTextDocument(QObject *parent=nullptr);
    ScreenplayTextDocument(QTextDocument *document, QObject *parent=nullptr);
    ~ScreenplayTextDocument();

    Q_PROPERTY(QTextDocument* textDocument READ textDocument WRITE setTextDocument NOTIFY textDocumentChanged)
    void setTextDocument(QTextDocument* val);
    QTextDocument* textDocument() const { return m_textDocument; }
    Q_SIGNAL void textDocumentChanged();

    Q_PROPERTY(Screenplay* screenplay READ screenplay WRITE setScreenplay NOTIFY screenplayChanged)
    void setScreenplay(Screenplay* val);
    Screenplay* screenplay() const { return m_screenplay; }
    Q_SIGNAL void screenplayChanged();

    Q_PROPERTY(ScreenplayFormat* formatting READ formatting WRITE setFormatting NOTIFY formattingChanged)
    void setFormatting(ScreenplayFormat* val);
    ScreenplayFormat* formatting() const { return m_formatting; }
    Q_SIGNAL void formattingChanged();

    Q_PROPERTY(bool syncEnabled READ isSyncEnabled WRITE setSyncEnabled NOTIFY syncEnabledChanged)
    void setSyncEnabled(bool val);
    bool isSyncEnabled() const { return m_syncEnabled; }
    Q_SIGNAL void syncEnabledChanged();

    Q_PROPERTY(bool updating READ isUpdating NOTIFY updatingChanged)
    bool isUpdating() const { return m_updating; }
    Q_SIGNAL void updatingChanged();

    Q_PROPERTY(int pageCount READ pageCount NOTIFY pageCountChanged)
    int pageCount() const { return m_pageCount; }
    Q_SIGNAL void pageCountChanged();

    Q_PROPERTY(int currentPage READ currentPage NOTIFY currentPageChanged)
    int currentPage() const { return m_currentPage; }
    Q_SIGNAL void currentPageChanged();

    Q_INVOKABLE void print(QObject *printerObject);

signals:
    void updateStarted();
    void updateFinished();

protected:
    // QQmlParserStatus implementation
    void classBegin();
    void componentComplete();

    // QObject interface
    void timerEvent(QTimerEvent *event);

private:
    void init();
    void setUpdating(bool val);
    void setPageCount(int val);
    void setCurrentPage(int val);

    void loadScreenplay();
    void loadScreenplayLater();

    void connectToScreenplaySignals();
    void connectToScreenplayFormatSignals();

    void disconnectFromScreenplaySignals();
    void disconnectFromScreenplayFormatSignals();

    // Hook to signals that convey change in sequencing of scenes
    void onSceneMoved(ScreenplayElement *ptr, int from, int to);
    void onSceneRemoved(ScreenplayElement *ptr, int index);
    void onSceneInserted(ScreenplayElement *element, int index);

    // Hook to signals that convey changes to a specific scene content
    void onSceneReset();
    void onSceneRefreshed();
    void onSceneHeadingChanged();
    void onSceneElementChanged(SceneElement *element, Scene::SceneElementChangeType type);

    // Hook to signals that convey change in formatting
    void onElementFormatChanged();
    void onDefaultFontChanged();

    // Hook to signals to know current element and cursor position,
    // so that we can report current page number.
    void onActiveSceneChanged();
    void onActiveSceneDestroyed(Scene *ptr);
    void onActiveSceneCursorPositionChanged();
    void evaluateCurrentPage();

    // Other methods
    void formatAllBlocks();
    void loadScreenplayElement(const ScreenplayElement *element, QTextCursor &cursor);
    void formatBlock(const QTextBlock &block, const QString &text=QString());

private:
    int m_pageCount = 0;
    bool m_updating = false;
    int m_currentPage = 0;
    bool m_syncEnabled = true;
    QString m_pageImageId;
    Scene *m_activeScene = nullptr;
    bool m_componentComplete = true;
    QQmlEngine *m_qmlEngine = nullptr;
    Screenplay* m_screenplay = nullptr;
    QTextDocument* m_textDocument = nullptr;
    ScreenplayFormat* m_formatting = nullptr;
    SimpleTimer m_loadScreenplayTimer;
    QTextFrameFormat m_sceneFrameFormat;
    bool m_connectedToScreenplaySignals = false;
    bool m_connectedToFormattingSignals = false;
    QPagedPaintDevice::PageSize m_paperSize = QPagedPaintDevice::Letter;
    friend class ScreenplayTextDocumentUpdate;
    ModificationTracker m_screenplayModificationTracker;
    ModificationTracker m_formattingModificationTracker;
    QMap<const ScreenplayElement*, QTextFrame*> m_elementFrameMap;
};

#endif // SCREENPLAYTEXTDOCUMENT_H
