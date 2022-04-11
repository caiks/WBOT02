#include "win001.h"
#include "./ui_win001.h"

Win001::Win001(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Win001)
{
    ui->setupUi(this);
}

Win001::~Win001()
{
    delete ui;
}

