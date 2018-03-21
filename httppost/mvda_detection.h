#ifndef MVDA_DETECTION_H
#define MVDA_DETECTION_H

#include <QObject>
#include <QFile>
#include <QAudio>
#include <QAudioInput>
#include <QAudioOutput>

#define FRAME_LEN 32

class mVDA_Detection : public QObject
{
    Q_OBJECT
public:
    explicit mVDA_Detection(QObject *parent = 0);
    void vdaStart();
    void vdaStop();

signals:
    void VDA_detected(short* outdata, int len);

private slots:
    void readData();

private:
    typedef struct
    {
        float M0;  //
       // float E0;
        int M0_count,frame;
        unsigned char mm;
    }VDA_conf;
    VDA_conf vda;
    QFile outputFile;   // class member.
    QIODevice *m_input;
    QFile putfile;
    QAudioInput* audio; // class member.
    QAudioOutput* outaudio;
    QAudioFormat format,outformat;
    QAudioDeviceInfo info;

    int voicebuff[65536];
    short VDA_voicebuff[65536];
    int k,k1;

    inline void VDA_init(){vda.M0 = 0;vda.frame = 0;vda.M0_count = 0;vda.mm = 0;}
    void Endpoint_detect(float *data, int len);
    char VDA(float indata[],int frame_len, int E0t, int M0t,
             float outdata[], int* outdata_length);
};

#endif // MVDA_DETECTION_H
