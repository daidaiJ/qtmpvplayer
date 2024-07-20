#ifndef LISTITEMWITHIDX_H
#define LISTITEMWITHIDX_H
#include <QObject>
#include <QListWidgetItem>
#include <QMouseEvent>
class ListItemWithIdx:public QObject,public QListWidgetItem{
    Q_OBJECT
public:
    int idx;
    ListItemWithIdx(int idx,const QString& str):idx(idx){
        setText(str);

    }
    int Index()const{
        return idx;
    }
};
#endif // LISTITEMWITHIDX_H
