#ifndef MCHECKBOX_H
#define MCHECKBOX_H

#include <QWidget>

namespace Ui
{
class MCheckBox;
}

class MCheckBox : public QWidget
{
    Q_OBJECT

public:
    explicit MCheckBox(QPixmap icon = QPixmap(), QWidget *parent = nullptr);
    ~MCheckBox();
    void set_icon(QPixmap icon, QString str = "");
    void resize_icon();

    bool    isChecked();
    void    setText(QString &text);
    void    setChecked(bool state);
    QString text();

private:
    Ui::MCheckBox *ui;
    QPixmap        pixmap;
};

#endif // MCHECKBOX_H
