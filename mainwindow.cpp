#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "spectrumwidget.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QSettings>
#include <QTextStream>
#include <QMessageBox>
#include <QCloseEvent>
#include <QAudioBuffer>
#include <QAudioFormat>

#include <vector>
#include <complex>
#include <algorithm>
#include <cstdlib>
#include <cmath>

static QString formatMs(qint64 ms)
{
    if (ms < 0) ms = 0;
    const qint64 totalSec = ms / 1000;
    const qint64 m = totalSec / 60;
    const qint64 s = totalSec % 60;
    return QString("%1:%2").arg(m).arg(s, 2, 10, QChar('0'));
}

static inline float clamp01(float x) { return std::max(0.0f, std::min(1.0f, x)); }

static void fftRadix2(std::vector<std::complex<float>> &a)
{
    const int n = (int)a.size();
    int j = 0;
    for (int i = 1; i < n; i++) {
        int bit = n >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if (i < j) std::swap(a[i], a[j]);
    }
    for (int len = 2; len <= n; len <<= 1) {
        const float ang = -2.0f * float(M_PI) / float(len);
        std::complex<float> wlen(std::cos(ang), std::sin(ang));
        for (int i = 0; i < n; i += len) {
            std::complex<float> w(1.f, 0.f);
            for (int k = 0; k < len / 2; k++) {
                auto u = a[i + k];
                auto v = a[i + k + len / 2] * w;
                a[i + k] = u + v;
                a[i + k + len / 2] = u - v;
                w *= wlen;
            }
        }
    }
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      rng(QRandomGenerator::securelySeeded())
{
    ui->setupUi(this);
    applyDarkGlassTheme();

    // Create multimedia objects
    player = new QMediaPlayer(this);
    audio  = new QAudioOutput(this);
    player->setAudioOutput(audio);

    bufferOut = new QAudioBufferOutput(this);
    connect(bufferOut, &QAudioBufferOutput::audioBufferReceived, this, &MainWindow::onAudioBuffer);
    player->setAudioBufferOutput(bufferOut);

    // Replace spectrum placeholder with our widget
    spectrum = new SpectrumWidget(this);
    ui->spectrumLayout->addWidget(spectrum);
    spectrum->setBars(QVector<float>(48, 0.0f));

    // Connect UI
    connect(ui->btnAdd, &QPushButton::clicked, this, &MainWindow::addFiles);
    connect(ui->btnRemove, &QPushButton::clicked, this, &MainWindow::removeSelected);
    connect(ui->btnUp, &QPushButton::clicked, this, &MainWindow::moveUp);
    connect(ui->btnDown, &QPushButton::clicked, this, &MainWindow::moveDown);
    connect(ui->btnClear, &QPushButton::clicked, this, &MainWindow::clearPlaylist);

    connect(ui->btnOpenPlaylist, &QPushButton::clicked, this, &MainWindow::openPlaylist);
    connect(ui->btnSavePlaylist, &QPushButton::clicked, this, &MainWindow::savePlaylist);

    connect(ui->btnPrev, &QToolButton::clicked, this, &MainWindow::previous);
    connect(ui->btnPlayPause, &QToolButton::clicked, this, &MainWindow::playPause);
    connect(ui->btnStop, &QToolButton::clicked, this, &MainWindow::stop);
    connect(ui->btnNext, &QToolButton::clicked, this, &MainWindow::next);

    connect(ui->chkShuffle, &QCheckBox::toggled, this, &MainWindow::toggleShuffle);
    connect(ui->btnRepeat, &QPushButton::clicked, this, &MainWindow::cycleRepeat);

    connect(ui->chkMute, &QCheckBox::toggled, this, &MainWindow::toggleMute);

    connect(ui->seekSlider, &QSlider::sliderMoved, this, &MainWindow::seekSliderMoved);
    connect(ui->searchEdit, &QLineEdit::textChanged, this, &MainWindow::applyFilter);

    connect(ui->listWidget, &QListWidget::itemClicked, this, [this]{ onItemClicked(); });
    connect(ui->listWidget, &QListWidget::itemDoubleClicked, this, [this]{ onItemDoubleClicked(); });

    // Enable drag & drop reorder in UI editor too
    ui->listWidget->setDragDropMode(QAbstractItemView::InternalMove);
    ui->listWidget->setDefaultDropAction(Qt::MoveAction);

    // Player signals
    connect(player, &QMediaPlayer::mediaStatusChanged, this, &MainWindow::onMediaStatusChanged);
    connect(player, &QMediaPlayer::positionChanged, this, &MainWindow::onPositionChanged);
    connect(player, &QMediaPlayer::durationChanged, this, &MainWindow::onDurationChanged);

    // Restore settings
    QSettings s("NovaCronos", "Qt6PlayerDesignerGlossy");
    restoreGeometry(s.value("win/geometry").toByteArray());

    playlistPath = s.value("playlist/path").toString();
    setCurrentPlaylistPath(playlistPath);


    ui->chkShuffle->setChecked(s.value("playback/shuffle", false).toBool());
    const int rep = s.value("playback/repeat", 0).toInt();
    repeatMode = (rep == 1) ? RepeatMode::One : (rep == 2 ? RepeatMode::All : RepeatMode::Off);
    ui->btnRepeat->setText(QString("Repeat: %1").arg(repeatMode==RepeatMode::Off?"Off":repeatMode==RepeatMode::One?"One":"All"));

    const QString last = s.value("playlist/last_m3u").toString();
    if (!last.isEmpty() && QFileInfo::exists(last)) {
        loadM3U(last);
        setCurrentPlaylistPath(last);
    }

    // Initial UI state
    ui->btnPlayPause->setEnabled(false);
    ui->btnStop->setEnabled(false);
    ui->btnNext->setEnabled(false);
    ui->btnPrev->setEnabled(false);
    ui->seekSlider->setEnabled(false);

    updateTimeLabels();
    statusBar()->showMessage("Ready");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *e)
{
    QSettings s("NovaCronos", "Qt6PlayerDesignerGlossy");
    s.setValue("win/geometry", saveGeometry());
    s.setValue("audio/mute", ui->chkMute->isChecked());
    s.setValue("playback/shuffle", ui->chkShuffle->isChecked());
    s.setValue("playback/repeat", repeatMode==RepeatMode::Off?0:repeatMode==RepeatMode::One?1:2);
    s.setValue("playlist/path", playlistPath);
    s.setValue("playlist/last_m3u", playlistPath);
    e->accept();
}

void MainWindow::applyDarkGlassTheme()
{
    // Bigger buttons + more glass effect (stronger highlights, blur-like layers).
    const char *qss = R"QSS(
        * { color: #f4f4f4; font-size: 13px; }

        QMainWindow, QWidget {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #1b1b22, stop:0.45 #121218, stop:1 #09090c);
        }

        /* "Glass card" containers */
        QFrame#cardFrame, QFrame#controlsFrame, QFrame#playlistFrame {
            background: rgba(255,255,255,6);
            border: 1px solid rgba(255,255,255,16);
            border-radius: 18px;
        }

        QLabel#titleLabel {
            font-size: 18px;
            font-weight: 700;
            color: #ffffff;
        }

        QLineEdit {
            background: rgba(0,0,0,55);
            border: 1px solid rgba(255,255,255,20);
            border-radius: 14px;
            padding: 10px 12px;
            selection-background-color: rgba(110,255,200,90);
        }
        QLineEdit:focus {
            border: 1px solid rgba(110,255,200,140);
            background: rgba(0,0,0,60);
        }

        QListWidget {
            background: rgba(0,0,0,58);
            border: 1px solid rgba(255,255,255,18);
            border-radius: 16px;
            padding: 8px;
            selection-background-color: rgba(110,255,200,70);
        }
        QListWidget::item { padding: 10px; border-radius: 10px; }
        QListWidget::item:selected { background: rgba(110,255,200,70); }

        QPushButton {
            min-height: 44px;
            padding: 10px 16px;
            border-radius: 16px;
            border: 1px solid rgba(255,255,255,22);
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 rgba(255,255,255,24),
                stop:0.45 rgba(255,255,255,10),
                stop:1 rgba(0,0,0,32));
        }
        QPushButton:hover {
            border: 1px solid rgba(110,255,200,160);
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 rgba(110,255,200,30),
                stop:1 rgba(0,0,0,35));
        }
        QPushButton:pressed { background: rgba(0,0,0,55); }
        QPushButton:disabled {
            color: rgba(255,255,255,60);
            border: 1px solid rgba(255,255,255,12);
            background: rgba(255,255,255,5);
        }

        QToolButton {
            min-width: 60px;
            min-height: 60px;
            font-size: 22px;
            font-weight: 700;
            border-radius: 20px;
            border: 1px solid rgba(255,255,255,22);
            background: qradialgradient(cx:0.5, cy:0.25, radius: 1.2,
                stop:0 rgba(255,255,255,35),
                stop:0.5 rgba(255,255,255,12),
                stop:1 rgba(0,0,0,40));
        }
        QToolButton:hover {
            border: 1px solid rgba(110,255,200,180);
            background: qradialgradient(cx:0.5, cy:0.25, radius: 1.2,
                stop:0 rgba(110,255,200,35),
                stop:1 rgba(0,0,0,45));
        }
        QToolButton:pressed { background: rgba(0,0,0,60); }
        QToolButton:disabled { color: rgba(255,255,255,60); }

        QCheckBox { spacing: 10px; font-size: 13px; }
        QCheckBox::indicator {
            width: 18px; height: 18px;
            border-radius: 5px;
            border: 1px solid rgba(255,255,255,28);
            background: rgba(0,0,0,45);
        }
        QCheckBox::indicator:checked {
            background: rgba(110,255,200,120);
            border: 1px solid rgba(110,255,200,170);
        }

        QSlider::groove:horizontal {
            height: 10px;
            border-radius: 5px;
            background: rgba(0,0,0,55);
            border: 1px solid rgba(255,255,255,16);
        }
        QSlider::sub-page:horizontal {
            background: rgba(110,255,200,110);
            border-radius: 5px;
        }
        QSlider::handle:horizontal {
            width: 20px;
            margin: -6px 0;
            border-radius: 10px;
            background: qradialgradient(cx:0.5, cy:0.25, radius: 1.0,
                stop:0 rgba(255,255,255,240),
                stop:0.5 rgba(110,255,200,170),
                stop:1 rgba(0,0,0,160));
            border: 1px solid rgba(255,255,255,30);
        }

        QStatusBar {
            background: rgba(0,0,0,45);
            border-top: 1px solid rgba(255,255,255,12);
        }
    )QSS";

    qApp->setStyleSheet(QString::fromUtf8(qss));
}

