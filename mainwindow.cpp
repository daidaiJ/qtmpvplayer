#include "mainwindow.h"
#include <QShortcut>
#include <QInputDialog>
#include <QSettings>
#include "ListItemWithIdx.h"
#include "ui_mainwindow.h"
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    //setWindowState(Qt::WindowMaximized);
    ui->setupUi(this);
    mplayer = new MPlayer(ui->mpv);
    mplayer->setPauseSyncCallBack([this](bool flag){this->setPause(flag);});
    mplayer->setSubSyncCB([this](const std::string& str){this->addSubIdx(str);});
    is_paused = true;
    configUI();
    load_config();
}

MainWindow::~MainWindow()
{
    save_config();
    if(mplayer){
        delete mplayer;
    }
    if(animat){
        delete animat;
    }
    delete ui;
}
void MainWindow::setPause(bool flag){
    is_paused = flag;
    if(is_paused){
        ui->pause->setIcon(this->pause);
    }else{
        ui->pause->setIcon(this->start);
    }
}
void MainWindow::configUI(){
    ui->soundslide->setMaximum(100);
    //ui->toolbar->raise();
    ui->toolbar->setParent(ui->player);
    ui->soundslide->setMinimum(0);
    ui->videolist->setStyleSheet("QPushButton{text-align : left;margin:1px;}");
    ui->speed->setCurrentText("1.0");
    ui->soundslide->setSliderPosition(100);
    ui->videolist->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->videolist->setSpacing(0);
    ui->tabWidget->setCurrentIndex(0);
    ui->videolist->setStyleSheet("QListView {outline: 0;}");
    QSizePolicy sp =ui->toolbar->sizePolicy();
    sp.setRetainSizeWhenHidden(false);
    this->start = QIcon(":/new/prefix1/active.svg");
    this->pause = QIcon(":/new/prefix1/hold.svg");
    QShortcut * esc = new QShortcut(QKeySequence(Qt::Key_Escape),this);
    QShortcut * space = new QShortcut(QKeySequence(Qt::Key_Space),this);
    connect(esc,SIGNAL(activated()), this,SLOT(on_trigge_fullscreen()));
    connect(space,SIGNAL(activated()), this,SLOT(on_pause_clicked()));
    connect(ui->openvideo,SIGNAL(triggered()),this,SLOT(on_input_url()));
    connect(mplayer,&MPlayer::end,this,&MainWindow::on_end_slot);
    connect(ui->videolist,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(on_videolist_customContextMenu(QPoint)));
    connect(ui->opendir,&QAction::triggered,this,&MainWindow::on_videodir_clicked);
    connect(ui->actionspause,&QAction::toggled,this,&MainWindow::changePause);
    connect(ui->actionautoresume,&QAction::toggled,this,&MainWindow::changeResume);
    connect(ui->soundslide,&QSlider::valueChanged,mplayer,&MPlayer::mute);
    connect(ui->sublist,&QListWidget::itemDoubleClicked,this,&MainWindow::skipToSub);
    // connect(ui->openvideo,&QAction::triggered,this,&MPlayer::)

}

// void MainWindow::resizeEvent(QResizeEvent* ev){
//     QWidget::resizeEvent(ev);
// }
void MainWindow::on_end_slot(){
    if(this->playstat>0)
    QMessageBox::information(this,url,tr("播放结束"),QMessageBox::Abort);

}
void MainWindow::hiddentoolbar(){
    ui->toolbar->setHidden(is_fullscreen);
}


void MainWindow::changeResume(bool checked){

    if(mplayer){
        mplayer->resume = checked;

    }
}
void MainWindow::changePause(bool checked){
    if(mplayer){
        mplayer->sub_pause = checked;
    }
}
void MainWindow::on_trigge_fullscreen(){
    if(!this->animat){
        this->animat = new QPropertyAnimation(ui->toolbar,"geometry");
    }

    animat->setStartValue(QRect(ui->toolbar->x(),ui->toolbar->y(),ui->toolbar->width(),ui->toolbar->height()));
     connect(animat,SIGNAL(finished()),this,SLOT(hiddentoolbar()));
    if(is_fullscreen){


        animat->setEndValue(QRect(ui->toolbar->x(),ui->toolbar->y()-ui->toolbar->height()-10,ui->toolbar->width(),ui->toolbar->height()));


    }else{
        // ui->toolbar->setHidden(false);
         animat->setEndValue(QRect(ui->toolbar->x(),ui->toolbar->y()+ui->toolbar->height()+10,ui->toolbar->width(),ui->toolbar->height()));

    }
    animat->setEasingCurve(QEasingCurve::Linear);
    animat->setDuration(1200);
    animat->start();
    is_fullscreen = !is_fullscreen;


}

void MainWindow::on_videolist_customContextMenu(const QPoint& pos){
    if(ui->videolist->selectedItems().count()==0){
        return;
    }
    QMenu * menu = new QMenu(ui->videolist);
    QAction* act = new QAction(menu);
    act->setText(tr("打开视频"));
    connect(act,SIGNAL(triggered()),this,SLOT(on_videoselect_clicked()));
    menu->addAction(act);
    menu->exec(QCursor::pos());
    delete menu;
}

