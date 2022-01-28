#ifndef MPVRENDERER_H_
#define MPVRENDERER_H_

#include <mpv/client.h>
#include <mpv/render_gl.h>

#include <QtQuick/QQuickFramebufferObject>
#include <QtCore/QLoggingCategory>

#include "qthelper.hpp"


Q_DECLARE_LOGGING_CATEGORY(wekdeMpv)

class MpvRender;

class MpvObject : public QQuickItem
{
    Q_OBJECT
    //QML_NAMED_ELEMENT(Renderer)
    Q_PROPERTY(QUrl source READ source WRITE setSource NOTIFY sourceChanged)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    Q_PROPERTY(bool mute READ mute WRITE setMute)
    Q_PROPERTY(QString logfile READ logfile WRITE setLogfile)
    Q_PROPERTY(int volume READ volume WRITE setVolume)

    mpv_handle* mpv;

public:
    static void on_update(void* ctx);

    explicit MpvObject(QQuickItem* parent = nullptr);
    virtual ~MpvObject();

    enum Status {
        Stopped,
        Playing,
        Paused,
    };
    Q_ENUM(Status)
    Status status() const;
    QUrl source() const;
    bool mute() const;
    QString logfile() const;
    int volume() const;

    void setSource(const QUrl& source);
    void setMute(const bool& mute);
    void setLogfile(const QString& logfile);
    void setVolume(const int& volume);

public slots:
    void play();
    void pause();
    void stop();

    bool command(const QVariant& params);
    bool setProperty(const QString& name, const QVariant& value);
    QVariant getProperty(const QString& name, bool* ok = nullptr) const;
    void initCallback();


    void prepareMpv();
    void resizeFb();
signals:
    void initFinished();
    void statusChanged();
    void sourceChanged();

private:
    bool inited = false;
    QUrl m_source;
    Status m_status = Stopped;

protected:
    QSGNode *updatePaintNode(QSGNode *, UpdatePaintNodeData *);

private:
    QThread *m_renderThread;
    MpvRender *m_mpvRender;
};

#endif