int MainWindow::snap5(int v)
{
    v = std::clamp(v, 0, 100);
    return (v / 5) * 5;
}

void MainWindow::setAllSinksVolume(int percent)
{
    const int p = std::clamp(percent, 0, 150);
    for (int sink = 0; sink <= 8; ++sink) {
        const QString cmd = QString("pactl -- set-sink-volume %1 %2%").arg(sink).arg(p);
        std::system(cmd.toLocal8Bit().constData());
    }
}

void MainWindow::setCurrentPlaylistPath(const QString &path)
{
    playlistPath = path;
    ui->playlistLabel->setText(path.isEmpty() ? "Playlist: (unsaved)" : QString("Playlist: %1").arg(QFileInfo(path).fileName()));

    QSettings s("NovaCronos", "Qt6PlayerDesignerGlossy");
    s.setValue("playlist/last_m3u", path);
}

bool MainWindow::loadM3U(const QString &path)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        statusBar()->showMessage("Failed to open playlist.");
        return false;
    }

    QVector<QUrl> urls;
    QStringList shown;

    QTextStream in(&f);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;
        if (line.startsWith('#')) continue;

        QUrl url = QUrl::fromUserInput(line);
        if (url.isLocalFile() && !QFileInfo::exists(url.toLocalFile()))
            continue;

        urls.push_back(url);
        shown.push_back(url.isLocalFile() ? url.toLocalFile() : url.toString());
    }

    playlistUrls = urls;
    ui->listWidget->clear();
    ui->listWidget->addItems(shown);

    currentIndex = playlistUrls.isEmpty() ? -1 : 0;

    ui->btnPlayPause->setEnabled(!playlistUrls.isEmpty());
    ui->btnStop->setEnabled(!playlistUrls.isEmpty());
    ui->btnNext->setEnabled(playlistUrls.size() > 1);
    ui->btnPrev->setEnabled(playlistUrls.size() > 1);
    ui->seekSlider->setEnabled(!playlistUrls.isEmpty());

    updateNowPlayingForIndex(currentIndex);
    statusBar()->showMessage(QString("Loaded playlist (%1 tracks)").arg(playlistUrls.size()));
    return true;
}

