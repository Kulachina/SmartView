function Component() {}

Component.prototype.createOperations = function () {
    component.createOperations();
    if (systemInfo.productType === "windows") {
        // Ярлык в меню Пуск
        component.addOperation("CreateShortcut",
            "@TargetDir@/SmartView.exe",
            "@StartMenuDir@/SmartView.lnk",
            "workingDirectory=@TargetDir@",
            "iconPath=@TargetDir@/SmartView.ico",
            "description=SmartView");
        // Ярлык на рабочем столе
        component.addOperation("CreateShortcut",
            "@TargetDir@/SmartView.exe",
            "@DesktopDir@/SmartView.lnk",
            "workingDirectory=@TargetDir@",
            "iconPath=@TargetDir@/SmartView.ico",
            "description=SmartView");
    }
};
