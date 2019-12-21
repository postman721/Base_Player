#include "mainwindow.h"
#include "ui_mainwindow.h"
int i;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    player= new QMediaPlayer(this);
    playlist= new QMediaPlaylist(player);
    slider= new QSlider(this);

    slider->setOrientation(Qt::Horizontal);

    ui->statusBar->addPermanentWidget(slider);

    connect(player, &QMediaPlayer::durationChanged, slider, &QSlider::setMaximum);
    connect(player, &QMediaPlayer::positionChanged, slider, &QSlider::setValue);
    connect(slider, &QSlider::sliderMoved, player, &QMediaPlayer::setPosition);
    ui->play->setEnabled(false);
    ui->remove->setEnabled(false);
    ui->statusBar->setEnabled(false);

    system("pactl -- set-sink-volume 0  35%");

    system("pactl -- set-sink-volume 1  35%");

    system("pactl -- set-sink-volume 2  35%");

    system("pactl -- set-sink-volume 3  35%");

    system("pactl -- set-sink-volume 4  35%");

}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_play_clicked()

{
    QString current = ui->listWidget->currentItem()->text();
    cout << current.toStdString() << endl;
    QFileInfo fileInfo(current);
    QString info(fileInfo.fileName());
    string show = info.toStdString();
    cout << show  << endl;
    ui->statusBar->showMessage(info);
    player->play();

}

//Playlist mode
void MainWindow::on_pushButton_clicked()
{
    ui->statusBar->showMessage("Playlist mode.");
    player->setPlaylist(playlist);
    player->play();
}


void MainWindow::on_pause_clicked()
{
    player->pause();
    ui->statusBar->showMessage("Paused.");
}

void MainWindow::on_stop_clicked()
{
    player->stop();
    ui->statusBar->showMessage("Stopped.");
}
void MainWindow::on_add_clicked()
{
        ui->statusBar->showMessage("Select files.");
        QStringList filename = QFileDialog::getOpenFileNames(this, "Select music files(*.*)");
        for(const QString & filename: filename){
            playlist->addMedia(QMediaContent(QUrl::fromLocalFile(filename)));
        }
        ui->listWidget->addItems(filename);
        int count= ui->listWidget->count();
        cout << count << endl;
        ui->remove->setEnabled(true);

        QFileInfo fileInfo(filename.at(0));
        QString info(fileInfo.fileName());
        string show = info.toStdString();
        cout << show  << endl;
        ui->statusBar->showMessage(info);
        ui->listWidget->setCurrentRow(0);
        ui->play->setEnabled(true);
        ui->remove->setEnabled(true);
        ui->statusBar->setEnabled(true);

        on_pushButton_clicked();
}


void MainWindow::on_listWidget_itemClicked()
{
   QString current = ui->listWidget->currentItem()->text();
   cout << current.toStdString() << endl;
   player->setMedia(QUrl::fromLocalFile(current));

   QFileInfo fileInfo(current);
   QString info(fileInfo.fileName());
   string show = info.toStdString();
   cout << show  << endl;
   ui->statusBar->showMessage(info);
}


void MainWindow::on_remove_clicked()
{
    QString current = ui->listWidget->currentItem()->text();
    cout << current.toStdString() << endl;
    int removing = ui->listWidget->currentRow();
    cout << ui->listWidget->currentRow() << endl;

    if (ui->listWidget->currentRow() > 0)
    {
        ui->listWidget->takeItem(removing);
    }

    else if (ui->listWidget->currentRow() == 0)
    {
        ui->listWidget->takeItem(removing);
        on_stop_clicked();
        ui->remove->setEnabled(false);
        ui->listWidget->clear();
        ui->statusBar->setEnabled(false);

    }

}