bool MainWindow::saveM3U(const QString &path) const
{
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) return false;

    QTextStream out(&f);
    out << "#EXTM3U\n";
    for (const QUrl &u : playlistUrls)
        out << (u.isLocalFile() ? u.toLocalFile() : u.toString()) << "\n";
    return true;
}

void MainWindow::addFiles()
{
    const QStringList files = QFileDialog::getOpenFileNames(
        this,
        "Select audio files",
        QString(),
        "Audio files (*.mp3 *.wav *.ogg *.flac *.m4a);;All files (*.*)"
    );
    if (files.isEmpty()) return;

    for (const QString &file : files) {
        playlistUrls.push_back(QUrl::fromLocalFile(file));
        ui->listWidget->addItem(file);
    }

    if (currentIndex < 0 && !playlistUrls.isEmpty())
        currentIndex = 0;

    ui->btnPlayPause->setEnabled(!playlistUrls.isEmpty());
    ui->btnStop->setEnabled(!playlistUrls.isEmpty());
    ui->btnNext->setEnabled(playlistUrls.size() > 1);
    ui->btnPrev->setEnabled(playlistUrls.size() > 1);
    ui->seekSlider->setEnabled(!playlistUrls.isEmpty());

    updateNowPlayingForIndex(currentIndex);
    statusBar()->showMessage(QString("Added %1 file(s)").arg(files.size()));
}

