#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QDebug>
#include <QFileInfo>
#include <QFile>
#include <QMessageBox>
#include <QScriptEngine>
#include <QScriptValue>
#include <QScriptValueIterator>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    API_id = "54-04-A6-CE-2C-F6";
         API_key = "qP5XSo3qqfXbPXx8A4oDqvWI";
         API_secret_key = "77e47f5e5e95374edf92e206ccfc37cd&";
         API_access_token="24.a3721b522758ddf7c8efd0a8a53fe0ae.2592000.1523625249.282335-9600066";
         API_language="zh";
         API_record_path="text1.pcm";//录音文件的路径
         flag=1;//标志,,第一次http回应要获得 API_access_token 值,第二次(以后)的http回应才是语音识别的返回结果,我把http回应都写到一个方法里,所以要区分一下
                    // flag=1是得到API_access_token   flag=0是得到返回的文本
        manager = new QNetworkAccessManager(this);
        connect(manager,SIGNAL(finished(QNetworkReply*)),this,SLOT(replyFinish(QNetworkReply*)));
        /*发送http请求,目的是得到 API_access_token口令*/
       QByteArray bbb;
       QString sstr = "https://openapi.baidu.com/oauth/2.0/token?grant_type=client_credentials&client_id=" + API_key + "&client_secret=" + API_secret_key;
       QUrl urll;
       urll.setUrl(sstr);
       manager->post(QNetworkRequest(urll),bbb);

       hecheng_manager = new QNetworkAccessManager(this);
       connect(hecheng_manager,SIGNAL(finished(QNetworkReply*)),
               this,SLOT(hecheng_replyFinish(QNetworkReply*)));

    nam = new QNetworkAccessManager(this);
    QObject::connect(nam, SIGNAL(finished(QNetworkReply*)),
             this, SLOT(finishedSlot(QNetworkReply*)));

    outputFile.setFileName("text1.pcm");
        format.setSampleRate(8000);
        format.setChannelCount(1);
        format.setSampleSize(16);
        format.setCodec("audio/pcm");
        format.setByteOrder(QAudioFormat::LittleEndian);
        format.setSampleType(QAudioFormat::SignedInt);
        info = QAudioDeviceInfo::defaultInputDevice();
        if (!info.isFormatSupported(format)) {
            qDebug("default format not supported try to use nearest");
           format = info.nearestFormat(format);
        }
        audio = new QAudioInput(format, this);

        putfile.setFileName("received.pcm");
        outformat.setSampleRate(8000);
        outformat.setChannelCount(1);
        outformat.setSampleSize(16);
        outformat.setCodec("audio/mp3");
        outformat.setByteOrder(QAudioFormat::LittleEndian);
        outformat.setSampleType(QAudioFormat::SignedInt);
        info = QAudioDeviceInfo::defaultInputDevice();
        if (!info.isFormatSupported(outformat)) {
            qDebug("default format not supported try to use nearest");
           outformat = info.nearestFormat(outformat);
        }
        outaudio = new QAudioOutput(outformat, this);
        media = new QMediaPlayer;

       // QLabel *label=new QLabel (this);
        pm = new QMovie(":/image/image/1.gif");
        ui->labe->resize(this->size());
        ui->labe->move(0,0);
        ui->labe->setMovie(pm);
        ui->labe->setScaledContents(true);
        pm->start();
        pm->stop();

        this->installEventFilter(this);
        this->setWindowFlags(Qt::FramelessWindowHint);  //隐藏窗体边框
        this->setAttribute(Qt::WA_TranslucentBackground);//设置窗体透明效果
        connect(media,&QMediaPlayer::stateChanged,this,&MainWindow::playstatechenged);

        MyGlobalShortCut* shortcut = new MyGlobalShortCut("Ctrl+F9", this);
        connect(shortcut, SIGNAL(activated()),this, SLOT(slot_shortcut()));

        mvda = new mVDA_Detection(this);
        mvda->vdaStart();
        connect(mvda, SIGNAL(VDA_detected(short*,int)), this, SLOT(ReadData(short*,int)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::slot_shortcut()
{
    qDebug("shortcut received !");
}

void MainWindow::ReadData(short* data,int len)
{
    FILE* fp1;
    int m=0;
    fp1=fopen("received.pcm","wb");
    for(m=0;m<len;m++)
    {
        if(fwrite(&(data[m]),2,1,fp1) !=1)
        {
         //   qDebug("文件写入错误");
        }
        else
        {
         //   qDebug("文件写入成功");
        }
    }
    fclose(fp1);
    QFile file("received.pcm");
    if( !(file.open(QIODevice::ReadOnly)))
    {
        QMessageBox::warning(this,"警告","打开语音文件失败！");
        return;
    }
    /*读入文件流*/
    QDataStream in(&file);
    m_buf = new char[file.size()];
    in.readRawData(m_buf,file.size());
    file.close();

    /*发送http请求,目的是得到语音文本*/
    QString  getTextUrl = "http://vop.baidu.com/server_api?lan=" + API_language + "&cuid=" + API_id + "&token=" + API_access_token;
    QUrl url;
    url.setUrl(getTextUrl);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "audio/pcm;rate=8000");
    QByteArray arr = QByteArray(m_buf, file.size());
    manager->post(request,arr);
    qDebug("已发送");
}

void MainWindow::playstatechenged(QMediaPlayer::State state)
{
    qDebug("ddddddddd");
    if(state == QMediaPlayer::StoppedState)
    {
        pm->stop();
    }
    if(state == QMediaPlayer::PlayingState)
    {
        pm->start();
    }
}

void MainWindow::keyPressEvent(QKeyEvent  *event)
{
    if(event->key()==Qt::Key_V)
    {
        if(event->isAutoRepeat()) return;
        outputFile.close();
        if(outputFile.open( QIODevice::WriteOnly | QIODevice::Truncate ))
        {
            qDebug("file is ok");
        }
        else
            qDebug("file is not ok");
        audio->start(&outputFile);
       // outputFile.readAll();
        qDebug("record start!");
    }
}

void MainWindow::keyReleaseEvent(QKeyEvent  *event)
{
    if(event->key()==Qt::Key_V)
    {
        if(event->isAutoRepeat()) return;
        audio->stop();
        outputFile.close();
        getText(API_id,API_access_token,API_language,API_record_path);
        //delete audio;
        qDebug("record end!");
    }
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if(event->type() == QEvent::WindowActivate)//窗口最小化还原后重绘整个窗口
    {
        this->repaint();
        qDebug()<<"repaint";
    }
    if(event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseMove)
    {
        QMouseEvent *mouEvent = (QMouseEvent *)event;
        if(event->type() == QEvent::MouseButtonPress)
        {
            clickPos = mapToGlobal(mouEvent->pos());//当前鼠标位置
            widgetPos = mapToGlobal(QPoint(0,0));//当前窗口位置

            //qDebug()<<widgetPos;
        }
        if(event->type() == QEvent::MouseMove)
        {
            this->move(widgetPos + mouEvent->globalPos() - clickPos);//窗口位置+鼠标移动位置之差=窗口新位置
        }
    }

    return QWidget::eventFilter(obj,event);
}

void MainWindow::paintEvent(QPaintEvent *)
{
   //  QPixmap pixmap = QPixmap(":/image/image/1.gif").scaled(this->size());
    // QPainter painter(this);
    // painter.drawPixmap(this->rect(), pixmap);

///////////
   /*  QPainterPath path;
     path.setFillRule(Qt::WindingFill);
     path.addRect(2, 30, this->width()-2, this->height()-2);

     QPainter painter1(this);
     painter1.setRenderHint(QPainter::Antialiasing, true);
     painter1.fillPath(path, QBrush(QColor(255, 255, 255,240)));
*/

/*    QColor color(255, 255, 255, 50);
    for(int i=0; i<1; i++)
    {
        QPainterPath path;
        path.setFillRule(Qt::WindingFill);
        path.addRect(1-i, 1-i, this->width()-(1-i)*2, this->height()-(1-i)*2);
        color.setAlpha(150 - sqrt(i)*50);
        painter.setPen(color);
        painter.drawPath(path);
    }*/
}

void MainWindow::replyFinish(QNetworkReply * reply)
{
    if(flag==1)
    {
         /*   QString all=reply->readAll();
            QJsonParseError jsonError;
            QJsonDocument parse_doucment = QJsonDocument::fromJson(all.toUtf8(), &jsonError);
            qDebug()<<jsonError.error<<endl;
            if(jsonError.error == QJsonParseError::NoError)
            {
              if(parse_doucment.isObject())
                {
                   QJsonObject jsonObj = parse_doucment.object();
                    if(jsonObj.contains("access_token"))
                        API_access_token=jsonObj.take("access_token").toString();
                }
             }*/

            QByteArray strJsonAccess=reply->readAll();//得到http返回的所有信息,这个返回的JSON包所携带的是API_access_token口令
            QString string = QString::fromUtf8(strJsonAccess);

            ui->textBrowser->append(QString::fromUtf8("返回命令：")+string);
            ui->textBrowser->append("      ");
            /*以下是对返回信息(JSON包)进行解析
            QScriptValue jsonAccess;
            QScriptEngine engineAccess;
            jsonAccess = engineAccess.evaluate("value = " + strJsonAccess);
            QScriptValueIterator iteratorAccess(jsonAccess);
            while (iteratorAccess.hasNext())
            {
                    iteratorAccess.next();
                  if(iteratorAccess.name()=="access_token")
                      API_access_token = iteratorAccess.value().toString();//得到 API_access_token,验证是百度用户
             }
            if(API_access_token=="")
            {
                QMessageBox::warning(this,"警告","access_token口令获取失败！");
                return;
            }*/
            flag=0;
            reply->deleteLater();//释放
    }
    else
    {
         //   QString strJsonText = reply->readAll();//对文本进行解析,,这个返回的JSON包所携带的是语音文本
           QByteArray strJsonText = reply->readAll();
            QString string = QString::fromUtf8(strJsonText);
            ui->textBrowser->append(QString::fromUtf8("返回语音：")+string);
            ui->textBrowser->append("      ");
           // QString strText="";
            QScriptValue jsontext;
            QScriptEngine engineText;
            jsontext = engineText.evaluate("value = " + strJsonText );
            QScriptValueIterator iteratorText(jsontext);
            while (iteratorText.hasNext())
            {
                   iteratorText.next();
                  if(iteratorText.name()=="result")
                  {
                      strText = iteratorText.value().toString();
                      break;
                  }
             }

            char* info =strText.toUtf8().data();
            QString URL ="http://www.tuling123.com/openapi/api";
            QString key ="dc6592cc8c414d64a60f2eb7994d5ef6";
            QUrl url(URL.toUtf8()+"?key="+key.toUtf8()+"&info="+QString::fromUtf8(info)+"&userid=123456789");
            QNetworkReply* reply = nam->post(QNetworkRequest(url), QByteArray(info));
            char* info1 =strText.toUtf8().data();
            ui->textBrowser->append(QString::fromUtf8("问：")+QString::fromUtf8(info1));
            ui->textBrowser->append("      \n");
            ui->lineEdit->clear();

    }

}

void MainWindow::hecheng_replyFinish(QNetworkReply *reply)
{
    QByteArray strJsonText = reply->readAll();
    QString string = QString::fromUtf8(strJsonText);
    ui->textBrowser->append(QString::fromUtf8("返回语音合成：")+string);
    ui->textBrowser->append("      ");
    putfile.close();
    if(putfile.open( QIODevice::WriteOnly | QIODevice::Truncate ))
    {
        qDebug("putfile is WriteOnly ok");
    }
    else
        qDebug("putfile is WriteOnly not ok");
    QMediaContent content;
    QDataStream out(&putfile);
    out<<strJsonText;
    out.device();
    putfile.close();
    if(putfile.open( QIODevice::ReadOnly))
    {
        qDebug("putfile read is ok");
    }
    else
        qDebug("putfile read is not ok");
    media->setMedia(content,out.device());
    media->setVolume(50);
    media->play();
}

void MainWindow::getText(QString para_API_id, QString para_API_access_token, QString para_API_language, QString para_API_record_path)
{
    QFile file(para_API_record_path);
        if( !(file.open(QIODevice::ReadOnly)))
        {
            QMessageBox::warning(this,"警告","打开语音文件失败！");
            return;
        }
        /*读入文件流*/
        QDataStream in(&file);
        m_buf = new char[file.size()];
        in.readRawData(m_buf,file.size());
        file.close();

        /*发送http请求,目的是得到语音文本*/
        QString  getTextUrl = "http://vop.baidu.com/server_api?lan=" + para_API_language + "&cuid=" + para_API_id + "&token=" + para_API_access_token;
        QUrl url;
        url.setUrl(getTextUrl);
        QNetworkRequest request(url);
        request.setHeader(QNetworkRequest::ContentTypeHeader, "audio/pcm;rate=8000");
        QByteArray arr = QByteArray(m_buf, file.size());
         manager->post(request,arr);
}

void MainWindow::stopRecording()
 {
   audio->stop();
   outputFile.close();
   //delete audio;
   qDebug("record end!");
 }

void MainWindow::on_pushButton_clicked()
{
    char* info =ui->lineEdit->text().toUtf8().data();
    QString URL ="http://www.tuling123.com/openapi/api";
    QString key ="dc6592cc8c414d64a60f2eb7994d5ef6";
    QUrl url(URL.toUtf8()+"?key="+key.toUtf8()+"&info="+QString::fromUtf8(info)+"&userid=123456789");
    QNetworkReply* reply = nam->post(QNetworkRequest(url), QByteArray(info));
    char* info1 =ui->lineEdit->text().toUtf8().data();
    ui->textBrowser->append(QString::fromUtf8("问：")+QString::fromUtf8(info1));
    ui->textBrowser->append("      ");
    ui->lineEdit->clear();
}

void MainWindow::finishedSlot(QNetworkReply *reply)
{
     QVariant statusCodeV =
     reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
     QVariant redirectionTargetUrl =
     reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
     if (reply->error() == QNetworkReply::NoError)
     {
         QByteArray bytes = reply->readAll();  // bytes

         QString text;
         QScriptValue jsontext;
         QScriptEngine engineText;
         jsontext = engineText.evaluate("value = " + bytes);
         QScriptValueIterator iteratorText(jsontext);
         while (iteratorText.hasNext())
         {
                iteratorText.next();
               if(iteratorText.name()=="text")
               {
                   text = iteratorText.value().toString();
                   ui->textBrowser->append(QString::fromUtf8("答：")+text);
                   QString texturl = "http://tsn.baidu.com/text2audio";
                   QString tex = text;
                   QString cuid = "54-04-A6-CE-2C-F6";
                   QByteArray aaa;

                   QUrl urll(texturl.toUtf8()+"?tex="+tex.toUtf8()+"&lan=zh&cuid="+cuid.toUtf8()+"&ctp=1&tok="+API_access_token+"&vol=9&pit=5&per=0&spd=4" );
                   QNetworkReply* reply = hecheng_manager->post(QNetworkRequest(urll), aaa);
                   break;
               }
          }


         //QString string = QString::fromUtf8(bytes);
         //ui->textBrowser->append(QString::fromUtf8("答：")+string);
         ui->textBrowser->append("      ");
     }
     else
     {
     }
     reply->deleteLater();
}

void MainWindow::on_pushButton_3_clicked()
{
    outputFile.close();
    if(outputFile.open( QIODevice::ReadOnly))
    {
        qDebug("outputFile read is ok");
    }
    else
        qDebug("outputFile read is not ok");
   //
    outaudio->start(&outputFile);
    qDebug("file read");
}

