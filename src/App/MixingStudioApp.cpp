#include <App/MixingStudioApp.h>

#include <Model/AudioEngine.h>
#include <ViewModel/RealMixerViewModel.h>

#include <QColor>
#include <QDateTime>
#include <QGuiApplication>
#include <QPalette>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QStringList>
#include <QVariantList>

namespace {

void applyNightPalette(QGuiApplication &guiApp)
{
    QPalette pal = guiApp.palette();
    const QColor windowBg(0x0b, 0x0f, 0x14);
    const QColor panel(0x14, 0x1b, 0x24);
    const QColor panelAlt(0x1a, 0x23, 0x30);
    const QColor text(0xe8, 0xee, 0xf6);
    const QColor muted(0x9a, 0xa8, 0xb8);
    const QColor button(0x2a, 0x35, 0x45);
    const QColor accent(0x3d, 0x9b, 0x84);
    const QColor mid(0x2c, 0x3a, 0x4d);

    pal.setColor(QPalette::Window, windowBg);
    pal.setColor(QPalette::WindowText, text);
    pal.setColor(QPalette::Base, panelAlt);
    pal.setColor(QPalette::AlternateBase, panel);
    pal.setColor(QPalette::Text, text);
    pal.setColor(QPalette::Button, button);
    pal.setColor(QPalette::ButtonText, text);
    pal.setColor(QPalette::BrightText, text);
    pal.setColor(QPalette::ToolTipBase, panel);
    pal.setColor(QPalette::ToolTipText, text);
    pal.setColor(QPalette::Highlight, accent);
    pal.setColor(QPalette::HighlightedText, Qt::white);
    pal.setColor(QPalette::Light, mid);
    pal.setColor(QPalette::Mid, mid);
    pal.setColor(QPalette::Dark, windowBg);
    pal.setColor(QPalette::Shadow, Qt::black);
    pal.setColor(QPalette::PlaceholderText, muted);
    guiApp.setPalette(pal);
}

} // namespace

int MixingStudioApp::run(int argc, char *argv[])
{
    qputenv("QT_QUICK_CONTROLS_STYLE", "Material");
    qputenv("QT_QUICK_CONTROLS_MATERIAL_THEME", "Dark");
    QQuickStyle::setStyle(QStringLiteral("Material"));

    QGuiApplication guiApp(argc, argv);
    applyNightPalette(guiApp);

    AudioEngine audioEngine;
    RealMixerViewModel viewModel(&audioEngine);

    QQmlApplicationEngine engine;
    // Pure View: MixingStudioViewBinder wires root signals/properties <-> ViewModel.
    engine.loadFromModule("MixingStudio", "Main");
    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    MixingStudioViewBinder binder(&viewModel);
    binder.bind(engine.rootObjects().constFirst());
    return guiApp.exec();
}

MixingStudioViewBinder::MixingStudioViewBinder(RealMixerViewModel *viewModel, QObject *parent)
    : QObject(parent)
    , m_viewModel(viewModel)
{
    Q_ASSERT(m_viewModel != nullptr);
}

void MixingStudioViewBinder::bind(QObject *viewRoot)
{
    Q_ASSERT(viewRoot != nullptr);
    m_viewRoot = viewRoot;

    bindRootSignals();

    // Targeted sync: never dump the full root on meter/playhead ticks.
    connect(m_viewModel, &RealMixerViewModel::playbackPositionChanged, this,
            &MixingStudioViewBinder::syncPlayback);
    connect(m_viewModel, &RealMixerViewModel::analysisMetersChanged, this,
            &MixingStudioViewBinder::syncMeters);
    connect(m_viewModel, &RealMixerViewModel::waveformPointsChanged, this,
            &MixingStudioViewBinder::syncWaveform);
    connect(m_viewModel, &RealMixerViewModel::spectrumLevelsChanged, this,
            &MixingStudioViewBinder::syncSpectrum);
    connect(m_viewModel, &RealMixerViewModel::playingChanged, this,
            &MixingStudioViewBinder::syncTransport);
    connect(m_viewModel, &RealMixerViewModel::masterVolumeChanged, this,
            &MixingStudioViewBinder::syncTransport);
    connect(m_viewModel, &RealMixerViewModel::mockValidationModeChanged, this,
            &MixingStudioViewBinder::syncTransport);
    connect(m_viewModel, &RealMixerViewModel::loopRangeChanged, this,
            &MixingStudioViewBinder::syncTransport);
    connect(m_viewModel, &RealMixerViewModel::statusMessageChanged, this,
            &MixingStudioViewBinder::syncTransport);
    connect(m_viewModel, &RealMixerViewModel::automationPointsChanged, this, [this]() {
        setRootProp("automationPoints", m_viewModel->automationPoints());
    });
    connect(m_viewModel, &RealMixerViewModel::tracksChanged, this,
            &MixingStudioViewBinder::syncTracks);
    connect(m_viewModel, &RealMixerViewModel::soloStateChanged, this,
            &MixingStudioViewBinder::syncTracks);
    connect(m_viewModel, &RealMixerViewModel::selectedTrackIndexChanged, this,
            &MixingStudioViewBinder::syncTracks);
    connect(m_viewModel, &RealMixerViewModel::assetSearchTextChanged, this,
            &MixingStudioViewBinder::syncLibrary);
    connect(m_viewModel, &RealMixerViewModel::filteredAssetNamesChanged, this,
            &MixingStudioViewBinder::syncLibrary);
    connect(m_viewModel, &RealMixerViewModel::selectedAssetIndexChanged, this,
            &MixingStudioViewBinder::syncLibrary);
    connect(m_viewModel, &RealMixerViewModel::recentProjectNamesChanged, this,
            &MixingStudioViewBinder::syncProjects);
    connect(m_viewModel, &RealMixerViewModel::selectedRecentProjectIndexChanged, this,
            &MixingStudioViewBinder::syncProjects);

    syncRoot();
}

