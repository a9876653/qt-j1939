#ifndef FRMCENTERICON_H
#define FRMCENTERICON_H

#include <QWidget>

namespace Ui {
class frmCenterIcon;
}

class frmCenterIcon : public QWidget
{
  Q_OBJECT

public:
  explicit frmCenterIcon(QPixmap icon, QWidget *parent = nullptr);
  ~frmCenterIcon();
  void set_icon(QPixmap icon, QString str = "");
  void resize_icon();
private:
  Ui::frmCenterIcon *ui;
  QPixmap pixmap;
};

#endif // FRMCENTERICON_H
