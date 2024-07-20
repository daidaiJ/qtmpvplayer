#ifndef MPVPLAYER_H
#define MPVPLAYER_H
#include "mpvclient.h"
#include <QWidget>
#include <QObject>
#include <QDebug>
#include <QTime>
#include <QCoreApplication>
#include "spdlog.h"
#include "async.h"
#include "sinks/basic_file_sink.h"
void wakeup(void *ctx);
class MPlayer: public QWidget{
    Q_OBJECT
public:
    MPlayer(QWidget* pa);
    virtual ~MPlayer();
    void create_player(QWidget* win);
    void setPpt(const QString &name,const QString& value);
    void seek(const std::string&);
    void seekSub(const std::string&);
    void pause(bool flag);
    void mute(int num);
    QString getPpt(const QString &name)const;
    int play(const QString& url);
    void changeSpeed(const QString& val);
    double getOSpeed()const {return vspeed;}
    void setPauseSyncCallBack(std::function<void(bool)> _cb);
    void setSubSyncCB(std::function<void(const std::string&)> _cb);
    bool sub_pause = true;
    bool resume = false;

private slots:
    void on_mpv_callback();

signals:
    void mpv_events();
    void end();
private:
    mpv_handle * mpv=nullptr;
    std::function<void(bool)>  cb=nullptr;
    std::function<void(const std::string&)>  subsync=nullptr;
    void handle_mpv_event(mpv_event* event);
    void sleep(int msec);
    int sid=0;
    double vspeed=0.0;
    double last_ts =0.0;
    double last_subtime =0.0;
    std::string history="";
    std::shared_ptr<spdlog::logger> log;
};

#endif // MPVPLAYER_H