void MixingStudioViewBinder::setRootProp(const char *name, const QVariant &value)
{
    if (m_viewRoot) {
        m_viewRoot->setProperty(name, value);
    }
}

void MixingStudioViewBinder::syncPlayback()
{
    if (!m_viewRoot || !m_viewModel) {
        return;
    }

    setRootProp("playbackProgress", m_viewModel->playbackProgress());

    // Time label at ~10 Hz ? avoid allocating QString every 8 ms tick.
    const qint64 tick = QDateTime::currentMSecsSinceEpoch();
    if (m_lastTimeTextMs < 0 || (tick - m_lastTimeTextMs) >= 100) {
        m_lastTimeTextMs = tick;
        setRootProp("playbackTimeText", m_viewModel->playbackTimeText());
    }
}

void MixingStudioViewBinder::syncMeters()
{
    if (!m_viewRoot || !m_viewModel) {
        return;
    }
    setRootProp("vuLevel", m_viewModel->vuLevel());
}

void MixingStudioViewBinder::syncWaveform()
{
    if (!m_viewRoot || !m_viewModel) {
        return;
    }
    setRootProp("waveformPoints", m_viewModel->waveformPoints());
}

void MixingStudioViewBinder::syncSpectrum()
{
    if (!m_viewRoot || !m_viewModel) {
        return;
    }
    setRootProp("spectrumLevels", m_viewModel->spectrumLevels());
}

void MixingStudioViewBinder::syncTransport()
{
    if (!m_viewRoot || !m_viewModel) {
        return;
    }
    setRootProp("playing", m_viewModel->playing());
    setRootProp("masterVolume", m_viewModel->masterVolume());
    setRootProp("mockValidationMode", m_viewModel->mockValidationMode());
    setRootProp("loopEnabled", m_viewModel->loopEnabled());
    setRootProp("loopStartProgress", m_viewModel->loopStartProgress());
    setRootProp("loopEndProgress", m_viewModel->loopEndProgress());
    setRootProp("statusMessage", m_viewModel->statusMessage());
    setRootProp("playbackTimeText", m_viewModel->playbackTimeText());
    m_lastTimeTextMs = QDateTime::currentMSecsSinceEpoch();
}

void MixingStudioViewBinder::syncTracks()
{
    if (!m_viewRoot || !m_viewModel) {
        return;
    }
    setRootProp("tracks", m_viewModel->tracksAsObjects());
    setRootProp("selectedTrackIndex", m_viewModel->selectedTrackIndex());
    setRootProp("anySolo", m_viewModel->anySolo());
}

void MixingStudioViewBinder::syncLibrary()
{
    if (!m_viewRoot || !m_viewModel) {
        return;
    }
    setRootProp("assetSearchText", m_viewModel->assetSearchText());
    setRootProp("assetNames", QVariant::fromValue(m_viewModel->filteredAssetNames()));
    setRootProp("selectedAssetIndex", m_viewModel->selectedAssetIndex());
}

void MixingStudioViewBinder::syncProjects()
{
    if (!m_viewRoot || !m_viewModel) {
        return;
    }
    setRootProp("projectNames", QVariant::fromValue(m_viewModel->recentProjectNames()));
    setRootProp("selectedRecentProjectIndex", m_viewModel->selectedRecentProjectIndex());
}

void MixingStudioViewBinder::syncRoot()
{
    if (!m_viewRoot || !m_viewModel) {
        return;
    }

    syncTransport();
    syncPlayback();
    syncMeters();
    syncWaveform();
    syncSpectrum();
    setRootProp("automationPoints", m_viewModel->automationPoints());
    syncTracks();
    syncLibrary();
    syncProjects();
}

