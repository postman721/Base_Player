#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMediaPlayer>
#include <QFileDialog>
#include <QSlider>
#include <QString>
#include <iostream>
#include <QProgressBar>
#include <QListWidgetItem>
using namespace std;
extern int i;
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_play_clicked();

    void on_pause_clicked();

    void on_stop_clicked();

    void on_add_clicked();

    void on_listWidget_itemDoubleClicked();

    void on_remove_clicked();

    void on_verticalSlider_valueChanged(int value);

private:
    Ui::MainWindow *ui;
    QMediaPlayer *player;
    QSlider *slider;
    QProgressBar *bar;
    QMediaPlaylist *playlist;
};

#endif // MAINWINDOW_H
