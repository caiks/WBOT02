#ifndef WIN001_H
#define WIN001_H

#include <QWidget>

namespace Ui {
class Win001;
}

class Win001 : public QWidget
{
    Q_OBJECT

public:
    explicit Win001(QWidget *parent = 0);
    ~Win001();

private:
    Ui::Win001 *ui;
};

#endif // WIN001_H