void MixingStudioViewBinder::bindRootSignals()
{
    if (!m_viewRoot || !m_viewModel) {
        return;
    }

    connect(m_viewRoot, SIGNAL(demoRequested()), m_viewModel, SLOT(importMockTrack()));
    connect(m_viewRoot, SIGNAL(sampleRequested()), m_viewModel, SLOT(loadSampleProject()));
    connect(m_viewRoot, SIGNAL(saveRequested()), m_viewModel, SLOT(saveProject()));
    connect(m_viewRoot, SIGNAL(exportRequested()), m_viewModel, SLOT(exportMix()));
    connect(m_viewRoot, SIGNAL(playRequested()), m_viewModel, SLOT(play()));
    connect(m_viewRoot, SIGNAL(pauseRequested()), m_viewModel, SLOT(pause()));
    connect(m_viewRoot, SIGNAL(stopRequested()), m_viewModel, SLOT(stop()));
    connect(m_viewRoot, SIGNAL(clearAutomationRequested()), m_viewModel, SLOT(clearAutomation()));
    connect(m_viewRoot, SIGNAL(deleteTrackRequested()), m_viewModel, SLOT(deleteSelectedTrack()));
    connect(m_viewRoot, SIGNAL(importAssetRequested()), m_viewModel, SLOT(importSelectedAsset()));
    connect(m_viewRoot, SIGNAL(restoreProjectRequested()), m_viewModel, SLOT(restoreSelectedRecentProject()));
    connect(m_viewRoot, SIGNAL(deleteProjectRequested()), m_viewModel, SLOT(deleteSelectedRecentProject()));

    connect(m_viewRoot, SIGNAL(importFilesRequested(QVariant)), this, SLOT(onImportFilesRequested(QVariant)));
    connect(m_viewRoot, SIGNAL(masterVolumeEdited(double)), this, SLOT(onMasterVolumeEdited(double)));
    connect(m_viewRoot, SIGNAL(mockToggled()), this, SLOT(onMockToggled()));
    connect(m_viewRoot, SIGNAL(seekRequested(double)), this, SLOT(onSeekRequested(double)));
    connect(m_viewRoot, SIGNAL(loopRangeRequested(double, double)), this,
            SLOT(onLoopRangeRequested(double, double)));
    connect(m_viewRoot, SIGNAL(loopEnabledToggled()), this, SLOT(onLoopEnabledToggled()));
    connect(m_viewRoot, SIGNAL(addAutomationRequested(double, double)), this,
            SLOT(onAddAutomationRequested(double, double)));
    connect(m_viewRoot, SIGNAL(moveAutomationRequested(int, double, double)), this,
            SLOT(onMoveAutomationRequested(int, double, double)));
    connect(m_viewRoot, SIGNAL(trackSelected(int)), this, SLOT(onTrackSelected(int)));
    connect(m_viewRoot, SIGNAL(searchTextEdited(QString)), this, SLOT(onSearchTextEdited(QString)));
    connect(m_viewRoot, SIGNAL(assetIndexSelected(int)), this, SLOT(onAssetIndexSelected(int)));
    connect(m_viewRoot, SIGNAL(projectIndexSelected(int)), this, SLOT(onProjectIndexSelected(int)));
}

void MixingStudioViewBinder::onImportFilesRequested(const QVariant &urlsVar)
{
    QStringList paths;
    if (urlsVar.canConvert<QStringList>()) {
        paths = urlsVar.toStringList();
    } else {
        for (const QVariant &u : urlsVar.toList()) {
            paths.append(u.toString());
        }
    }
    for (const QString &path : paths) {
        m_viewModel->importLocalFile(path);
    }
}

void MixingStudioViewBinder::onMasterVolumeEdited(double value)
{
    m_viewModel->setMasterVolume(static_cast<float>(value));
}

void MixingStudioViewBinder::onMockToggled()
{
    m_viewModel->setMockValidationMode(!m_viewModel->mockValidationMode());
}

void MixingStudioViewBinder::onSeekRequested(double progress)
{
    m_viewModel->seekToProgress(static_cast<float>(progress));
}

void MixingStudioViewBinder::onLoopRangeRequested(double startProgress, double endProgress)
{
    m_viewModel->setLoopRangeByProgress(static_cast<float>(startProgress),
                                        static_cast<float>(endProgress));
}

void MixingStudioViewBinder::onLoopEnabledToggled()
{
    m_viewModel->setLoopEnabled(!m_viewModel->loopEnabled());
}

void MixingStudioViewBinder::onAddAutomationRequested(double progress, double value)
{
    m_viewModel->addAutomationPoint(static_cast<float>(progress), static_cast<float>(value));
}

void MixingStudioViewBinder::onMoveAutomationRequested(int pointIndex, double progress, double value)
{
    m_viewModel->moveAutomationPoint(pointIndex, static_cast<float>(progress), static_cast<float>(value));
}

void MixingStudioViewBinder::onTrackSelected(int index)
{
    m_viewModel->setSelectedTrackIndex(index);
}

void MixingStudioViewBinder::onSearchTextEdited(const QString &text)
{
    m_viewModel->setAssetSearchText(text);
}

void MixingStudioViewBinder::onAssetIndexSelected(int index)
{
    m_viewModel->setSelectedAssetIndex(index);
}

void MixingStudioViewBinder::onProjectIndexSelected(int index)
{
    m_viewModel->setSelectedRecentProjectIndex(index);
}