void MainWindow::on_verticalSlider_valueChanged(int i)
{
    if(i == 0 ){


        system("pactl -- set-sink-volume 0  0%");

        system("pactl -- set-sink-volume 1  0%");

        system("pactl -- set-sink-volume 2  0%");

        system("pactl -- set-sink-volume 3  0%");

        system("pactl -- set-sink-volume 4  0%");



    }



    else if(i == 5 ){

        system("pactl -- set-sink-volume 0  5%");

        system("pactl -- set-sink-volume 1  5%");

        system("pactl -- set-sink-volume 2  5%");

        system("pactl -- set-sink-volume 3  5%");

        system("pactl -- set-sink-volume 4  5%");


    }



    else if(i == 10 ){

        system("pactl -- set-sink-volume 0  10%");

        system("pactl -- set-sink-volume 1  10%");

        system("pactl -- set-sink-volume 2  10%");

        system("pactl -- set-sink-volume 3  10%");

        system("pactl -- set-sink-volume 4  10%");


    }



    else if(i == 15 ){

        system("pactl -- set-sink-volume 0  15%");

        system("pactl -- set-sink-volume 1  15%");

        system("pactl -- set-sink-volume 2  15%");

        system("pactl -- set-sink-volume 3  15%");

        system("pactl -- set-sink-volume 4  15%");


    }



    else if(i == 20 ){

        system("pactl -- set-sink-volume 0  20%");

        system("pactl -- set-sink-volume 1  20%");

        system("pactl -- set-sink-volume 2  20%");

        system("pactl -- set-sink-volume 3  20%");

        system("pactl -- set-sink-volume 4  20%");


    }



    else if(i == 25 ){

        system("pactl -- set-sink-volume 0  25%");

        system("pactl -- set-sink-volume 1  25%");

        system("pactl -- set-sink-volume 2  25%");

        system("pactl -- set-sink-volume 3  25%");

        system("pactl -- set-sink-volume 4  25%");



    }



    else if(i == 30 ){

        system("pactl -- set-sink-volume 0  39%");

        system("pactl -- set-sink-volume 1  30%");

        system("pactl -- set-sink-volume 2  30%");

        system("pactl -- set-sink-volume 3  30%");

        system("pactl -- set-sink-volume 4  30%");


    }



    else if(i == 35 ){

        system("pactl -- set-sink-volume 0  35%");

        system("pactl -- set-sink-volume 1  35%");

        system("pactl -- set-sink-volume 2  35%");

        system("pactl -- set-sink-volume 3  35%");

        system("pactl -- set-sink-volume 4  35%");



    }



    else if(i == 40 ){

        system("pactl -- set-sink-volume 0  40%");

        system("pactl -- set-sink-volume 1  40%");

        system("pactl -- set-sink-volume 2  40%");

        system("pactl -- set-sink-volume 3  40%");

        system("pactl -- set-sink-volume 4  40%");


    }



    else if(i == 45 ){

        system("pactl -- set-sink-volume 0  45%");

        system("pactl -- set-sink-volume 1  45%");

        system("pactl -- set-sink-volume 2  45%");

        system("pactl -- set-sink-volume 3  45%");

        system("pactl -- set-sink-volume 4  45%");


    }



    else if(i == 50 ){

        system("pactl -- set-sink-volume 0  50%");

        system("pactl -- set-sink-volume 1  50%");

        system("pactl -- set-sink-volume 2  50%");

        system("pactl -- set-sink-volume 3  50%");

        system("pactl -- set-sink-volume 4  50%");

    }



    else if(i == 55 ){

        system("pactl -- set-sink-volume 0  55%");

        system("pactl -- set-sink-volume 1  55%");

        system("pactl -- set-sink-volume 2  55%");

        system("pactl -- set-sink-volume 3  55%");

        system("pactl -- set-sink-volume 4  55%");

    }



    else if(i == 60 ){

        system("pactl -- set-sink-volume 0  60%");

        system("pactl -- set-sink-volume 1  60%");

        system("pactl -- set-sink-volume 2  60%");

        system("pactl -- set-sink-volume 3  60%");

        system("pactl -- set-sink-volume 4  60%");

    }



    else if(i == 65 ){

        system("pactl -- set-sink-volume 0  65%");

        system("pactl -- set-sink-volume 1  65%");

        system("pactl -- set-sink-volume 2  65%");

        system("pactl -- set-sink-volume 3  65%");

        system("pactl -- set-sink-volume 4  65%");

    }



    else if(i == 70 ){


        system("pactl -- set-sink-volume 0  70%");

        system("pactl -- set-sink-volume 1  70%");

        system("pactl -- set-sink-volume 2  70%");

        system("pactl -- set-sink-volume 3  70%");

        system("pactl -- set-sink-volume 4  70%");

    }



    else if(i == 75 ){


        system("pactl -- set-sink-volume 0  75%");

        system("pactl -- set-sink-volume 1  75%");

        system("pactl -- set-sink-volume 2  75%");

        system("pactl -- set-sink-volume 3  75%");

        system("pactl -- set-sink-volume 4  75%");

    }



    else if(i == 80 ){

        system("pactl -- set-sink-volume 0  80%");

        system("pactl -- set-sink-volume 1  80%");

        system("pactl -- set-sink-volume 2  80%");

        system("pactl -- set-sink-volume 3  80%");

        system("pactl -- set-sink-volume 4  80%");

    }



    else if(i == 85 ){

        system("pactl -- set-sink-volume 0  85%");

        system("pactl -- set-sink-volume 1  85%");

        system("pactl -- set-sink-volume 2  85%");

        system("pactl -- set-sink-volume 3  85%");

        system("pactl -- set-sink-volume 4  85%");


    }



    else if(i == 90 ){

        system("pactl -- set-sink-volume 0  90%");

        system("pactl -- set-sink-volume 1  90%");

        system("pactl -- set-sink-volume 2  90%");

        system("pactl -- set-sink-volume 3  90%");

        system("pactl -- set-sink-volume 4  90%");

    }



    else if(i == 95 ){

        system("pactl -- set-sink-volume 0  95%");

        system("pactl -- set-sink-volume 1  95%");

        system("pactl -- set-sink-volume 2  95%");

        system("pactl -- set-sink-volume 3  95%");

        system("pactl -- set-sink-volume 4  95%");

    }



    else if(i == 100 ){

        system("pactl -- set-sink-volume 0  100%");

        system("pactl -- set-sink-volume 1  100%");

        system("pactl -- set-sink-volume 2  100%");

        system("pactl -- set-sink-volume 3  100%");

        system("pactl -- set-sink-volume 4  100%");

    }
}

