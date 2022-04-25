#include "win001.h"
#include "./ui_win001.h"

Win001::Win001(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Win001)
{
    ui->setupUi(this);
}

Win001::~Win001()
{
    delete ui;
}

void Win001::capture(int id, const QImage &preview)
{
    STARTT;
    image = preview;
    EVAL(image.format());
    EVAL(image.depth());
    EVAL(image.width());
    EVAL(image.height());
    EVAL(image.dotsPerMeterX());
    EVAL(image.dotsPerMeterY());
    ECHO(auto colour = image.pixel(QPoint(0,0)));
    EVALH(colour);
    EVAL(qAlpha(colour));
    EVAL(qRed(colour));
    EVAL(qGreen(colour));
    EVAL(qBlue(colour));
    EVAL(qGray(colour));
    EVAL((qRed(colour)+qGreen(colour)+qBlue(colour))/3);
    {
        mark = Clock::now();
        std::size_t total = 0;
        std::size_t size = image.sizeInBytes()/4;
        auto rgb = (QRgb*)image.constBits();
        for (std::size_t x = 0; x < size; x++)
        {
            total += qRed(*rgb) + qGreen(*rgb) + qBlue(*rgb);
            rgb++;
        }
        EVAL(total);
        total /= 3;
        total /= size;
        std::cout << "average per pixel:" << total << "\t" << ((Sec)(Clock::now() - mark)).count() << "s" << std::endl;
    }
}

