#ifndef MTABLEVIEW_H
#define MTABLEVIEW_H

#include <QWidget>
#include <QBrush>
#include <QVariant>
#include <QStandardItemModel>

namespace Ui
{
class MTableView;
}

class MTableView : public QWidget
{
    Q_OBJECT

public:
    explicit MTableView(QString  des,
                        int      row_cnt,
                        int      cloumn_cnt,
                        QString  row_header,
                        QString  cloumn_header,
                        QWidget *parent = nullptr);
    ~MTableView();

public slots:
    void slot_update(int index, QVariant value);

private:
    int  index2row(int index);
    int  index2cloumn(int index);
    void set_item_backcolor(int index, QBrush brush);
    void set_item_value(int index, QVariant value);

private:
    Ui::MTableView *ui;

    QStandardItemModel *data_model = nullptr;

    int      row_cnt    = 0;
    int      cloumn_cnt = 0;
    QVariant max        = 0;
    QVariant min        = 0;
    int      max_index  = 0;
    int      min_index  = 0;
    QBrush   normal_brush;
    QBrush   max_brush;
    QBrush   min_brush;
};

#endif // MTABLEVIEW_H
