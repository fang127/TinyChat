#include "LoadDialog.h"
#include "ui_LoadDialog.h"

#include <QMovie>

LoadDialog::LoadDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoadDialog)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(parent->size());

    QMovie *movie = new QMovie(":/res/loading.gif");
    ui->loadingLB->setMovie(movie);
    movie->start();
}

LoadDialog::~LoadDialog()
{
    delete ui;
}