void MainWindow::openPlaylist()
{
    const QString path = QFileDialog::getOpenFileName(
        this, "Open playlist", QString(),
        "Playlists (*.m3u *.m3u8);;All files (*.*)"
    );
    if (path.isEmpty()) return;
    if (loadM3U(path)) setCurrentPlaylistPath(path);
}

void MainWindow::savePlaylist()
{
    if (playlistUrls.isEmpty()) { statusBar()->showMessage("Nothing to save."); return; }
    if (playlistPath.isEmpty()) { savePlaylistAs(); return; }

    if (!saveM3U(playlistPath)) {
        QMessageBox::warning(this, "Save failed", "Could not save playlist.");
        return;
    }
    statusBar()->showMessage("Playlist saved.");
}

void MainWindow::savePlaylistAs()
{
    if (playlistUrls.isEmpty()) { statusBar()->showMessage("Nothing to save."); return; }

    const QString path = QFileDialog::getSaveFileName(
        this, "Save playlist as",
        playlistPath.isEmpty() ? QString("playlist.m3u") : playlistPath,
        "Playlists (*.m3u *.m3u8)"
    );
    if (path.isEmpty()) return;

    if (!saveM3U(path)) {
        QMessageBox::warning(this, "Save failed", "Could not save playlist.");
        return;
    }
    setCurrentPlaylistPath(path);
    statusBar()->showMessage("Playlist saved.");
}