void MainWindow::changeDir(const QDir& dir){
    if(!dir.exists()){
        qDebug()<<"目录不存在";
    }
    if(ui->videolist->count()>0){
        ui->videolist->clear();
    }
    for(auto  &file:dir.entryInfoList()){
        if(vfmtSet.count(file.suffix())!=0){
            auto l = new QListWidgetItem;
            l->setText(file.fileName());
            // l->connect(l.)
            ui->videolist->addItem(l);
        }
    }

    this->vdir = dir;
}


void MainWindow::on_videodir_clicked(){
    QDir defaultarr []={QDir("c:/video"),QDir("d:/video")};
    QString defaultpath ="";
    for(auto & dir:defaultarr){
        if(dir.exists()){
            defaultpath = dir.absolutePath();
        }
    }
    vdir = QFileDialog::getExistingDirectory(this,"选择一个视频目录",defaultpath,QFileDialog::ShowDirsOnly);
    qDebug()<<"find video dir"<<vdir;
    auto tdir = QDir(vdir);
    if(tdir.exists()){
        this->vdir = tdir;
        changeDir(this->vdir);
    }

}

void MainWindow::on_videoselect_clicked(){
    QListWidget * target = ui->videolist;
    auto itemText = target->selectedItems()[0]->text();
    changeVideo(itemText);
    qDebug()<<__LINE__<<"path"<<itemText;
}

void MainWindow::changeVideo(const QString& url){
    if(url.size()>0){
        if(!vdir.isEmpty()){
            this->url = this->vdir.absoluteFilePath(url);
        }
        auto ret =mplayer->play(this->url);

        if(ret==0){
            this->playstat++;
            is_paused =false;
            sublist.clear();
            ui->sublist->clear();
            cursubIdx =-1;
            auto lastdot = this->url.lastIndexOf(".",0);
            auto filename = this->url.mid(0,lastdot);

            for(auto &suf:subsuffix){
                auto tmpfile = QFile(filename+suf);
                if(tmpfile.exists()){
                    on_open_subtxt(tmpfile,suf);
                    break;
                }


            }
        }else{
            QMessageBox::warning(this,this->url,tr("播放错误"),QMessageBox::Abort);
        }
        qDebug()<<__LINE__<<"video url"<<this->url;

    }

}

void MainWindow::on_open_subtxt(QFile& file,QString &suf){
    qDebug()<<__LINE__<<"字幕文件"<<file.fileName()<<"格式"<<suf;
}

void MainWindow::on_last_clicked()
{
     mplayer->seek("-1");
    if(cursubIdx>0){
         cursubIdx -=1;
    }

}




void MainWindow::on_pause_clicked()
{

    mplayer->pause(!is_paused);
    is_paused =!is_paused;
    if(is_paused){
        ui->pause->setIcon(this->pause);
    }else{
        ui->pause->setIcon(this->start);
    }

}


void MainWindow::on_next_clicked()
{
    mplayer->seek("1");
    cursubIdx +=1;
}


void MainWindow::on_input_url(){
    bool bok=false;
    QString url = QInputDialog::getText(this,"video source",tr("请输入视频来源url"),QLineEdit::Normal,"",&bok);
    if(bok&&!url.isEmpty()){

        this->changeVideo(url);
    }
}

void MainWindow::save_config(){
    if(mplayer){
        QSettings *set = new QSettings("config.ini",QSettings::IniFormat);
        set->setValue("player/subpause",mplayer->sub_pause);
        set->setValue("player/autoresume",mplayer->resume);
        delete set;
    }
}

void MainWindow::load_config(){

    QSettings *set = new QSettings("config.ini",QSettings::IniFormat);
    auto subpause = set->value("player/subpause").toString();
    auto autoresume = set->value("player/autoresume").toString();
    if(subpause=="true"||subpause==""){
        ui->actionspause->setChecked(true);
        mplayer->sub_pause = true;
    }
    if(autoresume=="true"){
        ui->actionautoresume->setChecked(true);
        mplayer->resume = true;
    }

    delete set;
}

void MainWindow::on_speed_currentTextChanged(const QString &arg1)
{
    if(mplayer&&mplayer->getOSpeed()>0.1){

        auto targetVal =QString("%1").arg(arg1.toDouble()*mplayer->getOSpeed());
        mplayer->changeSpeed(targetVal);

    }
}


void MainWindow::addSubIdx(const std::string& subline){
    qDebug()<<"字幕同步"<<subline;
    if(sublist.find(subline)==sublist.end()){
        sublist.emplace(subline,subCNT++);
        cursubIdx +=1;
        auto l = new ListItemWithIdx(subCNT-1,QString(subline.c_str()));
        //connect(l,&ListItemWithIdx::clickListItem,this,&MainWindow::skipToSub);
        ui->sublist->addItem(l);
    }
}

void MainWindow::skipToSub(QListWidgetItem* item){
    ListItemWithIdx * tmp = static_cast<ListItemWithIdx*>(item);
    qDebug()<<"重定位到"<<tmp->idx;
    auto delta = tmp->idx-cursubIdx;
    if(delta!=0&&mplayer){
        auto val = std::to_string(delta);
        mplayer->pause(true);
        mplayer->seekSub(val);
        cursubIdx = tmp->idx;
        mplayer->pause(false);
    }

}
