#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QFileDialog>
#include <QMessageBox>
#include <unordered_set>
#include "mpvplayer.h"
QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

static std::unordered_set<QString> vfmtSet={"mkv","ts","mpeg","avi","mp4"};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pause_clicked();
    void on_last_clicked();
    void on_next_clicked();
    void on_end_slot();
    void on_videodir_clicked();
    void on_input_url();
    void on_videoselect_clicked();
    void on_speed_currentTextChanged(const QString &arg1);
    void on_videolist_customContextMenu(const QPoint& pos);
    void on_trigge_fullscreen();
private:
    void changeDir(const QDir& dir);

    void changeVideo(const QString& url);
    // void resizeEvent(QResizeEvent* ev) override;
    Ui::MainWindow *ui;
    bool is_paused=false;
    bool is_fullscreen=false;
    int tx=0,ty=0;
    QString url ="";
    QDir vdir ;
    MPlayer * mplayer=nullptr;
    int playstat=0;
    QIcon start,pause;
    void configUI();
};
#endif // MAINWINDOW_H
