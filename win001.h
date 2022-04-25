#ifndef WIN001_H
#define WIN001_H

#include "dev.h"
#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class Win001; }
QT_END_NAMESPACE

class Win001 : public QWidget
{
    Q_OBJECT

public:
    Win001(QWidget *parent = nullptr);
    ~Win001();

public Q_SLOTS:
    void capture(int id, const QImage &preview);

public:
    QImage image;

private:
    Ui::Win001 *ui;
};
#endif // WIN001_H
