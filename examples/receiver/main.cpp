/**
 * Copyright (C) 2020, Val Doroshchuk <valbok@gmail.com>
 */

/**
 * Shows how to use Motion-JPEG streams with Qt5Multimedia.
 *
 * $ ./receiver [url-to-stream-over-http]
 */

#include <Capture/mjpeg_stream.h>
#include <private/qdeclarativevideooutput_p.h>

#include <QtQml/QQmlContext>
#include <QtQml/QQmlEngine>
#include <QApplication>
#include <QtQuick/QQuickItem>
#include <QtQuick/QQuickView>

#include <QAbstractVideoSurface>

#include <QNetworkAccessManager>
#include <QNetworkReply>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QString url = "http://127.0.0.1:8080/stream";
    if (argc > 1)
        url = argv[1];

    QQuickView viewer;
    viewer.setSource(QUrl("qrc:///main.qml"));
    viewer.setResizeMode(QQuickView::SizeRootObjectToView);
    QObject::connect(viewer.engine(), SIGNAL(quit()), &viewer, SLOT(close()));

    QQuickItem *rootObject = viewer.rootObject();
    auto videoOutput = rootObject->findChild<QDeclarativeVideoOutput *>("videoOutput");

    QNetworkAccessManager qnam;
	auto reply = qnam.get(QNetworkRequest(QUrl(url)));

    QObject::connect(reply, &QNetworkReply::finished, [&] {
        if (reply->error())
            qWarning() << "ERROR:" << reply->errorString().toLatin1().constData();
    });

    Capture::mjpeg_stream stream([&](const unsigned char *data, size_t size) {
        QVideoFrame frame = QImage::fromData(data, size, "JPG");
        if (!videoOutput->videoSurface()->isActive()) {
            QVideoSurfaceFormat format(frame.size(), frame.pixelFormat());
            videoOutput->videoSurface()->start(format);
        }
        videoOutput->videoSurface()->present(frame);
    });

    QObject::connect(reply, &QIODevice::readyRead, [&]{
        auto d = reply->readAll();
        stream.read(d.constData(), d.size());
    });

    viewer.setMinimumSize(QSize(300, 360));
    viewer.resize(800, 600);
    viewer.show();

    return app.exec();
}