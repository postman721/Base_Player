#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QAudioBufferOutput>
#include <QVector>
#include <QUrl>
#include <QRandomGenerator>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

#if QT_VERSION < QT_VERSION_CHECK(6, 2, 0)
#error "This project requires Qt 6.2+ (QAudioBufferOutput)."
#endif

class SpectrumWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
    void closeEvent(QCloseEvent *e) override;

private slots:
    // Playlist
    void addFiles();
    void openPlaylist();
    void savePlaylist();
    void savePlaylistAs();
    void clearPlaylist();
    void removeSelected();
    void moveUp();
    void moveDown();
    void applyFilter(const QString &text);

    // Playback
    void playPause();
    void stop();
    void next();
    void previous();
    void seekSliderMoved(int posMs);
    void toggleMute(bool on);

    // Modes
    void toggleShuffle(bool on);
    void cycleRepeat();

    // List
    void onItemClicked();
    void onItemDoubleClicked();

    // Media callbacks
    void onMediaStatusChanged(QMediaPlayer::MediaStatus st);
    void onPositionChanged(qint64 pos);
    void onDurationChanged(qint64 dur);

    // Spectrum
    void onAudioBuffer(const QAudioBuffer &buffer);

private:
    enum class RepeatMode { Off, One, All };

    void applyDarkGlassTheme();
    void updateNowPlayingForIndex(int index);
    void updateTimeLabels();

    void playIndex(int index);
    int  pickNextIndex() const;
    int  pickPreviousIndex() const;
    void playNextInternal();
    void playPreviousInternal();

    bool loadM3U(const QString &path);
    bool saveM3U(const QString &path) const;
    void setCurrentPlaylistPath(const QString &path);

    static int snap5(int v);
    void setAllSinksVolume(int percent);

private:
    Ui::MainWindow *ui = nullptr;

    QMediaPlayer *player = nullptr;
    QAudioOutput *audio = nullptr;
    QAudioBufferOutput *bufferOut = nullptr;

    SpectrumWidget *spectrum = nullptr;

    QVector<QUrl> playlistUrls;
    int currentIndex = -1;

    bool shuffleEnabled = false;
    RepeatMode repeatMode = RepeatMode::Off;
    bool userStopped = false;

    mutable QRandomGenerator rng;

    qint64 durationMs = 0;
    qint64 positionMs = 0;

    QString playlistPath;
};

#endif // MAINWINDOW_H
