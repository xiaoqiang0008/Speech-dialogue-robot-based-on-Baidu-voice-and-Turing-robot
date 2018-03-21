#include "mvoice_recognition.h"

mVoice_Recognition::mVoice_Recognition(QObject *parent) : QObject(parent)
{
    API_id = "54-04-A6-CE-2C-F6";
    API_access_token="24.a3721b522758ddf7c8efd0a8a53fe0ae.2592000.1523625249.282335-9600066";
    API_language="zh";
    flag=1;

    manager = new QNetworkAccessManager(this);
    connect(manager,SIGNAL(finished(QNetworkReply*)),
            this,SLOT(slot_replyFinish(QNetworkReply*)));
}

void mVoice_Recognition::sendvoice(QByteArray arr)
{
    QString  getTextUrl = "http://vop.baidu.com/server_api?lan=" + API_language +
            "&cuid=" + API_id + "&token=" + API_access_token;
    QUrl url;
    url.setUrl(getTextUrl);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "audio/pcm;rate=8000");
    manager->post(request,arr);
   // qDebug("已发送");
}

void mVoice_Recognition::slot_replyFinish(QNetworkReply *reply)
{
    QString strText;
    if(flag==1)
    {
        QByteArray strJsonAccess=reply->readAll();//得到http返回的所有信息,这个返回的JSON包所携带的是API_access_token口令
        QString string = QString::fromUtf8(strJsonAccess);
       // ui->textBrowser->append(QString::fromUtf8("返回命令：")+string);
       // ui->textBrowser->append("      ");
        qDebug("%s",string);
        flag=0;
        reply->deleteLater();//释放
    }
    else
    {
        QByteArray strJsonText = reply->readAll();
       // QString string = QString::fromUtf8(strJsonText);
       // ui->textBrowser->append(QString::fromUtf8("返回语音：")+string);
       // ui->textBrowser->append("      ");
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
                  emit mVoice_Rec_Received(strText);
                  break;
              }
         }
    }
}
