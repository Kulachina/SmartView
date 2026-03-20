#include "update.h"
#include <QCoreApplication>
#include <QDebug>
#include <QDateTime>

Updater::Updater(QObject *parent)
    : QObject(parent)
    , m_process(new QProcess(this))
    , m_settings("SmartViewTeam", "SmartView")
    , m_silentCheck(false)
{
    // Подключаем сигналы
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &Updater::onUpdaterFinished);
    connect(m_process, &QProcess::errorOccurred,
            this, &Updater::onUpdaterError);
}

void Updater::checkForUpdates(bool silent)
{
    m_silentCheck = silent;

    // Если проверка не в тихом режиме, проверяем когда проверяли в последний раз
    if (!silent && !shouldCheckToday()) {
        qDebug() << "Already checked today, skipping";
        return;
    }

    QString updaterPath = getUpdaterPath();
    if (updaterPath.isEmpty()) {
        if (!m_silentCheck) {
            emit updateError("Не найден инструмент обновления");
        }
        return;
    }

    qDebug() << "Checking for updates..." << updaterPath;

    // Запускаем апдейтер с параметром --checkupdates
    m_process->start(updaterPath, QStringList() << "--checkupdates" << "--silent");

    // Ждем до 30 секунд
    if (!m_process->waitForFinished(30000)) {
        m_process->kill();
        if (!m_silentCheck) {
            emit updateError("Превышено время ожидания");
        }
    }
}

void Updater::onUpdaterFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    qDebug() << "Updater finished with exit code:" << exitCode;

    // Сохраняем время последней проверки
    saveLastCheckTime();

    // Коды возврата MaintenanceTool:
    // 0 - обновление установлено или не требуется
    // 2 - обновление найдено
    // 3 - обновлений нет

    if (exitCode == 2) {
        // Найдено обновление
        qDebug() << "Update found!";
        emit updateAvailable(""); // Можно передать версию, если нужно
    }
    else if (exitCode == 0 || exitCode == 3) {
        // Обновлений нет
        if (!m_silentCheck) {
            emit noUpdateAvailable();
        }
    }
    else {
        // Ошибка
        if (!m_silentCheck) {
            emit updateError(QString("Ошибка проверки (код: %1)").arg(exitCode));
        }
    }
}

void Updater::onUpdaterError(QProcess::ProcessError error)
{
    QString errorMsg;
    switch (error) {
    case QProcess::FailedToStart:
        errorMsg = "Не удалось запустить обновление";
        break;
    case QProcess::Crashed:
        errorMsg = "Процесс обновления аварийно завершился";
        break;
    default:
        errorMsg = "Ошибка при проверке обновлений";
    }

    if (!m_silentCheck) {
        emit updateError(errorMsg);
    }
}

void Updater::startUpdater()
{
    QString updaterPath = getUpdaterPath();
    if (updaterPath.isEmpty()) {
        emit updateError("Не найден инструмент обновления");
        return;
    }

    qDebug() << "Starting updater..." << updaterPath;

    // Запускаем апдейтер с параметром --checkupdates
    // Апдейтер сам покажет диалог
    if (!QProcess::startDetached(updaterPath, QStringList() << "--checkupdates")) {
        emit updateError("Не удалось запустить обновление");
    } else {
        // Закрываем приложение
        QCoreApplication::quit();
    }
}

QString Updater::getUpdaterPath()
{
    QString appDir = QCoreApplication::applicationDirPath();

    QString updaterName = "SmartViewUpdater.exe";

    QString updaterPath = QDir::toNativeSeparators(appDir + "/" + updaterName);

    if (!QFile::exists(updaterPath)) {
        qWarning() << "Updater not found:" << updaterPath;
        return QString();
    }

    return updaterPath;
}

void Updater::saveLastCheckTime()
{
    m_settings.setValue("LastUpdateCheck", QDateTime::currentDateTime());
}

bool Updater::shouldCheckToday()
{
    QDateTime lastCheck = m_settings.value("LastUpdateCheck").toDateTime();
    if (!lastCheck.isValid()) {
        return true; // Никогда не проверяли
    }

    // Проверяем, прошло ли больше 24 часов
    return lastCheck.addSecs(24 * 60 * 60) < QDateTime::currentDateTime();
}