void MainWindow::clearPlaylist()
{
    ui->listWidget->clear();
    playlistUrls.clear();
    currentIndex = -1;
    stop();
    setCurrentPlaylistPath(QString());
    ui->nowPlayingLabel->setText("Now playing: —");
    spectrum->setBars(QVector<float>(48, 0.0f));
    statusBar()->showMessage("Playlist cleared.");
}

void MainWindow::removeSelected()
{
    const int row = ui->listWidget->currentRow();
    if (row < 0 || row >= ui->listWidget->count()) return;

    delete ui->listWidget->takeItem(row);
    if (row >= 0 && row < playlistUrls.size())
        playlistUrls.removeAt(row);

    if (playlistUrls.isEmpty()) { clearPlaylist(); return; }

    if (currentIndex >= playlistUrls.size())
        currentIndex = playlistUrls.size() - 1;

    updateNowPlayingForIndex(currentIndex);
}

void MainWindow::moveUp()
{
    int row = ui->listWidget->currentRow();
    if (row <= 0) return;

    auto *item = ui->listWidget->takeItem(row);
    ui->listWidget->insertItem(row - 1, item);
    ui->listWidget->setCurrentRow(row - 1);

    playlistUrls.move(row, row - 1);
    if (currentIndex == row) currentIndex = row - 1;
    else if (currentIndex == row - 1) currentIndex = row;
}

void MainWindow::moveDown()
{
    int row = ui->listWidget->currentRow();
    if (row < 0 || row >= ui->listWidget->count() - 1) return;

    auto *item = ui->listWidget->takeItem(row);
    ui->listWidget->insertItem(row + 1, item);
    ui->listWidget->setCurrentRow(row + 1);

    playlistUrls.move(row, row + 1);
    if (currentIndex == row) currentIndex = row + 1;
    else if (currentIndex == row + 1) currentIndex = row;
}

void MainWindow::applyFilter(const QString &text)
{
    const QString t = text.trimmed();
    for (int i = 0; i < ui->listWidget->count(); ++i) {
        auto *it = ui->listWidget->item(i);
        const bool match = t.isEmpty() || it->text().contains(t, Qt::CaseInsensitive);
        it->setHidden(!match);
    }
}

void MainWindow::toggleShuffle(bool on)
{
    shuffleEnabled = on;
    statusBar()->showMessage(on ? "Shuffle: ON" : "Shuffle: OFF");
}

void MainWindow::cycleRepeat()
{
    if (repeatMode == RepeatMode::Off) repeatMode = RepeatMode::One;
    else if (repeatMode == RepeatMode::One) repeatMode = RepeatMode::All;
    else repeatMode = RepeatMode::Off;

    ui->btnRepeat->setText(QString("Repeat: %1").arg(
        repeatMode==RepeatMode::Off?"Off":repeatMode==RepeatMode::One?"One":"All"));
}

void MainWindow::toggleMute(bool on)
{
    if (audio) audio->setMuted(on);
}



void MainWindow::seekSliderMoved(int posMs)
{
    if (player) player->setPosition(posMs);
}

void MainWindow::playPause()
{
    if (playlistUrls.isEmpty()) { statusBar()->showMessage("No tracks."); return; }
    userStopped = false;

    if (player->playbackState() == QMediaPlayer::PlayingState) {
        player->pause();
        ui->btnPlayPause->setText("▶");
        statusBar()->showMessage("Paused.");
        return;
    }

    if (currentIndex < 0) currentIndex = 0;

    if (player->playbackState() == QMediaPlayer::PausedState) {
        player->play();
    } else {
        playIndex(currentIndex);
    }

    ui->btnPlayPause->setText("⏸");
    ui->btnStop->setEnabled(true);
}

void MainWindow::stop()
{
    userStopped = true;
    if (player) player->stop();
    ui->btnPlayPause->setText("▶");
    ui->seekSlider->setValue(0);
    positionMs = 0;
    updateTimeLabels();
}

