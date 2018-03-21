#ifndef MVOICE_RECOGNITION_H
#define MVOICE_RECOGNITION_H

#include <QObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QScriptEngine>
#include <QScriptValue>
#include <QScriptValueIterator>
#include <QString>

class mVoice_Recognition : public QObject
{
    Q_OBJECT
public:
    explicit mVoice_Recognition(QObject *parent = 0);
    void sendvoice(QByteArray arr);

private:
    QNetworkAccessManager *manager;
    QString API_access_token;
    QString API_id;
    QString API_language;
    int flag;

    void slot_replyFinish(QNetworkReply* reply);


signals:
    void mVoice_Rec_Received(QString strText);

public slots:


};

#endif // MVOICE_RECOGNITION_H
