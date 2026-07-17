#pragma once

#include <QObject>
#include <QPointer>
#include <QVariant>

class RealMixerViewModel;

// App assembly: create Model/ViewModel, load pure QML, wire View ↔ ViewModel.
class MixingStudioApp
{
public:
    MixingStudioApp() = default;
    ~MixingStudioApp() = default;

    MixingStudioApp(const MixingStudioApp &) = delete;
    MixingStudioApp &operator=(const MixingStudioApp &) = delete;

    int run(int argc, char *argv[]);
};

// Pure View root ↔ RealMixerViewModel (declared with App; no separate binder file).
class MixingStudioViewBinder : public QObject
{
    Q_OBJECT

public:
    explicit MixingStudioViewBinder(RealMixerViewModel *viewModel, QObject *parent = nullptr);

    void bind(QObject *viewRoot);

private slots:
    void onImportFilesRequested(const QVariant &urlsVar);
    void onMasterVolumeEdited(double value);
    void onMockToggled();
    void onSeekRequested(double progress);
    void onLoopRangeRequested(double startProgress, double endProgress);
    void onLoopEnabledToggled();
    void onAddAutomationRequested(double progress, double value);
    void onMoveAutomationRequested(int pointIndex, double progress, double value);
    void onTrackSelected(int index);
    void onSearchTextEdited(const QString &text);
    void onAssetIndexSelected(int index);
    void onProjectIndexSelected(int index);

private:
    void bindRootSignals();
    void syncRoot();
    void setRootProp(const char *name, const QVariant &value);

    RealMixerViewModel *m_viewModel = nullptr;
    QPointer<QObject> m_viewRoot;
};