void MainWindow::next() { playNextInternal(); }
void MainWindow::previous() { playPreviousInternal(); }

void MainWindow::onItemClicked()
{
    int row = ui->listWidget->currentRow();
    if (row < 0 || row >= playlistUrls.size()) return;
    currentIndex = row;
    updateNowPlayingForIndex(currentIndex);
}

void MainWindow::onItemDoubleClicked()
{
    int row = ui->listWidget->currentRow();
    if (row < 0 || row >= playlistUrls.size()) return;
    playIndex(row);
}

void MainWindow::updateNowPlayingForIndex(int index)
{
    if (index < 0 || index >= playlistUrls.size()) {
        ui->nowPlayingLabel->setText("Now playing: —");
        return;
    }
    const QUrl &u = playlistUrls[index];
    const QString shown = u.isLocalFile() ? u.toLocalFile() : u.toString();
    ui->nowPlayingLabel->setText(QString("Now playing: %1").arg(QFileInfo(shown).fileName()));
}

void MainWindow::playIndex(int index)
{
    if (index < 0 || index >= playlistUrls.size()) return;

    currentIndex = index;
    userStopped = false;

    player->setSource(playlistUrls[index]);
    player->play();

    ui->btnPlayPause->setText("⏸");
    ui->btnPlayPause->setEnabled(true);
    ui->btnStop->setEnabled(true);
    ui->btnNext->setEnabled(playlistUrls.size() > 1);
    ui->btnPrev->setEnabled(playlistUrls.size() > 1);
    ui->seekSlider->setEnabled(true);

    ui->listWidget->setCurrentRow(index);
    updateNowPlayingForIndex(index);
}

int MainWindow::pickNextIndex() const
{
    const int n = playlistUrls.size();
    if (n <= 0) return -1;
    if (currentIndex < 0) return 0;

    if (repeatMode == RepeatMode::One) return currentIndex;

    if (shuffleEnabled) {
        if (n == 1) return 0;
        int next = currentIndex;
        for (int tries = 0; tries < 20 && next == currentIndex; ++tries)
            next = int(rng.bounded(n));
        if (next == currentIndex) next = (currentIndex + 1) % n;
        return next;
    }

    int next = currentIndex + 1;
    if (next < n) return next;

    if (repeatMode == RepeatMode::All) return 0;
    return -1;
}

int MainWindow::pickPreviousIndex() const
{
    const int n = playlistUrls.size();
    if (n <= 0) return -1;
    if (currentIndex < 0) return 0;

    if (shuffleEnabled) {
        if (n == 1) return 0;
        int prev = currentIndex;
        for (int tries = 0; tries < 20 && prev == currentIndex; ++tries)
            prev = int(rng.bounded(n));
        if (prev == currentIndex) prev = (currentIndex - 1 + n) % n;
        return prev;
    }

    int prev = currentIndex - 1;
    if (prev >= 0) return prev;

    if (repeatMode == RepeatMode::All) return n - 1;
    return 0;
}

void MainWindow::playNextInternal()
{
    const int next = pickNextIndex();
    if (next == -1) { stop(); statusBar()->showMessage("End of playlist."); return; }
    playIndex(next);
}

void MainWindow::playPreviousInternal()
{
    const int prev = pickPreviousIndex();
    if (prev == -1) return;
    playIndex(prev);
}

void MainWindow::onMediaStatusChanged(QMediaPlayer::MediaStatus st)
{
    if (st == QMediaPlayer::EndOfMedia) {
        if (!userStopped) playNextInternal();
    }
}

void MainWindow::onPositionChanged(qint64 pos)
{
    positionMs = pos;
    if (!ui->seekSlider->isSliderDown())
        ui->seekSlider->setValue(int(pos));
    updateTimeLabels();
}

void MainWindow::onDurationChanged(qint64 dur)
{
    durationMs = dur;
    ui->seekSlider->setMaximum(int(dur));
    updateTimeLabels();
}

