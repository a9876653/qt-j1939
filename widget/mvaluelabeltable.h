#ifndef MVALUELABELTABLE_H
#define MVALUELABELTABLE_H

#include <QWidget>
#include <QBrush>
#include <QVariant>
#include <QTimer>
#include "mvaluelabel.h"

namespace Ui
{
class MValueLabelTable;
}

class MValueLabelTable : public QWidget
{
    Q_OBJECT

public:
    explicit MValueLabelTable(QString  des,
                              int      row_cnt,
                              int      cloumn_cnt,
                              QString  row_header,
                              QString  cloumn_header,
                              QWidget *parent = nullptr);
    ~MValueLabelTable();

    void insert(int row, int column, MValueLabel *label);

    int get_row_cnt();

public slots:
    void slot_update(QVariant value);

private slots:
    void slot_update_max_min();

protected:
    void showEvent(QShowEvent *event);
    void hideEvent(QHideEvent *event);

private:
    Ui::MValueLabelTable *ui;
    int                   row_cnt    = 0;
    int                   cloumn_cnt = 0;
    QVariant              max        = 0;
    QVariant              min        = 999999;
    int                   max_index  = 0;
    int                   min_index  = 0;
    QBrush                normal_brush;
    QBrush                max_brush;
    QBrush                min_brush;

    MValueLabel *max_item = nullptr;
    MValueLabel *min_item = nullptr;

    QTimer update_timer;
};

#endif // MVALUELABELTABLE_H
