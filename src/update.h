#ifndef UPDATER_H
#define UPDATER_H
#include <QObject>
#include <QProcess>
#include <QTimer>
#include <QSettings>
#include <QDir>

class Updater : public QObject
{
    Q_OBJECT

public:
    explicit Updater(QObject *parent = nullptr);

    // Запустить проверку обновлений
    void checkForUpdates(bool silent = false);

    // Запустить процесс обновления
    void startUpdater();

signals:
    void updateAvailable(const QString &version);
    void noUpdateAvailable();
    void updateError(const QString &error);

private slots:
    void onUpdaterFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onUpdaterError(QProcess::ProcessError error);

private:
    QString getUpdaterPath();
    void saveLastCheckTime();
    bool shouldCheckToday();

    QProcess *m_process;
    QSettings m_settings;
    bool m_silentCheck;
};

#endif // UPDATER_H
