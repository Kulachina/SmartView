function Component() {
    // Конструктор
}

Component.prototype.createOperations = function() {
    // Вызов стандартных операций
    component.createOperations();

    // Работаем только в Windows
    if (installer.value("os") === "win") {
        var targetExe = installer.value("TargetDir") + "/SmartView.exe";
        var iconPath = installer.value("TargetDir") + "/SmartView.ico";

        // Создание ярлыка на рабочем столе
        var desktopShortcut = installer.value("DesktopDir") + "/SmartView.lnk";
        component.addOperation("CreateShortcut", targetExe, desktopShortcut, "iconPath=" + iconPath);

        // Создание ярлыка в меню "Пуск"
        var startMenuShortcut = installer.value("StartMenuDir") + "/SmartView.lnk";
        component.addOperation("CreateShortcut", targetExe, startMenuShortcut, "iconPath=" + iconPath);

        // Удаление ярлыков только при полном удалении (не при обновлении!)
        if (installer.isUninstaller()) {
            component.addOperation("DeleteFile", desktopShortcut);
            component.addOperation("DeleteFile", startMenuShortcut);
        }
    }
};

// Вызывается после установки
Component.prototype.finished = function() {
    console.log("Установка/обновление завершено!");
};