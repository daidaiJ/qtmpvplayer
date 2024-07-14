#include <mpvplayer.h>
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_INFO
void MPlayer::create_player(QWidget* win){
    mpv = mpv_create();
    if(!mpv){
        qDebug()<<tr("cant create mpv instance");
        throw std::runtime_error(tr("cant create mpv instance").toStdString());
    }
    int64_t wid = win->winId();
    mpv_set_option(mpv,"wid",MPV_FORMAT_INT64,&wid);
    mpv_set_option_string(mpv, "input-default-bindings", "no");//设置默认输入键绑定
    mpv_set_option_string(mpv, "input-vo-keyboard", "no");//设置默认键盘输入键绑定
    mpv_set_option_string(mpv, "ontop", "no");//设置默认键盘输入键绑定
    mpv_set_option_string(mpv, "hwdec", "auto");// 设置硬件加速
    mpv_set_option_string(mpv, "sub-auto", "exact"); // 自动加载视频同名字幕
    mpv_set_option_string(mpv, "sub-use-margins", "yes"); // 将字幕渲染到下部黑边上
    mpv_set_option_string(mpv, "sub-font", "sans-serif"); // 自动加载视频同名字幕
    mpv_set_option_string(mpv, "sub-back-color", "#00000000"); // 将黑边上的字幕颜色设置为白色
    mpv_set_option_string(mpv, "audio-pitch-correction", "yes"); // 修复音频加速带来的变调
    mpv_set_option_string(mpv, "cache", "auto"); // 启用网络缓存
    mpv_observe_property(mpv, 0, "time-pos", MPV_FORMAT_DOUBLE);
    mpv_observe_property(mpv,0,"sub-end",MPV_FORMAT_DOUBLE);
    connect(this, &MPlayer::mpv_events, this, &MPlayer::on_mpv_callback,
            Qt::QueuedConnection);
    mpv_set_wakeup_callback(mpv, wakeup, this);
    if (mpv_initialize(mpv) < 0)
        throw std::runtime_error("mpv failed to initialize");


}

MPlayer::MPlayer(QWidget* win){
    create_player(win);
    spdlog::set_pattern("**%v");
    this->log = spdlog::basic_logger_mt<spdlog::async_factory>("async_file_logger", "logs/sublog.txt");
}

MPlayer::~MPlayer(){
    if (mpv)
        mpv_terminate_destroy(mpv);
}

void MPlayer::setPpt(const QString &name, const QString &value)
{
    mpv_set_option_string(mpv, name.toLatin1().data(), value.toLatin1().data());
}

QString MPlayer::getPpt(const QString &name) const
{
    return (QString)mpv_get_property_string(mpv, name.toLatin1().data());
}


 void wakeup(void *ctx)
{
    // 此回调可从任何mpv线程调用（但也可以从调用mpv API的线程递归地返回）
    // 只是需要通知要唤醒的Qt GUI线程（以便它可以使用mpv_wait_event（）），并尽快返回
    MPlayer *mvpPlayer = (MPlayer *)ctx;
    emit mvpPlayer->mpv_events();
}

void MPlayer::on_mpv_callback(){
    while(mpv){
        mpv_event * event = mpv_wait_event(mpv,0);
        if(event->event_id==MPV_EVENT_NONE){
            break;
        }
        handle_mpv_event(event);
    }
}

void MPlayer::seek(const std::string& val){
    if(this->last_subtime>0.1){
        return seekSub(val);
    }
    if(mpv){
        const char* args[]={"seek",val.data(),NULL};
        mpv_command_async(mpv,0,args);
    }
}

void MPlayer::seekSub(const std::string& val){
    if(mpv){
        const char* args[]={"sub-seek",val.data(),NULL};
        mpv_command_async(mpv,0,args);
    }
}

int MPlayer::play(const QString& url){
    if(mpv){
        const char* args[]={"loadfile",url.toUtf8().data(),NULL};
        sid =0;
        mpv_get_property(mpv,"speed",MPV_FORMAT_DOUBLE,&this->vspeed);
        log->info("videoname : {}",url.split("/\\",Qt::SplitBehaviorFlags::SkipEmptyParts).last().toStdString());
       return mpv_command_async(mpv,0,args);
    }
    return 1;
}

void MPlayer::pause(bool flag){

    mpv_set_option_string(mpv,"pause", flag?"yes":"no");
}

void MPlayer::mute(int num){
    double vlome = num;
    mpv_set_property_async(mpv, 0, "volume", MPV_FORMAT_DOUBLE, &vlome);
}

void MPlayer::changeSpeed(const QString& val){
    if(mpv){
        const char* args[]={"no-osd","multiply","speed",val.toUtf8().data(),NULL};
         mpv_command_async(mpv,0,args);
    }
}

void MPlayer::sleep(int msec){
    QTime dieTime = QTime::currentTime().addMSecs(msec);
    while( QTime::currentTime() < dieTime )
        // 让出20ms
        QCoreApplication::processEvents(QEventLoop::AllEvents, 20);
}
void MPlayer::handle_mpv_event(mpv_event* ev){
    switch (ev->event_id)
    {
    // 属性改变事件发生
    case MPV_EVENT_PROPERTY_CHANGE:
    {
        mpv_event_property *prop = (mpv_event_property *)ev->data;

        if (strcmp(prop->name, "time-pos") == 0)
        {
            if (prop->format == MPV_FORMAT_DOUBLE)
            {
                this->last_ts =std::max(this->last_ts,*(double*)prop->data);
            }
            else if (prop->format == MPV_FORMAT_NONE)
            {
                // 该属性不可用，可能意味着播放已停止
                emit end();
                qDebug() << "播放结束";
            }
        }else if(strcmp(prop->name, "sub-end") == 0){
            if (prop->format == MPV_FORMAT_DOUBLE&&this->sid!=-1)
            {
                // 获得播放时间
                //double time = *(double *)prop->data;
                //qDebug() << "播放时间: " << QString::number(time, 10, 2);
                // if(mpv_get_property_async(mpv,0,"sid",MPV_FORMAT_INT64)<0){
                //     sid =-1;
                //     return;
                // }
                auto cts =*(double*)prop->data;

                this->pause(true);
                //延迟 0.5s 后取消暂停


                    auto str = mpv_get_property_string(mpv,"sub-text");
                    if(history!=str){
                        qDebug()<<cts<<str;
                        log->info("sub-text : {}",str);
                        history =str;
                    }

                this->sleep(1000);
                 this->last_subtime = cts;

                this->pause(false);
            }

        }else if (prop->format == MPV_FORMAT_NONE)
        {
            // 该属性不可用，可能意味着播放已停止
            int64_t ret;
            // 加个剩余时间检测，以防误报视频结束
            mpv_get_property(mpv,"time-remaining",MPV_FORMAT_INT64,&ret);
            if(ret>1){
                return;
            }
            emit end();
            qDebug() << "播放结束";
        }

    }
    break;

        // palyer退出事件发生
    case MPV_EVENT_SHUTDOWN:
    {
        mpv_terminate_destroy(mpv);
        mpv = NULL;
    }
    break;

    default: ;
    }
}