void MainWindow::updateTimeLabels()
{
    ui->timeLeft->setText(formatMs(positionMs));
    ui->timeRight->setText(formatMs(durationMs));
}

void MainWindow::onAudioBuffer(const QAudioBuffer &buffer)
{
    if (!buffer.isValid() || buffer.frameCount() <= 0) return;

    const QAudioFormat fmt = buffer.format();
    if (!fmt.isValid()) return;

    const int ch = fmt.channelCount();
    if (ch <= 0) return;

    const int fftSize = 1024;
    const int barCount = 48;

    static std::vector<float> fftInput;
    static std::vector<float> window;
    static QVector<float> bars(barCount, 0.0f);

    if ((int)window.size() != fftSize) {
        window.resize(fftSize);
        for (int i = 0; i < fftSize; ++i)
            window[i] = 0.5f - 0.5f * std::cos(2.0f * float(M_PI) * float(i) / float(fftSize - 1));
    }

    auto pushMono = [&](auto *data, qint64 sampleCount, float scale)
    {
        const qint64 frames = sampleCount / ch;
        for (qint64 f = 0; f < frames; ++f) {
            float sum = 0.f;
            for (int c = 0; c < ch; ++c) sum += float(data[f * ch + c]) * scale;
            fftInput.push_back(sum / float(ch));
        }
    };

    const qint64 totalSamples = buffer.frameCount() * ch;
    switch (fmt.sampleFormat()) {
    case QAudioFormat::Float: pushMono(buffer.constData<float>(), totalSamples, 1.0f); break;
    case QAudioFormat::Int16: pushMono(buffer.constData<qint16>(), totalSamples, 1.0f / 32768.0f); break;
    case QAudioFormat::Int32: pushMono(buffer.constData<qint32>(), totalSamples, 1.0f / 2147483648.0f); break;
    default: return;
    }

    const int maxKeep = fftSize * 8;
    if ((int)fftInput.size() > maxKeep)
        fftInput.erase(fftInput.begin(), fftInput.end() - maxKeep);

    if ((int)fftInput.size() < fftSize) return;

    std::vector<std::complex<float>> a(fftSize);
    const int start = (int)fftInput.size() - fftSize;
    for (int i = 0; i < fftSize; ++i) {
        float s = fftInput[start + i] * window[i];
        a[i] = std::complex<float>(s, 0.f);
    }

    fftRadix2(a);

    const int nBins = fftSize / 2;
    std::vector<float> mag(nBins);
    mag[0] = 0.f;
    for (int i = 1; i < nBins; ++i) mag[i] = std::abs(a[i]);

    QVector<float> newBars(barCount);
    for (int b = 0; b < barCount; ++b) {
        float t0 = float(b) / float(barCount);
        float t1 = float(b + 1) / float(barCount);

        auto toIndex = [&](float t) {
            float x = std::pow(10.0f, t * 2.0f) - 1.0f;
            float xn = x / (std::pow(10.0f, 2.0f) - 1.0f);
            int idx = 1 + int(xn * float(nBins - 2));
            return std::clamp(idx, 1, nBins - 1);
        };

        const int i0 = toIndex(t0);
        const int i1 = toIndex(t1);

        float sum = 0.f;
        int count = 0;
        for (int i = i0; i <= i1; ++i) { sum += mag[i]; ++count; }

        float v = (count > 0) ? (sum / float(count)) : 0.f;
        v = std::log10(1.0f + v * 20.0f);
        newBars[b] = clamp01(v);
    }

    const float attack = 0.60f;
    const float release = 0.10f;

    for (int i = 0; i < barCount; ++i) {
        float cur = bars[i];
        float tgt = newBars[i];
        cur = (tgt > cur) ? (cur + (tgt - cur) * attack) : (cur + (tgt - cur) * release);
        bars[i] = clamp01(cur);
    }

    if (spectrum) spectrum->setBars(bars);
}
