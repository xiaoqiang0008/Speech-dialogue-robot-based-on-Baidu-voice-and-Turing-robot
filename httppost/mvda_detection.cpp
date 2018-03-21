#include "mvda_detection.h"

mVDA_Detection::mVDA_Detection(QObject *parent) : QObject(parent)
{
    k=0;k1=0;
    outputFile.setFileName("b1.pcm");
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
    outformat.setCodec("audio/pcm");
    outformat.setByteOrder(QAudioFormat::LittleEndian);
    outformat.setSampleType(QAudioFormat::SignedInt);
    info = QAudioDeviceInfo::defaultInputDevice();
    if (!info.isFormatSupported(outformat)) {
        qDebug("default format not supported try to use nearest");
       outformat = info.nearestFormat(outformat);
    }
    outaudio = new QAudioOutput(outformat, this);
    VDA_init();
}

void mVDA_Detection::vdaStart()
{
    m_input = audio->start();
    connect(m_input, SIGNAL(readyRead()), this, SLOT(readData()));
}

void mVDA_Detection::vdaStop()
{
    audio->stop();
}

void mVDA_Detection::readData()
{
    int i=0,j=0,temp=0,k=0;
    float indata[65536],data[65536];
    QByteArray m_buff = m_input->readAll();
    for(i=0;i<m_buff.length();i+=2)
    {
        indata[k]=0; temp=0;
        memcpy(&temp,m_buff.data()+i,2);
        indata[k] = (float)temp;
        if(indata[k]>65535/2)
            indata[k]=indata[k]-65536;
        k++;
    }
    for(j=0;j<k/FRAME_LEN;j++)
    {
        for(i=0;i<FRAME_LEN;i++)
        {
            data[i] = indata[i+j*FRAME_LEN];
        }
        Endpoint_detect(data,FRAME_LEN);

    }
}

void mVDA_Detection::Endpoint_detect(float* data,int len)
{
    int j=0;
    float outdata[65536];
    int outdata_length=0;
    static char vda=0,frist_point=0,end_point=0;
    static char end_count=0,frist_count=0;

    vda=VDA(data,len,100,70,outdata,&outdata_length);   //端点检测
    if(vda == 1) qDebug("vda=%d",vda);
    else if(vda == 0) qDebug("vda=%d",vda);
    if(vda == 0 && frist_point == 0 && end_point == 0)
    {
        frist_count=0;
        k1=0;
    }
    if(vda == 1 && frist_point == 1)
    {
        end_count=0;
    }
    if(vda == 1 && frist_point == 0) //语音前段点检测
    {
        frist_count++;
        if(frist_count >1)
        {
            frist_count=0;
            frist_point=1;
            end_point = 0;
            qDebug("检测到端点，开始录音");
        }
        for(j=0;j<outdata_length;j++)
        {
            VDA_voicebuff[k1] = outdata[j];
            if(VDA_voicebuff[k1]>65535/2)
                VDA_voicebuff[k1]=VDA_voicebuff[k1]-65536;
            k1++;
        }
     }
     else if(frist_point == 1)   //开始录音
     {
         for(j=0;j<outdata_length;j++)
         {
             VDA_voicebuff[k1] = outdata[j];
             if(VDA_voicebuff[k1]>65535/2)
                 VDA_voicebuff[k1]=VDA_voicebuff[k1]-65536;
             k1++;
         }
     }

    if(vda == 0 && frist_point == 1)   //后端点检测
    {
        end_count++;
      /*  for(j=0;j<outdata_length;j++)
        {
            VDA_voicebuff[k1] = outdata[j];
            if(VDA_voicebuff[k1]>65535/2)
                VDA_voicebuff[k1]=VDA_voicebuff[k1]-65536;
            k1++;
        }*/
        if(end_count > 2)
        {
            frist_point = 0;
            end_point = 1;
            end_count = 0;
            qDebug("检测到后端点，结束录音");
           // ui->widget->update();
        }
    }
    if(vda == 0 && end_point == 1)    //录音结束
    {
        frist_point = 0;
        end_point = 0;
        //k1=0;
        qDebug("一段语音处理结束");
        emit VDA_detected(VDA_voicebuff,k1);
    }
}

char mVDA_Detection::VDA(float indata[],int frame_len, int E0t, int M0t,
         float outdata[], int* outdata_length)
{
    static float E0=0;
    int j=0,m=0;
    vda.frame++;
    if(vda.frame < E0t)
    {
        for(j=0;j<frame_len;j++)
        {
            //端点检测能量值求取
            E0 = E0+indata[j]*indata[j];

        }
        m=-1;
    }
    else if(vda.frame >= E0t && vda.mm == 0)
    {
        E0 = E0/(frame_len*E0t);   //求取背景噪声的阈值
        qDebug("读到数据监测 %2.3f",E0);
        vda.mm=1;
        m=-1;
    }
    else
    {
        vda.frame=E0t+10;
        for(j=0;j<frame_len;j++)
        {
            vda.M0 += indata[j]*indata[j];
            outdata[j+vda.M0_count*frame_len] = indata[j];
        }
        vda.M0_count++;
        if(vda.M0_count == M0t)
        {
            vda.M0 = vda.M0/((vda.M0_count+1)*frame_len);
            if(vda.M0 > E0*120)
            {
                m=1;
                *outdata_length = M0t*frame_len;
            }
            else
            {
                m=0;
            }
            vda.M0=0;
            vda.M0_count = 0;
        }
        else m=-2;
    }
    return (char)m;
}
