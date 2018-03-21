#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QFile>
#include <QAudioInput>
#include <QAudioOutput>
#include <QDebug>
#include <QTimer>
#include <QDir>
#include <iostream>
#include <QMediaPlayer>
#include <QMovie>
#include <QPainter>
#include <QPixmap>
#include <QMouseEvent>
#include <QEvent>
#include <MyGlobalShortCut/MyGlobalShortCut.h>
#include "mvda_detection.h"

namespace Ui {
class MainWindow;
class mVDA_Detection;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
private:
    Ui::MainWindow *ui;

    mVDA_Detection* mvda;
    QNetworkAccessManager* nam;
    QNetworkAccessManager* hecheng_manager;
    QNetworkRequest request;

    QFile outputFile;   // class member.
    QFile putfile;
    QAudioInput* audio; // class member.
    QAudioOutput* outaudio;
    QAudioFormat format,outformat;
    QAudioDeviceInfo info;
    QMediaPlayer* media;
    QMovie *pm;
    QString strText;
   // QTimer* audiotimer;

    QNetworkAccessManager *manager;
        QNetworkRequest *req;
        QString API_access_token;
        QString API_id;
        QString API_key;
        QString API_secret_key;
        QString API_record_path;
        //QString  API_record_format;
       // QString API_record_HZ;
        QString API_language;
        char * m_buf;
        int flag;

        QPoint widgetPos;
        QPoint clickPos;

    void getText(QString para_API_id,
                 QString para_API_access_token,
                 QString para_API_language,
                 QString para_API_record_path);
    void paintEvent(QPaintEvent *);
    bool eventFilter(QObject *obj, QEvent *event);
    void keyPressEvent(QKeyEvent  *event);           //按键按下事件
    void keyReleaseEvent(QKeyEvent  *event);         //按键松开事件


private slots:
    void finishedSlot(QNetworkReply*);

    void on_pushButton_clicked();

    void stopRecording();

    void replyFinish(QNetworkReply *);
    void hecheng_replyFinish(QNetworkReply *);
    void on_pushButton_3_clicked();
    void playstatechenged(QMediaPlayer::State state);
    void slot_shortcut();
    void ReadData(short *data, int len);

};

#endif // MAINWINDOW_H
